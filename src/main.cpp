#include <SDL.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "fmt/format.h"
#include "mod/Generator.h"
#include "mod/InfoString.h"
#include "mod/Reader.h"
#include "mod/writer/RawWriter.h"
#include "mod/writer/WavWriter.h"

extern void fill_audio(void *udata, Uint8 *stream, int len);

bool initAudio(void *userdata) {
  SDL_AudioSpec wanted;

  /* Set the audio format */
  wanted.freq = 11025*2;
  wanted.format = AUDIO_U8;
  wanted.channels = 1;   /* 1 = mono, 2 = stereo */
  wanted.samples = 440*4; /* Good low-latency value for callback */
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
//  int8_t original = -123;
//  float toFloat = convertFromS8((const uint8_t *)&original);
//  int8_t restored;
//  convertToS8(toFloat, (uint8_t *)&restored);
//
//  std::cout << "Original: " << (int)original << "\n"
//            << "To float: " << toFloat << "\n"
//            << "Restored: " << (int)restored << "\n";
//
//  original = -123/2;
//  toFloat = convertFromS8((const uint8_t *)&original);
//  convertToS8(toFloat, (uint8_t *)&restored);
//  std::cout << "Original: " << (int)original << "\n"
//            << "To float: " << toFloat << "\n"
//            << "Restored: " << (int)restored << "\n";


  mod::Mod serializedMod = mod::Reader::read("melnorm.mod");

  std::cout << mod::InfoString::toString(serializedMod) << "\n";

  mod::Generator generator(std::move(serializedMod), mod::Encoding::Unsigned8);

//  generator.setCurrentOrder(0);
//  generator.setCurrentRow(0x35);
//  generator.solo(3);
//  generator.solo(2);




//  mod::WavWriter writer;
//
//  std::ofstream stream("output.wav");
//
//  writer.write(generator, stream);
//
//  stream.close();




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

  generator.generate(stream, len, 1.0f);
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