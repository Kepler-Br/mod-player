#pragma once

#include <istream>
#include <memory>

#include "TrackerLoader.h"
#include "mod/Mod.h"

namespace mod {

class ModLoader : public TrackerLoader {
 private:
  /**
   *
   * @param stream
   * @throws runtime_error
   * @return
   */
  static Note serializeNote(std::istream &stream);

  /**
   *
   * @param stream
   * @throws runtime_error
   * @return
   */
  static Row serializeRow(std::istream &stream, size_t channels);

  /**
   *
   * @param stream
   * @param channels
   * @param totalRows
   * @throws runtime_error
   * @return
   */
  static Pattern serializePattern(std::istream &stream, size_t channels,
                         size_t totalRows);

  /**
   *
   * @param stream
   * @throws runtime_error
   * @return
   */
  static Sample serializeSample(std::istream &stream);


  /**
   * @param stream
   * @throws runtime_error
   * @return
   */
  [[nodiscard]] static size_t getChannels(std::istream &stream);
  /**
   * @param stream
   * @throws runtime_error
   */
  [[nodiscard]] static std::vector<Sample> readSamples(std::istream &stream);
  /**
   * @param stream
   * @throws runtime_error
   */
  [[nodiscard]] static std::vector<Pattern> readPatterns(std::istream &stream,
                                                         size_t patternsNumber);
  /**
   * @param stream
   * @throws runtime_error
   * @throws invalid_argument
   */
  static void readSamplesAudioData(std::istream &stream,
                                   std::vector<Sample> &samples, Encoding audioDataEncoding);
  /**
   * @param stream
   * @throws runtime_error
   */
  [[nodiscard]] static std::vector<int> readOrders(std::istream &stream);
  /**
   * @param stream
   * @throws runtime_error
   */
  [[nodiscard]] static std::string readName(std::istream &stream);
 public:
  ~ModLoader() override = default;

  std::shared_ptr<Mod> load(std::istream &stream) override;
  std::shared_ptr<Mod> load(const std::string &path) override;
};

}  // namespace mod