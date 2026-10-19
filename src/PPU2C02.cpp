#include "PPU2C02.hpp"
#include <cstring>
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
          uint8_t pixel = ((tile_lsb & 0x01) << 1) | (tile_msb & 0x01);
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
    data = control.reg;
    break;
  case 0x0001: // Mask
    data = mask.reg;
    break;
  case 0x0002: // Status
    data = status.reg;
    break;
  case 0x0003: // OAM Address
    break;
  case 0x0004: // OAM Data
    data = pOAM[oam_addr];
    break;
  case 0x0005: // Scroll
    break;
  case 0x0006: // PPU Address
    break;
  case 0x0007: // PPU Data
    data = ppu_data_buffer;
    ppu_data_buffer = ppuRead(vram_addr.reg);

    if (vram_addr.reg > 0x3F00) {
      data = ppu_data_buffer;
    }
    vram_addr.reg += control.increment_mode ? 32 : 1;
    break;
  }

  return data;
}

void PPU2C02::cpuWrite(uint16_t addr, uint8_t data) {
  switch (addr) {
  case 0x0000: // Control
    control.reg = data;
    tram_addr.nametable_x = control.nametable_x;
    tram_addr.nametable_y = control.nametable_y;
    break;
  case 0x0001: // Mask
    mask.reg = data;
    break;
  case 0x0002: // Status

    break;
  case 0x0003: // OAM Address
    oam_addr = data;
    break;
  case 0x0004: // OAM Data
    pOAM[oam_addr] = data;
    break;
  case 0x0005: // Scroll
    if (address_latch == 0) {
      fine_x = data & 0x07;
      tram_addr.coarse_x = data >> 3;
      address_latch = 1;

    } else {
      tram_addr.fine_y = data & 0x07;
      tram_addr.coarse_y = data >> 3;
      address_latch = 0;
    }

    break;
  case 0x0006: // PPU Address
    if (address_latch == 0) {
      tram_addr.reg = (tram_addr.reg & 0x00FF) | (data << 8);
      address_latch = 1;

    } else {
      tram_addr.reg = (tram_addr.reg & 0xFF00) | data;
      vram_addr = tram_addr;
      address_latch = 0;
    }
    break;
  case 0x0007: // PPU Data
    ppuWrite(vram_addr.reg, data);
    vram_addr.reg += (control.increment_mode ? 32 : 1);
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
    addr &= 0x0FFF;

    if (cart->mirror == Cartridge::MIRROR::VERTICAL) {
      // Vertical
      if (addr >= 0x0000 && addr <= 0x03FF)
        data = nameTable[0][addr & 0x03FF];
      if (addr >= 0x0400 && addr <= 0x07FF)
        data = nameTable[1][addr & 0x03FF];
      if (addr >= 0x0800 && addr <= 0x0BFF)
        data = nameTable[0][addr & 0x03FF];
      if (addr >= 0x0C00 && addr <= 0x0FFF)
        data = nameTable[1][addr & 0x03FF];
    } else if (cart->mirror == Cartridge::MIRROR::HORIZONTAL) {
      // Horizontal
      if (addr >= 0x0000 && addr <= 0x03FF)
        data = nameTable[0][addr & 0x03FF];
      if (addr >= 0x0400 && addr <= 0x07FF)
        data = nameTable[0][addr & 0x03FF];
      if (addr >= 0x0800 && addr <= 0x0BFF)
        data = nameTable[1][addr & 0x03FF];
      if (addr >= 0x0C00 && addr <= 0x0FFF)
        data = nameTable[1][addr & 0x03FF];
    }

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

void PPU2C02::reset() {
  fine_x = 0x00;
  address_latch = 0x00;
  ppu_data_buffer = 0x00;
  scanline = 0;
  cycle = 0;
  bg_next_tile_id = 0x00;
  bg_next_tile_attrib = 0x00;
  bg_next_tile_lsb = 0x00;
  bg_next_tile_msb = 0x00;
  bg_shifter_pattern_lo = 0x0000;
  bg_shifter_pattern_hi = 0x0000;
  bg_shifter_attrib_lo = 0x0000;
  bg_shifter_attrib_hi = 0x0000;
  status.reg = 0x00;
  mask.reg = 0x00;
  control.reg = 0x00;
  vram_addr.reg = 0x0000;
  tram_addr.reg = 0x0000;
}

void PPU2C02::ppuWrite(uint16_t addr, uint8_t data) {
  addr &= 0x3FFF;

  if (cart->ppuWrite(addr, data)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {

    PatternTable[(addr & 0x1000) >> 12][addr & 0x0FFF] = data;

  } else if (addr >= 0x2000 && addr <= 0x3EFF) {
    addr &= 0x0FFF;

    if (cart->mirror == Cartridge::MIRROR::VERTICAL) {
      // Vertical
      if (addr >= 0x0000 && addr <= 0x03FF)
        nameTable[0][addr & 0x03FF] = data;
      if (addr >= 0x0400 && addr <= 0x07FF)
        nameTable[1][addr & 0x03FF] = data;
      if (addr >= 0x0800 && addr <= 0x0BFF)
        nameTable[0][addr & 0x03FF] = data;
      if (addr >= 0x0C00 && addr <= 0x0FFF)
        nameTable[1][addr & 0x03FF] = data;
    } else if (cart->mirror == Cartridge::MIRROR::HORIZONTAL) {
      // Horizontal
      if (addr >= 0x0000 && addr <= 0x03FF)
        nameTable[0][addr & 0x03FF] = data;
      if (addr >= 0x0400 && addr <= 0x07FF)
        nameTable[0][addr & 0x03FF] = data;
      if (addr >= 0x0800 && addr <= 0x0BFF)
        nameTable[1][addr & 0x03FF] = data;
      if (addr >= 0x0C00 && addr <= 0x0FFF)
        nameTable[1][addr & 0x03FF] = data;
    }

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

  // Background rendering ---------------------------------------------------
  auto IncrementScrollX = [&]() {
    if (mask.render_background || mask.render_sprites) {
      if (vram_addr.coarse_x == 31) {
        vram_addr.coarse_x = 0;
        vram_addr.nametable_x = ~vram_addr.nametable_x;
      } else {
        vram_addr.coarse_x++;
      }
    }
  };

  auto IncrementScrollY = [&]() {
    if (mask.render_background || mask.render_sprites) {
      if (vram_addr.fine_y < 7) {
        vram_addr.fine_y++;
      } else {
        vram_addr.fine_y = 0;

        if (vram_addr.coarse_y == 29) {
          vram_addr.coarse_y = 0;
          vram_addr.nametable_y = ~vram_addr.nametable_y;
        } else if (vram_addr.coarse_y == 31) {
          vram_addr.coarse_y = 0;
        } else {
          vram_addr.coarse_y++;
        }
      }
    }
  };

  auto TransferAddressX = [&]() {
    if (mask.render_background || mask.render_sprites) {
      vram_addr.nametable_x = tram_addr.nametable_x;
      vram_addr.coarse_x = tram_addr.coarse_x;
    }
  };

  auto TransferAddressY = [&]() {
    if (mask.render_background || mask.render_sprites) {
      vram_addr.fine_y = tram_addr.fine_y;
      vram_addr.nametable_y = tram_addr.nametable_y;
      vram_addr.coarse_y = tram_addr.coarse_y;
    }
  };

  auto LoadBackgroundShifters = [&]() {
    bg_shifter_pattern_lo = (bg_shifter_pattern_lo & 0xFF00) | bg_next_tile_lsb;
    bg_shifter_pattern_hi = (bg_shifter_pattern_hi & 0xFF00) | bg_next_tile_msb;
    bg_shifter_attrib_lo = (bg_shifter_attrib_lo & 0xFF00) |
                           ((bg_next_tile_attrib & 0b01) ? 0xFF : 0x00);
    bg_shifter_attrib_hi = (bg_shifter_attrib_hi & 0xFF00) |
                           ((bg_next_tile_attrib & 0b10) ? 0xFF : 0x00);
  };

  auto UpdateShifters = [&]() {
    if (mask.render_background) {
      bg_shifter_pattern_lo <<= 1;
      bg_shifter_pattern_hi <<= 1;

      bg_shifter_attrib_lo <<= 1;
      bg_shifter_attrib_hi <<= 1;
    }

    if (mask.render_sprites && cycle >= 1 && cycle < 258) {
      for (int i = 0; i < sprite_count; i++) {
        if (spriteScanline[i].x > 0) {
          spriteScanline[i].x--;
        } else {
          sprite_shifter_pattern_lo[i] <<= 1;
          sprite_shifter_pattern_hi[i] <<= 1;
        }
      }
    }
  };

  if (scanline >= -1 && scanline < 240) {
    if (scanline == 0 && cycle == 0) {
      cycle = 1;
    }

    if (scanline == -1 && cycle == 1) {
      status.vertical_blank = 0;
      status.sprite_zero_hit = 0;
      status.sprite_overflow = 0;
      for (int i = 0; i > sprite_count; i++) {
        sprite_shifter_pattern_lo[i] = 0;
        sprite_shifter_pattern_hi[i] = 0;
      }
    }

    if ((cycle >= 2 && cycle < 258) || (cycle >= 321 && cycle < 338)) {
      UpdateShifters();

      switch ((cycle - 1) % 8) {
      case 0:
        LoadBackgroundShifters();

        bg_next_tile_id = ppuRead(0x2000 | (vram_addr.reg & 0x0FFF));
        break;
      case 2:
        bg_next_tile_attrib = ppuRead(0x23C0 | (vram_addr.nametable_y << 11) |
                                      (vram_addr.nametable_x << 10) |
                                      ((vram_addr.coarse_y >> 2) << 3) |
                                      (vram_addr.coarse_x >> 2));

        if (vram_addr.coarse_y & 0x02)
          bg_next_tile_attrib >>= 4;
        if (vram_addr.coarse_x & 0x02)
          bg_next_tile_attrib >>= 2;
        bg_next_tile_attrib &= 0x03;
        break;

      case 4:
        bg_next_tile_lsb =
            ppuRead((control.pattern_background << 12) +
                    ((uint16_t)bg_next_tile_id << 4) + (vram_addr.fine_y) + 0);

        break;
      case 6:
        bg_next_tile_msb =
            ppuRead((control.pattern_background << 12) +
                    ((uint16_t)bg_next_tile_id << 4) + (vram_addr.fine_y) + 8);
        break;
      case 7:
        IncrementScrollX();
        break;
      }
    }

    if (cycle == 256) {
      IncrementScrollY();
    }

    if (cycle == 257) {
      LoadBackgroundShifters();
      TransferAddressX();
    }

    if (cycle == 338 || cycle == 340) {
      bg_next_tile_id = ppuRead(0x2000 | (vram_addr.reg & 0x0FFF));
    }

    if (scanline == -1 && cycle >= 280 && cycle < 305) {
      TransferAddressY();
    }
  }

  // Foreground rendering
  // --------------------------------------------------------------------------
  if (cycle == 257 && scanline >= 0) {
    std::memset(spriteScanline, 0xFF, 8 * sizeof(ObjectAttributeMemory));
    sprite_count = 0;
    uint8_t nOAMEntry = 0;
    bSpriteZeroHitPossible = false;
    while (nOAMEntry < 64 && sprite_count < 9) {
      int16_t diff = ((int16_t)scanline - (int16_t)OAM[nOAMEntry].y);
      if (diff >= 0 && diff < (control.sprite_size ? 16 : 8)) {
        if (sprite_count < 8) {

          if (nOAMEntry == 0) {
            bSpriteZeroHitPossible = true;
          }
          std::memcpy(&spriteScanline[sprite_count], &OAM[nOAMEntry],
                      sizeof(ObjectAttributeMemory));
        }

        sprite_count++;
      }
      nOAMEntry++;
    }
    status.sprite_overflow = (sprite_count > 8);
  }

  if (cycle == 340) {
    for (uint8_t i = 0; i < sprite_count; i++) {
      uint8_t sprite_pattern_bits_lo, sprite_pattern_bits_hi;
      uint16_t sprite_pattern_addr_lo, sprite_pattern_addr_hi;

      if (!control.sprite_size) {
        // 8x8 Sprite Mode
        if (!(spriteScanline[i].attribute & 0x80)) {
          // Sprite Not Flipped Vertically
          sprite_pattern_addr_lo = (control.pattern_sprite << 12) |
                                   (spriteScanline[i].tile_id << 4) |
                                   (scanline - spriteScanline[i].y);
        } else {
          // Sprite Vertically Flipped
          sprite_pattern_addr_lo = (control.pattern_sprite << 12) |
                                   (spriteScanline[i].tile_id << 4) |
                                   (7 - (scanline - spriteScanline[i].y));
        }

      } else {
        // 8x16 Sprite Mode
        //
        if (!(spriteScanline[i].attribute & 0x80)) {
          // Sprite Not Flipped Vertically
          if (scanline - spriteScanline[i].y < 8) {

            // Reading Top half Tile
            sprite_pattern_addr_lo =
                ((spriteScanline[i].tile_id & 0x01) << 12) |
                ((spriteScanline[i].tile_id & 0xFE) << 4) |
                ((scanline - spriteScanline[i].y) & 0x07);

          } else {
            // Reading bottom half Tile
            sprite_pattern_addr_lo =
                ((spriteScanline[i].tile_id & 0x01) << 12) |
                (((spriteScanline[i].tile_id & 0xFE) + 1) << 4) |
                ((scanline - spriteScanline[i].y) & 0x07);
          }
        } else {

          // Sprite Vertically Flipped
          if (scanline - spriteScanline[i].y < 8) {
            // Reading Top half Tile
            sprite_pattern_addr_lo =
                ((spriteScanline[i].tile_id & 0x01) << 12) |
                ((spriteScanline[i].tile_id & 0xFE) << 4) |
                ((7 - (scanline - spriteScanline[i].y)) & 0x07);

          } else {
            // Reading bottom half Tile
            sprite_pattern_addr_lo =
                ((spriteScanline[i].tile_id & 0x01) << 12) |
                (((spriteScanline[i].tile_id & 0xFE) + 1) << 4) |
                ((7 - (scanline - spriteScanline[i].y)) & 0x07);
          }
        }
      }
      sprite_pattern_addr_hi = sprite_pattern_addr_lo + 8;
      sprite_pattern_bits_lo = ppuRead(sprite_pattern_addr_lo);
      sprite_pattern_bits_hi = ppuRead(sprite_pattern_addr_hi);

      if (spriteScanline[i].attribute & 0x40) {
        // This little lambda function "flips" a byte
        // so 0b11100000 becomes 0b00000111. It's very
        // clever, and stolen completely from here:
        // https://stackoverflow.com/a/2602885
        auto flipbyte = [](uint8_t b) {
          b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
          b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
          b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
          return b;
        };

        // Flip Patterns Horizontally
        sprite_pattern_bits_lo = flipbyte(sprite_pattern_bits_lo);
        sprite_pattern_bits_hi = flipbyte(sprite_pattern_bits_hi);
      }

      sprite_shifter_pattern_lo[i] = sprite_pattern_bits_lo;
      sprite_shifter_pattern_hi[i] = sprite_pattern_bits_hi;
    }
  }

  if (scanline == 240) {
  }

  if (scanline >= 241 && scanline < 261) {
    if (scanline == 241 && cycle == 1) {
      status.vertical_blank = 1;
      if (control.enable_nmi)
        nmi = true;
    }
  }

  uint8_t bg_pixel = 0x00;   // The 2-bit pixel to be rendered
  uint8_t bg_palette = 0x00; // The 3-bit index of the palette the pixel indexes

  if (mask.render_background) {
    uint16_t bit_mux = 0x8000 >> fine_x;

    uint8_t p0_pixel = (bg_shifter_pattern_lo & bit_mux) > 0;
    uint8_t p1_pixel = (bg_shifter_pattern_hi & bit_mux) > 0;
    bg_pixel = (p1_pixel << 1) | p0_pixel;

    uint8_t bg_pal0 = (bg_shifter_attrib_lo & bit_mux) > 0;
    uint8_t bg_pal1 = (bg_shifter_attrib_hi & bit_mux) > 0;
    bg_palette = (bg_pal1 << 1) | bg_pal0;
  }

  uint8_t fg_pixel = 0x00;
  uint8_t fg_palette = 0x00;
  uint8_t fg_priority = 0x00;

  if (mask.render_sprites) {

    bSpriteZeroBeingRendered = false;

    for (uint8_t i = 0; i < sprite_count; i++) {
      if (spriteScanline[i].x == 0) {
        uint8_t fg_pixel_lo = (sprite_shifter_pattern_lo[i] & 0x80) > 0;
        uint8_t fg_pixel_hi = (sprite_shifter_pattern_hi[i] & 0x80) > 0;
        fg_pixel = (fg_pixel_hi << 1) | fg_pixel_lo;

        fg_palette = (spriteScanline[i].attribute & 0x03) + 0x04;
        fg_priority = (spriteScanline[i].attribute & 0x20) == 0;

        if (fg_pixel != 0) {
          if (i == 0) { // is this sprite zero?
            bSpriteZeroBeingRendered = true;
          }
          break;
        }
      }
    }
  }

  uint8_t pixel = 0x00;   // Final pixel
  uint8_t palette = 0x00; // Final palette

  if (bg_pixel == 0 && fg_pixel == 0) {
    pixel = 0x00;
    palette = 0x00;
  }

  else if (bg_pixel == 0 && fg_pixel > 0) {
    // The background pixel is transparent
    // The foreground pixel is visible
    pixel = fg_pixel;
    palette = fg_palette;

  } else if (bg_pixel > 0 && fg_pixel == 0) {
    // The background pixel is visible
    // The foreground pixel is transparent
    pixel = bg_pixel;
    palette = bg_palette;
  }

  else if (bg_pixel > 0 && fg_pixel > 0) {
    if (fg_priority) {
      pixel = fg_pixel;
      palette = fg_palette;
    } else {
      pixel = bg_pixel;
      palette = bg_palette;
    }

    if (bSpriteZeroHitPossible && bSpriteZeroBeingRendered) {
      if (mask.render_background & mask.render_sprites) {
        if (~(mask.render_background_left | mask.render_sprites_left)) {
          if (cycle >= 9 && cycle < 258) {
            status.sprite_zero_hit = 1;
          }
        } else {
          if (cycle >= 1 && cycle < 258) {
            status.sprite_zero_hit = 1;
          }
        }
      }
    }
  }

  if (cycle - 1 >= 0 && cycle - 1 < 256 && scanline >= 0 && scanline < 240) {
    sprScreen[scanline * 256 + (cycle - 1)] =
        GetColourFromPaletteRam(palette, pixel);
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
