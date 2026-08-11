#include "PPU2C02.hpp"
#include <cstdint>

PPU2C02::PPU2C02() {
  palScreen[0x00] = Color{84, 84, 84, 255};
  palScreen[0x01] = Color{0, 30, 116, 255};
  palScreen[0x02] = Color{8, 16, 144, 255};
  palScreen[0x03] = Color{48, 0, 136, 255};
  palScreen[0x04] = Color{68, 0, 100, 255};
  palScreen[0x05] = Color{92, 0, 48, 255};
  palScreen[0x06] = Color{84, 4, 0, 255};
  palScreen[0x07] = Color{60, 24, 0, 255};
  palScreen[0x08] = Color{32, 42, 0, 255};
  palScreen[0x09] = Color{8, 58, 0, 255};
  palScreen[0x0A] = Color{0, 64, 0, 255};
  palScreen[0x0B] = Color{0, 60, 0, 255};
  palScreen[0x0C] = Color{0, 50, 60, 255};
  palScreen[0x0D] = Color{0, 0, 0, 255};
  palScreen[0x0E] = Color{0, 0, 0, 255};
  palScreen[0x0F] = Color{0, 0, 0, 255};

  palScreen[0x10] = Color{152, 150, 152, 255};
  palScreen[0x11] = Color{8, 76, 196, 255};
  palScreen[0x12] = Color{48, 50, 236, 255};
  palScreen[0x13] = Color{92, 30, 228, 255};
  palScreen[0x14] = Color{136, 20, 176, 255};
  palScreen[0x15] = Color{160, 20, 100, 255};
  palScreen[0x16] = Color{152, 34, 32, 255};
  palScreen[0x17] = Color{120, 60, 0, 255};
  palScreen[0x18] = Color{84, 90, 0, 255};
  palScreen[0x19] = Color{40, 114, 0, 255};
  palScreen[0x1A] = Color{8, 124, 0, 255};
  palScreen[0x1B] = Color{0, 118, 40, 255};
  palScreen[0x1C] = Color{0, 102, 120, 255};
  palScreen[0x1D] = Color{0, 0, 0, 255};
  palScreen[0x1E] = Color{0, 0, 0, 255};
  palScreen[0x1F] = Color{0, 0, 0, 255};

  palScreen[0x20] = Color{236, 238, 236, 255};
  palScreen[0x21] = Color{76, 154, 236, 255};
  palScreen[0x22] = Color{120, 124, 236, 255};
  palScreen[0x23] = Color{176, 98, 236, 255};
  palScreen[0x24] = Color{228, 84, 236, 255};
  palScreen[0x25] = Color{236, 88, 180, 255};
  palScreen[0x26] = Color{236, 106, 100, 255};
  palScreen[0x27] = Color{212, 136, 32, 255};
  palScreen[0x28] = Color{160, 170, 0, 255};
  palScreen[0x29] = Color{116, 196, 0, 255};
  palScreen[0x2A] = Color{76, 208, 32, 255};
  palScreen[0x2B] = Color{56, 204, 108, 255};
  palScreen[0x2C] = Color{56, 180, 204, 255};
  palScreen[0x2D] = Color{60, 60, 60, 255};
  palScreen[0x2E] = Color{0, 0, 0, 255};
  palScreen[0x2F] = Color{0, 0, 0, 255};

  palScreen[0x30] = Color{236, 238, 236, 255};
  palScreen[0x31] = Color{168, 204, 236, 255};
  palScreen[0x32] = Color{188, 188, 236, 255};
  palScreen[0x33] = Color{212, 178, 236, 255};
  palScreen[0x34] = Color{236, 174, 236, 255};
  palScreen[0x35] = Color{236, 174, 212, 255};
  palScreen[0x36] = Color{236, 180, 176, 255};
  palScreen[0x37] = Color{228, 196, 144, 255};
  palScreen[0x38] = Color{204, 210, 120, 255};
  palScreen[0x39] = Color{180, 222, 120, 255};
  palScreen[0x3A] = Color{168, 226, 144, 255};
  palScreen[0x3B] = Color{152, 226, 180, 255};
  palScreen[0x3C] = Color{160, 214, 228, 255};
  palScreen[0x3D] = Color{160, 162, 160, 255};
  palScreen[0x3E] = Color{0, 0, 0, 255};
  palScreen[0x3F] = Color{0, 0, 0, 255};
}

const Color *PPU2C02::GetScreen() {
  return sprScreen;
}

