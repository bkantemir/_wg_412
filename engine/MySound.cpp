#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>

#include "platform.h"
#include "MySound.h"
#include "TheApp.h"

ma_engine MySound::miniaidio_engine;
std::vector<MySound*> MySound::soundSamples;
std::vector<MySound*> MySound::soundsPlaying;

float MySound::soundClose;
float MySound::soundFar;
float MySound::soundRange;
float MySound::closestCloseness;
uint32_t MySound::closestFrameN;

int MySound::soundNpop01 = -1;
int MySound::soundNfart01 = -1;
int MySound::soundNwhat01 = -1;
int MySound::soundNhit01 = -1;
int MySound::soundNclick01 = -1;

int MySound::soundNcoupling = -1;
int MySound::soundNuncoupling = -1;
int MySound::soundNrailbit = -1;
int MySound::soundNrailswitch = -1;
int MySound::soundNrailin = -1;
int MySound::soundNrailout = -1;

int MySound::ambientStartN;
int MySound::ambientTotalOpts;
MySound* MySound::pAmbientSound;

extern std::string filesRoot;
extern TheApp theApp;


int MySound::init(ProgressBar* pPBar) {
    if (ma_engine_init(NULL, &miniaidio_engine) != MA_SUCCESS) {
        mylog("Failed to initialize audio engine.\n");
        return -1;
    }
    ma_engine_set_volume(&miniaidio_engine, 0.4);

    soundNpop01 = loadSound("/dt/sound/pop02.mp3", NULL, pPBar, 0, 0, 0.5, 1);
    soundNfart01 = loadSound("/dt/sound/fart01.mp3", NULL, pPBar, 0, 0, 1, 1);
    soundNwhat01 = loadSound("/dt/sound/what01.mp3", NULL, pPBar, 0, 0, 0.9, 0.7);
    soundNhit01 = loadSound("/dt/sound/hit01.mp3", NULL, pPBar, 0, 0, 1, 1);
    soundNclick01 = loadSoundFlat("/dt/sound/click01.mp3", NULL, pPBar, 1, 0.3);

    soundNuncoupling = loadSound("/dt/md/rr/sound/uncoupling02.mp3", NULL, pPBar, 0, 0, 0.9, 3);
    soundNcoupling = loadSound("/dt/md/rr/sound/coupling01.mp3", NULL, pPBar, 0, 0, 1.5, 1);
    soundNrailbit = loadSound("/dt/md/rr/sound/railbit01.mp3", NULL, pPBar, 0, 0, 0.8, 1.9);// 1.2);
    soundNrailswitch = loadSound("/dt/md/rr/sound/railswitch01.mp3", NULL, pPBar, 0, 0, 1.2, 0.6);
    soundNrailin = loadSound("/dt/md/rr/sound/railin01.mp3", NULL, pPBar, 0, 0, 1.3, 1);// 0.4, 1);
    soundNrailout = loadSound("/dt/md/rr/sound/railout01.mp3", NULL, pPBar, 0, 0, 1.3, 1);//0.7,1);

    initAmbient(NULL, pPBar, "us");
    return 1;
}

int MySound::initAmbient(std::vector<MySound*>* pSoundsSet, ProgressBar* pPBar,std::string countryCode) {
    ambientStartN = soundSamples.size();

    loadSoundFlat("/dt/md/rr/sound/ambient/rr01.mp3", NULL, pPBar, 1, 1.2);
    loadSoundFlat("/dt/md/rr/sound/ambient/rr03.mp3", NULL, pPBar, 1, 0.24);
    loadSoundFlat("/dt/md/rr/sound/ambient/rr01.mp3", NULL, pPBar, 1, 1.2);
    loadSoundFlat("/dt/md/rr/sound/ambient/rr03.mp3", NULL, pPBar, 1, 0.24);
    //us specific
    loadSoundFlat("/dt/md/rr/us/sound/v01f.mp3", NULL, pPBar, 0.95, 0.7);
    loadSoundFlat("/dt/md/rr/us/sound/v03m-e.mp3", NULL, pPBar, 0.85, 0.16);
    loadSoundFlat("/dt/md/rr/us/sound/v04m-e.mp3", NULL, pPBar, 0.85, 0.12);

    ambientTotalOpts= soundSamples.size() - ambientStartN;
    int ambientN = getRandom(0, ambientTotalOpts-1) + ambientStartN;
    pAmbientSound = soundSamples.at(ambientN);
    //playSound(ambientN);
    playSound00(pAmbientSound, pAmbientSound->volume_default, pAmbientSound->pitch_default, 0);
    return 1;
}
int MySound::cleanUp() {
    for (int i = soundSamples.size() - 1; i >= 0; i--) {
        MySound* pMS = soundSamples.at(i);
        if (pMS == NULL)
            continue;
        ma_sound_uninit(&pMS->miniaudio_sound);
        delete pMS;
    }
    soundSamples.clear();

    for (int i = soundsPlaying.size() - 1; i >= 0; i--) {
        MySound* pMS = soundsPlaying.at(i);
        if (pMS == NULL)
            continue;
        ma_sound_uninit(&pMS->miniaudio_sound);
        delete pMS;
    }
    soundsPlaying.clear();

    ma_engine_uninit(&miniaidio_engine);
    return 1;
}


