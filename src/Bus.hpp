#pragma once
#include "APU2A03.hpp"
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
  APU2A03 apu;

  uint8_t cpuRam[2048];

  std::shared_ptr<Cartridge> cart;

  uint8_t controller[2];

public:
  void cpuWrite(uint16_t addr, uint8_t data);
  uint8_t cpuRead(uint16_t addr, bool bReadOnly = false);

  double dAudioSample = 0.0f;
  void SetSampleFrequency(uint32_t sample_rate);

public: // System Interface
  void InsertCartridge(const std::shared_ptr<Cartridge> &cartridge);
  void reset();
  bool clock();

private:
  uint32_t nSystemClockCounter = 0;

  uint8_t controller_state[2];

  uint8_t dma_page = 0x00;
  uint8_t dma_addr = 0x00;
  uint8_t dma_data = 0x00;

  bool dma_transfer = false;
  bool dma_dummy = true;

  double dAudioTimePerSystemSample = 0.0f;
  double dAudioTimePerNESClock = 0.0f;
  double dAudioTime = 0.0f;
};
