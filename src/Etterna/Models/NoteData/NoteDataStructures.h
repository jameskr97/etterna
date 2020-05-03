// Only NoteData structures independent of the rest of Stepmania live here

#ifndef __NDSTRUCTS__
#define __NDSTRUCTS__

struct NoteInfo
{
	unsigned int notes;
	float rowTime;
};

struct NoteInfo2
{
	int notes;
	int rowTime;
};

struct DifficultyRating
{
	float overall;
	float stream;
	float jumpstream;
	float handstream;
	float stamina;
	float jack;
	float chordjack;
	float technical;
};

enum Skillset
{
	Skill_Overall,
	Skill_Stream,
	Skill_Jumpstream,
	Skill_Handstream,
	Skill_Stamina,
	Skill_JackSpeed,
	Skill_Chordjack,
	Skill_Technical,
	NUM_Skillset,
	Skillset_Invalid,
};

// we do actually want to register these with lua i guess
enum CalcPatternMod
{
	OHJump, // pattern mod (values between 0-1)
	Anchor, // pattern mod (values between 0.9 - ~ 1.1)
	Roll,   // pattern mod (values between 0-1)
	HS,		// pattern mod (values between 0-1)
	Jump,   // pattern mod (values between 0-1)
	CJ,		// pattern mod (values between 0-1)
	StreamMod,
	OHTrill,
	Chaos,
	FlamJam,
	NUM_CalcPatternMod,
	CalcPatternMod_Invalid,
};
enum CalcDiffValue
{
	BaseNPS, // unadjusted base nps difficulty
	BaseMS,  // unadjusted base ms difficulty
	BaseMSD, // unadjusted weighted values
	MSD,	 // pattern and stam adjusted difficulty values
	NUM_CalcDiffValue,
	CalcDiffValue_Invalid,
};
enum CalcDebugMisc
{
	PtLoss, // expected points loss (not really a diff thing but w.e)
	StamMod,// stam adjust (values between 1- ~1.15)
	NUM_CalcDebugMisc,
	CalcDebugMisc_Invalid,
};

#endif
