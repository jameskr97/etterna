#include "Etterna/Globals/global.h"
#include "Etterna/Models/Misc/Foreach.h"
#include "Etterna/Models/Misc/GameConstantsAndTypes.h"
#include "Etterna/Models/Misc/HighScore.h"
#include <MinaCalc/MinaCalc.h>
#include "Etterna/Models/NoteData/NoteData.h"
#include "Etterna/Models/NoteData/NoteDataStructures.h"
#include "RageUtil/Misc/RageLog.h"
#include "RageUtil/Misc/RageTimer.h"
#include "ScoreManager.h"
#include "Etterna/Models/Songs/Song.h"
#include "Etterna/FileTypes/XmlFile.h"
#include "Etterna/FileTypes/XmlFileUtil.h"
#include "arch/LoadingWindow/LoadingWindow.h"
#include "RageUtil/Misc/RageThreads.h"

ScoreManager* SCOREMAN = NULL;

ScoreManager::ScoreManager()
{
	// Register with Lua.
	{
		Lua* L = LUA->Get();
		lua_pushstring(L, "SCOREMAN");
		this->PushSelf(L);
		lua_settable(L, LUA_GLOBALSINDEX);
		LUA->Release(L);
	}
}

ScoreManager::~ScoreManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal("SCOREMAN");
}

ScoresAtRate::ScoresAtRate()
{
	bestGrade = Grade_Invalid;
	scores.clear();
	PBptr = nullptr;
	noccPBptr = nullptr;
}

HighScore*
ScoresAtRate::AddScore(HighScore& hs)
{
	const std::string& key = hs.GetScoreKey();
	bestGrade = std::min(hs.GetWifeGrade(), bestGrade);
	scores.emplace(key, hs);

	if (!PBptr || PBptr->GetSSRNormPercent() < hs.GetSSRNormPercent())
		PBptr = &scores.find(key)->second;

	if (hs.GetChordCohesion() == 0) {
		if (!noccPBptr ||
			noccPBptr->GetSSRNormPercent() < hs.GetSSRNormPercent())
			noccPBptr = &scores.find(key)->second;
	}

	SCOREMAN->RegisterScore(&scores.find(key)->second);
	SCOREMAN->AddToKeyedIndex(&scores.find(key)->second);
	return &(scores.find(key)->second);
}

std::vector<std::string>
ScoresAtRate::GetSortedKeys()
{
	std::map<float, string, std::greater<float>> tmp;
	std::vector<std::string> o;
	FOREACHUM(string, HighScore, scores, i)
	tmp.emplace(i->second.GetWifeScore(), i->first);
	FOREACHM(float, string, tmp, j)
	o.emplace_back(j->second);
	return o;
}

/*
const vector<HighScore*> ScoresAtRate::GetScores() const {
	map<float, string, greater<float>> tmp;
	vector<HighScore*> o;
	FOREACHUM_CONST(string, HighScore, scores, i)
		tmp.emplace(i->second.GetWifeScore(), i->first);
	FOREACHM(float, string, tmp, j)
		o.emplace_back(j->second);
	return o;
}
*/

void
ScoreManager::PurgeScores()
{
	TopSSRs.clear();
	TopSSRs.shrink_to_fit();

	AllScores.clear();
	AllScores.shrink_to_fit();

	ScoresByKey.clear();

	pscores.clear();
}

void
ScoreManager::PurgeProfileScores(const std::string& profileID)
{
	TopSSRs.clear();
	TopSSRs.shrink_to_fit();
	for (auto& score : AllProfileScores[profileID]) {
		{
			auto it = find(AllScores.begin(), AllScores.end(), score);
			if (it != AllScores.end())
				AllScores.erase(it);
		}
		auto it = ScoresByKey.find(score->GetChartKey());
		if (it != ScoresByKey.end())
			ScoresByKey.erase(it);
	}
	AllScores.shrink_to_fit();
	AllProfileScores[profileID].clear();
	AllProfileScores[profileID].shrink_to_fit();

	pscores[profileID].clear();
}

