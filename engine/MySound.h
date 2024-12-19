#pragma once
#include "miniaudio.h"
#include <string>
#include <vector>
#include "SceneSubj.h"


class MySound
{
public:
	ma_sound miniaudio_sound;
	unsigned int birthFrameN;
	unsigned int lengthFramesN=1000000;
	bool flat = false; //ignore distance
	float forSize = 0;
	float forForce = 0;
	float pitch_default = 1;
	float volume_default = 1;
	char source256[256] = "";

	static ma_engine miniaidio_engine;
	static std::vector<MySound*> soundSamples;
	static std::vector<MySound*> soundsPlaying;

	static float soundClose;
	static float soundFar;
	static float soundRange;
	static float closestCloseness;
	static 	uint32_t closestFrameN;

	static int soundNuncoupling;
	static int soundNcoupling;
	static int soundNrailbit;
	static int soundNrailswitch;
	static int soundNrailin;
	static int soundNrailout;

	static int soundNpop01;
	static int soundNfart01;
	static int soundNwhat01;
	static int soundNhit01;
	static int soundNclick01;

	static int ambientStartN;
	static int ambientTotalOpts;
	static MySound* pAmbientSound;

public:
	static int init(ProgressBar* pPBar = NULL);
	static int initAmbient(std::vector<MySound*>* pSoundsSet, ProgressBar* pPBar, std::string countryCode="us");
	static int cleanUp();
	static int loadSoundFlat(std::string filePath, std::vector<MySound*>* pSoundsSet,ProgressBar* pPBar = NULL,
		float pitch_default = 1, float volume_default = 1);
	static int loadSound(std::string filePath, std::vector<MySound*>* pSoundsSet, ProgressBar* pPBar = NULL, 
		float forSize = 0, float forForce = 0, float pitch_default = 1, float volume_default = 1);
	static int playSound(int sN,SceneSubj* pSS=NULL, bool frequent=false, float size=1,float force=1);
	static int playSound00(MySound* pSound, float volume, float pitch, float pan);
	static int checkSound();
};
