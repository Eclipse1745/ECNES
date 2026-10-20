#include "NesAudio.hpp"

constexpr unsigned kBuffer = 1024;
constexpr float kGain = 2.0f;

NesAudio::NesAudio() {
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(kBuffer);
  stream_ = LoadAudioStream(kSampleRate, 32, 1); // 32 bits = float, mono
  PlayAudioStream(stream_);
}

NesAudio::~NesAudio() {
  UnloadAudioStream(stream_);
  CloseAudioDevice();
}

void NesAudio::Start() {
  ResumeAudioStream(stream_);
}

void NesAudio::Pause() {
  PauseAudioStream(stream_);
  queue_.clear();
}

void NesAudio::Push(float sample) {
  queue_.push_back(sample * kGain);
}

void NesAudio::Pump() {
  while (IsAudioStreamProcessed(stream_) && queue_.size() >= kBuffer) {
    UpdateAudioStream(stream_, queue_.data(), kBuffer);
    queue_.erase(queue_.begin(), queue_.begin() + kBuffer);
  }
}