void
ScoreManager::RatingOverTime()
{
	if (false)
		for (auto* s : AllScores)
			s->GenerateValidationKeys();
}

ScoresForChart::ScoresForChart()
{
	bestGrade = Grade_Invalid;
	ScoresByRate.clear();
}

HighScore*
ScoresForChart::GetPBAt(float& rate)
{
	int key = RateToKey(rate);
	if (ScoresByRate.count(key))
		return ScoresByRate.at(key).PBptr;
	return NULL;
}

HighScore*
ScoresForChart::GetPBUpTo(float& rate)
{
	int key = RateToKey(rate);
	FOREACHM(int, ScoresAtRate, ScoresByRate, i)
	if (i->first <= key)
		return i->second.PBptr;

	return NULL;
}

HighScore*
ScoresForChart::AddScore(HighScore& hs)
{
	bestGrade = std::min(hs.GetWifeGrade(), bestGrade);

	float rate = hs.GetMusicRate();
	int key = RateToKey(rate);
	auto hsPtr = ScoresByRate[key].AddScore(hs);
	// ok let's try this --lurker
	SetTopScores();
	hs.SetTopScore(hsPtr->GetTopScore());
	return hsPtr;
}

std::vector<float>
ScoresForChart::GetPlayedRates()
{
	std::vector<float> o;
	FOREACHM(int, ScoresAtRate, ScoresByRate, i)
	o.emplace_back(KeyToRate(i->first));
	return o;
}

std::vector<int>
ScoresForChart::GetPlayedRateKeys()
{
	std::vector<int> o;
	FOREACHM(int, ScoresAtRate, ScoresByRate, i)
	o.emplace_back(i->first);
	return o;
}

std::vector<std::string>
ScoresForChart::GetPlayedRateDisplayStrings()
{
	std::vector<float> rates = GetPlayedRates();
	std::vector<std::string> o;
	for (size_t i = 0; i < rates.size(); ++i)
		o.emplace_back(RateKeyToDisplayString(rates[i]));
	return o;
}

std::string ScoresForChart::RateKeyToDisplayString(float rate)
{
	std::string rs = ssprintf("%.2f", rate);
	int j = 1;
	if (rs.find_last_not_of('0') == rs.find('.'))
		j = 2;
	rs.erase(rs.find_last_not_of('0') + j, rs.npos);
	rs.append("x");
	return rs;
}

// seems like this could be handled more elegantly by splitting operations up
// -mina
void
ScoresForChart::SetTopScores()
{
	std::vector<HighScore*> eligiblescores;
	FOREACHM(int, ScoresAtRate, ScoresByRate, i)
	{
		auto& hs = i->second.noccPBptr;
		if (hs && hs->GetSSRCalcVersion() == GetCalcVersion() &&
			hs->GetEtternaValid() && hs->GetChordCohesion() == 0 &&
			hs->GetGrade() != Grade_Failed)
			eligiblescores.emplace_back(hs);
	}

	// if there aren't 2 noccpbs in top scores we might as well use old cc scores -mina
	if (eligiblescores.size() < 2) {
		FOREACHM(int, ScoresAtRate, ScoresByRate, i)
		{
			auto& hs = i->second.PBptr;
			if (hs && hs->GetSSRCalcVersion() == GetCalcVersion() &&
				hs->GetEtternaValid() && hs->GetChordCohesion() != 0 &&
				hs->GetGrade() != Grade_Failed)
				eligiblescores.emplace_back(hs);
		}
	}
	
	if (eligiblescores.empty())
		return;

	if (eligiblescores.size() == 1) {
		eligiblescores[0]->SetTopScore(1);
		return;
	}

	else {
		auto ssrcomp = [](HighScore* a, HighScore* b) {
			return (a->GetSkillsetSSR(Skill_Overall) >
					b->GetSkillsetSSR(Skill_Overall));
		};
		std::sort(eligiblescores.begin(), eligiblescores.end(), ssrcomp);

		for (auto hs : eligiblescores)
			hs->SetTopScore(0);

		eligiblescores[0]->SetTopScore(1);
		eligiblescores[1]->SetTopScore(2);
	}
}

