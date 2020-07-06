#ifndef SONG_H
#define SONG_H

#include "Etterna/Models/Misc/Difficulty.h"
#include "RageUtil/Misc/RageTypes.h"
#include "RageUtil/Utils/RageUtil_AutoPtr.h"
#include "RageUtil/Utils/RageUtil_CachedObject.h"
#include "Etterna/Models/StepsAndStyles/Steps.h"
#include "Etterna/Models/Misc/TimingData.h"
#include <set>

class Style;
class StepsID;
struct lua_State;
struct BackgroundChange;

void
FixupPath(std::string& path, const std::string& sSongPath);
std::string
GetSongAssetPath(const std::string& sPath, const std::string& sSongPath);

/** @brief The version of the .ssc file format. */
const static float STEPFILE_VERSION_NUMBER = 0.83f;

/** @brief How many edits for this song can each profile have? */
const int MAX_EDITS_PER_SONG_PER_PROFILE = 15;
/** @brief How many edits for this song can be available? */
const int MAX_EDITS_PER_SONG = MAX_EDITS_PER_SONG_PER_PROFILE * NUM_ProfileSlot;

/** @brief The different background layers available. */
enum BackgroundLayer
{
	BACKGROUND_LAYER_1,
	BACKGROUND_LAYER_2,
	// BACKGROUND_LAYER_3, // StepCollection get
	NUM_BackgroundLayer,
	BACKGROUND_LAYER_Invalid
};

/** @brief A custom foreach loop for the different background layers. */
#define FOREACH_BackgroundLayer(bl) FOREACH_ENUM(BackgroundLayer, bl)

/** @brief Different instrument tracks for band type games. */
enum InstrumentTrack
{
	InstrumentTrack_Guitar,
	InstrumentTrack_Rhythm,
	InstrumentTrack_Bass,
	NUM_InstrumentTrack,
	InstrumentTrack_Invalid
};
const std::string&
InstrumentTrackToString(InstrumentTrack it);
InstrumentTrack
StringToInstrumentTrack(const std::string& s);

/** @brief The collection of lyrics for the Song. */
struct LyricSegment
{
	float m_fStartTime;	  /** @brief When does the lyric show up? */
	std::string m_sLyric; /** @brief The lyrics themselves. */
	RageColor m_Color;	  /** @brief The color of the lyrics. */
};

/** @brief Holds all music metadata and steps for one song. */
class Song
{
	std::string m_sSongDir;

  public:
	void SetSongDir(const std::string& sDir) { m_sSongDir = sDir; }
	const std::string& GetSongDir() { return m_sSongDir; }

	/** @brief When should this song be displayed in the music wheel? */
	enum SelectionDisplay
	{
		SHOW_ALWAYS, /**< always show on the wheel. */
		SHOW_NEVER	 /**< never show on the wheel (unless song hiding is turned
						off). */
	} m_SelectionDisplay;

	Song();
	~Song();
	void Reset();
	void DetachSteps();

	/**
	 * @brief Load a song from the chosen directory.
	 *
	 * This assumes that there is no song present right now.
	 * @param sDir the song directory from which to load. */
	void FinalizeLoading();
	bool LoadFromSongDir(std::string sDir, Calc* calc = nullptr);
	// This one takes the effort to reuse Steps pointers as best as it can
	bool ReloadFromSongDir(const std::string& sDir);
	bool ReloadFromSongDir() { return ReloadFromSongDir(GetSongDir()); }
	void ReloadIfNoMusic();

	std::string m_sFileHash;
	std::string GetFileHash();

	/**
	 * @brief Call this after loading a song to clean up invalid data.
	 * @param fromCache was this data loaded from the cache file?
	 * @param duringCache was this data loaded during the cache process? */
	void TidyUpData(bool fromCache = false,
					bool duringCache = false,
					Calc* calc = nullptr);

	/**
	 * @brief Get the new radar values, and determine the last second at the
	 * same time. This is called by TidyUpData, after saving the Song.
	 * @param fromCache was this data loaded from the cache file?
	 * @param duringCache was this data loaded during the cache process? */
	void ReCalculateRadarValuesAndLastSecond(bool fromCache = false,
											 bool duringCache = false,
											 Calc* calc = nullptr);
	/**
	 * @brief Translate any titles that aren't in english.
	 * This is called by TidyUpData. */
	void TranslateTitles();

	/**
	 * @brief Save to the new SSC file format.
	 * @param sPath the path where we're saving the file.
	 * @param bSavingCache a flag to determine if we're saving cache data.
	 */
	bool SaveToSSCFile(const std::string& sPath, bool bSavingCache);
	bool SaveToETTFile(const std::string& sPath, bool bSavingCache);

