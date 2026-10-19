#include <iostream>
#include <memory>
#include <string>

#include "raylib.h"

#include "Bus.hpp"
#include "Cartridge.hpp"
#include "M6502.hpp"

static std::string Hex(uint32_t n, uint8_t d) {
  std::string s(d, '0');
  for (int i = d - 1; i >= 0; i--, n >>= 4)
    s[i] = "0123456789ABCDEF"[n & 0xF];
  return s;
}

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

int main(int argc, char *argv[]) {

  std::string romPath = "TestRoms/nestest.nes"; // fallback if no argument given
  int nScale = 1; // fallback if no scale argument given

  if (argc >= 2) {
    romPath = argv[1];
  } else {
    std::cout << "No ROM specified, defaulting to " << romPath
              << " (usage: " << argv[0] << " <rom.nes> [scale]) \n";
  }

  if (argc >= 3) {
    nScale = std::stoi(argv[2]);
    if (nScale < 1) {
      std::cerr << "Scale must be at least 1, got " << nScale
                << " -- using 1 instead\n";
      nScale = 1;
    }
  } else {
    std::cout << "No scale specified, defaulting to " << nScale
              << " (usage: " << argv[0] << " <rom.nes> [scale])\n";
  }

  const int nOriginalW = 780;
  const int nOriginalH = 480;

  InitWindow(nOriginalW * nScale, nOriginalH * nScale, "PPU/CPU Debugger");
  SetTargetFPS(60);

  RenderTexture2D target = LoadRenderTexture(nOriginalW, nOriginalH);

  Bus nes;
  auto cart = std::make_shared<Cartridge>(romPath);
  nes.InsertCartridge(cart);
  std::map<uint16_t, std::string> mapAsm = nes.cpu.disassemble(0x0000, 0xFFFF);
  nes.reset();

  bool bEmulationRun = false;
  float fResidualTime = 0.0f;
  uint8_t selectedPalette = 0x00;

  PixelBuffer screenBuf, patternTableBuf[2];
  screenBuf.Init(256, 240);
  patternTableBuf[0].Init(128, 128);
  patternTableBuf[1].Init(128, 128);

  const Color colBg = {0, 0, 90, 255};

  auto DrawCpu = [&](int x, int y) {
    DrawText("STATUS:", x, y, 10, WHITE);
    DrawText("N", x + 64, y, 10, nes.cpu.status & M6502::N ? GREEN : RED);
    DrawText("V", x + 80, y, 10, nes.cpu.status & M6502::V ? GREEN : RED);
    DrawText("-", x + 96, y, 10, nes.cpu.status & M6502::U ? GREEN : RED);
    DrawText("B", x + 112, y, 10, nes.cpu.status & M6502::B ? GREEN : RED);
    DrawText("D", x + 128, y, 10, nes.cpu.status & M6502::D ? GREEN : RED);
    DrawText("I", x + 144, y, 10, nes.cpu.status & M6502::I ? GREEN : RED);
    DrawText("Z", x + 160, y, 10, nes.cpu.status & M6502::Z ? GREEN : RED);
    DrawText("C", x + 178, y, 10, nes.cpu.status & M6502::C ? GREEN : RED);

    DrawText(("PC: $" + Hex(nes.cpu.PC, 4)).c_str(), x, y + 10, 10, WHITE);
    DrawText(
        ("A: $" + Hex(nes.cpu.A, 2) + "  [" + std::to_string(nes.cpu.A) + "]")
            .c_str(),
        x, y + 20, 10, WHITE);
    DrawText(
        ("X: $" + Hex(nes.cpu.X, 2) + "  [" + std::to_string(nes.cpu.X) + "]")
            .c_str(),
        x, y + 30, 10, WHITE);
    DrawText(
        ("Y: $" + Hex(nes.cpu.Y, 2) + "  [" + std::to_string(nes.cpu.Y) + "]")
            .c_str(),
        x, y + 40, 10, WHITE);
    DrawText(("Stack P: $" + Hex(nes.cpu.SP, 4)).c_str(), x, y + 50, 10, WHITE);
  };

  auto DrawCode = [&](int x, int y, int nLines) {
    auto it_a = mapAsm.find(nes.cpu.PC);
    int nLineY = (nLines >> 1) * 10 + y;
    if (it_a != mapAsm.end()) {
      DrawText(it_a->second.c_str(), x, nLineY, 10, SKYBLUE);
      while (nLineY < (nLines * 10) + y) {
        nLineY += 10;
        if (++it_a != mapAsm.end())
          DrawText(it_a->second.c_str(), x, nLineY, 10, WHITE);
      }
    }
    it_a = mapAsm.find(nes.cpu.PC);
    nLineY = (nLines >> 1) * 10 + y;
    if (it_a != mapAsm.end()) {
      while (nLineY > y) {
        nLineY -= 10;
        if (--it_a != mapAsm.end())
          DrawText(it_a->second.c_str(), x, nLineY, 10, WHITE);
      }
    }
  };

  while (!WindowShouldClose()) {
    float fElapsedTime = GetFrameTime();

    nes.controller[0] = 0x00;
    nes.controller[0] |= IsKeyDown(KEY_O) ? 0x80 : 0x00;
    nes.controller[0] |= IsKeyDown(KEY_P) ? 0x40 : 0x00;
    nes.controller[0] |= IsKeyDown(KEY_RIGHT_SHIFT) ? 0x20 : 0x00;
    nes.controller[0] |= IsKeyDown(KEY_ENTER) ? 0x10 : 0x00;
    nes.controller[0] |= IsKeyDown(KEY_W) ? 0x08 : 0x00;
    nes.controller[0] |= IsKeyDown(KEY_S) ? 0x04 : 0x00;
    nes.controller[0] |= IsKeyDown(KEY_A) ? 0x02 : 0x00;
    nes.controller[0] |= IsKeyDown(KEY_D) ? 0x01 : 0x00;

    if (bEmulationRun) {
      if (fResidualTime > 0.0f) {
        fResidualTime -= fElapsedTime;
      } else {
        fResidualTime += (1.0f / 60.0f) - fElapsedTime;
        do {
          nes.clock();
        } while (!nes.ppu.frame_complete);
        nes.ppu.frame_complete = false;
      }
    } else {
      if (IsKeyPressed(KEY_C)) {
        do {
          nes.cpu.clock();
        } while (!nes.cpu.complete());
        do {
          nes.clock();
        } while (nes.cpu.complete());
      }
      if (IsKeyPressed(KEY_F)) {
        do {
          nes.clock();
        } while (!nes.ppu.frame_complete);
        do {
          nes.clock();
        } while (!nes.cpu.complete());
        nes.ppu.frame_complete = false;
      }
    }

    if (IsKeyPressed(KEY_R))
      nes.cpu.reset();
    if (IsKeyPressed(KEY_SPACE))
      bEmulationRun = !bEmulationRun;
    if (IsKeyPressed(KEY_TAB))
      selectedPalette = (selectedPalette + 1) & 0x07;

    screenBuf.Update(nes.ppu.GetScreen());
    patternTableBuf[0].Update(nes.ppu.GetPatternTable(0, selectedPalette));
    patternTableBuf[1].Update(nes.ppu.GetPatternTable(1, selectedPalette));

    BeginTextureMode(target);
    ClearBackground(colBg);

    DrawCpu(516, 2);
    // DrawCode(516, 72, 26);

    // Draw OAM Contents (first 26 out of 64)
    for (int i = 0; i < 26; i++) {
      std::string s = Hex(i, 2) + ": (" +
                      std::to_string(nes.ppu.pOAM[i * 4 + 3]) + ", " +
                      std::to_string(nes.ppu.pOAM[i * 4 + 0]) + ") " +
                      "ID: " + Hex(nes.ppu.pOAM[i * 4 + 1], 2) +
                      " AT: " + Hex(nes.ppu.pOAM[i * 4 + 2], 2);
      DrawText(s.c_str(), 516, 72 + i * 10, 10, WHITE);
    }

    const int nSwatchSize = 6;
    for (int p = 0; p < 8; p++)
      for (int s = 0; s < 4; s++) {
        Color c = nes.ppu.GetColourFromPaletteRam(p, s);
        DrawRectangle(516 + p * (nSwatchSize * 5) + s * nSwatchSize, 340,
                      nSwatchSize, nSwatchSize, c);
      }

    DrawRectangleLines(516 + selectedPalette * (nSwatchSize * 5) - 1, 339,
                       nSwatchSize * 4, nSwatchSize, WHITE);

    DrawTexture(patternTableBuf[0].texture, 516, 348, WHITE);
    DrawTexture(patternTableBuf[1].texture, 648, 348, WHITE);
    DrawTextureEx(screenBuf.texture, {0, 0}, 0.0f, 2.0f, WHITE);

    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(
        target.texture,
        {0, 0, (float)target.texture.width, -(float)target.texture.height},
        {0, 0, (float)(nOriginalW * nScale), (float)(nOriginalH * nScale)},
        {0, 0}, 0.0f, WHITE);
    EndDrawing();
  }

  screenBuf.Unload();
  patternTableBuf[0].Unload();
  patternTableBuf[1].Unload();
  UnloadRenderTexture(target);
  CloseWindow();

  return 0;
}