std::vector<HighScore*>
ScoresForChart::GetAllPBPtrs()
{
	std::vector<HighScore*> o;
	FOREACHM(int, ScoresAtRate, ScoresByRate, i)
	o.emplace_back(i->second.PBptr);
	return o;
}

std::vector<std::vector<HighScore*>>
ScoreManager::GetAllPBPtrs(const std::string& profileID)
{
	std::vector<std::vector<HighScore*>> vec;
	FOREACHUM(string, ScoresForChart, pscores[profileID], i)
	{
		if (!SONGMAN->IsChartLoaded(i->first))
			continue;
		vec.emplace_back(i->second.GetAllPBPtrs());
	}
	return vec;
}

HighScore*
ScoreManager::GetChartPBAt(const std::string& ck,
						   float& rate,
						   const std::string& profileID)
{
	if (KeyHasScores(ck, profileID))
		return pscores[profileID].at(ck).GetPBAt(rate);
	return NULL;
}

HighScore*
ScoreManager::GetChartPBUpTo(const std::string& ck,
							 float& rate,
							 const std::string& profileID)
{
	if (KeyHasScores(ck, profileID))
		return pscores[profileID].at(ck).GetPBUpTo(rate);
	return NULL;
}

void
ScoreManager::SetAllTopScores(const std::string& profileID)
{
	FOREACHUM(string, ScoresForChart, pscores[profileID], i)
	{
		if (!SONGMAN->IsChartLoaded(i->first))
			continue;
		i->second.SetTopScores();
	}
}

bool
ScoresAtRate::HandleNoCCPB(HighScore& hs)
{
	// lurker says:
	// don't even TRY to fuck with nocc pb unless the score is nocc
	if (hs.GetChordCohesion() == 0) {
		// Set any nocc pb
		if (noccPBptr == nullptr) {
			noccPBptr = &hs;
			return true;
		} else {
			// update nocc pb if a better score is found
			if (noccPBptr->GetSSRNormPercent() < hs.GetSSRNormPercent()) {
				noccPBptr = &hs;
				return true;
			}
		}
	}
	return false;
}

