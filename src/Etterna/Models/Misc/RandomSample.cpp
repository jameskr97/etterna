﻿#include "Etterna/Globals/global.h"
#include "Core/Services/Locator.hpp"
#include "RageUtil/Sound/RageSound.h"
#include "RageUtil/Utils/RageUtil.h"
#include "RandomSample.h"

RandomSample::RandomSample()
{
	m_iIndexLastPlayed = -1;
}

RandomSample::~RandomSample()
{
	UnloadAll();
}

bool
RandomSample::Load(const RString& sFilePath, int iMaxToLoad)
{
	if (GetExtension(sFilePath) == "")
		return LoadSoundDir(sFilePath, iMaxToLoad);
	else
		return LoadSound(sFilePath);
}

void
RandomSample::UnloadAll()
{
	for (unsigned i = 0; i < m_pSamples.size(); i++)
		delete m_pSamples[i];
	m_pSamples.clear();
}

bool
RandomSample::LoadSoundDir(RString sDir, int iMaxToLoad)
{
	if (sDir == "")
		return true;

	// make sure there's a slash at the end of this path
	if (sDir.Right(1) != "/")
		sDir += "/";

	vector<RString> arraySoundFiles;
	GetDirListing(sDir + "*.mp3", arraySoundFiles);
	GetDirListing(sDir + "*.oga", arraySoundFiles);
	GetDirListing(sDir + "*.ogg", arraySoundFiles);
	GetDirListing(sDir + "*.wav", arraySoundFiles);

	std::shuffle(
	  arraySoundFiles.begin(), arraySoundFiles.end(), g_RandomNumberGenerator);
	arraySoundFiles.resize(min(arraySoundFiles.size(), (unsigned)iMaxToLoad));

	for (unsigned i = 0; i < arraySoundFiles.size(); i++)
		LoadSound(sDir + arraySoundFiles[i]);

	return true;
}

bool
RandomSample::LoadSound(const RString& sSoundFilePath)
{
	Locator::getLogger()->trace("RandomSample::LoadSound({})", sSoundFilePath.c_str());

	auto* pSS = new RageSound;
	if (!pSS->Load(sSoundFilePath)) {
		Locator::getLogger()->trace("Error loading \"{}\": {}",
				   sSoundFilePath.c_str(),
				   pSS->GetError().c_str());
		delete pSS;
		return false;
	}

	m_pSamples.push_back(pSS);

	return true;
}

int
RandomSample::GetNextToPlay()
{
	// play one of the samples
	if (m_pSamples.empty())
		return -1;

	int iIndexToPlay = 0;
	for (int i = 0; i < 5; i++) {
		iIndexToPlay = RandomInt(m_pSamples.size());
		if (iIndexToPlay != m_iIndexLastPlayed)
			break;
	}

	m_iIndexLastPlayed = iIndexToPlay;
	return iIndexToPlay;
}

void
RandomSample::PlayRandom()
{
	int iIndexToPlay = GetNextToPlay();
	if (iIndexToPlay == -1)
		return;
	m_pSamples[iIndexToPlay]->Play(true);
}

void
RandomSample::PlayCopyOfRandom()
{
	int iIndexToPlay = GetNextToPlay();
	if (iIndexToPlay == -1)
		return;
	m_pSamples[iIndexToPlay]->PlayCopy(true);
}

void
RandomSample::Stop()
{
	if (m_iIndexLastPlayed == -1) // nothing is currently playing
		return;

	m_pSamples[m_iIndexLastPlayed]->Stop();
}
