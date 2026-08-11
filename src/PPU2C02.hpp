#pragma once
#include "Cartridge.hpp"
#include "raylib.h"
#include <cstdint>
#include <memory>
class PPU2C02 {
public:
  PPU2C02();

  uint8_t nameTable[2][1024];
  uint8_t PaletteTable[3];
  uint8_t PatternTable[2][4096];

  // Communication with Main Bus
  uint8_t cpuRead(uint16_t addr, bool bReadOnly = false);
  void cpuWrite(uint16_t addr, uint8_t data);

  // Communications with PPU Bus
  uint8_t ppuRead(uint16_t addr, bool bReadOnly = false);
  void ppuWrite(uint16_t addr, uint8_t data);

public: // System Interface
  void ConnectCartridge(const std::shared_ptr<Cartridge> &cartridge);
  void clock();

  bool nmi;

private:
  std::shared_ptr<Cartridge> cart;

public:
  // Debugging Utilities
  const Color *GetScreen();
  const Color *GetNameTable(uint8_t i);
  const Color *GetPatternTable(uint8_t i, uint8_t palette);
  Color GetColourFromPaletteRam(uint8_t palette, uint8_t pixel);
  bool frame_complete = false;

private:
  int16_t scanline = 0;
  int16_t cycle = 0;
  union {
    struct {
      uint8_t unused : 5;
      uint8_t sprite_overflow : 1;
      uint8_t sprite_zero_hit : 1;
      uint8_t vertical_blank : 1;
    };

    uint8_t reg;
  } status;

  union {
    struct {
      uint8_t grayscale : 1;
      uint8_t render_background_left : 1;
      uint8_t render_sprites_left : 1;
      uint8_t render_background : 1;
      uint8_t render_sprites : 1;
      uint8_t enhance_red : 1;
      uint8_t enhance_green : 1;
      uint8_t enhance_blue : 1;
    };

    uint8_t reg;
  } mask;

  union PPUCTRL {
    struct {
      uint8_t nametable_x : 1;
      uint8_t nametable_y : 1;
      uint8_t increment_mode : 1;
      uint8_t pattern_sprite : 1;
      uint8_t pattern_background : 1;
      uint8_t sprite_size : 1;
      uint8_t slave_mode : 1; // unused
      uint8_t enable_nmi : 1;
    };

    uint8_t reg;
  } control;

  uint8_t address_latch = 0x00;
  uint8_t ppu_data_buffer = 0x00;
  uint16_t ppu_address = 0x00;

private:
  Color palScreen[0x40];
  Color sprScreen[256 * 240];
  Color sprNameTable[2][256 * 240];
  Color sprPatternTable[2][128 * 128];
};
