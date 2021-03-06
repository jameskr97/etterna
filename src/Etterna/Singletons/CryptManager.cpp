#include "Etterna/Globals/global.h"
#include "Etterna/Models/Misc/CryptHelpers.h"
#include "CryptManager.h"
#include "Etterna/Models/Lua/LuaBinding.h"
#include "LuaManager.h"
#include "RageUtil/File/RageFile.h"
#include "RageUtil/File/RageFileManager.h"
#include "RageUtil/Misc/RageLog.h"
#include "RageUtil/Utils/RageUtil.h"

#include "tomcrypt.h"

CryptManager* CRYPTMAN =
  nullptr; // global and accessible from anywhere in our program

static const std::string PRIVATE_KEY_PATH = "Data/private.rsa";
static const std::string PUBLIC_KEY_PATH = "Data/public.rsa";
static const std::string ALTERNATE_PUBLIC_KEY_DIR = "Data/keys/";

static bool
HashFile(RageFileBasic& f, unsigned char buf_hash[20], int iHash)
{
	hash_state hash;
	int iRet = hash_descriptor[iHash].init(&hash);
	ASSERT_M(iRet == CRYPT_OK, error_to_string(iRet));

	std::string s;
	while (!f.AtEOF()) {
		s.erase();
		if (f.Read(s, 1024 * 4) == -1) {
			LOG->Warn("Error reading %s: %s",
					  f.GetDisplayPath().c_str(),
					  f.GetError().c_str());
			hash_descriptor[iHash].done(&hash, buf_hash);
			return false;
		}

		iRet = hash_descriptor[iHash].process(
		  &hash, (const unsigned char*)s.data(), s.size());
		ASSERT_M(iRet == CRYPT_OK, error_to_string(iRet));
	}

	iRet = hash_descriptor[iHash].done(&hash, buf_hash);
	ASSERT_M(iRet == CRYPT_OK, error_to_string(iRet));

	return true;
}

#if defined(DISABLE_CRYPTO)
CryptManager::CryptManager() {}
CryptManager::~CryptManager() {}
void
CryptManager::GenerateRSAKey(unsigned int keyLength,
							 std::string privFilename,
							 std::string pubFilename)
{
}
void
CryptManager::SignFileToFile(const std::string& sPath,
							 std::string sSignatureFile)
{
}
bool
CryptManager::VerifyFileWithFile(std::string sPath,
								 std::string sSignatureFile,
								 std::string sPublicKeyFile)
{
	return true;
}
bool
CryptManager::VerifyFileWithFile(std::string sPath, std::string sSignatureFile)
{
	return true;
}

void
CryptManager::GetRandomBytes(void* pData, int iBytes)
{
	uint8_t* pBuf = (uint8_t*)pData;
	while (iBytes--)
		*pBuf++ = (uint8_t)RandomInt(256);
}

#else

static const int KEY_LENGTH = 1024;
#define MAX_SIGNATURE_SIZE_BYTES 1024 // 1 KB

/*
 openssl genrsa -out testing -outform DER
 openssl rsa -in testing -out testing2 -outform DER
 openssl rsa -in testing -out testing2 -pubout -outform DER
 openssl pkcs8 -inform DER -outform DER -nocrypt -in private.rsa -out
 private.der
*/

static PRNGWrapper* g_pPRNG = nullptr;

CryptManager::CryptManager()
{
	// Register with Lua.
	{
		Lua* L = LUA->Get();
		lua_pushstring(L, "CRYPTMAN");
		this->PushSelf(L);
		lua_settable(L, LUA_GLOBALSINDEX);
		LUA->Release(L);
	}

	g_pPRNG = new PRNGWrapper(&yarrow_desc);
}