	/** @brief Save to the SSC and SM files no matter what. */
	void Save();
	/**
	 * @brief Save the current Song to a cache file using the preferred format.
	 * @return its success or failure. */
	bool SaveToCacheFile();
	/**
	 * @brief Save the current Song to a SM file.
	 * @return its success or failure. */
	bool SaveToSMFile();
	/**
	 * @brief Save the current Song to a DWI file if possible.
	 * @return its success or failure. */
	bool SaveToDWIFile();

	const std::string& GetSongFilePath() const;
	std::string GetCacheFilePath() const;

	// Directory this song data came from:
	const std::string& GetSongDir() const { return m_sSongDir; }

	/**
	 * @brief Filename associated with this file.
	 * This will always have a .SSC extension. If we loaded a .SSC, this will
	 * point to it, but if we loaded any other type, this will point to
	 * a generated .SSC filename. */
	std::string m_sSongFileName;

	/** @brief The group this Song is in. */
	std::string m_sGroupName;

	/**
	 * @brief the Profile this came from.
	 * This is ProfileSlot_Invalid if it wasn't loaded from a profile. */
	ProfileSlot m_LoadedFromProfile;
	/** @brief Is the song file itself a symlink to another file? */
	bool m_bIsSymLink;
	bool m_bEnabled;

	/** @brief The title of the Song. */
	std::string m_sMainTitle;
	/** @brief The subtitle of the Song, if it exists. */
	std::string m_sSubTitle;
	/** @brief The artist of the Song, if it exists. */
	std::string m_sArtist;
	/** @brief The transliterated title of the Song, if it exists. */
	std::string m_sMainTitleTranslit;
	/** @brief The transliterated subtitle of the Song, if it exists. */
	std::string m_sSubTitleTranslit;
	/** @brief The transliterated artist of the Song, if it exists. */
	std::string m_sArtistTranslit;

	/* If PREFSMAN->m_bShowNative is off, these are the same as GetTranslit*
	 * below. Otherwise, they return the main titles. */
	const std::string& GetDisplayMainTitle() const;
	const std::string& GetDisplaySubTitle() const;
	const std::string& GetDisplayArtist() const;
	const std::string& GetMainTitle() const;

	/**
	 * @brief Retrieve the transliterated title, or the main title if there is
	 * no translit.
	 * @return the proper title. */
	const std::string& GetTranslitMainTitle() const
	{
		return m_sMainTitleTranslit.size() ? m_sMainTitleTranslit
										   : m_sMainTitle;
	}

	vector<Steps*> GetStepsToSave(bool bSavingCache = true,
								  std::string path = "");

	/**
	 * @brief Retrieve the transliterated subtitle, or the main subtitle if
	 * there is no translit.
	 * @return the proper subtitle. */
	const std::string& GetTranslitSubTitle() const
	{
		return m_sSubTitleTranslit.size() ? m_sSubTitleTranslit : m_sSubTitle;
	}
	/**
	 * @brief Retrieve the transliterated artist, or the main artist if there is
	 * no translit.
	 * @return the proper artist. */
	const std::string& GetTranslitArtist() const
	{
		return m_sArtistTranslit.size() ? m_sArtistTranslit : m_sArtist;
	}

	// "title subtitle"
	std::string displayfulltitle;
	std::string translitfulltitle;
	const std::string& GetDisplayFullTitle() const { return displayfulltitle; }
	const std::string& GetTranslitFullTitle() const
	{
		return translitfulltitle;
	}

	/** @brief The version of the song/file. */
	float m_fVersion;
	/** @brief The genre of the song/file. */
	std::string m_sGenre;

	/**
	 * @brief The person who worked with the song file who should be credited.
	 * This is read and saved, but never actually used. */
	std::string m_sCredit;

	std::string m_sOrigin; // song origin (for .ssc format)

	std::string m_sMusicFile;
	std::string m_PreviewFile;
	std::string m_sInstrumentTrackFile[NUM_InstrumentTrack];

	/** @brief The length of the music file. */
	float m_fMusicLengthSeconds;
	float m_fMusicSampleStartSeconds;
	float m_fMusicSampleLengthSeconds;
	DisplayBPM m_DisplayBPMType;
	float m_fSpecifiedBPMMin;
	float m_fSpecifiedBPMMax; // if a range, then Min != Max

