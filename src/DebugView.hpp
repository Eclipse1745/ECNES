#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "raylib.h"

#include "Bus.hpp"

class DebugView {
public:
  static constexpr int kWidth = 780;
  static constexpr int kHeight = 480;

  DebugView(Bus &nes, int scale);
  ~DebugView();

  DebugView(const DebugView &) = delete;
  DebugView &operator=(const DebugView &) = delete;

  void NextPalette();       // cycle which palette the pattern tables use
  void ToggleDisassembly(); // switch the side panel between OAM and code
  void ToggleDebug();       // debug panels on/off, game only when off
  void Draw();

private:
  struct PixelBuffer {
    Texture2D texture{};

    void Init(int w, int h) {
      Image img = GenImageColor(w, h, BLACK);
      texture = LoadTextureFromImage(img);
      UnloadImage(img);
    }
    void Update(const Color *pixels) {
      UpdateTexture(texture, pixels);
    }
    void Unload() {
      UnloadTexture(texture);
    }
  };

  void DrawCpu(int x, int y);
  void DrawCode(int x, int y, int nLines);
  void DrawOam(int x, int y, int nLines);
  void DrawPalettes(int x, int y);

  Bus &nes_;
  int scale_;
  RenderTexture2D target_{};
  PixelBuffer screen_;
  PixelBuffer patternTable_[2];
  uint8_t selectedPalette_ = 0x00;
  bool showCode_ = false;
  bool showDebug_ = true;
  std::map<uint16_t, std::string> asm_;
};