void
CryptManager::GenerateGlobalKeys()
{
	//
	// generate keys if none are available
	//
	bool bGenerate = false;
	RSAKeyWrapper key;
	std::string sKey;
	std::string sError;
	if (!DoesFileExist(PRIVATE_KEY_PATH) ||
		!GetFileContents(PRIVATE_KEY_PATH, sKey) || !key.Load(sKey, sError))
		bGenerate = true;
	if (!sError.empty())
		LOG->Warn("Error loading RSA key: %s", sError.c_str());

	sError.clear();
	if (!DoesFileExist(PUBLIC_KEY_PATH) ||
		!GetFileContents(PUBLIC_KEY_PATH, sKey) || !key.Load(sKey, sError))
		bGenerate = true;
	if (!sError.empty())
		LOG->Warn("Error loading RSA key: %s", sError.c_str());

	if (bGenerate) {
		LOG->Warn("Keys missing or failed to load.  Generating new keys");
		GenerateRSAKeyToFile(KEY_LENGTH, PRIVATE_KEY_PATH, PUBLIC_KEY_PATH);
	}
}

CryptManager::~CryptManager()
{
	SAFE_DELETE(g_pPRNG);
	// Unregister with Lua.
	LUA->UnsetGlobal("CRYPTMAN");
}

static bool
WriteFile(const std::string& sFile, const std::string& sBuf)
{
	RageFile output;
	if (!output.Open(sFile, RageFile::WRITE)) {
		LOG->Warn("WriteFile: opening %s failed: %s",
				  sFile.c_str(),
				  output.GetError().c_str());
		return false;
	}

	if (output.Write(sBuf) == -1 || output.Flush() == -1) {
		LOG->Warn("WriteFile: writing %s failed: %s",
				  sFile.c_str(),
				  output.GetError().c_str());
		output.Close();
		FILEMAN->Remove(sFile);
		return false;
	}

	return true;
}

void
CryptManager::GenerateRSAKey(unsigned int keyLength,
							 std::string& sPrivKey,
							 std::string& sPubKey)
{
	int iRet;

	rsa_key key;
	iRet = rsa_make_key(
	  &g_pPRNG->m_PRNG, g_pPRNG->m_iPRNG, keyLength / 8, 65537, &key);
	if (iRet != CRYPT_OK) {
		LOG->Warn(
		  "GenerateRSAKey(%i) error: %s", keyLength, error_to_string(iRet));
		return;
	}

	unsigned char buf[1024];
	unsigned long iSize = sizeof(buf);
	iRet = rsa_export(buf, &iSize, PK_PUBLIC, &key);
	if (iRet != CRYPT_OK) {
		LOG->Warn("Export error: %s", error_to_string(iRet));
		return;
	}

	sPubKey = std::string((const char*)buf, iSize);

	iSize = sizeof(buf);
	iRet = rsa_export(buf, &iSize, PK_PRIVATE, &key);
	if (iRet != CRYPT_OK) {
		LOG->Warn("Export error: %s", error_to_string(iRet));
		return;
	}

	sPrivKey = std::string((const char*)buf, iSize);
}

void
CryptManager::GenerateRSAKeyToFile(unsigned int keyLength,
								   const std::string& privFilename,
								   const std::string& pubFilename)
{
	std::string sPrivKey, sPubKey;
	GenerateRSAKey(keyLength, sPrivKey, sPubKey);

	if (!WriteFile(pubFilename, sPubKey))
		return;

	if (!WriteFile(privFilename, sPrivKey)) {
		FILEMAN->Remove(privFilename);
		return;
	}
}

void
CryptManager::SignFileToFile(const std::string& sPath,
							 std::string sSignatureFile)
{
	std::string sPrivFilename = PRIVATE_KEY_PATH;
	if (sSignatureFile.empty())
		sSignatureFile = sPath + SIGNATURE_APPEND;

	std::string sPrivKey;
	if (!GetFileContents(sPrivFilename, sPrivKey))
		return;

	std::string sSignature;
	if (!Sign(sPath, sSignature, sPrivKey))
		return;

	WriteFile(sSignatureFile, sSignature);
}