	std::string m_sBannerFile; // typically a 16:5 ratio graphic (e.g. 256x80)
	std::string m_sJacketFile; // typically square (e.g. 192x192, 256x256)
	std::string m_sCDFile;	   // square (e.g. 128x128 [DDR 1st-3rd])
	std::string
	  m_sDiscFile; // rectangular (e.g. 256x192 [Pump], 200x150 [MGD3])
	std::string m_sLyricsFile;
	std::string m_sBackgroundFile;
	std::string m_sCDTitleFile;
	std::string m_sPreviewVidFile;

	std::string m_sMusicPath;
	std::string m_PreviewPath;
	std::string m_sInstrumentTrackPath[NUM_InstrumentTrack];
	std::string m_sBannerPath; // typically a 16:5 ratio graphic (e.g. 256x80)
	std::string m_sJacketPath; // typically square (e.g. 192x192, 256x256)
	std::string m_sCDPath;	   // square (e.g. 128x128 [DDR 1st-3rd])
	std::string
	  m_sDiscPath; // rectangular (e.g. 256x192 [Pump], 200x150 [MGD3])
	std::string m_sLyricsPath;
	std::string m_sBackgroundPath;
	std::string m_sCDTitlePath;
	std::string m_sPreviewVidPath;

	vector<std::string> ImageDir;

	static std::string GetSongAssetPath(std::string sPath,
										const std::string& sSongPath);
	const std::string& GetMusicPath() const { return m_sMusicPath; }
	const std::string& GetInstrumentTrackPath(InstrumentTrack it) const

	{
		return m_sInstrumentTrackPath[it];
	}
	const std::string& GetBannerPath() const { return m_sBannerPath; }
	const std::string& GetJacketPath() const { return m_sJacketPath; }
	const std::string& GetCDImagePath() const { return m_sCDPath; }
	const std::string& GetDiscPath() const { return m_sDiscPath; }
	const std::string& GetLyricsPath() const { return m_sLyricsPath; }
	const std::string& GetBackgroundPath() const { return m_sBackgroundPath; }
	const std::string& GetCDTitlePath() const { return m_sCDTitlePath; }
	const std::string& GetPreviewVidPath() const { return m_sPreviewVidPath; }
	const std::string& GetPreviewMusicPath() const { return m_PreviewPath; }
	float GetPreviewStartSeconds() const;
	std::string GetCacheFile(std::string sPath);

	// how have i not jammed anything here yet - mina

	/** @brief Get the highest value for a specific skillset for the song at a
	 given rate, within the step types of the current game mode. */
	float HighestMSDOfSkillset(Skillset skill, float rate) const;
	bool IsSkillsetHighestOfAnySteps(Skillset ss, float rate) const;
	/** @brief This functions returns whether the song has a chart within the
	 current game mode and of the given rate that matches the current
	 filter.*/
	bool MatchesFilter(const float rate) const;

	bool HasChartByHash(const std::string& hash);

	// For loading only:
	bool m_bHasMusic, m_bHasBanner, m_bHasBackground;

	bool HasMusic() const;
	bool HasInstrumentTrack(InstrumentTrack it) const;
	bool HasBanner() const;
	bool HasBackground() const;
	bool HasJacket() const;
	bool HasCDImage() const;
	bool HasDisc() const;
	bool HasCDTitle() const;
	bool HasBGChanges() const;
	bool HasLyrics() const;
	bool HasPreviewVid() const;

	bool Matches(const std::string& sGroup, const std::string& sSong) const;

	/** @brief The Song's TimingData. */
	TimingData m_SongTiming;

	float GetFirstBeat() const;
	float GetFirstSecond() const;
	float GetLastBeat() const;
	float GetLastSecond() const;
	float GetSpecifiedLastBeat() const;
	float GetSpecifiedLastSecond() const;

	void SetFirstSecond(float f);
	void SetLastSecond(float f);
	void SetSpecifiedLastSecond(float f);

	typedef vector<BackgroundChange> VBackgroundChange;

  private:
	/** @brief The first second that a note is hit. */
	float firstSecond;
	/** @brief The last second that a note is hit. */
	float lastSecond;
	/** @brief The last second of the song for playing purposes. */
	float specifiedLastSecond;
	/**
	 * @brief The background changes (sorted by layer) that are for this Song.
	 * This uses an AutoPtr instead of a raw pointer so that the
	 * auto gen'd copy constructor works correctly.
	 * This must be sorted before gameplay. */
	AutoPtrCopyOnWrite<VBackgroundChange>
	  m_BackgroundChanges[NUM_BackgroundLayer];
	/**
	 * @brief The foreground changes that are for this Song.
	 * This uses an AutoPtr instead of a raw pointer so that the
	 * auto gen'd copy constructor works correctly.
	 * This must be sorted before gameplay. */
	AutoPtrCopyOnWrite<VBackgroundChange> m_ForegroundChanges;

