#include "DebugView.hpp"

#include <algorithm>

#include "M6502.hpp"

namespace {

std::string Hex(uint32_t n, uint8_t d) {
  std::string s(d, '0');
  for (int i = d - 1; i >= 0; i--, n >>= 4)
    s[i] = "0123456789ABCDEF"[n & 0xF];
  return s;
}

constexpr int kPanelX = 516;
const Color kBackground = {0, 0, 90, 255};

} // namespace

DebugView::DebugView(Bus &nes, int scale)
    : nes_(nes), scale_(scale < 1 ? 1 : scale) {
  target_ = LoadRenderTexture(kWidth, kHeight);
  screen_.Init(256, 240);
  patternTable_[0].Init(128, 128);
  patternTable_[1].Init(128, 128);
}

DebugView::~DebugView() {
  screen_.Unload();
  patternTable_[0].Unload();
  patternTable_[1].Unload();
  UnloadRenderTexture(target_);
}

void DebugView::NextPalette() {
  selectedPalette_ = (selectedPalette_ + 1) & 0x07;
}

void DebugView::ToggleDisassembly() {
  showCode_ = !showCode_;
  if (showCode_ && asm_.empty())
    asm_ = nes_.cpu.disassemble(0x0000, 0xFFFF);
}

void DebugView::ToggleDebug() {
  showDebug_ = !showDebug_;
}

void DebugView::Draw() {
  screen_.Update(nes_.ppu.GetScreen());

  if (!showDebug_) {
    // Game only: fill the window, keep aspect ratio, letterbox the rest.
    float s = std::min(GetScreenWidth() / 256.0f, GetScreenHeight() / 240.0f);
    float w = 256 * s, h = 240 * s;
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(
        screen_.texture, {0, 0, 256, 240},
        {(GetScreenWidth() - w) / 2, (GetScreenHeight() - h) / 2, w, h}, {0, 0},
        0.0f, WHITE);
    EndDrawing();
    return;
  }

  patternTable_[0].Update(nes_.ppu.GetPatternTable(0, selectedPalette_));
  patternTable_[1].Update(nes_.ppu.GetPatternTable(1, selectedPalette_));

  BeginTextureMode(target_);
  ClearBackground(kBackground);

  DrawCpu(kPanelX, 2);
  if (showCode_)
    DrawCode(kPanelX, 72, 26);
  else
    DrawOam(kPanelX, 72, 26);
  DrawPalettes(kPanelX, 340);

  DrawTexture(patternTable_[0].texture, 516, 348, WHITE);
  DrawTexture(patternTable_[1].texture, 648, 348, WHITE);
  DrawTextureEx(screen_.texture, {0, 0}, 0.0f, 2.0f, WHITE);

  EndTextureMode();

  BeginDrawing();
  ClearBackground(BLACK);
  DrawTexturePro(
      target_.texture,
      {0, 0, (float)target_.texture.width, -(float)target_.texture.height},
      {0, 0, (float)(kWidth * scale_), (float)(kHeight * scale_)}, {0, 0}, 0.0f,
      WHITE);
  EndDrawing();
}

void DebugView::DrawCpu(int x, int y) {
  const auto &cpu = nes_.cpu;

  DrawText("STATUS:", x, y, 10, WHITE);
  DrawText("N", x + 64, y, 10, cpu.status & M6502::N ? GREEN : RED);
  DrawText("V", x + 80, y, 10, cpu.status & M6502::V ? GREEN : RED);
  DrawText("-", x + 96, y, 10, cpu.status & M6502::U ? GREEN : RED);
  DrawText("B", x + 112, y, 10, cpu.status & M6502::B ? GREEN : RED);
  DrawText("D", x + 128, y, 10, cpu.status & M6502::D ? GREEN : RED);
  DrawText("I", x + 144, y, 10, cpu.status & M6502::I ? GREEN : RED);
  DrawText("Z", x + 160, y, 10, cpu.status & M6502::Z ? GREEN : RED);
  DrawText("C", x + 178, y, 10, cpu.status & M6502::C ? GREEN : RED);

  DrawText(("PC: $" + Hex(cpu.PC, 4)).c_str(), x, y + 10, 10, WHITE);
  DrawText(
      ("A: $" + Hex(cpu.A, 2) + "  [" + std::to_string(cpu.A) + "]").c_str(), x,
      y + 20, 10, WHITE);
  DrawText(
      ("X: $" + Hex(cpu.X, 2) + "  [" + std::to_string(cpu.X) + "]").c_str(), x,
      y + 30, 10, WHITE);
  DrawText(
      ("Y: $" + Hex(cpu.Y, 2) + "  [" + std::to_string(cpu.Y) + "]").c_str(), x,
      y + 40, 10, WHITE);
  DrawText(("Stack P: $" + Hex(cpu.SP, 4)).c_str(), x, y + 50, 10, WHITE);
}

void DebugView::DrawCode(int x, int y, int nLines) {
  auto it_a = asm_.find(nes_.cpu.PC);
  int nLineY = (nLines >> 1) * 10 + y;
  if (it_a != asm_.end()) {
    DrawText(it_a->second.c_str(), x, nLineY, 10, SKYBLUE);
    while (nLineY < (nLines * 10) + y) {
      nLineY += 10;
      if (++it_a != asm_.end())
        DrawText(it_a->second.c_str(), x, nLineY, 10, WHITE);
    }
  }
  it_a = asm_.find(nes_.cpu.PC);
  nLineY = (nLines >> 1) * 10 + y;
  if (it_a != asm_.end()) {
    while (nLineY > y) {
      nLineY -= 10;
      if (--it_a != asm_.end())
        DrawText(it_a->second.c_str(), x, nLineY, 10, WHITE);
    }
  }
}

void DebugView::DrawOam(int x, int y, int nLines) {
  // First nLines of the 64 OAM entries
  for (int i = 0; i < nLines; i++) {
    std::string s = Hex(i, 2) + ": (" +
                    std::to_string(nes_.ppu.pOAM[i * 4 + 3]) + ", " +
                    std::to_string(nes_.ppu.pOAM[i * 4 + 0]) + ") " +
                    "ID: " + Hex(nes_.ppu.pOAM[i * 4 + 1], 2) +
                    " AT: " + Hex(nes_.ppu.pOAM[i * 4 + 2], 2);
    DrawText(s.c_str(), x, y + i * 10, 10, WHITE);
  }
}

void DebugView::DrawPalettes(int x, int y) {
  const int nSwatchSize = 6;
  for (int p = 0; p < 8; p++)
    for (int s = 0; s < 4; s++) {
      Color c = nes_.ppu.GetColourFromPaletteRam(p, s);
      DrawRectangle(x + p * (nSwatchSize * 5) + s * nSwatchSize, y, nSwatchSize,
                    nSwatchSize, c);
    }

  DrawRectangleLines(x + selectedPalette_ * (nSwatchSize * 5) - 1, y - 1,
                     nSwatchSize * 4, nSwatchSize, WHITE);
}