int MySound::loadSound(std::string filePath, std::vector<MySound*>* pSoundsSet, ProgressBar* pPBar, float forSize, float forForce,
    float pitch_default, float volume_default) {

    if (pSoundsSet == NULL)
        pSoundsSet = &soundSamples;

    std::string fullPath = filesRoot + filePath;
    MySound* pMS = new MySound();
    strcpy_s(pMS->source256, 256, filePath.c_str());
    pSoundsSet->push_back(pMS);
    if (ma_sound_init_from_file(&miniaidio_engine, fullPath.c_str(), 0, NULL, NULL, &pMS->miniaudio_sound) != MA_SUCCESS) {
        mylog("ERROR in MySound::loadSound: Can't load %s.\n", fullPath.c_str());
        return -1;
    }
    //mylog("sizeof(MySound)=%d sizeof(*pMS)=%d sizeof(pMS->miniaudio_sound)=%d\n", (int)sizeof(MySound), (int)sizeof(*pMS), (int)sizeof(pMS->miniaudio_sound));

    pMS->flat = false;
    pMS->forSize = forSize;
    pMS->forForce = forForce;
    pMS->pitch_default = pitch_default;
    pMS->volume_default = volume_default;

    pPBar->nextStep(pPBar);

    return pSoundsSet->size() - 1;
}
int MySound::loadSoundFlat(std::string filePath, std::vector<MySound*>* pSoundsSet, ProgressBar* pPBar,float pitch_default, float volume_default) {

    if (pSoundsSet == NULL)
        pSoundsSet = &soundSamples;

    std::string fullPath = filesRoot + filePath;
    MySound* pMS = new MySound();
    strcpy_s(pMS->source256, 256, filePath.c_str());
    pSoundsSet->push_back(pMS);
    if (ma_sound_init_from_file(&miniaidio_engine, fullPath.c_str(), 0, NULL, NULL, &pMS->miniaudio_sound) != MA_SUCCESS) {
        mylog("ERROR in MySound::loadSoundFlat: Can't load %s.\n", fullPath.c_str());
        return -1;
    }
    pMS->flat = true;
    pMS->pitch_default = pitch_default;
    pMS->volume_default = volume_default;
 
    pPBar->nextStep(pPBar);
    return pSoundsSet->size() - 1;
}

