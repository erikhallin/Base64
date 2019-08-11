#include <stdio.h>
#include "sound.h"

sound::sound()
{
    //Open device
    m_pALCdevice = alcOpenDevice(NULL);
    if(!m_pALCdevice)
    {
        m_error=1;
        cout<<"ERROR no sound device\n";
        return;
    }
    //Create context
    m_pALCcontext = alcCreateContext(m_pALCdevice, NULL);
    alcMakeContextCurrent(m_pALCcontext);
    if(!m_pALCcontext)
    {
        m_error=2;
        cout<<"ERROR no sound context\n";
        return;
    }
    //Make buffers and sources
    alGenBuffers(_numof_sound_buffers, m_arrBuffer);
    alGenSources(_numof_sound_sources, m_arrSource);
    if(alGetError() != AL_NO_ERROR)
    {
        m_error=3;
        cout<<"ERROR could not generate sources or buffers\n";
        return;
    }

    m_error=0;//no errors

    //Set distance model
    alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);

    m_ready=true;
}

sound::~sound()
{
    //clean up
    alDeleteSources(_numof_sound_sources, m_arrSource);
    alDeleteBuffers(_numof_sound_buffers, m_arrBuffer);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_pALCcontext);
    alcCloseDevice(m_pALCdevice);
}

int sound::get_error(void)
{
    return m_error;
}

bool sound::load_WAVE_from_file(int sound_id, const char* filename)
{
    if(sound_id>=_numof_sound_buffers)//to many sounds
    {
        cout<<"ERROR not space for more sound files\n";
        m_error=3;
        return false;
    }

    FILE *fp = NULL;
    fp=fopen(filename,"rb");
    if (!fp)
    {
        fclose(fp);
        m_error=3;
        cout<<"ERROR could not open sound file\n";
        return false;
    }

    char type[4];
    DWORD size,chunkSize;
    short formatType,channels;
    DWORD sampleRate,avgBytesPerSec;
    short bytesPerSample,bitsPerSample;
    DWORD dataSize;

    int offset[13];


    fread(type,sizeof(char),4,fp);
    offset[0]=ftell(fp);
    if(type[0]!='R' || type[1]!='I' || type[2]!='F' || type[3]!='F')
    {
        fclose(fp);
        m_error=4;
        cout<<"ERROR sound file have no RIFF header\n";
        return false;
    }

    fread(&size, sizeof(DWORD),1,fp);
    cout<<"SIZE: "<<size<<endl;//TEMP
    offset[1]=ftell(fp);
    fread(type, sizeof(char),4,fp);
    offset[2]=ftell(fp);
    if (type[0]!='W' || type[1]!='A' || type[2]!='V' || type[3]!='E')
    {
        fclose(fp);
        m_error=5;
        cout<<"ERROR sound file have no WAVE header\n";
        return false;
    }
    fread(type,sizeof(char),4,fp);
    offset[3]=ftell(fp);
    if (type[0]!='f' || type[1]!='m' || type[2]!='t' || type[3]!=' ')
    {
        fclose(fp);
        m_error=6;
        cout<<"ERROR sound file have no fmt header\n";
        return false;
    }

    fread(&chunkSize,sizeof(DWORD),1,fp);
    offset[4]=ftell(fp);
    fread(&formatType,sizeof(short),1,fp);
    offset[5]=ftell(fp);
    fread(&channels,sizeof(short),1,fp);
    offset[6]=ftell(fp);
    fread(&sampleRate,sizeof(DWORD),1,fp);
    offset[7]=ftell(fp);
    fread(&avgBytesPerSec,sizeof(DWORD),1,fp);
    offset[8]=ftell(fp);
    fread(&bytesPerSample,sizeof(short),1,fp);
    offset[9]=ftell(fp);
    fread(&bitsPerSample,sizeof(short),1,fp);
    offset[10]=ftell(fp);

    //find data
    bool data_found=false;
    while( fread(type,sizeof(char),4,fp)!=EOF )
    {
        offset[11]=ftell(fp);
        if (type[0]=='d' && type[1]=='a' && type[2]=='t' && type[3]=='a')
        {
            data_found=true;
            break;
        }
        else
        {
            //go back 3 pos and look again
            fseek(fp,-3,SEEK_CUR);
        }
    }
    if(!data_found)
    {
        fclose(fp);
        m_error=7;
        cout<<"ERROR sound file have no DATA header\n";
        return false;
    }

    /*fread(type,sizeof(char),4,fp); OLD
    offset[11]=ftell(fp);
    if (type[0]!='d' || type[1]!='a' || type[2]!='t' || type[3]!='a')
    {
        fclose(fp);
        m_error=7;
        cout<<"ERROR sound file have no DATA header\n";
        return false;
    }*/

    fread(&dataSize,sizeof(DWORD),1,fp);
    offset[12]=ftell(fp);

    unsigned char* buf= new unsigned char[dataSize];
    fread(buf,sizeof(BYTE),dataSize,fp);

    //TEMP
    cout<<"chunkSize: "<<chunkSize<<endl;
    cout<<"formatType: "<<formatType<<endl;
    cout<<"channels: "<<channels<<endl;
    cout<<"sampleRate: "<<sampleRate<<endl;
    cout<<"avgBytesPerSec: "<<avgBytesPerSec<<endl;
    cout<<"bytesPerSample: "<<bytesPerSample<<endl;
    cout<<"bitsPerSample: "<<bitsPerSample<<endl;
    cout<<"dataSize: "<<dataSize<<endl;
    //TEMP

    //Sound Format
    ALenum format=AL_FALSE;
    if(bitsPerSample == 8)
    {
        if(channels == 1)
            format = AL_FORMAT_MONO8;
        else if(channels == 2)
            format = AL_FORMAT_STEREO8;
    }
    else if(bitsPerSample == 16)
    {
        if(channels == 1)
            format = AL_FORMAT_MONO16;
        else if(channels == 2)
            format = AL_FORMAT_STEREO16;
    }
    if(!format)
    {
        fclose(fp);
        m_error=8;
        cout<<"ERROR wrong bit per sample in sound file\n";
        return false;
    }

    //Sound Frequency
    ALuint frequency=sampleRate;

    alBufferData(m_arrBuffer[sound_id], format, buf, dataSize, frequency);
    delete[] buf;
    if(alGetError() != AL_NO_ERROR)
    {
        fclose(fp);
        m_error=9;
        cout<<"ERROR could not load AL buffer\n";
        return false;
    }

    fclose(fp);

    return true;
}

