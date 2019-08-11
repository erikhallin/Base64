#ifndef SOUND_H
#define SOUND_H

#include "sound_list.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <AL/al.h>
#include <AL/alc.h>

#define _numof_sound_buffers 40 //number of sounds
#define _numof_sound_sources 20 //number of sounds that could be play at the same time


using namespace std;

class sound
{
    public:
        sound();                           //Open OpenAL
        ~sound();                          //Close OpenAL

        //Variables
        bool m_ready;

        //Functions
        int  get_error(void);
        bool load_WAVE_from_file(int sound_id, const char* file);//give filename and sound id number, returns if source/buffer was created
        bool load_WAVE_from_string(int sound_id, string data_file);
        bool playWAVE(int sound_id, float data[]);    //data contains: ListenerPos    0:1:2,  ListenerVel  3:4:5,
                                                      //               ListenerOri at-6:7:8,          up-9:10:11,
                                                      //               SourcePos 12:13:14,    SourceVel 15:16:17,
                                                      //               Pitch           18,    Gain            19,  Loop         20
        void playSimpleSound(int sound_id,float volume); //volume 0.0 - 1.0
        int  getAndLoadFreeSource(int buffer_id);
        void updateSound(int sound_id,float data[]);
        void stopSound(int sound_id);

    private:
        //Variables
        int         m_error;
        ALCdevice*  m_pALCdevice;
        ALCcontext* m_pALCcontext;
        ALuint      m_arrSource[_numof_sound_sources];
        ALuint      m_arrBuffer[_numof_sound_buffers];

        //Functions

};


#endif

/*  EXAMPLE

Pre-comp
    Set number of sounds (numOfSound)

Initializing

    sound Sound;

Loading
    Sound.loadWAVE(0,"test1.wav");

Playing
    float data[]={0,0,0, 0,0,0, 0,0,-1,
                  0,1,0, 0,0,0, 0,0,0,
                  1,  1,  0};
    Sound.playWAVE(0,data);
*/
