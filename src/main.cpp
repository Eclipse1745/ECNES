#include <iostream>
#include <memory>
#include <string>

#include "raylib.h"

#include "Bus.hpp"
#include "Cartridge.hpp"
#include "DebugView.hpp"
#include "NesAudio.hpp"

constexpr float kFrameTime = 1.0f / 60.0988f;

// Keyboard -> NES controller 1. Bit order is A, B, Select, Start, Up, Down,
uint8_t ReadController() {
  uint8_t state = 0x00;
  if (IsKeyDown(KEY_O))
    state |= 0x80; // A
  if (IsKeyDown(KEY_P))
    state |= 0x40; // B
  if (IsKeyDown(KEY_RIGHT_SHIFT))
    state |= 0x20; // Select
  if (IsKeyDown(KEY_ENTER))
    state |= 0x10; // Start
  if (IsKeyDown(KEY_W))
    state |= 0x08; // Up
  if (IsKeyDown(KEY_S))
    state |= 0x04; // Down
  if (IsKeyDown(KEY_A))
    state |= 0x02; // Left
  if (IsKeyDown(KEY_D))
    state |= 0x01; // Right
  return state;
}

void RunFrame(Bus &nes, NesAudio &audio) {
  do {
    if (nes.clock())
      audio.Push(static_cast<float>(nes.dAudioSample));
  } while (!nes.ppu.frame_complete);
  nes.ppu.frame_complete = false;
}

void StepInstruction(Bus &nes) {
  do {
    nes.cpu.clock();
  } while (!nes.cpu.complete());
  do {
    nes.clock();
  } while (nes.cpu.complete());
}

void StepFrame(Bus &nes) {
  do {
    nes.clock();
  } while (!nes.ppu.frame_complete);
  do {
    nes.clock();
  } while (!nes.cpu.complete());
  nes.ppu.frame_complete = false;
}

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
    try {
      nScale = std::stoi(argv[2]);
    } catch (const std::exception &) {
      std::cerr << "Scale must be a number, got '" << argv[2]
                << "' -- using 1 instead\n";
      nScale = 1;
    }
    if (nScale < 1) {
      std::cerr << "Scale must be at least 1, got " << nScale
                << " -- using 1 instead\n";
      nScale = 1;
    }
  } else {
    std::cout << "No scale specified, defaulting to " << nScale
              << " (usage: " << argv[0] << " <rom.nes> [scale])\n";
  }

  InitWindow(DebugView::kWidth * nScale, DebugView::kHeight * nScale,
             "PPU/CPU Debugger");
  SetTargetFPS(60);

  {
    Bus nes;
    nes.InsertCartridge(std::make_shared<Cartridge>(romPath));

    NesAudio audio;
    nes.SetSampleFrequency(NesAudio::kSampleRate);
    nes.reset();

    DebugView view(nes, nScale);

    bool bEmulationRun = false;
    float fResidualTime = 0.0f;

    while (!WindowShouldClose()) {
      nes.controller[0] = ReadController();

      if (bEmulationRun) {
        if (fResidualTime > 0.0f) {
          fResidualTime -= GetFrameTime();
        } else {
          fResidualTime += kFrameTime - GetFrameTime();
          RunFrame(nes, audio);
        }
      } else {
        if (IsKeyPressed(KEY_C))
          StepInstruction(nes);
        if (IsKeyPressed(KEY_F))
          StepFrame(nes);
      }

      audio.Pump();

      if (IsKeyPressed(KEY_R))
        nes.cpu.reset();
      if (IsKeyPressed(KEY_SPACE)) {
        bEmulationRun = !bEmulationRun;
        if (bEmulationRun)
          audio.Start();
        else
          audio.Pause();
      }
      if (IsKeyPressed(KEY_TAB))
        view.NextPalette();
      if (IsKeyPressed(KEY_H))
        view.ToggleDebug();
      if (IsKeyPressed(KEY_L))
        view.ToggleDisassembly();

      view.Draw();
    }
  }

  CloseWindow();
  return 0;
}