static const float ld_update = 0.02f;
void
ScoreManager::RecalculateSSRs(LoadingWindow* ld, const std::string& profileID)
{
	RageTimer ld_timer;
	std::vector<HighScore*>& scores = SCOREMAN->scorestorecalc;

	if (ld != nullptr) {
		ld->SetProgress(0);
		ld_timer.Touch();
		ld->SetIndeterminate(false);
		ld->SetTotalWork(scores.size());
		ld->SetText("\nUpdating Ratings");
	}
	int onePercent = std::max(static_cast<int>(scores.size() / 100 * 5), 1);
	int scoreindex = 0;
	mutex stepsMutex;
	std::vector<std::string> currentSteps;
	class StepsLock
	{
	  public:
		mutex& stepsMutex;
		std::vector<std::string>& currentSteps;
		string& ck;
		StepsLock(std::vector<std::string>& vec, mutex& mut, string& k)
		  : currentSteps(vec)
		  , stepsMutex(mut)
		  , ck(k)
		{
			bool active = true;
			{
				lock_guard<mutex> lk(stepsMutex);
				active = find(currentSteps.begin(), currentSteps.end(), ck) !=
						 currentSteps.end();
				if (!active)
					currentSteps.emplace_back(ck);
			}
			while (active) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				{
					lock_guard<mutex> lk(stepsMutex);
					active =
					  find(currentSteps.begin(), currentSteps.end(), ck) !=
					  currentSteps.end();
					if (!active)
						currentSteps.emplace_back(ck);
				}
			}
		}
		~StepsLock()
		{
			lock_guard<mutex> lk(stepsMutex);
			currentSteps.erase(
			  find(currentSteps.begin(), currentSteps.end(), ck));
		}
	};
	std::function<void(std::pair<vectorIt<HighScore*>, vectorIt<HighScore*>>,
				  ThreadData*)>
	  callback =
		[&stepsMutex, &currentSteps](
		  std::pair<vectorIt<HighScore*>, vectorIt<HighScore*>> workload,
		  ThreadData* data) {
			auto pair =
			  static_cast<std::pair<int, LoadingWindow*>*>(data->data);
			auto onePercent = pair->first;
			auto ld = pair->second;
			int scoreIndex = 0;
			int lastUpdate = 0;
			for (auto it = workload.first; it != workload.second; it++) {
				auto hs = *it;
				if (ld && scoreIndex % onePercent == 0) {
					data->_progress += scoreIndex - lastUpdate;
					lastUpdate = scoreIndex;
					data->setUpdated(true);
				}
				++scoreIndex;
				if (hs->GetSSRCalcVersion() == GetCalcVersion())
					continue;
				std::string ck = hs->GetChartKey();
				StepsLock lk(currentSteps, stepsMutex, ck);
				Steps* steps = SONGMAN->GetStepsByChartkey(ck);

				if (!steps)
					continue;

				if (!steps->IsRecalcValid()) {
					hs->ResetSkillsets();
					continue;
				}

				float ssrpercent = hs->GetSSRNormPercent();
				float musicrate = hs->GetMusicRate();

				if (ssrpercent <= 0.f || hs->GetGrade() == Grade_Failed) {
					hs->ResetSkillsets();
					continue;
				}

				TimingData* td = steps->GetTimingData();
				NoteData nd;
				steps->GetNoteData(nd);

				nd.LogNonEmptyRows();
				auto& nerv = nd.GetNonEmptyRowVector();
				auto& etaner = td->BuildAndGetEtaner(nerv);
				auto& serializednd = nd.SerializeNoteData(etaner);

				auto dakine = MinaSDCalc(serializednd,
										 steps->GetNoteData().GetNumTracks(),
										 musicrate,
										 ssrpercent,
										 1.f,
										 td->HasWarps());
				FOREACH_ENUM(Skillset, ss)
				hs->SetSkillsetSSR(ss, dakine[ss]);
				hs->SetSSRCalcVersion(GetCalcVersion());

				td->UnsetEtaner();
				nd.UnsetNerv();
				nd.UnsetSerializedNoteData();
				steps->Compress();

				/* Some scores were being incorrectly marked as ccon despite
				chord cohesion being disabled. Re-determine chord cohesion
				status from notecount, this should be robust as every score up
				to this point should be a fully completed pass. This will also
				allow us to mark files with 0 chords as being nocc (since it
				doesn't apply to them). -mina */
				int totalstepsnotes =
				  steps->GetRadarValues()[RadarCategory_Notes];
				int totalscorenotes = 0;
				totalscorenotes += hs->GetTapNoteScore(TNS_W1);
				totalscorenotes += hs->GetTapNoteScore(TNS_W2);
				totalscorenotes += hs->GetTapNoteScore(TNS_W3);
				totalscorenotes += hs->GetTapNoteScore(TNS_W4);
				totalscorenotes += hs->GetTapNoteScore(TNS_W5);
				totalscorenotes += hs->GetTapNoteScore(TNS_Miss);

				if (totalstepsnotes - totalscorenotes == 0)
					hs->SetChordCohesion(1); // the set function isn't inverted
											 // but the get function is, this
											 // sets bnochordcohesion to 1
				else
					hs->SetChordCohesion(0);
			}
		};
	auto onUpdate = [ld](int progress) {
		if (ld)
			ld->SetProgress(progress);
	};
	parallelExecution<HighScore*>(
	  scores,
	  onUpdate,
	  callback,
	  (void*)new std::pair<int, LoadingWindow*>(onePercent, ld));
	return;
}

// should deal with this misnomer - mina
void
ScoreManager::EnableAllScores()
{
	for (size_t i = 0; i < AllScores.size(); ++i)
		AllScores[i]->SetEtternaValid(true);

	return;
}

