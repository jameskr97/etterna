#include "Etterna/Globals/global.h"
#include "Etterna/Models/Misc/LocalizedString.h"
#include "Etterna/Models/Misc/PlayerState.h"
#include "Etterna/Singletons/ProfileManager.h"
#include "RageUtil/File/RageFileManager.h"
#include "Core/Services/Locator.hpp"
#include "Etterna/Singletons/ScreenManager.h"
#include "ScreenServiceAction.h"
#include "Etterna/Models/Songs/Song.h"
#include "Etterna/Singletons/SongManager.h"
#include "Etterna/Globals/StepMania.h"
#include "Etterna/Singletons/ThemeManager.h"
#include "Etterna/Models/Misc/Foreach.h"

static LocalizedString EDITS_CLEARED("ScreenServiceAction",
									 "%d edits cleared, %d errors.");
static LocalizedString STATS_NOT_SAVED(
  "ScreenServiceAction",
  "Stats not saved - No memory cards ready.");
static LocalizedString STATS_NOT_LOADED(
  "ScreenServiceAction",
  "Stats not loaded - No memory cards ready.");
static LocalizedString THERE_IS_NO_PROFILE(
  "ScreenServiceAction",
  "There is no machine profile on P%d card.");
static LocalizedString PROFILE_CORRUPT(
  "ScreenServiceAction",
  "The profile on P%d card contains corrupt or tampered data.");

static void
CopyEdits(const std::string& sFromProfileDir,
		  const std::string& sToProfileDir,
		  int& iNumSucceeded,
		  int& iNumOverwritten,
		  int& iNumIgnored,
		  int& iNumErrored)
{
	iNumSucceeded = 0;
	iNumOverwritten = 0;
	iNumIgnored = 0;
	iNumErrored = 0;

	{
		std::string sFromDir = sFromProfileDir + EDIT_STEPS_SUBDIR;
		std::string sToDir = sToProfileDir + EDIT_STEPS_SUBDIR;

		vector<std::string> vsFiles;
		GetDirListing(sFromDir + "*.edit", vsFiles, false, false);
		for (auto& i : vsFiles) {
			if (DoesFileExist(sToDir + i))
				iNumOverwritten++;
			bool bSuccess = FileCopy(sFromDir + i, sToDir + i);
			if (bSuccess)
				iNumSucceeded++;
			else
				iNumErrored++;
		}
	}
}

static LocalizedString EDITS_NOT_COPIED(
  "ScreenServiceAction",
  "Edits not copied - No memory cards ready.");
static LocalizedString COPIED_TO_CARD("ScreenServiceAction",
									  "Copied to P%d card:");
static LocalizedString COPIED("ScreenServiceAction", "%d copied");
static LocalizedString OVERWRITTEN("ScreenServiceAction", "%d overwritten");
static LocalizedString ADDED("ScreenServiceAction", "%d added");
static LocalizedString IGNORED("ScreenServiceAction", "%d ignored");
static LocalizedString FAILED("ScreenServiceAction", "%d failed");
static LocalizedString DELETED("ScreenServiceAction", "%d deleted");

static std::string
CopyEdits(const std::string& sFromProfileDir,
		  const std::string& sToProfileDir,
		  const std::string& sDisplayDir)
{
	int iNumSucceeded = 0;
	int iNumOverwritten = 0;
	int iNumIgnored = 0;
	int iNumErrored = 0;

	CopyEdits(sFromProfileDir,
			  sToProfileDir,
			  iNumSucceeded,
			  iNumOverwritten,
			  iNumIgnored,
			  iNumErrored);

	vector<std::string> vs;
	vs.push_back(sDisplayDir);
	vs.push_back(ssprintf(COPIED.GetValue(), iNumSucceeded) + ", " +
				 ssprintf(OVERWRITTEN.GetValue(), iNumOverwritten));
	if (iNumIgnored)
		vs.push_back(ssprintf(IGNORED.GetValue(), iNumIgnored));
	if (iNumErrored)
		vs.push_back(ssprintf(FAILED.GetValue(), iNumErrored));
	return join("\n", vs);
}

static void
SyncFiles(const std::string& sFromDir,
		  const std::string& sToDir,
		  const std::string& sMask,
		  int& iNumAdded,
		  int& iNumDeleted,
		  int& iNumOverwritten,
		  int& iNumFailed)
{
	vector<std::string> vsFilesSource;
	GetDirListing(sFromDir + sMask, vsFilesSource, false, false);

	vector<std::string> vsFilesDest;
	GetDirListing(sToDir + sMask, vsFilesDest, false, false);

	vector<std::string> vsToDelete;
	GetAsNotInBs(vsFilesDest, vsFilesSource, vsToDelete);

	for (auto& i : vsToDelete) {
		std::string sFile = sToDir + i;
        Locator::getLogger()->trace("Delete \"{}\"", sFile.c_str());

		if (FILEMAN->Remove(sFile))
			++iNumDeleted;
		else
			++iNumFailed;
	}

	for (auto& i : vsFilesSource) {
		std::string sFileFrom = sFromDir + i;
		std::string sFileTo = sToDir + i;
        Locator::getLogger()->trace("Copy \"{}\"", sFileFrom.c_str());
		bool bOverwrite = DoesFileExist(sFileTo);
		bool bSuccess = FileCopy(sFileFrom, sFileTo);
		if (bSuccess) {
			if (bOverwrite)
				++iNumOverwritten;
			else
				++iNumAdded;
		} else
			++iNumFailed;
	}
	FILEMAN->FlushDirCache(sToDir);
}

static void
SyncEdits(const std::string& sFromDir,
		  const std::string& sToDir,
		  int& iNumAdded,
		  int& iNumDeleted,
		  int& iNumOverwritten,
		  int& iNumFailed)
{
	iNumAdded = 0;
	iNumDeleted = 0;
	iNumOverwritten = 0;
	iNumFailed = 0;

	SyncFiles(sFromDir + EDIT_STEPS_SUBDIR,
			  sToDir + EDIT_STEPS_SUBDIR,
			  "*.edit",
			  iNumAdded,
			  iNumDeleted,
			  iNumOverwritten,
			  iNumFailed);
}

static LocalizedString COPIED_FROM_CARD("ScreenServiceAction",
										"Copied from P%d card:");

static LocalizedString PREFERENCES_RESET("ScreenServiceAction",
										 "Preferences reset.");
static std::string
ResetPreferences()
{
	StepMania::ResetPreferences();
	return PREFERENCES_RESET.GetValue();
}

REGISTER_SCREEN_CLASS(ScreenServiceAction);
void
ScreenServiceAction::BeginScreen()
{
	std::string sActions = THEME->GetMetric(m_sName, "Actions");
	vector<std::string> vsActions;
	split(sActions, ",", vsActions);

	vector<std::string> vsResults;
	FOREACH(std::string, vsActions, s)
	{
		std::string (*pfn)() = NULL;

		if (*s == "ResetPreferences")
			pfn = ResetPreferences;

		ASSERT_M(pfn != NULL, *s);

		std::string sResult = pfn();
		vsResults.push_back(sResult);
	}

	ScreenPrompt::SetPromptSettings(join("\n", vsResults), PROMPT_OK);

	ScreenPrompt::BeginScreen();
}
