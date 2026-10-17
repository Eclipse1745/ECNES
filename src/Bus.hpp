#pragma once
#include "Cartridge.hpp"
#include "M6502.hpp"
#include "PPU2C02.hpp"
#include <cstdint>
#include <memory>

class Bus {
public:
  Bus();

public: // Devices on Bus
  M6502 cpu;
  PPU2C02 ppu;
  std::array<uint8_t, 1024 * 64> cpuRam = {};

  std::shared_ptr<Cartridge> cart;

  uint8_t controller[2];

public:
  void cpuWrite(uint16_t addr, uint8_t data);
  uint8_t cpuRead(uint16_t addr, bool bReadOnly = false);

public: // System Interface
  void InsertCartridge(const std::shared_ptr<Cartridge> &cartridge);
  void reset();
  void clock();

private:
  uint32_t nSystemClockCounter = 0;

  uint8_t controller_state[2];

  uint8_t dma_page = 0x00;
  uint8_t dma_addr = 0x00;
  uint8_t dma_data = 0x00;

  bool dma_transfer = false;
  bool dma_dummy = true;
};
