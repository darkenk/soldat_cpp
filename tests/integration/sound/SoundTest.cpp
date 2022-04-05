#include <SDL2/SDL.h>
#include <chrono>
#include <cstring>
#include <gtest/gtest.h>
#include <soloud.h>
#include <soloud_wav.h>

class SoundTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    ASSERT_EQ(0, SDL_Init(SDL_INIT_AUDIO));
  }

  void TearDown() override
  {
    SDL_Quit();
  }
};

struct AudioData
{
  Uint8 *buffer = nullptr;
  Uint32 pos = 0;
  Uint32 size = 0;
  Uint8 silence = 0;
};

void AudioCallback(void *userdata, Uint8 *stream, int len)
{
  AudioData *data = reinterpret_cast<AudioData *>(userdata);
  std::memset(stream, data->silence, len);
  Uint32 size = std::min(data->size - data->pos, (Uint32)len);
  SDL_MixAudio(stream, data->buffer + data->pos, size, SDL_MIX_MAXVOLUME);
  data->pos += size;
}

TEST_F(SoundTest, PlaySomething)
{
  SDL_AudioSpec out;
  SDL_AudioSpec fileSpec;
  AudioData audioData;

  ASSERT_NE(nullptr, SDL_LoadWAV("fall.wav", &fileSpec, &audioData.buffer, &audioData.size));
  fileSpec.callback = AudioCallback;
  fileSpec.silence = 0;

  fileSpec.userdata = &audioData;
  ASSERT_EQ(0, SDL_OpenAudio(&fileSpec, &out));
  audioData.silence = out.silence;
  SDL_PauseAudio(0);
  while (audioData.pos != audioData.size)
  {
    SDL_Delay(16);
  }

  SDL_FreeWAV(audioData.buffer);
  SDL_LockAudio();
  SDL_CloseAudio();
  SDL_UnlockAudio();
}

TEST_F(SoundTest, PlaySoloud)
{
  SoLoud::Soloud engine;
  SoLoud::Wav wave;
  engine.init();
  wave.load("fall.wav");
  engine.play(wave);
  SDL_Delay(100);
  engine.deinit();
}
