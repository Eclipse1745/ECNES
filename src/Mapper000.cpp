#include "Mapper000.hpp"

Mapper000::Mapper000(uint8_t prgBanks, uint8_t chrBanks)
    : Mapper(prgBanks, chrBanks) {
}

bool Mapper000::cpuMapRead(uint16_t addr, uint32_t &mapped_addr) {
  if (addr >= 0x8000 && addr <= 0xFFFF) {
    mapped_addr = addr & (nPrgBanks > 1 ? 0x7FFF : 0x3FFF);
    return true;
  }

  return false;
}

bool Mapper000::cpuMapWrite(uint16_t addr, uint32_t &mapped_addr) {
  if (addr >= 0x8000 && addr <= 0xFFFF) {
    mapped_addr = addr & (nPrgBanks > 1 ? 0x7FFF : 0x3FFF);
    return true;
  }

  return false;
}

bool Mapper000::ppuMapRead(uint16_t addr, uint32_t &mapped_addr) {
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    mapped_addr = addr;
    return true;
  }

  return false;
}

bool Mapper000::ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) {
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    if (nChrBanks == 0) {
      mapped_addr = addr;
      return true;
    }
  }

  return false;
}
