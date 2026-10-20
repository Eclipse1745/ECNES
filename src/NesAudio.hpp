#pragma once

#include <vector>

#include "raylib.h"

class NesAudio {
public:
  static constexpr int kSampleRate = 44100; // pass to Bus::SetSampleFrequency()

  NesAudio();
  ~NesAudio();

  void Start();            // resume playback
  void Pause();            // pause and drop queued samples
  void Push(float sample); // queue one sample
  void Pump();

private:
  AudioStream stream_{};
  std::vector<float> queue_;
};
