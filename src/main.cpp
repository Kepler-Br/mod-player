#include <SDL.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "fmt/format.h"
#include "mod/Generator.h"
#include "mod/InfoString.h"
#include "mod/Row.h"
#include "mod/writer/RawWriter.h"
#include "mod/writer/WavWriter.h"
#include "mod/loaders/TrackerLoader.h"
#include "mod/loaders/ModLoader.h"

extern void fill_audio(void *udata, Uint8 *stream, int len);

bool initAudio(void *userdata) {
  SDL_AudioSpec wanted;

  /* Set the audio format */
  wanted.freq = (int)(11025 * 2.0f);
//  wanted.freq = 8353;
  wanted.format = AUDIO_S8;
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

void setCallbacks(mod::Generator &generator) {
  generator.setNextRowCallback(
      [](mod::Generator &generator, size_t oldIndex, size_t newIndex) {
        const mod::Row &currentRow = generator.getCurrentRow();

        std::cout << std::setfill('0') << std::setw(2) << std::hex << oldIndex
                  << std::dec << " ";
        std::cout << mod::InfoString::fancyRow(currentRow) << std::endl;
      });

  generator.setNextOrderCallback(
      [](mod::Generator &generator, size_t oldOrderIndex, size_t newOrderIndex,
         size_t oldPatternIndex, size_t newPatternIndex) {
        std::cout << "Pattern: " << newPatternIndex
                  << "; Order: " << newOrderIndex << std::endl;
      });

  generator.setStateChangedCallback([](mod::Generator &generator,
                                       mod::GeneratorState oldState,
                                       mod::GeneratorState newState) {
    if (newState == mod::GeneratorState::Playing) {
      std::cout << "Playing\n";
    } else if (newState == mod::GeneratorState::Paused) {
      std::cout << "Paused\n";
    }
  });
}

int main() {
  using namespace mod;
  // TODO: Make use of new templated struct "ChangedValue"
  // TODO: comandr.mod + 11025.0f * 0.4f not working
  // TODO: Speed change not affected by frequency
  // TODO: Add to generator setter for order and row simultaneously

  std::shared_ptr<TrackerLoader> trackerLoader = std::make_shared<ModLoader>();

  std::shared_ptr<Mod> serializedMod = trackerLoader->load("ignore-mods/thradd.mod");

  std::cout << mod::InfoString::toString(*serializedMod) << "\n";

  mod::Generator generator(serializedMod, mod::Encoding::Signed8);
  generator.setFrequency(11025.0f * 2.0f);
//  generator.setFrequency(8363.0f);
//  generator.setFrequency(8550.0f);
//  generator.setFrequency(8353.0f);
  generator.setVolume(4.0f);

  setCallbacks(generator);

  generator.start();

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

//  for (int i = 0; i < len; i++) {
//    std::cout << (int)stream[i] << "|";
//  }
//
//  std::cout << "\n";
//  for (int i = 0; i < len; i++) {
//    if (stream[i] != (uint8_t)63) {
//      std::cout << "Index: " << i << "; Len: " << len << "; Value: " << (int)(stream[i]) << "\n";
//    }
//  }
}