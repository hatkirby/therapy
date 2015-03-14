#include "muxer.h"
#include <cstdlib>
#include <sndfile.h>
#include <portaudio.h>
#include <list>
#include <cmath>

#define SAMPLE_RATE (44100)
#define DELAY_IN_SECS (0.075)
#define GAIN (1.0)
#define FEEDBACK (0.2)
#define DRY (1.0)
#define WET (0.5)

const int delaySize = SAMPLE_RATE * DELAY_IN_SECS;

class Sound {
  public:
    Sound(const char* filename, float vol);
    ~Sound();
    
    float* ptr;
    unsigned long pos;
    unsigned long len;
    float vol;
};

struct Muxer {
  std::list<Sound> playing;
  PaStream* stream;
  float* delay;
  unsigned long delayPos;
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
    float in = 0.0;
    
    for (auto& sound : muxer->playing)
    {
      if (sound.pos < sound.len)
      {
        in += sound.ptr[sound.pos++] * sound.vol;
      }
    }
    
    if (in >  1) in = 1;
    if (in < -1) in = -1;
    
    float sample = muxer->delay[muxer->delayPos] * GAIN;
    muxer->delay[muxer->delayPos] = in + (muxer->delay[muxer->delayPos] * FEEDBACK);
    muxer->delayPos++;
    if (muxer->delayPos > delaySize) muxer->delayPos = 0;
    *out++ = (in * DRY) + (sample * WET);
  }
  
  return 0;
}

static Muxer* muxer;

void initMuxer()
{
  muxer = new Muxer();
  
  dealWithPaError(Pa_Initialize());
  dealWithPaError(Pa_OpenDefaultStream(&muxer->stream, 0, 1, paFloat32, SAMPLE_RATE, paFramesPerBufferUnspecified, paMuxerCallback, muxer));
  dealWithPaError(Pa_StartStream(muxer->stream));
  
  muxer->delay = (float*) calloc(delaySize, sizeof(float));
}

void destroyMuxer()
{
  dealWithPaError(Pa_AbortStream(muxer->stream));
  dealWithPaError(Pa_CloseStream(muxer->stream));
  dealWithPaError(Pa_Terminate());
  
  free(muxer->delay);
  delete muxer;
  muxer = 0;
}

void playSound(const char* filename, float vol)
{
  // First, clear out any sounds that have finished playing
  muxer->playing.remove_if([] (Sound& value) { return value.pos >= value.len; });
  
  // Then, add the new sound
  muxer->playing.emplace_back(filename, vol);
}

Sound::Sound(const char* filename, float vol)
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
  this->vol = vol;
  
  sf_readf_float(file, ptr, info.frames);
  
  sf_close(file);
}

Sound::~Sound()
{
  free(ptr);
}