void
ScoreManager::CalcPlayerRating(float& prating,
							   float* pskillsets,
							   const std::string& profileID)
{
	SetAllTopScores(profileID);

	std::vector<float> skillz;
	FOREACH_ENUM(Skillset, ss)
	{
		// actually skip overall, and jack stamina for now
		if (ss == Skill_Overall)
			continue;

		SortTopSSRPtrs(ss, profileID);
		pskillsets[ss] = AggregateSSRs(ss, 0.f, 10.24f, 1) * 1.04f;
		CLAMP(pskillsets[ss], 0.f, 100.f);
		skillz.push_back(pskillsets[ss]);
	}

	std::sort(skillz.begin(), skillz.end());

	float skillsetsum = 0.f;
	for (size_t i = 1; i < skillz.size(); ++i) // drop the lowest skillset
		skillsetsum += skillz[i];

	prating = skillsetsum / 6.f;
}

// perhaps we will need a generalized version again someday, but not today
float
ScoreManager::AggregateSSRs(Skillset ss,
							float rating,
							float res,
							int iter) const
{
	double sum;
	do {
		rating += res;
		sum = 0.0;
		for (int i = 0; i < static_cast<int>(TopSSRs.size()); i++) {
			if (TopSSRs[i]->GetSSRCalcVersion() == GetCalcVersion() &&
				TopSSRs[i]->GetEtternaValid() &&
				TopSSRs[i]->GetChordCohesion() == 0 &&
				TopSSRs[i]->GetTopScore() != 0)
				sum += std::max(
				  0.0,
				  2.f / erfc(0.1 * (TopSSRs[i]->GetSkillsetSSR(ss) - rating)) -
					2);
		}
	} while (pow(2, rating * 0.1) < sum);
	if (iter == 11)
		return rating;
	return AggregateSSRs(ss, rating - res, res / 2.f, iter + 1);
}

void
ScoreManager::SortTopSSRPtrs(Skillset ss, const std::string& profileID)
{
	TopSSRs.clear();
	FOREACHUM(string, ScoresForChart, pscores[profileID], i)
	{
		if (!SONGMAN->IsChartLoaded(i->first))
			continue;
		std::vector<HighScore*> pbs = i->second.GetAllPBPtrs();
		FOREACH(HighScore*, pbs, hs) { TopSSRs.emplace_back(*hs); }
	}

	auto ssrcomp = [&ss](HighScore* a, HighScore* b) {
		return (a->GetSkillsetSSR(ss) > b->GetSkillsetSSR(ss));
	};
	std::sort(TopSSRs.begin(), TopSSRs.end(), ssrcomp);
}

HighScore*
ScoreManager::GetTopSSRHighScore(unsigned int rank, int ss)
{
	if (ss >= 0 && ss < NUM_Skillset && rank < TopSSRs.size())
		return TopSSRs[rank];

	return NULL;
}

void
ScoreManager::ImportScore(const HighScore& hs_, const std::string& profileID)
{
	HighScore hs = hs_;

	// don't import duplicated scores
	// this may have strange ramifications - mina
	// actually i'll just disable this for the time being and give myself time
	// to test it later
	// if(!ScoresByKey.count(hs.GetScoreKey()))

	RegisterScoreInProfile(pscores[profileID][hs.GetChartKey()].AddScore(hs),
						   profileID);
}

void
ScoreManager::RegisterScoreInProfile(HighScore* hs_, const std::string& profileID)
{
	AllProfileScores[profileID].emplace_back(hs_);
}

