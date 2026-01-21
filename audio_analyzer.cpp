#include <cmath> // for std::sqrt

#include "core/templates/vector.h"
#include "core/math/audio_frame.h"

#include "audio_analyzer.h"

AudioAnalyzer::statistics_t AudioAnalyzer::stats(Vector<AudioFrame> p_frames) const {
  statistics_t result{};

  if (p_frames.is_empty()) {
    return result; // returns zeros
  }

  // --- Compute mean ---
  AudioFrame sum{0.0f, 0.0f};

  for (int i = 0; i < p_frames.size(); ++i) {
    sum.left  += p_frames[i].left;
    sum.right += p_frames[i].right;
  }

  result.mean.left  = sum.left  / p_frames.size();
  result.mean.right = sum.right / p_frames.size();

  // --- Compute variance ---
  AudioFrame var{0.0f, 0.0f};

  for (int i = 0; i < p_frames.size(); ++i) {
      float dl = p_frames[i].left  - result.mean.left;
      float dr = p_frames[i].right - result.mean.right;

      var.left  += dl * dl;
      var.right += dr * dr;
  }

  var.left  /= p_frames.size();
  var.right /= p_frames.size();

  // --- Standard deviation ---
  result.stddev.left  = std::sqrt(var.left);
  result.stddev.right = std::sqrt(var.right);

  return result;
}