bool sound::load_WAVE_from_string(int sound_id, string data_file)
{
    if(sound_id>=_numof_sound_buffers)//to many sounds
    {
        cout<<"ERROR not space for more sound files\n";
        m_error=3;
        return false;
    }

    int size_of_file=(int)data_file.length();
    const unsigned char* fp=(const unsigned char*)data_file.c_str();
    int location=0;
    if(!fp)
    {
        m_error=3;
        cout<<"ERROR could not open sound file\n";
        return false;
    }

    char type[4];
    DWORD size,chunkSize;
    short formatType,channels;
    DWORD sampleRate,avgBytesPerSec;
    short bytesPerSample,bitsPerSample;
    DWORD dataSize;

    type[0]=(char)fp[location+0];
    type[1]=(char)fp[location+1];
    type[2]=(char)fp[location+2];
    type[3]=(char)fp[location+3];
    location+=4;

    if(type[0]!='R' || type[1]!='I' || type[2]!='F' || type[3]!='F')
    {
        m_error=4;
        cout<<"ERROR sound file have no RIFF header\n";
        cout<<type<<endl;
        return false;
    }

    //get size
    size=(DWORD) *((int*) &fp[location]);
    location+=4;
    //cout<<"SIZE: "<<size<<endl;//temp

    type[0]=(char)fp[location+0];
    type[1]=(char)fp[location+1];
    type[2]=(char)fp[location+2];
    type[3]=(char)fp[location+3];
    location+=4;

    if (type[0]!='W' || type[1]!='A' || type[2]!='V' || type[3]!='E')
    {
        m_error=5;
        cout<<"ERROR sound file have no WAVE header\n";
        return false;
    }
    type[0]=(char)fp[location+0];
    type[1]=(char)fp[location+1];
    type[2]=(char)fp[location+2];
    type[3]=(char)fp[location+3];
    location+=4;

    if (type[0]!='f' || type[1]!='m' || type[2]!='t' || type[3]!=' ')
    {
        m_error=6;
        cout<<"ERROR sound file have no fmt header\n";
        return false;
    }

    chunkSize=(DWORD) *((int*) &fp[location]);
    location+=sizeof(DWORD);
    cout<<"chunkSize: "<<chunkSize<<endl;
    formatType=(short) *((short*) &fp[location]);
    location+=sizeof(short);
    cout<<"formatType: "<<formatType<<endl;
    channels=(short) *((short*) &fp[location]);
    location+=sizeof(short);
    cout<<"channels: "<<channels<<endl;
    sampleRate=(DWORD) *((int*) &fp[location]);
    location+=sizeof(DWORD);
    cout<<"sampleRate: "<<sampleRate<<endl;
    avgBytesPerSec=(DWORD) *((int*) &fp[location]);
    location+=sizeof(int);
    cout<<"avgBytesPerSec: "<<avgBytesPerSec<<endl;
    bytesPerSample=(short) *((short*) &fp[location]);
    location+=sizeof(short);
    cout<<"bytesPerSample: "<<bytesPerSample<<endl;
    bitsPerSample=(short) *((short*) &fp[location]);
    location+=sizeof(short);
    cout<<"bitsPerSample: "<<bitsPerSample<<endl;

    //find data
    bool data_found=false;
    while( location<size_of_file-4 )
    {
        type[0]=(char)fp[location+0];
        type[1]=(char)fp[location+1];
        type[2]=(char)fp[location+2];
        type[3]=(char)fp[location+3];
        location+=4;

        if (type[0]=='d' && type[1]=='a' && type[2]=='t' && type[3]=='a')
        {
            data_found=true;
            break;
        }
        else
        {
            //go back 3 pos and look again
            location-=3;
        }
    }
    if(!data_found)
    {
        m_error=7;
        cout<<"ERROR sound file have no DATA header\n";
        return false;
    }

    dataSize=(DWORD) *((int*) &fp[location]);
    location+=sizeof(DWORD);

    //location is now at data position

    //Sound Format
    ALenum format=AL_FALSE;
    if(bitsPerSample == 8)
    {
        if(channels == 1)
            format = AL_FORMAT_MONO8;
        else if(channels == 2)
            format = AL_FORMAT_STEREO8;
    }
    else if(bitsPerSample == 16)
    {
        if(channels == 1)
            format = AL_FORMAT_MONO16;
        else if(channels == 2)
            format = AL_FORMAT_STEREO16;
    }
    if(!format)
    {
        m_error=8;
        cout<<"ERROR wrong bit per sample in sound file\n";
        cout<<bitsPerSample<<endl;
        return false;
    }

    //Sound Frequency
    ALuint frequency=sampleRate;

    alBufferData(m_arrBuffer[sound_id], format, &fp[location], dataSize, frequency);

    if(alGetError() != AL_NO_ERROR)
    {
        //fclose(fp);
        m_error=9;
        cout<<"ERROR could not load AL buffer\n";
        return false;
    }

    return true;
}