const Color *PPU2C02::GetNameTable(uint8_t i) {
  return sprNameTable[i];
}
Color PPU2C02::GetColourFromPaletteRam(uint8_t pallete, uint8_t pixel) {
  return palScreen[ppuRead(0x3F00 + (pallete << 2) + pixel)];
}
const Color *PPU2C02::GetPatternTable(uint8_t i, uint8_t pallete) {
  for (uint16_t tileY = 0; tileY < 16; tileY++) {
    for (uint16_t tileX = 0; tileX < 16; tileX++) {
      uint16_t offset = tileY * 256 + tileX * 16;

      for (uint16_t row = 0; row < 8; row++) {
        uint8_t tile_lsb = ppuRead(i * 0x1000 + offset + row + 0);
        uint8_t tile_msb = ppuRead(i * 0x1000 + offset + row + 8);

        for (uint16_t col = 0; col < 8; col++) {
          uint8_t pixel = (tile_lsb & 0x01) + (tile_msb & 0x01);
          tile_lsb >>= 1;
          tile_msb >>= 1;

          int px = tileX * 8 + (7 - col);
          int py = tileY * 8 + row;
          sprPatternTable[i][py * 128 + px] =
              GetColourFromPaletteRam(pallete, pixel);
        }
      }
    }
  }

  return sprPatternTable[i];
}

uint8_t PPU2C02::cpuRead(uint16_t addr, bool rdonly) {
  uint8_t data = 0x00;

  switch (addr) {
  case 0x0000: // Control
    break;
  case 0x0001: // Mask
    break;
  case 0x0002: // Status
    data = (status.reg & 0xE0) | (ppu_data_buffer & 0x1F);
    status.vertical_blank = 0;
    address_latch = 0;
    break;
  case 0x0003: // OAM Address
    break;
  case 0x0004: // OAM Data
    break;
  case 0x0005: // Scroll
    break;
  case 0x0006: // PPU Address
    break;
  case 0x0007: // PPU Data
    data = ppu_data_buffer;
    ppu_data_buffer = ppuRead(ppu_address);

    if (ppu_address > 0x3F00) {
      data = ppu_data_buffer;
    }
    ppu_address++;
    break;
  }

  return data;
}

void PPU2C02::cpuWrite(uint16_t addr, uint8_t data) {
  switch (addr) {
  case 0x0000: // Control
    control.reg = data;
    break;
  case 0x0001: // Mask
    mask.reg = data;
    break;
  case 0x0002: // Status
    break;
  case 0x0003: // OAM Address
    break;
  case 0x0004: // OAM Data
    break;
  case 0x0005: // Scroll
    break;
  case 0x0006: // PPU Address
    if (address_latch == 0) {
      ppu_address = (ppu_address & 0x00FF) | (data << 8);
      address_latch = 1;

    } else {
      ppu_address = (ppu_address & 0xFF00) | data;
      address_latch = 0;
    }
    break;
  case 0x0007: // PPU Data
    ppuWrite(ppu_address, data);
    ppu_address++;
    break;
  }
}

uint8_t PPU2C02::ppuRead(uint16_t addr, bool rdonly) {
  uint8_t data = 0x00;
  addr &= 0x3FFF;

  if (cart->ppuRead(addr, data)) {

  } else if (addr >= 0x0000 && addr <= 0x1FFF) {

    data = PatternTable[(addr & 0x1000) >> 12][addr & 0x0FFF];

  } else if (addr >= 0x2000 && addr <= 0x3EFF) {

  } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
    addr &= 0x001F;
    if (addr == 0x0010)
      addr = 0x0000;
    if (addr == 0x0014)
      addr = 0x0004;
    if (addr == 0x0018)
      addr = 0x0008;
    if (addr == 0x001C)
      addr = 0x000C;
    data = PaletteTable[addr];
  }

  return data;
}

void PPU2C02::ppuWrite(uint16_t addr, uint8_t data) {
  addr &= 0x3FFF;

  if (cart->ppuWrite(addr, data)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {

    PatternTable[(addr & 0x1000) >> 12][addr & 0x0FFF] = data;

  } else if (addr >= 0x2000 && addr <= 0x3EFF) {

  } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
    addr &= 0x001F;
    if (addr == 0x0010)
      addr = 0x0000;
    if (addr == 0x0014)
      addr = 0x0004;
    if (addr == 0x0018)
      addr = 0x0008;
    if (addr == 0x001C)
      addr = 0x000C;
    PaletteTable[addr] = data;
  }
}

void PPU2C02::ConnectCartridge(const std::shared_ptr<Cartridge> &cartridge) {
  this->cart = cartridge;
}

void PPU2C02::clock() {

  if (cycle - 1 >= 0 && cycle - 1 < 256 && scanline >= 0 && scanline < 240) {
    sprScreen[scanline * 256 + (cycle - 1)] =
        palScreen[(rand() % 2) ? 0x3F : 0x30];
  }

  if (scanline == -1 && cycle == 1) {
    status.vertical_blank = 0;
    status.sprite_zero_hit = 0;
    status.sprite_overflow = 0;
  }

  if (scanline == 241 && cycle == 1) {
    status.vertical_blank = 1;
    if (control.enable_nmi)
      nmi = true;
  }

  cycle++;
  if (cycle >= 341) {
    cycle = 0;
    scanline++;
    if (scanline >= 261) {
      scanline = -1;
      frame_complete = true;
    }
  }
}
