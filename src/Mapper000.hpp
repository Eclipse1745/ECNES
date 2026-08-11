#pragma once
#include "Mapper.hpp"

class Mapper000 : public Mapper {
public:
  Mapper000(uint8_t prgBanks, uint8_t chrBanks);

  bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
  bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr) override;
  bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
  bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override;
};