bool sound::playWAVE(int sound_id, float data[21])
{
    if(sound_id>=_numof_sound_buffers)//to many sounds
    {
        cout<<"ERROR outside sound buffer array\n";
        return false;
    }

    //Find free source
    int freeSource=-1;
    for (int i=0;i<10;i++)
    {
        int status;
        alGetSourcei(m_arrSource[i], AL_SOURCE_STATE, &status);
        if(status != AL_PLAYING)
        {
            freeSource=i;
            break;
        }
    }

    if (freeSource<0)
    {
        freeSource=0;
        alSourceStop(m_arrSource[freeSource]);
    }

    float ListenerPos[]={data[0],data[1],data[2]};
    float ListenerVel[]={data[3],data[4],data[5]};
    float ListenerOri[]={data[6],data[7],data[8],data[9],data[10],data[11]};
    float SourcePos[]={data[12],data[13],data[14]};
    float SourceVel[]={data[15],data[16],data[17]};

    alListenerfv(AL_POSITION,    ListenerPos);
    alListenerfv(AL_VELOCITY,    ListenerVel);
    alListenerfv(AL_ORIENTATION, ListenerOri);

    alSourcei (m_arrSource[freeSource], AL_BUFFER,   m_arrBuffer[sound_id]);
    alSourcef (m_arrSource[freeSource], AL_PITCH,    data[18] );
    alSourcef (m_arrSource[freeSource], AL_GAIN,     data[19] );
    alSourcefv(m_arrSource[freeSource], AL_POSITION, SourcePos);
    alSourcefv(m_arrSource[freeSource], AL_VELOCITY, SourceVel);
    alSourcei (m_arrSource[freeSource], AL_LOOPING,  (ALboolean)data[20] );
    alSourcef (m_arrSource[freeSource], AL_ROLLOFF_FACTOR,  1          );

    alSourcePlay(m_arrSource[freeSource]);


    if(alGetError() != AL_NO_ERROR)
    {
        m_error=10;
        cout<<"ERROR could not play sound\n";
        return false;
    }

    return true;
}

