#include "Bus.hpp"

Bus::Bus() {
  for (auto &i : cpuRam) {
    i = 0x00;
  }
  cpu.ConnectBus(this);
}

void Bus::reset() {
  cpu.reset();
  nSystemClockCounter = 0;
}

uint8_t Bus::cpuRead(uint16_t addr, bool bReadOnly) {
  uint8_t data = 0x00;
  if (cart->cpuRead(addr, data)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    data = cpuRam[addr & 0x07FF];
  } else if (addr >= 0x2000 && addr <= 0x3FFF) {
    data = ppu.cpuRead(addr & 0x0007, bReadOnly);
  }

  return data;
}

void Bus::cpuWrite(uint16_t addr, uint8_t data) {

  if (cart->cpuWrite(addr, data)) {
  }

  else if (addr >= 0x0000 && addr <= 0x1FFF) {
    cpuRam[addr & 0x07FF] = data;
  } else if (addr >= 0x2000 && addr <= 0x3FFF) {
    ppu.cpuWrite(addr & 0x0007, data);
  }
}

void Bus::InsertCartridge(const std::shared_ptr<Cartridge> &cartridge) {
  this->cart = cartridge;
  ppu.ConnectCartridge(cartridge);
}

void Bus::clock() {
  ppu.clock();
  if (nSystemClockCounter % 3 == 0) {
    cpu.clock();
  }

  if (ppu.nmi) {
    ppu.nmi = false;
    cpu.callNMI();
  }
  nSystemClockCounter++;
}
