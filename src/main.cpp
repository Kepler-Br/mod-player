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
  wanted.freq = 48000;
//  wanted.freq = 8353;
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

void setCallbacks(mod::Generator &generator) {
  generator.setNextRowCallback(
      [](mod::Generator &caller, const mod::ChangedRowEvent &event) {
        const mod::Row &currentRow = caller.getCurrentRow();
        const mod::Row &nextRow = caller.getRow(event.row.newValue);

        std::cout << std::setfill('0') << std::setw(2) << std::hex << event.row.newValue
                  << std::dec << " ";
        std::cout << mod::InfoString::fancyRow(nextRow) << std::endl;
      });

  generator.setNextOrderCallback(
      [](mod::Generator &caller, const mod::ChangedOrderEvent &event) {
        std::cout << "Pattern: " << event.pattern.newValue
                  << "; Order: " << event.order.newValue << std::endl;
      });

  generator.setStateChangedCallback([](mod::Generator &caller,
                                       const mod::ChangedStateEvent &event) {
    if (event.state.newValue == mod::GeneratorState::Playing) {
      std::cout << "Playing\n";
    } else if (event.state.newValue == mod::GeneratorState::Paused) {
      std::cout << "Paused\n";
    }
  });
}

int main() {
  using namespace mod;
  // TODO: comandr.mod + 11025.0f * 0.4f not working
  // TODO: Speed change not affected by frequency
  // TODO: Freq 48000 and buffer 1024*16 is infinity loop
  // TODO: Freq 48000 some instruments anomalies
  // TODO: Change time per row to float

  std::shared_ptr<TrackerLoader> trackerLoader = std::make_shared<ModLoader>();

  std::shared_ptr<Mod> serializedMod = trackerLoader->load("ignore-mods/pkunk.mod");

  std::cout << mod::InfoString::toString(*serializedMod) << "\n";

  mod::Generator generator(serializedMod, mod::Encoding::Unsigned8);
  generator.setFrequency(11025.0f * 2.0f);
  generator.setFrequency(48000.0f);
//  generator.setFrequency(8363.0f);
//  generator.setFrequency(8550.0f);
//  generator.setFrequency(8353.0f);
  generator.setVolume(1.0f);

  setCallbacks(generator);

  generator.restart();

//    generator.setCurrentOrder(8);
  //  generator.setCurrentRow(0x35);
  //  generator.solo(3);
  //  generator.solo(2);

//    mod::WavWriter writer;
//
//    std::ofstream stream("output.wav");
//
//    writer.write(generator, stream);
//
//    stream.close();

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