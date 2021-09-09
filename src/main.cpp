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
//  wanted.freq = 10000;
  wanted.format = AUDIO_S16;
  wanted.channels = 1;   /* 1 = mono, 2 = stereo */
  wanted.samples = 440; /* Good low-latency value for callback */
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
    mod::Reader::read("ignore-mods/spathi.mod");

  std::cout << mod::InfoString::toString(serializedMod) << "\n";

  mod::Generator generator(std::move(serializedMod), mod::Encoding::Signed16);
  generator.setFrequency(11025*2);
  generator.setVolume(0.5f);

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
    SDL_Delay(1000);
  }
  SDL_CloseAudio();

  return 0;
}

void fill_audio(void *udata, Uint8 *stream, int len) {
  auto &generator = *(mod::Generator *)udata;

  generator.generate(stream, len);
}