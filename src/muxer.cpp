#include "muxer.h"
#include <cstdlib>
#include <sndfile.h>
#include <portaudio.h>
#include <list>

#define SAMPLE_RATE (44100)

class Sound {
  public:
    Sound(const char* filename);
    ~Sound();
    
    float* ptr;
    unsigned long pos;
    unsigned long len;
};

struct Muxer {
  std::list<Sound> playing;
  PaStream* stream;
};

inline void dealWithPaError(PaError err)
{
  if (err != paNoError)
  {
    printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    exit(-1);
  }
}

int paMuxerCallback(const void*, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* userData)
{
  Muxer* muxer = (Muxer*) userData;
  float* out = (float*) outputBuffer;
  
  for (unsigned long i = 0; i<framesPerBuffer; i++)
  {
    unsigned long curAmount = 0;
    *out = 0;
    
    for (auto& sound : muxer->playing)
    {
      if (sound.pos < sound.len)
      {
        *out *= curAmount++;
        *out += sound.ptr[sound.pos++];
        *out /= (float) curAmount;
      }
    }
    
    out++;
  }
  
  return 0;
}

static Muxer* muxer;

void initMuxer()
{
  muxer = new Muxer();
  
  dealWithPaError(Pa_Initialize());
  dealWithPaError(Pa_OpenDefaultStream(&(muxer->stream), 0, 1, paFloat32, SAMPLE_RATE, paFramesPerBufferUnspecified, paMuxerCallback, muxer));
  dealWithPaError(Pa_StartStream(muxer->stream));
}

void destroyMuxer()
{
  dealWithPaError(Pa_AbortStream(muxer->stream));
  dealWithPaError(Pa_CloseStream(muxer->stream));
  dealWithPaError(Pa_Terminate());
  
  delete muxer;
  muxer = 0;
}

void playSound(const char* filename)
{
  // First, clear out any sounds that have finished playing
  muxer->playing.remove_if([] (Sound& value) { return value.pos >= value.len; });
  
  // Then, add the new sound
  muxer->playing.emplace_back(filename);
}

Sound::Sound(const char* filename)
{
  SF_INFO info;
  SNDFILE* file = sf_open(filename, SFM_READ, &info);
  if (file == nullptr)
  {
    printf("LibSndFile error: %s\n", sf_strerror(file));
    exit(-1);
  }
  
  ptr = (float*) malloc(info.frames * info.channels * sizeof(float));
  len = info.frames * info.channels;
  pos = 0;
  
  sf_readf_float(file, ptr, info.frames);
  
  sf_close(file);
}

Sound::~Sound()
{
  free(ptr);
}