bool
CryptManager::Sign(const std::string& sPath,
				   std::string& sSignatureOut,
				   const std::string& sPrivKey)
{
	if (!IsAFile(sPath)) {
		LOG->Trace("SignFileToFile: \"%s\" doesn't exist", sPath.c_str());
		return false;
	}

	RageFile file;
	if (!file.Open(sPath)) {
		LOG->Warn("SignFileToFile: open(%s) failed: %s",
				  sPath.c_str(),
				  file.GetError().c_str());
		return false;
	}

	RSAKeyWrapper key;
	std::string sError;
	if (!key.Load(sPrivKey, sError)) {
		LOG->Warn("Error loading RSA key: %s", sError.c_str());
		return false;
	}

	int iHash = register_hash(&sha1_desc);
	ASSERT(iHash >= 0);

	unsigned char buf_hash[20];
	if (!HashFile(file, buf_hash, iHash))
		return false;

	unsigned char signature[256];
	unsigned long signature_len = sizeof(signature);

	int iRet = rsa_sign_hash_ex(buf_hash,
								sizeof(buf_hash),
								signature,
								&signature_len,
								LTC_PKCS_1_V1_5,
								&g_pPRNG->m_PRNG,
								g_pPRNG->m_iPRNG,
								iHash,
								0,
								&key.m_Key);
	if (iRet != CRYPT_OK) {
		LOG->Warn("SignFileToFile error: %s", error_to_string(iRet));
		return false;
	}

	sSignatureOut.assign((const char*)signature, signature_len);
	return true;
}

bool
CryptManager::VerifyFileWithFile(const std::string& sPath,
								 const std::string& sSignatureFile)
{
	if (VerifyFileWithFile(sPath, sSignatureFile, PUBLIC_KEY_PATH))
		return true;

	vector<std::string> asKeys;
	GetDirListing(ALTERNATE_PUBLIC_KEY_DIR, asKeys, false, true);
	for (unsigned i = 0; i < asKeys.size(); ++i) {
		const std::string& sKey = asKeys[i];
		LOG->Trace("Trying alternate key \"%s\" ...", sKey.c_str());

		if (VerifyFileWithFile(sPath, sSignatureFile, sKey))
			return true;
	}

	return false;
}

bool
CryptManager::VerifyFileWithFile(const std::string& sPath,
								 std::string sSignatureFile,
								 const std::string& sPublicKeyFile)
{
	if (sSignatureFile.empty())
		sSignatureFile = sPath + SIGNATURE_APPEND;

	std::string sPublicKey;
	if (!GetFileContents(sPublicKeyFile, sPublicKey))
		return false;

	int iBytes = FILEMAN->GetFileSizeInBytes(sSignatureFile);
	if (iBytes > MAX_SIGNATURE_SIZE_BYTES)
		return false;

	std::string sSignature;
	if (!GetFileContents(sSignatureFile, sSignature))
		return false;

	RageFile file;
	if (!file.Open(sPath)) {
		LOG->Warn("Verify: open(%s) failed: %s",
				  sPath.c_str(),
				  file.GetError().c_str());
		return false;
	}

	return Verify(file, sSignature, sPublicKey);
}

bool
CryptManager::Verify(RageFileBasic& file,
					 const std::string& sSignature,
					 const std::string& sPublicKey)
{
	RSAKeyWrapper key;
	std::string sError;
	if (!key.Load(sPublicKey, sError)) {
		LOG->Warn("Error loading RSA key: %s", sError.c_str());
		return false;
	}

	int iHash = register_hash(&sha1_desc);
	ASSERT(iHash >= 0);

	unsigned char buf_hash[20];
	HashFile(file, buf_hash, iHash);

	int iMatch;
	int iRet = rsa_verify_hash_ex((const unsigned char*)sSignature.data(),
								  sSignature.size(),
								  buf_hash,
								  sizeof(buf_hash),
								  LTC_PKCS_1_EMSA,
								  iHash,
								  0,
								  &iMatch,
								  &key.m_Key);

	if (iRet != CRYPT_OK) {
		LOG->Warn("Verify(%s) failed: %s",
				  file.GetDisplayPath().c_str(),
				  error_to_string(iRet));
		return false;
	}

	if (iMatch == 0) {
		LOG->Warn("Verify(%s) failed: signature mismatch",
				  file.GetDisplayPath().c_str());
		return false;
	}

	return true;
}

void
CryptManager::GetRandomBytes(void* pData, int iBytes)
{
	int iRet = prng_descriptor[g_pPRNG->m_iPRNG].read(
	  reinterpret_cast<unsigned char*>(pData), iBytes, &g_pPRNG->m_PRNG);
	ASSERT(iRet == iBytes);
}
#endif

