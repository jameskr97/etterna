#pragma once
#ifndef SM_DOWNMANAGER

#define SM_DOWNMANAGER

#include "Etterna/Globals/global.h"
#include "CommandLineActions.h"
#include "RageUtil/File/RageFile.h"
#include "Etterna/Models/Misc/HighScore.h"
#include "ScreenManager.h"
#include "RageUtil/File/RageFileManager.h"
#include "curl/curl.h"
#include "Etterna/Models/Misc/Difficulty.h"
#include <deque>

class DownloadablePack;

class ProgressData
{
  public:
	curl_off_t total{ 0 };		// total bytes
	curl_off_t downloaded{ 0 }; // bytes downloaded
	float time{ 0 };			// seconds passed
};

class RageFileWrapper
{
  public:
	RageFile file;
	size_t bytes{ 0 };
	bool stop{ false };
};

class Download
{
  public:
	std::function<void(Download*)> Done;
	Download(std::string url,
			 std::string filename = "",
			 std::function<void(Download*)> done = [](Download*) { return; });
	~Download();
	void Install();
	void Update(float fDeltaSeconds);
	void Failed();
	std::string StartMessage()
	{
		return "Downloading file " + m_TempFileName + " from " + m_Url;
	};
	std::string Status()
	{
		return m_TempFileName + "\n" + speed + " KB/s\n" + "Downloaded " +
			   std::to_string((progress.downloaded > 0 ? progress.downloaded
												  : p_RFWrapper.bytes) /
						 1024) +
			   (progress.total > 0
				  ? "/" + std::to_string(progress.total / 1024) + " (KB)"
				  : "");
	}
	CURL* handle{ nullptr };
	int running{ 1 };
	ProgressData progress;
	std::string speed{ "" };
	curl_off_t downloadedAtLastUpdate{ 0 };
	curl_off_t lastUpdateDone{ 0 };
	std::string m_Url{ "" };
	RageFileWrapper p_RFWrapper;
	DownloadablePack* p_Pack{ nullptr };
	std::string m_TempFileName{ "" };
	// Lua
	void PushSelf(lua_State* L);

  protected:
	std::string MakeTempFileName(std::string s);
};

class DownloadablePack
{
  public:
	std::string name{ "" };
	size_t size{ 0 };
	int id{ 0 };
	float avgDifficulty{ 0 };
	std::string url{ "" };
	std::string mirror{ "" };
	bool downloading{ false };
	// Lua
	void PushSelf(lua_State* L);
};

class HTTPRequest
{
  public:
	HTTPRequest(CURL* h,
				std::function<void(HTTPRequest&, CURLMsg*)> done =
				  [](HTTPRequest& req, CURLMsg*) { return; },
				curl_httppost* postform = nullptr,
				std::function<void(HTTPRequest&, CURLMsg*)> fail =
				  [](HTTPRequest& req, CURLMsg*) { return; })
	  : handle(h)
	  , form(postform)
	  , Done(done)
	  , Failed(fail){};
	CURL* handle{ nullptr };
	curl_httppost* form{ nullptr };
	std::string result;
	std::function<void(HTTPRequest&, CURLMsg*)> Done;
	std::function<void(HTTPRequest&, CURLMsg*)> Failed;
};
class OnlineTopScore
{
  public:
	float wifeScore{ 0.0f };
	std::string songName;
	float rate{ 0.0f };
	float ssr{ 0.0f };
	float overall{ 0.0f };
	std::string chartkey;
	std::string scorekey;
	Difficulty difficulty;
	std::string steps;
};
struct OnlineHighScore : HighScore
{
  public:
	bool hasReplay;
	bool HasReplayData() override { return hasReplay; }
};
class OnlineScore
{
  public:
	std::map<Skillset, float> SSRs;
	float rate{ 0.0f };
	float wife{ 0.0f };
	int maxcombo{ 0 };
	int miss{ 0 };
	int bad{ 0 };
	int good{ 0 };
	int great{ 0 };
	int perfect{ 0 };
	int marvelous{ 0 };
	int minehits{ 0 };
	int held{ 0 };
	std::string songId;
	int letgo{ 0 };
	bool valid{ false };
	bool nocc{ false };
	std::string username;
	float playerRating{ 0.0f };
	std::string modifiers;
	std::string scoreid;
	std::string avatar;
	int userid;
	DateTime datetime;
	bool hasReplay{ false };
	std::vector<std::pair<float, float>> replayData;
	std::string countryCode;
	OnlineHighScore hs;
	void Push(lua_State* L) { hs.PushSelf(L); }
	bool HasReplayData() { return hasReplay; }
};

class DownloadManager
{
  public:
	static LuaReference EMPTY_REFERENCE;
	DownloadManager();
	~DownloadManager();
	std::map<std::string, Download*> downloads; // Active downloads
	std::vector<HTTPRequest*>
	  HTTPRequests; // Active HTTP requests (async, curlMulti)

