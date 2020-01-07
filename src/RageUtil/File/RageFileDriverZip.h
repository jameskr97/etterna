﻿#ifndef RAGE_FILE_DRIVER_ZIP_H
#define RAGE_FILE_DRIVER_ZIP_H

#include "RageFileDriver.h"
#include "RageUtil/Misc/RageThreads.h"

/** @brief A read-only file driver for ZIPs. */
class RageFileDriverZip : public RageFileDriver
{
  public:
	RageFileDriverZip();
	explicit RageFileDriverZip(const RString& sPath);
	bool Load(const RString& sPath);
	bool Load(RageFileBasic* pFile);

	~RageFileDriverZip() override;

	RageFileBasic* Open(const RString& sPath, int iMode, int& iErr) override;
	void FlushDirCache(const RString& sPath) override;

	void DeleteFileWhenFinished() { m_bFileOwned = true; }

	/* Lower-level access: */
	enum ZipCompressionMethod
	{
		STORED = 0,
		DEFLATED = 8
	};
	struct FileInfo
	{
		RString m_sName;
		int m_iOffset;
		int m_iDataOffset;

		ZipCompressionMethod m_iCompressionMethod;
		int m_iCRC32;
		int m_iCompressedSize, m_iUncompressedSize;

		/* If 0, unknown. */
		int m_iFilePermissions;
	};
	const FileInfo* GetFileInfo(const RString& sPath) const;

	RString GetGlobalComment() const { return m_sComment; }

  private:
	bool m_bFileOwned;

	RageFileBasic* m_pZip;
	std::vector<FileInfo*> m_pFiles;

	RString m_sPath;
	RString m_sComment;

	/* Open() must be threadsafe.  Mutex access to "zip", since we seek
	 * around in it when reading files. */
	RageMutex m_Mutex;

	bool ParseZipfile();
	bool ReadEndCentralRecord(int& total_entries_central_dir,
							  int& offset_start_central_directory);
	int ProcessCdirFileHdr(FileInfo& info);
	bool SeekToEndCentralRecord();
	bool ReadLocalFileHeader(FileInfo& info);
};

#endif
