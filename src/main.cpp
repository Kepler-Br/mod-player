#include <SDL.h>

#include <iostream>
#include <vector>

#include "fmt/format.h"
#include "mod/Generator.h"
#include "mod/InfoString.h"
#include "mod/Reader.h"

extern void fill_audio(void *udata, Uint8 *stream, int len);

bool initAudio(void *userdata) {
  SDL_AudioSpec wanted;

  /* Set the audio format */
  wanted.freq = 22050;
  wanted.format = AUDIO_U8;
  wanted.channels = 1;   /* 1 = mono, 2 = stereo */
  wanted.samples = 1024; /* Good low-latency value for callback */
  wanted.callback = fill_audio;
  wanted.userdata = userdata;

  /* Open the audio device, forcing the desired format */
  if (SDL_OpenAudio(&wanted, nullptr) < 0) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return false;
  }
  return true;
}

int main() {
  mod::Mod serializedMod =
      mod::Reader::read("pkunk.mod");

  std::cout << mod::InfoString::toString(serializedMod) << "\n";

  mod::Generator generator(std::move(serializedMod));



  if (!initAudio(&generator)) {
    return -1;
  }

  SDL_PauseAudio(0);
  while (generator.getState() == mod::GeneratorState::Playing) {
    SDL_Delay(3000);
  }
  SDL_CloseAudio();

  return 0;
}

// void fill_audio(void *udata, Uint8 *stream, int len) {
//     static float counter = 1.0f;
//
//     for (int i = 0; i < len; i++) {
//         float someSin = std::cos(counter);
//
//         stream[i] = (Uint8)((someSin + 1.0f) / 2.0f * 80.0f);
//         counter += M_PI_4 / 18.0f;
//     }
// }

void fill_audio(void *udata, Uint8 *stream, int len) {
  static size_t counter = 0;
  static size_t sampleIndex = 0;

  auto &generator = *(mod::Generator *)udata;
//  mod::Sample &sample = samples[sampleIndex];
//  std::vector<uint8_t> &data = sample.getData();

  generator.generate(stream, len);
//  for (int i = 0; i < len; i++) {
//    //    float processed = 0.0f;
//
////    if (counter >= data.size()) {
////      counter = 0;
////      sampleIndex++;
////      sampleIndex = sampleIndex % samples.size();
////      sample = samples[sampleIndex % samples.size()];
////      data = sample.getData();
////    }
//
//    //    if (counter < data.size()) {
//    //    processed = (float)data[counter % data.size()];
//    //    }
//    if (sampleIndex < samples.size()) {
//      stream[i] = (Uint8)data[counter];
//    } else {
//      stream[i] = 0x80;
//    }
//
//    counter++;
//  }
}