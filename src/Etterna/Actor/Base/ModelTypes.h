/* ModelTypes - Types defined in msLib.h.  C arrays converted to use std::vector
 */

#ifndef MODEL_TYPES_H
#define MODEL_TYPES_H

#include "RageUtil/Misc/RageTypes.h"

struct msTriangle
{
	uint16_t nVertexIndices[3];
};

struct msMesh
{
	std::string sName;
	char nMaterialIndex;

	vector<RageModelVertex> Vertices;

	// OPTIMIZATION: If all verts in a mesh are transformed by the same bone,
	// then send the transform to the graphics card for the whole mesh instead
	// of transforming each vertex on the CPU;
	char m_iBoneIndex; // -1 = no bone

	vector<msTriangle> Triangles;
};

class RageTexture;

// merge this into Sprite?
class AnimatedTexture
{
  public:
	AnimatedTexture();
	~AnimatedTexture();

	void LoadBlank();
	void Load(const std::string& sTexOrIniFile);
	void Unload();
	void Update(float fDelta);

	RageTexture* GetCurrentTexture();

	[[nodiscard]] int GetNumStates() const;
	void SetState(int iNewState);
	[[nodiscard]] float GetAnimationLengthSeconds() const;
	void SetSecondsIntoAnimation(float fSeconds);
	[[nodiscard]] float GetSecondsIntoAnimation() const;
	RageVector2 GetTextureTranslate();

	bool m_bSphereMapped;
	BlendMode m_BlendMode;

	[[nodiscard]] bool NeedsNormals() const { return m_bSphereMapped; }

  private:
	RageVector2 m_vTexOffset;
	RageVector2 m_vTexVelocity;

	int m_iCurState;
	float m_fSecsIntoFrame;
	struct AnimatedTextureState
	{
		AnimatedTextureState(RageTexture* pTexture_,
							 float fDelaySecs_,
							 RageVector2 vTranslate_)
		  : pTexture(pTexture_)
		  , fDelaySecs(fDelaySecs_)
		  , vTranslate(vTranslate_)
		{
		}

		RageTexture* pTexture;
		float fDelaySecs;
		RageVector2 vTranslate;
	};
	vector<AnimatedTextureState> vFrames;
};

struct msMaterial
{
	int nFlags;
	std::string sName;
	RageColor Ambient;
	RageColor Diffuse;
	RageColor Specular;
	RageColor Emissive;
	float fShininess;
	float fTransparency;

	AnimatedTexture diffuse;
	AnimatedTexture alpha;

	[[nodiscard]] bool NeedsNormals() const
	{
		return diffuse.NeedsNormals() || alpha.NeedsNormals();
	}
};

struct msPositionKey
{
	float fTime;
	RageVector3 Position;
};

struct msRotationKey
{
	float fTime;
	RageVector4 Rotation;
};

struct msBone
{
	int nFlags;
	std::string sName;
	std::string sParentName;
	RageVector3 Position;
	RageVector3 Rotation;

	vector<msPositionKey> PositionKeys;
	vector<msRotationKey> RotationKeys;
};

struct msAnimation
{
	[[nodiscard]] int FindBoneByName(const std::string& sName) const
	{
		for (unsigned i = 0; i < Bones.size(); i++)
			if (Bones[i].sName == sName)
				return i;
		return -1;
	}

	bool LoadMilkshapeAsciiBones(const std::string& sAniName,
								 std::string sPath);

	vector<msBone> Bones;
	int nTotalFrames;
};

struct myBone_t
{
	RageMatrix m_Relative;
	RageMatrix m_Absolute;
	RageMatrix m_Final;
};

#endif