std::string
CryptManager::GetMD5ForFile(const std::string& fn)
{
	RageFile file;
	if (!file.Open(fn, RageFile::READ)) {
		LOG->Warn("GetMD5: Failed to open file '%s'", fn.c_str());
		return std::string();
	}
	int iHash = register_hash(&md5_desc);
	ASSERT(iHash >= 0);

	unsigned char digest[16];
	HashFile(file, digest, iHash);

	return std::string((const char*)digest, sizeof(digest));
}

std::string
CryptManager::GetMD5ForString(const std::string& sData)
{
	unsigned char digest[16];

	int iHash = register_hash(&md5_desc);

	hash_state hash;
	hash_descriptor[iHash].init(&hash);
	hash_descriptor[iHash].process(
	  &hash, (const unsigned char*)sData.data(), sData.size());
	hash_descriptor[iHash].done(&hash, digest);

	return std::string((const char*)digest, sizeof(digest));
}

std::string
CryptManager::GetSHA1ForString(const std::string& sData)
{
	unsigned char digest[20];

	int iHash = register_hash(&sha1_desc);

	hash_state hash;
	hash_descriptor[iHash].init(&hash);
	hash_descriptor[iHash].process(
	  &hash, (const unsigned char*)sData.data(), sData.size());
	hash_descriptor[iHash].done(&hash, digest);

	return std::string((const char*)digest, sizeof(digest));
}

std::string
CryptManager::GetSHA1ForFile(const std::string& fn)
{
	RageFile file;
	if (!file.Open(fn, RageFile::READ)) {
		LOG->Warn("GetSHA1: Failed to open file '%s'", fn.c_str());
		return std::string();
	}
	int iHash = register_hash(&sha1_desc);
	ASSERT(iHash >= 0);

	unsigned char digest[20];
	HashFile(file, digest, iHash);

	return std::string((const char*)digest, sizeof(digest));
}

std::string
CryptManager::GetPublicKeyFileName()
{
	return PUBLIC_KEY_PATH;
}

/* Generate a version 4 random UUID. */
std::string
CryptManager::GenerateRandomUUID()
{
	uint32_t buf[4];
	CryptManager::GetRandomBytes(buf, sizeof(buf));

	buf[1] &= 0xFFFF0FFF;
	buf[1] |= 0x00004000;
	buf[2] &= 0x0FFFFFFF;
	buf[2] |= 0xA0000000;

	return ssprintf("%08x-%04x-%04x-%04x-%04x%08x",
					buf[0],
					buf[1] >> 16,
					buf[1] & 0xFFFF,
					buf[2] >> 16,
					buf[2] & 0xFFFF,
					buf[3]);
}

// lua start

/** @brief Allow Lua to have access to the CryptManager. */
class LunaCryptManager : public Luna<CryptManager>
{
  public:
	static int MD5String(T* p, lua_State* L)
	{
		std::string md5out;
		md5out = p->GetMD5ForString(SArg(1));
		lua_pushlstring(L, md5out.c_str(), md5out.size());
		return 1;
	}
	static int MD5File(T* p, lua_State* L)
	{
		std::string md5fout;
		md5fout = p->GetMD5ForFile(SArg(1));
		lua_pushlstring(L, md5fout.c_str(), md5fout.size());
		return 1;
	}
	static int SHA1String(T* p, lua_State* L)
	{
		std::string sha1out;
		sha1out = p->GetSHA1ForString(SArg(1));
		lua_pushlstring(L, sha1out.c_str(), sha1out.size());
		return 1;
	}
	static int SHA1File(T* p, lua_State* L)
	{
		std::string sha1fout;
		sha1fout = p->GetSHA1ForFile(SArg(1));
		lua_pushlstring(L, sha1fout.c_str(), sha1fout.size());
		return 1;
	}
	static int GenerateRandomUUID(T* p, lua_State* L)
	{
		std::string uuidOut;
		uuidOut = p->GenerateRandomUUID();
		lua_pushlstring(L, uuidOut.c_str(), uuidOut.size());
		return 1;
	}

	LunaCryptManager()
	{
		ADD_METHOD(MD5String);
		ADD_METHOD(MD5File);
		ADD_METHOD(SHA1String);
		ADD_METHOD(SHA1File);
		ADD_METHOD(GenerateRandomUUID);
	}
};

LUA_REGISTER_CLASS(CryptManager)

// lua end