// Write scores to xml
XNode*
ScoresAtRate::CreateNode(const int& rate) const
{
	XNode* o = new XNode("ScoresAt");
	int saved = 0;

	// prune out sufficiently low scores
	FOREACHUM_CONST(string, HighScore, scores, i)
	{
		if (i->second.GetWifeScore() > SCOREMAN->minpercent) {
			o->AppendChild(i->second.CreateEttNode());
			saved++;
		}
	}

	if (o->ChildrenEmpty())
		return o;

	std::string rs = ssprintf("%.3f", static_cast<float>(rate) / 10000.f);
	// should be safe as this is only called if there is at least 1 score (which
	// would be the pb)
	o->AppendAttr("PBKey", PBptr->GetScoreKey());
	if (noccPBptr != nullptr) {
		if (PBptr->GetScoreKey() != noccPBptr->GetScoreKey())
			o->AppendAttr("noccPBKey", noccPBptr->GetScoreKey()); // don't write
																  // unless it's
																  // different
																  // from the
																  // pbkey -mina
	}

	o->AppendAttr("BestGrade", GradeToString(bestGrade));
	o->AppendAttr("Rate", rs);

	return o;
}

XNode*
ScoresForChart::CreateNode(const std::string& ck) const
{
	Chart loot = ch;
	loot.FromKey(ck); // needs to be here (or somewhere along the line, maybe
					  // not exactly here) -mina
	XNode* o = loot.CreateNode(false);

	FOREACHM_CONST(int, ScoresAtRate, ScoresByRate, i)
	{
		auto node = i->second.CreateNode(i->first);
		if (!node->ChildrenEmpty())
			o->AppendChild(node);
		else
			delete node;
	}
	return o;
}

XNode*
ScoreManager::CreateNode(const std::string& profileID) const
{
	XNode* o = new XNode("PlayerScores");
	FOREACHUM_CONST(string, ScoresForChart, pscores.find(profileID)->second, ch)
	{
		auto node = ch->second.CreateNode(ch->first);
		if (!node->ChildrenEmpty())
			o->AppendChild(node);
		else
			delete node;
	}

	return o;
}

// Read scores from xml
void
ScoresAtRate::LoadFromNode(const XNode* node,
						   const std::string& ck,
						   const float& rate,
						   const std::string& profileID)
{
	std::string sk;
	FOREACH_CONST_Child(node, p)
	{
		p->GetAttrValue("Key", sk);
		scores[sk].LoadFromEttNode(p);

		// Set any pb
		if (PBptr == nullptr)
			PBptr = &scores.find(sk)->second;
		else {
			// update pb if a better score is found
			if (PBptr->GetSSRNormPercent() < scores[sk].GetSSRNormPercent())
				PBptr = &scores.find(sk)->second;
		}

		HandleNoCCPB(scores[sk]);

		// Fill in stuff for the highscores
		scores[sk].SetChartKey(ck);
		scores[sk].SetScoreKey(sk);
		scores[sk].SetMusicRate(rate);

		bestGrade = std::min(scores[sk].GetWifeGrade(), bestGrade);

		// Very awkward, need to figure this out better so there isn't
		// unnecessary redundancy between loading and adding
		SCOREMAN->RegisterScore(&scores.find(sk)->second);
		SCOREMAN->AddToKeyedIndex(&scores.find(sk)->second);
		SCOREMAN->RegisterScoreInProfile(&scores.find(sk)->second, profileID);
		if (scores[sk].GetSSRCalcVersion() != GetCalcVersion() && SONGMAN->IsChartLoaded(ck))
			SCOREMAN->scorestorecalc.emplace_back(&scores[sk]);
	}
}

void
ScoresForChart::LoadFromNode(const XNode* node,
							 const std::string& ck,
							 const std::string& profileID)
{
	std::string rs = "";
	int rate;

	if (node->GetName() == "Chart")
		ch.LoadFromNode(node);

	if (node->GetName() == "ChartScores") {
		node->GetAttrValue("Key", rs);
		ch.key = rs;
		node->GetAttrValue("Song", ch.lastsong);
		node->GetAttrValue("Pack", ch.lastpack);
	}

	FOREACH_CONST_Child(node, p)
	{
		ASSERT(p->GetName() == "ScoresAt");
		p->GetAttrValue("Rate", rs);
		rate = 10 * StringToInt(rs.substr(0, 1) + rs.substr(2, 4));
		ScoresByRate[rate].LoadFromNode(p, ck, KeyToRate(rate), profileID);
		bestGrade = std::min(ScoresByRate[rate].bestGrade, bestGrade);
	}
}

