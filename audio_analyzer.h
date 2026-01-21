#pragma once

#include "core/templates/vector.h"
#include "core/math/audio_frame.h"

/**
 * @brief Tools for debugging audio streams produced by tap circuits.
 */
class AudioAnalyzer {
  public:
    struct statistics_t {
      AudioFrame mean;
      AudioFrame stddev;
    };

    statistics_t stats(Vector<AudioFrame> p_frames) const;
};