	vector<std::string> GetChangesToVectorString(
	  const vector<BackgroundChange>& changes) const;

  public:
	const vector<BackgroundChange>& GetBackgroundChanges(
	  BackgroundLayer bl) const;
	vector<BackgroundChange>& GetBackgroundChanges(BackgroundLayer bl);
	const vector<BackgroundChange>& GetForegroundChanges() const;
	vector<BackgroundChange>& GetForegroundChanges();

	vector<std::string> GetBGChanges1ToVectorString() const;
	vector<std::string> GetBGChanges2ToVectorString() const;
	vector<std::string> GetFGChanges1ToVectorString() const;

	vector<std::string> GetInstrumentTracksToVectorString() const;

	/**
	 * @brief The list of LyricSegments.
	 * This must be sorted before gameplay. */
	vector<LyricSegment> m_LyricSegments;

	void AddBackgroundChange(BackgroundLayer blLayer,
							 const BackgroundChange& seg);
	void AddForegroundChange(const BackgroundChange& seg);
	void AddLyricSegment(LyricSegment seg);

	void GetDisplayBpms(DisplayBpms& AddTo) const;
	const BackgroundChange& GetBackgroundAtBeat(BackgroundLayer iLayer,
												float fBeat) const;

	Steps* CreateSteps();
	void InitSteps(Steps* pSteps);

	std::string GetOrTryAtLeastToGetSimfileAuthor();

	bool HasSignificantBpmChangesOrStops() const;
	float GetStepsSeconds() const;
	bool IsLong() const;
	bool IsMarathon() const;

	// plays music for chart preview and is available to lua -mina
	void PlaySampleMusicExtended();

	bool SongCompleteForStyle(const Style* st) const;
	bool HasStepsType(StepsType st) const;
	bool HasStepsTypeAndDifficulty(StepsType st, Difficulty dc) const;
	const vector<Steps*>& GetAllSteps() const { return m_vpSteps; }
	const vector<Steps*>& GetStepsByStepsType(StepsType st) const
	{
		return m_vpStepsByType[st];
	}
	/** @brief Get the steps of all types within the current game mode */
	const vector<Steps*> GetStepsOfCurrentGameMode() const;
	bool HasEdits(StepsType st) const;

	bool IsFavorited() { return isfavorited; }
	void SetFavorited(bool b) { isfavorited = b; }
	bool HasGoal() { return hasgoal; }
	void SetHasGoal(bool b) { hasgoal = b; }
	bool IsPermaMirror() { return permamirror; }
	void SetPermaMirror(bool b) { permamirror = b; }

	void SetEnabled(bool b) { m_bEnabled = b; }
	bool GetEnabled() const { return m_bEnabled; }

	/**
	 * @brief Add the chosen Steps to the Song.
	 * We are responsible for deleting the memory pointed to by pSteps!
	 * @param pSteps the new steps. */
	void AddSteps(Steps* pSteps);
	void DeleteSteps(const Steps* pSteps, bool bReAutoGen = true);

	void FreeAllLoadedFromProfile(ProfileSlot slot = ProfileSlot_Invalid,
								  const set<Steps*>* setInUse = NULL);
	bool WasLoadedFromProfile() const
	{
		return m_LoadedFromProfile != ProfileSlot_Invalid;
	}
	void GetStepsLoadedFromProfile(ProfileSlot slot,
								   vector<Steps*>& vpStepsOut) const;
	int GetNumStepsLoadedFromProfile(ProfileSlot slot) const;
	bool IsEditAlreadyLoaded(Steps* pSteps) const;

	bool IsStepsUsingDifferentTiming(Steps* pSteps) const;
	bool AnyChartUsesSplitTiming() const;
	void UnloadAllCalcDebugOutput();
	/**
	 * @brief An array of keysound file names (e.g. "beep.wav").
	 * The index in this array corresponds to the index in TapNote.
	 * If you  change the index in here, you must change all NoteData too.
	 * Any note that doesn't have a value in the range of this array
	 * means "this note doesn't have a keysound". */
	vector<std::string> m_vsKeysoundFile;

	CachedObject<Song> m_CachedObject;

	// Lua
	void PushSelf(lua_State* L);

	vector<Steps*> m_vpSteps;
	vector<Steps*> m_UnknownStyleSteps;

  private:
	bool isfavorited = false;
	bool permamirror = false;
	bool hasgoal = false;
	/** @brief the Steps that belong to this Song. */
	/** @brief the Steps of a particular StepsType that belong to this Song. */
	vector<Steps*> m_vpStepsByType[NUM_StepsType];
	/** @brief the Steps that are of unrecognized Styles. */
};

#endif