void
ScoreManager::LoadFromNode(const XNode* node, const std::string& profileID)
{
	FOREACH_CONST_Child(node, p)
	{
		// ASSERT(p->GetName() == "Chart");
		std::string tmp;
		p->GetAttrValue("Key", tmp);
		const std::string ck = tmp;
		pscores[profileID][ck].LoadFromNode(p, ck, profileID);
	}
}

ScoresAtRate*
ScoresForChart::GetScoresAtRate(const int& rate)
{
	auto it = ScoresByRate.find(rate);
	if (it != ScoresByRate.end())
		return &it->second;
	return NULL;
}

ScoresForChart*
ScoreManager::GetScoresForChart(const std::string& ck, const std::string& profileID)
{
	auto it = (pscores[profileID]).find(ck);
	if (it != (pscores[profileID]).end())
		return &it->second;
	return NULL;
}

#include "Etterna/Models/Lua/LuaBinding.h"

class LunaScoresAtRate : public Luna<ScoresAtRate>
{
  public:
	static int GetScores(T* p, lua_State* L)
	{
		lua_newtable(L);
		std::vector<std::string> keys = p->GetSortedKeys();
		for (size_t i = 0; i < keys.size(); ++i) {
			HighScore& wot = p->scores[keys[i]];
			wot.PushSelf(L);
			lua_rawseti(L, -2, i + 1);
		}

		return 1;
	}

	LunaScoresAtRate() { ADD_METHOD(GetScores); }
};

LUA_REGISTER_CLASS(ScoresAtRate)

class LunaScoresForChart : public Luna<ScoresForChart>
{
  public:
	LunaScoresForChart() = default;
};

LUA_REGISTER_CLASS(ScoresForChart)

class LunaScoreManager : public Luna<ScoreManager>
{
  public:
	static int GetScoresByKey(T* p, lua_State* L)
	{
		const std::string& ck = SArg(1);
		ScoresForChart* scores = p->GetScoresForChart(ck);

		if (scores != nullptr) {
			lua_newtable(L);
			std::vector<int> ratekeys = scores->GetPlayedRateKeys();
			std::vector<std::string> ratedisplay = scores->GetPlayedRateDisplayStrings();
			for (size_t i = 0; i < ratekeys.size(); ++i) {
				LuaHelpers::Push(L, ratedisplay[i]);
				scores->GetScoresAtRate(ratekeys[i])->PushSelf(L);
				lua_rawset(L, -3);
			}

			return 1;
		}

		lua_pushnil(L);
		return 1;
	}

	static int SortSSRs(T* p, lua_State* L)
	{
		p->SortTopSSRPtrs(Enum::Check<Skillset>(L, 1));
		return 1;
	}

	static int ValidateAllScores(T* p, lua_State* L)
	{
		p->EnableAllScores();
		return 0;
	}

	static int GetTopSSRHighScore(T* p, lua_State* L)
	{
		HighScore* ths =
		  p->GetTopSSRHighScore(IArg(1) - 1, Enum::Check<Skillset>(L, 2));
		if (ths != nullptr)
			ths->PushSelf(L);
		else
			lua_pushnil(L);
		return 1;
	}

	static int GetMostRecentScore(T* p, lua_State* L)
	{
		// this _should_ always be viable if only called from eval
		HighScore* last = p->GetMostRecentScore();
		last->PushSelf(L);
		return 1;
	}
	DEFINE_METHOD(GetTempReplayScore, tempscoreforonlinereplayviewing);
	LunaScoreManager()
	{
		ADD_METHOD(GetScoresByKey);
		ADD_METHOD(SortSSRs);
		ADD_METHOD(ValidateAllScores);
		ADD_METHOD(GetTopSSRHighScore);
		ADD_METHOD(GetMostRecentScore);
		ADD_METHOD(GetTempReplayScore);
	}
};

LUA_REGISTER_CLASS(ScoreManager)
