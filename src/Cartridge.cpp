#include "Cartridge.hpp"
#include "Mapper000.hpp"
#include <fstream>
#include <iostream>

Cartridge::Cartridge(const std::string &filename) {
  // iNES Format Header
  struct sHeader {
    char name[4];
    uint8_t prg_rom_chunks;
    uint8_t chr_rom_chunks;
    uint8_t mapper1;
    uint8_t mapper2;
    uint8_t prg_ram_size;
    uint8_t tv_system1;
    uint8_t tv_system2;
    char unused[5];
  } header;

  std::ifstream inputfs;
  inputfs.open(filename, std::ifstream::binary);
  if (!inputfs.is_open()) {
    std::cerr << "Failed to open ROM: " << filename << '\n';
    return;
  }
  if (inputfs.is_open()) {
    inputfs.read((char *)&header, sizeof(sHeader));

    if (header.mapper1 & 0x04)
      inputfs.seekg(512, std::ios_base::cur);

    // Extract Mapper ID;
    nMapperID = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);

    uint8_t nFileType = 1;

    if (nFileType == 0) {
    }
    if (nFileType == 1) {
      nPRGBanks = header.prg_rom_chunks;
      vPRGMemory.resize(nPRGBanks * 16384);
      inputfs.read((char *)vPRGMemory.data(), vPRGMemory.size());

      nCHRBanks = header.chr_rom_chunks;
      vCHRMemory.resize(nCHRBanks * 8192);
      inputfs.read((char *)vCHRMemory.data(), vCHRMemory.size());
    }
    if (nFileType == 2) {
    }

    // Load appropriate mapper
    switch (nMapperID) {
    case 0:
      pMapper = std::make_shared<Mapper000>(nPRGBanks, nCHRBanks);
      break;
    }
    inputfs.close();
  }
}

bool Cartridge::cpuRead(uint16_t addr, uint8_t &data) {
  uint32_t mapped_addr = 0;
  if (pMapper->cpuMapRead(addr, mapped_addr)) {
    data = vPRGMemory[mapped_addr];
    return true;
  } else
    return false;
}

bool Cartridge::cpuWrite(uint16_t addr, uint8_t data) {
  uint32_t mapped_addr = 0;
  if (pMapper->cpuMapWrite(addr, mapped_addr)) {
    vPRGMemory[mapped_addr] = data;
    return true;
  } else
    return false;
}

bool Cartridge::ppuRead(uint16_t addr, uint8_t &data) {
  uint32_t mapped_addr = 0;
  if (pMapper->ppuMapRead(addr, mapped_addr)) {
    data = vCHRMemory[mapped_addr];
    return true;
  } else
    return false;
}

bool Cartridge::ppuWrite(uint16_t addr, uint8_t data) {
  uint32_t mapped_addr = 0;
  if (pMapper->ppuMapRead(addr, mapped_addr)) {
    vCHRMemory[mapped_addr] = data;
    return true;
  } else
    return false;
}