void sound::playSimpleSound(int sound_id, float gain)
{
    if(sound_id>=_numof_sound_buffers)//to many sounds
    {
        cout<<"ERROR outside sound buffer array\n";
        return;
    }

    //Find free source
    int freeSource=-1;
    for (int i=0;i<8;i++)
    {
        int status;
        alGetSourcei(m_arrSource[i], AL_SOURCE_STATE, &status);
        if(status != AL_PLAYING)
        {
            freeSource=i;
            break;
        }
    }

    if (freeSource<0)
    {
        freeSource=0;
        alSourceStop(m_arrSource[freeSource]);
    }

    float ListenerPos[]={0,0,0};
    float ListenerVel[]={0,0,0};
    float ListenerOri[]={0,0,-1,0,1,0};
    float SourcePos[]={0,0,0};
    float SourceVel[]={0,0,0};

    alListenerfv(AL_POSITION,    ListenerPos);
    alListenerfv(AL_VELOCITY,    ListenerVel);
    alListenerfv(AL_ORIENTATION, ListenerOri);

    alSourcei (m_arrSource[freeSource], AL_BUFFER,   m_arrBuffer[sound_id]);
    alSourcef (m_arrSource[freeSource], AL_PITCH,    1.0f     );
    alSourcef (m_arrSource[freeSource], AL_GAIN,     gain     );
    alSourcefv(m_arrSource[freeSource], AL_POSITION, SourcePos);
    alSourcefv(m_arrSource[freeSource], AL_VELOCITY, SourceVel);
    alSourcei (m_arrSource[freeSource], AL_LOOPING,  AL_FALSE );

    alSourcePlay(m_arrSource[freeSource]);
    if(alGetError() != AL_NO_ERROR)
    {
        m_error=10;
        cout<<"ERROR could not play sound\n";
        return;
    }
}

int sound::getAndLoadFreeSource(int buffer_id)
{
    int freeSource=-1;
    for (int i=10;i<20;i++) //10-19 long sounds
    {
        int status;
        alGetSourcei(m_arrSource[i], AL_SOURCE_STATE, &status);
        if(status != AL_PLAYING) {freeSource=i; break;}
    }
    if (freeSource!=-1)
    {
        alSourcei (m_arrSource[freeSource], AL_BUFFER,   m_arrBuffer[buffer_id-1]);
        alSourcePlay(m_arrSource[freeSource]);
    }

    return freeSource;
}

void sound::updateSound(int source_id, float data[])
{
    float ListenerPos[]={data[0],data[1],data[2]};
    float ListenerVel[]={data[3],data[4],data[5]};
    float ListenerOri[]={data[6],data[7],data[8],data[9],data[10],data[11]};
    float SourcePos[]={data[12],data[13],data[14]};
    float SourceVel[]={data[15],data[16],data[17]};

    alListenerfv(AL_POSITION,    ListenerPos);
    alListenerfv(AL_VELOCITY,    ListenerVel);
    alListenerfv(AL_ORIENTATION, ListenerOri);

    alSourcef (m_arrSource[source_id], AL_PITCH,    data[18] );
    alSourcef (m_arrSource[source_id], AL_GAIN,     data[19] );
    alSourcefv(m_arrSource[source_id], AL_POSITION, SourcePos);
    alSourcefv(m_arrSource[source_id], AL_VELOCITY, SourceVel);
    alSourcei (m_arrSource[source_id], AL_LOOPING,  (ALboolean)data[20] );
    alSourcef (m_arrSource[source_id], AL_ROLLOFF_FACTOR,  1          );

    int status;
    alGetSourcei(m_arrSource[source_id], AL_SOURCE_STATE, &status);
    if(status != AL_PLAYING)
    {
        alSourcePlay(m_arrSource[source_id]);
    }
}

void sound::stopSound(int sound_id)
{
    alSourceStop(m_arrSource[sound_id]);
}
