#include "Bus.hpp"

Bus::Bus() {
  for (auto &i : cpuRam) {
    i = 0x00;
  }
  cpu.ConnectBus(this);
}

void Bus::reset() {

  cpu.reset();
  ppu.reset();
  nSystemClockCounter = 0;
}

uint8_t Bus::cpuRead(uint16_t addr, bool bReadOnly) {
  uint8_t data = 0x00;
  if (cart->cpuRead(addr, data)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    data = cpuRam[addr & 0x07FF];
  } else if (addr >= 0x2000 && addr <= 0x3FFF) {
    data = ppu.cpuRead(addr & 0x0007, bReadOnly);
  } else if (addr >= 0x4016 && addr <= 0x4017) {
    data = (controller_state[addr & 0x0001] & 0x80) > 0;
    controller_state[addr & 0x0001] <<= 1;
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
  } else if (addr >= 0x4016 && addr <= 0x4017) {
    controller_state[addr & 0x0001] = controller[addr & 0x0001];
  } else if (addr == 0x4014) {
    dma_page = data;
    dma_addr = 0x00;
    dma_transfer = true;
  }
}

void Bus::InsertCartridge(const std::shared_ptr<Cartridge> &cartridge) {
  this->cart = cartridge;
  ppu.ConnectCartridge(cartridge);
}

void Bus::clock() {
  ppu.clock();
  if (nSystemClockCounter % 3 == 0) {
    if (dma_transfer) {
      if (dma_dummy) {
        if (nSystemClockCounter % 2 == 1) {
          dma_dummy = false;
        }

      } else {
        if (nSystemClockCounter % 2 == 0) {
          dma_data = cpuRead(dma_page << 8 | dma_addr);

        } else {
          ppu.pOAM[dma_addr] = dma_data;
          dma_addr++;
          if (dma_addr == 0x00) {
            dma_transfer = false;
            dma_dummy = true;
          }
        }
      }
    } else {
      cpu.clock();
    }
  }

  if (ppu.nmi) {
    ppu.nmi = false;
    cpu.callNMI();
  }

  nSystemClockCounter++;
}