	std::map<std::string, Download*> finishedDownloads;
	std::map<std::string, Download*> pendingInstallDownloads;
	CURLM* mPackHandle{ nullptr }; // Curl multi handle for packs downloads
	CURLM* mHTTPHandle{ nullptr }; // Curl multi handle for httpRequests
	CURLMcode ret;
	int downloadingPacks{ 0 };
	int HTTPRunning{ 0 };
	bool loggingIn{
		false
	}; // Currently logging in (Since it's async, to not try twice)
	bool gameplay{ false }; // Currently in gameplay
	bool initialized{ false };
	std::string error{ "" };
	std::vector<DownloadablePack> downloadablePacks;
	std::string authToken{ "" };   // Session cookie content
	std::string sessionUser{ "" }; // Currently logged in username
	std::string sessionPass{ "" }; // Currently logged in password
	std::string lastVersion{
		""
	}; // Last version according to server (Or current if non was obtained)
	std::string registerPage{
		""
	}; // Register page from server (Or empty if non was obtained)
	std::map<std::string, std::vector<OnlineScore>> chartLeaderboards;
	std::vector<std::string> countryCodes;
	std::map<Skillset, int>
	  sessionRanks; // Leaderboard ranks for logged in user by skillset
	std::map<Skillset, double> sessionRatings;
	std::map<Skillset, std::vector<OnlineTopScore>> topScores;
	bool LoggedIn();

	void AddFavorite(const std::string& chartkey);
	void RemoveFavorite(const std::string& chartkey);
	void RefreshFavourites();
	std::vector<std::string> favorites;

	void AddGoal(const std::string& chartkey,
				 float wife,
				 float rate,
				 DateTime& timeAssigned);
	void UpdateGoal(const std::string& chartkey,
					float wife,
					float rate,
					bool achieved,
					DateTime& timeAssigned,
					DateTime& timeAchieved);
	void RemoveGoal(const std::string& chartkey, float wife, float rate);

	void EndSessionIfExists(); // Calls EndSession if logged in
	void EndSession();		   // Sends session destroy request
	void StartSession(std::string user,
					  std::string pass,
					  std::function<void(bool loggedIn)>
						done); // Sends login request if not already logging in
	void OnLogin();
	bool UploadScores(); // Uploads all scores not yet uploaded to current
	bool UpdateOnlineScoreReplayData();	// attempts updates existing replaydata
						 // server (Async, 1 request per score)
	void RefreshPackList(const std::string& url);

	void init();
	Download* DownloadAndInstallPack(const std::string& url, std::string filename = "");
	Download* DownloadAndInstallPack(DownloadablePack* pack,
									 bool mirror = false);
	void Update(float fDeltaSeconds);
	void UpdatePacks(float fDeltaSeconds);
	void UpdateHTTP(float fDeltaSeconds);
	bool InstallSmzip(const std::string& sZipFile);

	void UpdateDLSpeed();
	void UpdateDLSpeed(bool gameplay);

	std::string GetError() { return error; }
	bool Error() { return error == ""; }
	bool EncodeSpaces(string& str);

	void UploadScoreWithReplayData(HighScore* hs);
	void UploadScoreWithReplayDataFromDisk(
	  const std::string& sk,
	  std::function<void()> callback = std::function<void()>());
	void UpdateOnlineScoreReplayData(
	  const std::string& sk,
	  std::function<void()> callback = std::function<void()>());
	void UploadScore(HighScore* hs);

	bool ShouldUploadScores();

	inline void AddSessionCookieToCURL(CURL* curlHandle);
	inline void SetCURLPostToURL(CURL* curlHandle, std::string url);
	inline void SetCURLURL(CURL* curlHandle, std::string url);

	HTTPRequest* SendRequest(std::string requestName,
							 std::vector<std::pair<string, string>> params,
							 std::function<void(HTTPRequest&, CURLMsg*)> done,
							 bool requireLogin = true,
							 bool post = false,
							 bool async = true,
							 bool withBearer = true);
	HTTPRequest* SendRequestToURL(std::string url,
								  std::vector<std::pair<string, string>> params,
								  std::function<void(HTTPRequest&, CURLMsg*)> done,
								  bool requireLogin,
								  bool post,
								  bool async,
								  bool withBearer);
	void RefreshLastVersion();
	void RefreshRegisterPage();
	bool currentrateonly = false;
	bool topscoresonly = true;
	void RefreshCountryCodes();
	void RequestReplayData(const std::string& scorekey,
						   int userid,
						   const std::string& username,
						   const std::string& chartkey,
						   LuaReference& callback = EMPTY_REFERENCE);
	void RequestChartLeaderBoard(const std::string& chartkey,
								 LuaReference& ref = EMPTY_REFERENCE);
	void RefreshUserData();
	std::string countryCode;
	void RefreshUserRank();
	void RefreshTop25(Skillset ss);
	void DownloadCoreBundle(const std::string& whichoneyo, bool mirror = true);
	std::map<std::string, std::vector<DownloadablePack*>> bundles;
	void RefreshCoreBundles();
	std::vector<DownloadablePack*> GetCoreBundle(const std::string& whichoneyo);
	OnlineTopScore GetTopSkillsetScore(unsigned int rank,
									   Skillset ss,
									   bool& result);
	float GetSkillsetRating(Skillset ss);
	int GetSkillsetRank(Skillset ss);

	// most recent single score upload result -mina
	RString mostrecentresult = "";
	std::deque<std::pair<DownloadablePack*, bool>> DownloadQueue; // (pack,isMirror)
	const int maxPacksToDownloadAtOnce = 1;
	const float DownloadCooldownTime = 5.f;
	float timeSinceLastDownload = 0.f;

	// Lua
	void PushSelf(lua_State* L);
};

extern std::shared_ptr<DownloadManager> DLMAN;

#endif

