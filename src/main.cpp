#include <SDL.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "../ignore-mods/arilou.mod.h"
#include "MemoryStream.h"
#include "fmt/format.h"
#include "mod/Generator.h"
#include "mod/InfoString.h"
#include "mod/Row.h"
#include "mod/loaders/ModLoader.h"
#include "mod/loaders/TrackerLoader.h"
#include "mod/writer/RawWriter.h"
#include "mod/writer/WavWriter.h"

extern void fill_audio(void *udata, Uint8 *stream, int len);

SDL_AudioSpec initAudio(SDL_AudioSpec wanted) {
  /* Set the audio format */
  //  wanted.freq = (int)(11025 * 2.0f);
  //  wanted.freq = 48000;
  //  wanted.freq = 48000;
  //  wanted.freq = 8353;
  //  wanted.format = AUDIO_U8;
  //  wanted.channels = 1;   /* 1 = mono, 2 = stereo */
  //  wanted.samples = 1024; /* Good low-latency value for callback */
  //  wanted.callback = fill_audio;
  //  wanted.userdata = userdata;

  SDL_AudioSpec obtained;
  /* Open the audio device, forcing the desired format */
  if (SDL_OpenAudio(&wanted, &obtained) < 0) {
    throw std::runtime_error(
        fmt::format("Couldn't open audio: {}", SDL_GetError()));
  }
  return obtained;
}

void setCallbacks(mod::Generator &generator) {
  generator.setNextRowCallback(
      [](mod::Generator &caller, const mod::ChangedRowEvent &event) {
        const mod::Row &currentRow = caller.getCurrentRow();
        const mod::Row &nextRow = caller.getRow(event.row.newValue);

        std::cout << std::setfill('0') << std::setw(2) << std::hex
                  << event.row.newValue << std::dec << " ";
        std::cout << mod::InfoString::fancyRow(nextRow) << std::endl;
      });

  generator.setNextOrderCallback(
      [](mod::Generator &caller, const mod::ChangedOrderEvent &event) {
        std::cout << "Pattern: " << event.pattern.newValue
                  << "; Order: " << event.order.newValue << std::endl;
      });

  generator.setStateChangedCallback(
      [](mod::Generator &caller, const mod::ChangedStateEvent &event) {
        if (event.state.newValue == mod::GeneratorState::Playing) {
          std::cout << "Playing\n";
        } else if (event.state.newValue == mod::GeneratorState::Paused) {
          std::cout << "Paused\n";
        }
      });
}

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

void emMainLoop() {}

#endif

bool printDifference(SDL_AudioSpec wanted, SDL_AudioSpec obtained) {
  bool different = false;

  if (wanted.freq != obtained.freq) {
    std::cout << fmt::format("Wanted {0}: {1}; Obtained {0}: {2}", "freq",
                             wanted.freq, obtained.freq)
              << std::endl;
    different = true;
  }
  if (wanted.channels != obtained.channels) {
    std::cout << fmt::format("Wanted {0}: {1}; Obtained {0}: {2}", "channels",
                             wanted.channels, obtained.channels)
              << std::endl;
    different = true;
  }
  if (wanted.samples != obtained.samples) {
    std::cout << fmt::format("Wanted {0}: {1}; Obtained {0}: {2}", "samples",
                             wanted.samples, obtained.samples)
              << std::endl;
    different = true;
  }
  if (wanted.format != obtained.format) {
    std::cout << fmt::format("Wanted {0}: {1}; Obtained {0}: {2}", "format",
                             wanted.format, obtained.format)
              << std::endl;
    std::cout << fmt::format("Obtained info: Big: {}; Float: {}; Signed: {}",
                             SDL_AUDIO_ISBIGENDIAN(obtained.format),
                             SDL_AUDIO_ISFLOAT(obtained.format),
                             SDL_AUDIO_ISSIGNED(obtained.format))
              << std::endl;
    SDL_AUDIO_ISBIGENDIAN(obtained.format);
    different = true;
  }

  return different;
}

mod::Encoding sdlToEncoding(int sdl) {
  switch (sdl) {
    case AUDIO_U8:
      return mod::Encoding::Unsigned8;
    case AUDIO_S8:
      return mod::Encoding::Signed8;
    case AUDIO_U16:
      return mod::Encoding::Unsigned16;
    case AUDIO_S16:
      return mod::Encoding::Signed16;
    default:
      return mod::Encoding::Unknown;
  }
}

void playMod(std::istream &stream) {
  using namespace mod;

  std::shared_ptr<TrackerLoader> trackerLoader = std::make_shared<ModLoader>();
  std::shared_ptr<Mod> serializedMod = trackerLoader->load(stream);

  std::cout << mod::InfoString::toString(*serializedMod) << "\n";

  mod::Generator generator(serializedMod, mod::Encoding::Unsigned8);

  setCallbacks(generator);

  generator.restart();
  SDL_AudioSpec wanted;
  wanted.freq = (int)(11025 * 2.0f);
  //  wanted.freq = 48000;
  //  wanted.freq = 48000;
  //  wanted.freq = 8353;
  wanted.format = AUDIO_U8;
  wanted.channels = 1;   /* 1 = mono, 2 = stereo */
  wanted.samples = 1024; /* Good low-latency value for callback */
  wanted.callback = fill_audio;
  wanted.userdata = &generator;
  SDL_AudioSpec obtained = initAudio(wanted);
  printDifference(wanted, obtained);
  generator.setFrequency((float)obtained.freq);
  generator.setEncoding(sdlToEncoding(obtained.format));

  SDL_PauseAudio(0);
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(&emMainLoop, 60, true);
#else
  while (generator.getState() == mod::GeneratorState::Playing) {
    SDL_Delay(1000);
  }
  SDL_CloseAudio();
#endif
}

int main() {
  using namespace mod;
  // TODO: comandr.mod + 11025.0f * 0.4f not working
  // TODO: Speed change not affected by frequency
  // TODO: Freq 48000 and buffer 1024*16 is infinity loop
  // TODO: Freq 48000 some instruments anomalies on pkunk.mod, order 8
  // TODO: Change time per row to float

  std::ifstream stream("ignore-mods/slyhome.mod");

  //  MemoryStream stream((char *)modArray, modSize);

  playMod(stream);

  return 0;
}

void fill_audio(void *udata, Uint8 *stream, int len) {
  auto &generator = *(mod::Generator *)udata;

  generator.generate(stream, len);
}