int MySound::playSound(int sN, SceneSubj* pSS, bool frequent, float size, float force) {

    MySound* pMS00 = soundSamples.at(sN);
    //create and deploy copy
    MySound* pMS = new MySound();
    memcpy(pMS, pMS00, sizeof(MySound));
    ma_sound_init_copy(&miniaidio_engine, &pMS00->miniaudio_sound, NULL, NULL, &pMS->miniaudio_sound);
    soundsPlaying.push_back(pMS);

    ma_sound* pSsound = &pMS->miniaudio_sound;

    float volume = pMS->volume_default;
    float pitch = pMS->pitch_default;
    float pan = 0;
    if (!pMS->flat && pSS!=NULL) {
        float dist = v3lengthFromTo(theApp.mainCamera.ownCoords.pos, pSS->absCoords.pos);
        if (dist < soundClose)
            dist = soundClose;
        float closeness = 1.0 - (dist - soundClose) / soundRange;
        if (closeness <= 0)
            return 0;
        if (closeness > 1)
            closeness = 1;
        if (frequent) {
            if (closestCloseness >= closeness)
                return 0;

            //mylog("%d dist=%.2f closeness0=%.2f closeness=%.2f \n", theApp.frameN, dist, closeness0, closeness);

            if (closestCloseness < closeness)
                closestCloseness = closeness;
        }
        volume = pMS->volume_default * closeness;
        if (pMS->forForce != 0) {
            volume = volume * (force / pMS->forForce);
        }
        float glCoords[4];
        mat4x4_mul_vec4plus(glCoords, theApp.mainCamera.mViewProjection, pSS->absCoords.pos, 1, true);
        pan = minimax(glCoords[0], -1, 1);
        if (pMS->forSize != 0)
            pitch = pitch * (pMS->forSize / size);
    }
    playSound00(pMS, volume, pitch, pan);
    return 1;
}
int MySound::playSound00(MySound* pMS, float volume, float pitch, float pan){
    ma_sound* pSsound = &pMS->miniaudio_sound;
    ma_sound_set_pitch(pSsound, pitch);
    ma_sound_set_volume(pSsound, volume);
    ma_sound_set_pan(pSsound, pan);// -stereo - 1 - left, +1 - right
    //use ma_sound_at_end(). Looping of a sound can be controlled with ma_sound_set_looping(). Use ma_sound_is_looping()
    pMS->birthFrameN = theApp.frameN;
    ma_sound_start(pSsound);
    return 1;
}
int MySound::checkSound() {
    //clean up array first
    for (int i = soundsPlaying.size() - 1; i >= 0; i--) {
        MySound* pMS = soundsPlaying.at(i);
        if (ma_sound_is_playing(&pMS->miniaudio_sound))
            continue; //still playing
        //if here - finished - delete
        ma_sound_uninit(&pMS->miniaudio_sound);
        delete pMS;
        soundsPlaying.erase(soundsPlaying.begin() + i);
    }
    if (soundsPlaying.size() > 4) {
        //too much - delete the oldest - the first one
        int totalN = soundsPlaying.size();
        for (int i = 0; i < totalN; i++) {
            MySound* pMS = soundsPlaying.at(i);
            if (ma_sound_is_looping(&pMS->miniaudio_sound))
                continue;
            //here - the oldest one
            ma_sound_stop(&pMS->miniaudio_sound);
            ma_sound_uninit(&pMS->miniaudio_sound);
            delete pMS;
            soundsPlaying.erase(soundsPlaying.begin() + i);
            break;
        }
    }
    if (closestFrameN != theApp.frameN) {
        closestFrameN = theApp.frameN;
        closestCloseness *= 0.98;

        //mylog("%d closestCloseness=%.2f\n", theApp.frameN, closestCloseness);

    }
    //ambient
    if (theApp.bEditMode) {
        //shut down ambient
        if (ma_sound_is_playing(&pAmbientSound->miniaudio_sound))
            ma_sound_stop(&pAmbientSound->miniaudio_sound);
    }
    else {//play ambient
        bool startNewAmbientSound = false;
        if (ma_sound_is_playing(&pAmbientSound->miniaudio_sound) == false){
            unsigned int d = theApp.frameN - pAmbientSound->birthFrameN;
            if (pAmbientSound->lengthFramesN > d)//update lifetime
                pAmbientSound->lengthFramesN = d;
            startNewAmbientSound = true;
        }
        else {//still playing
            unsigned int d = theApp.frameN - pAmbientSound->birthFrameN;
            if (d > pAmbientSound->lengthFramesN - 15)//half-second
                startNewAmbientSound = true;
        }

        if (startNewAmbientSound) {
            int ambientN = getRandom(0, ambientTotalOpts - 1) + ambientStartN;
            MySound* pSelected = soundSamples.at(ambientN);
            if (ambientTotalOpts == 1 || pAmbientSound != pSelected) {
                pAmbientSound = pSelected;
                playSound00(pAmbientSound, pAmbientSound->volume_default, pAmbientSound->pitch_default, 0);

                //mylog("%d ambient optN %d %s\n",theApp.frameN, ambientN- ambientStartN, pAmbientSound->source256);
            }
        }
    }
    return 1;
}