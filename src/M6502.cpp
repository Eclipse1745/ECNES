#include "M6502.hpp"
#include "Bus.hpp"
#include <cstdint>

void M6502::reset() {
  A = X = Y = 0;
  SP = 0xFD;
  status = 0x00 | U | I | B;

  uint16_t lo = read(rstVectorL);
  uint16_t hi = read(rstVectorH);
  PC = (hi << 8) | lo;

  fetched = 0x00;
  addr_rel = 0x0000;
  addr_abs = 0x0000;
  cycles = 0;
}

void M6502::initLookupTable() {

  lookup = {{// 0x00 - 0x0F
             {"BRK", &M6502::BRK, &M6502::IMP, 7},
             {"ORA", &M6502::ORA, &M6502::IZX, 6},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"???", &M6502::NOP, &M6502::IMP, 3},
             {"ORA", &M6502::ORA, &M6502::ZP0, 3},
             {"ASL", &M6502::ASL, &M6502::ZP0, 5},
             {"???", &M6502::XXX, &M6502::IMP, 5},
             {"PHP", &M6502::PHP, &M6502::IMP, 3},
             {"ORA", &M6502::ORA, &M6502::IMM, 2},
             {"ASL", &M6502::ASL, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"ORA", &M6502::ORA, &M6502::ABS, 4},
             {"ASL", &M6502::ASL, &M6502::ABS, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},

             // 0x10 - 0x1F
             {"BPL", &M6502::BPL, &M6502::REL, 2},
             {"ORA", &M6502::ORA, &M6502::IZY, 5},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"ORA", &M6502::ORA, &M6502::ZPX, 4},
             {"ASL", &M6502::ASL, &M6502::ZPX, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},
             {"CLC", &M6502::CLC, &M6502::IMP, 2},
             {"ORA", &M6502::ORA, &M6502::ABY, 4},
             {"???", &M6502::NOP, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 7},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"ORA", &M6502::ORA, &M6502::ABX, 4},
             {"ASL", &M6502::ASL, &M6502::ABX, 7},
             {"???", &M6502::XXX, &M6502::IMP, 7},

             // 0x20 - 0x2F
             {"JSR", &M6502::JSR, &M6502::ABS, 6},
             {"AND", &M6502::AND, &M6502::IZX, 6},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"BIT", &M6502::BIT, &M6502::ZP0, 3},
             {"AND", &M6502::AND, &M6502::ZP0, 3},
             {"ROL", &M6502::ROL, &M6502::ZP0, 5},
             {"???", &M6502::XXX, &M6502::IMP, 5},
             {"PLP", &M6502::PLP, &M6502::IMP, 4},
             {"AND", &M6502::AND, &M6502::IMM, 2},
             {"ROL", &M6502::ROL, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"BIT", &M6502::BIT, &M6502::ABS, 4},
             {"AND", &M6502::AND, &M6502::ABS, 4},
             {"ROL", &M6502::ROL, &M6502::ABS, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},

             // 0x30 - 0x3F
             {"BMI", &M6502::BMI, &M6502::REL, 2},
             {"AND", &M6502::AND, &M6502::IZY, 5},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"AND", &M6502::AND, &M6502::ZPX, 4},
             {"ROL", &M6502::ROL, &M6502::ZPX, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},
             {"SEC", &M6502::SEC, &M6502::IMP, 2},
             {"AND", &M6502::AND, &M6502::ABY, 4},
             {"???", &M6502::NOP, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 7},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"AND", &M6502::AND, &M6502::ABX, 4},
             {"ROL", &M6502::ROL, &M6502::ABX, 7},
             {"???", &M6502::XXX, &M6502::IMP, 7},

             // 0x40 - 0x4F
             {"RTI", &M6502::RTI, &M6502::IMP, 6},
             {"EOR", &M6502::EOR, &M6502::IZX, 6},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"???", &M6502::NOP, &M6502::IMP, 3},
             {"EOR", &M6502::EOR, &M6502::ZP0, 3},
             {"LSR", &M6502::LSR, &M6502::ZP0, 5},
             {"???", &M6502::XXX, &M6502::IMP, 5},
             {"PHA", &M6502::PHA, &M6502::IMP, 3},
             {"EOR", &M6502::EOR, &M6502::IMM, 2},
             {"LSR", &M6502::LSR, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"JMP", &M6502::JMP, &M6502::ABS, 3},
             {"EOR", &M6502::EOR, &M6502::ABS, 4},
             {"LSR", &M6502::LSR, &M6502::ABS, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},

             // 0x50 - 0x5F
             {"BVC", &M6502::BVC, &M6502::REL, 2},
             {"EOR", &M6502::EOR, &M6502::IZY, 5},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"EOR", &M6502::EOR, &M6502::ZPX, 4},
             {"LSR", &M6502::LSR, &M6502::ZPX, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},
             {"CLI", &M6502::CLI, &M6502::IMP, 2},
             {"EOR", &M6502::EOR, &M6502::ABY, 4},
             {"???", &M6502::NOP, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 7},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"EOR", &M6502::EOR, &M6502::ABX, 4},
             {"LSR", &M6502::LSR, &M6502::ABX, 7},
             {"???", &M6502::XXX, &M6502::IMP, 7},

             // 0x60 - 0x6F
             {"RTS", &M6502::RTS, &M6502::IMP, 6},
             {"ADC", &M6502::ADC, &M6502::IZX, 6},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"???", &M6502::NOP, &M6502::IMP, 3},
             {"ADC", &M6502::ADC, &M6502::ZP0, 3},
             {"ROR", &M6502::ROR, &M6502::ZP0, 5},
             {"???", &M6502::XXX, &M6502::IMP, 5},
             {"PLA", &M6502::PLA, &M6502::IMP, 4},
             {"ADC", &M6502::ADC, &M6502::IMM, 2},
             {"ROR", &M6502::ROR, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"JMP", &M6502::JMP, &M6502::IND, 5},
             {"ADC", &M6502::ADC, &M6502::ABS, 4},
             {"ROR", &M6502::ROR, &M6502::ABS, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},

             // 0x70 - 0x7F
             {"BVS", &M6502::BVS, &M6502::REL, 2},
             {"ADC", &M6502::ADC, &M6502::IZY, 5},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"ADC", &M6502::ADC, &M6502::ZPX, 4},
             {"ROR", &M6502::ROR, &M6502::ZPX, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},
             {"SEI", &M6502::SEI, &M6502::IMP, 2},
             {"ADC", &M6502::ADC, &M6502::ABY, 4},
             {"???", &M6502::NOP, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 7},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"ADC", &M6502::ADC, &M6502::ABX, 4},
             {"ROR", &M6502::ROR, &M6502::ABX, 7},
             {"???", &M6502::XXX, &M6502::IMP, 7},

             // 0x80 - 0x8F
             {"???", &M6502::NOP, &M6502::IMP, 2},
             {"STA", &M6502::STA, &M6502::IZX, 6},
             {"???", &M6502::NOP, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 6},
             {"STY", &M6502::STY, &M6502::ZP0, 3},
             {"STA", &M6502::STA, &M6502::ZP0, 3},
             {"STX", &M6502::STX, &M6502::ZP0, 3},
             {"???", &M6502::XXX, &M6502::IMP, 3},
             {"DEY", &M6502::DEY, &M6502::IMP, 2},
             {"???", &M6502::NOP, &M6502::IMP, 2},
             {"TXA", &M6502::TXA, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"STY", &M6502::STY, &M6502::ABS, 4},
             {"STA", &M6502::STA, &M6502::ABS, 4},
             {"STX", &M6502::STX, &M6502::ABS, 4},
             {"???", &M6502::XXX, &M6502::IMP, 4},

             // 0x90 - 0x9F
             {"BCC", &M6502::BCC, &M6502::REL, 2},
             {"STA", &M6502::STA, &M6502::IZY, 6},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 6},
             {"STY", &M6502::STY, &M6502::ZPX, 4},
             {"STA", &M6502::STA, &M6502::ZPX, 4},
             {"STX", &M6502::STX, &M6502::ZPY, 4},
             {"???", &M6502::XXX, &M6502::IMP, 4},
             {"TYA", &M6502::TYA, &M6502::IMP, 2},
             {"STA", &M6502::STA, &M6502::ABY, 5},
             {"TXS", &M6502::TXS, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 5},
             {"???", &M6502::NOP, &M6502::IMP, 5},
             {"STA", &M6502::STA, &M6502::ABX, 5},
             {"???", &M6502::XXX, &M6502::IMP, 5},
             {"???", &M6502::XXX, &M6502::IMP, 5},

             // 0xA0 - 0xAF
             {"LDY", &M6502::LDY, &M6502::IMM, 2},
             {"LDA", &M6502::LDA, &M6502::IZX, 6},
             {"LDX", &M6502::LDX, &M6502::IMM, 2},
             {"???", &M6502::XXX, &M6502::IMP, 6},
             {"LDY", &M6502::LDY, &M6502::ZP0, 3},
             {"LDA", &M6502::LDA, &M6502::ZP0, 3},
             {"LDX", &M6502::LDX, &M6502::ZP0, 3},
             {"???", &M6502::XXX, &M6502::IMP, 3},
             {"TAY", &M6502::TAY, &M6502::IMP, 2},
             {"LDA", &M6502::LDA, &M6502::IMM, 2},
             {"TAX", &M6502::TAX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"LDY", &M6502::LDY, &M6502::ABS, 4},
             {"LDA", &M6502::LDA, &M6502::ABS, 4},
             {"LDX", &M6502::LDX, &M6502::ABS, 4},
             {"???", &M6502::XXX, &M6502::IMP, 4},

             // 0xB0 - 0xBF
             {"BCS", &M6502::BCS, &M6502::REL, 2},
             {"LDA", &M6502::LDA, &M6502::IZY, 5},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 5},
             {"LDY", &M6502::LDY, &M6502::ZPX, 4},
             {"LDA", &M6502::LDA, &M6502::ZPX, 4},
             {"LDX", &M6502::LDX, &M6502::ZPY, 4},
             {"???", &M6502::XXX, &M6502::IMP, 4},
             {"CLV", &M6502::CLV, &M6502::IMP, 2},
             {"LDA", &M6502::LDA, &M6502::ABY, 4},
             {"TSX", &M6502::TSX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 4},
             {"LDY", &M6502::LDY, &M6502::ABX, 4},
             {"LDA", &M6502::LDA, &M6502::ABX, 4},
             {"LDX", &M6502::LDX, &M6502::ABY, 4},
             {"???", &M6502::XXX, &M6502::IMP, 4},

             // 0xC0 - 0xCF
             {"CPY", &M6502::CPY, &M6502::IMM, 2},
             {"CMP", &M6502::CMP, &M6502::IZX, 6},
             {"???", &M6502::NOP, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"CPY", &M6502::CPY, &M6502::ZP0, 3},
             {"CMP", &M6502::CMP, &M6502::ZP0, 3},
             {"DEC", &M6502::DEC, &M6502::ZP0, 5},
             {"???", &M6502::XXX, &M6502::IMP, 5},
             {"INY", &M6502::INY, &M6502::IMP, 2},
             {"CMP", &M6502::CMP, &M6502::IMM, 2},
             {"DEX", &M6502::DEX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"CPY", &M6502::CPY, &M6502::ABS, 4},
             {"CMP", &M6502::CMP, &M6502::ABS, 4},
             {"DEC", &M6502::DEC, &M6502::ABS, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},

             // 0xD0 - 0xDF
             {"BNE", &M6502::BNE, &M6502::REL, 2},
             {"CMP", &M6502::CMP, &M6502::IZY, 5},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"CMP", &M6502::CMP, &M6502::ZPX, 4},
             {"DEC", &M6502::DEC, &M6502::ZPX, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},
             {"CLD", &M6502::CLD, &M6502::IMP, 2},
             {"CMP", &M6502::CMP, &M6502::ABY, 4},
             {"NOP", &M6502::NOP, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 7},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"CMP", &M6502::CMP, &M6502::ABX, 4},
             {"DEC", &M6502::DEC, &M6502::ABX, 7},
             {"???", &M6502::XXX, &M6502::IMP, 7},

             // 0xE0 - 0xEF
             {"CPX", &M6502::CPX, &M6502::IMM, 2},
             {"SBC", &M6502::SBC, &M6502::IZX, 6},
             {"???", &M6502::NOP, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"CPX", &M6502::CPX, &M6502::ZP0, 3},
             {"SBC", &M6502::SBC, &M6502::ZP0, 3},
             {"INC", &M6502::INC, &M6502::ZP0, 5},
             {"???", &M6502::XXX, &M6502::IMP, 5},
             {"INX", &M6502::INX, &M6502::IMP, 2},
             {"SBC", &M6502::SBC, &M6502::IMM, 2},
             {"NOP", &M6502::NOP, &M6502::IMP, 2},
             {"???", &M6502::SBC, &M6502::IMP, 2},
             {"CPX", &M6502::CPX, &M6502::ABS, 4},
             {"SBC", &M6502::SBC, &M6502::ABS, 4},
             {"INC", &M6502::INC, &M6502::ABS, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},

             // 0xF0 - 0xFF
             {"BEQ", &M6502::BEQ, &M6502::REL, 2},
             {"SBC", &M6502::SBC, &M6502::IZY, 5},
             {"???", &M6502::XXX, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 8},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"SBC", &M6502::SBC, &M6502::ZPX, 4},
             {"INC", &M6502::INC, &M6502::ZPX, 6},
             {"???", &M6502::XXX, &M6502::IMP, 6},
             {"SED", &M6502::SED, &M6502::IMP, 2},
             {"SBC", &M6502::SBC, &M6502::ABY, 4},
             {"NOP", &M6502::NOP, &M6502::IMP, 2},
             {"???", &M6502::XXX, &M6502::IMP, 7},
             {"???", &M6502::NOP, &M6502::IMP, 4},
             {"SBC", &M6502::SBC, &M6502::ABX, 4},
             {"INC", &M6502::INC, &M6502::ABX, 7},
             {"???", &M6502::XXX, &M6502::IMP, 7}}};
}

M6502::M6502() {
  initLookupTable();
}

uint8_t M6502::read(uint16_t addr) {
  return bus->cpuRead(addr, false);
}

void M6502::write(uint16_t addr, uint8_t data) {
  bus->cpuWrite(addr, data);
}
uint8_t M6502::fetch() {

  if (!(lookup[opcode].addrmode == &M6502::IMP)) {
    fetched = read(addr_abs);
  }

  return fetched;
}

uint8_t M6502::getFlag(FLAGS6502 f) {
  return ((status & f) > 0) ? 1 : 0;
}

void M6502::setFlag(FLAGS6502 f, bool v) {
  if (v) {
    status |= f;
  } else {
    status &= ~f;
  }
}

void M6502::StackPush(uint8_t data) {
  write(0x0100 + SP, data);
  SP--;
}

uint8_t M6502::StackPull() {
  SP++;
  return read(0x0100 + SP);
}

uint8_t M6502::clock() {

  if (cycles == 0) {
    opcode = read(PC);
    PC++;
    cycles = lookup[opcode].cycles;
    uint8_t additional_cycle1 = (this->*lookup[opcode].addrmode)();
    uint8_t additional_cycle2 = (this->*lookup[opcode].operate)();
    cycles += (additional_cycle1 & additional_cycle2);
  }

  cycles--;
  return 1;
}

// === Addressing Modes ===
uint8_t M6502::IMP() {
  fetched = A;
  return 0;
}

uint8_t M6502::IMM() {
  addr_abs = PC;
  PC++;
  return 0;
}

uint8_t M6502::ZP0() {
  addr_abs = read(PC++);
  return 0;
}

uint8_t M6502::ZPX() {
  addr_abs = (read(PC) + X) & 0x00FF;
  PC++;
  addr_abs &= 0x00FF;
  return 0;
}

uint8_t M6502::ZPY() {

  addr_abs = (read(PC) + Y) & 0x00FF;
  PC++;
  addr_abs &= 0x00FF;
  return 0;
}

uint8_t M6502::REL() {
  addr_rel = read(PC++);
  if (addr_rel & 0x80) {
    addr_rel |= 0xFF00;
  }
  return 0;
}

uint8_t M6502::ABS() {
  uint16_t lo = read(PC++);
  uint16_t hi = read(PC++);

  addr_abs = (hi << 8) | lo;
  return 0;
}

uint8_t M6502::ABX() {
  uint16_t lo = read(PC++);
  uint16_t hi = read(PC++);

  addr_abs = (hi << 8) | lo;
  addr_abs += X;

  if ((addr_abs & 0xFF00) != (hi << 8)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t M6502::ABY() {
  uint16_t lo = read(PC++);
  uint16_t hi = read(PC++);

  addr_abs = (hi << 8) | lo;
  addr_abs += Y;

  if ((addr_abs & 0xFF00) != (hi << 8)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t M6502::IND() {
  uint16_t lo = read(PC++);
  uint16_t hi = read(PC++);

  uint16_t ptr = (hi << 8) | lo;

  if (lo == 0x00FF) {
    addr_abs = (read(ptr & 0xFF00) << 8) | read(ptr);
  } else {
    addr_abs = (read(ptr + 1) << 8) | read(ptr);
  }

  return 0;
}

uint8_t M6502::IZX() {
  uint16_t arg = read(PC++);
  uint16_t lo = read((arg + X) & 0x00FF);
  uint16_t hi = read((arg + X + 1) & 0x00FF);

  addr_abs = (hi << 8) | lo;

  return 0;
}

uint8_t M6502::IZY() {
  uint16_t arg = read(PC++);

  uint16_t lo = read(arg & 0x00FF);
  uint16_t hi = read((arg + 1) & 0x00FF);

  addr_abs = (hi << 8) | lo;
  addr_abs += Y;

  if ((addr_abs & 0xFF00) != (hi << 8)) {
    return 1;
  } else {
    return 0;
  }
}

// === Instructions ===
// Load/Store Operations
uint8_t M6502::LDA() {
  A = fetch();
  setFlag(Z, A == 0);
  setFlag(N, A & 0x80);
  return 1;
}
uint8_t M6502::LDX() {
  X = fetch();
  setFlag(Z, X == 0);
  setFlag(N, X & 0x80);
  return 1;
}
uint8_t M6502::LDY() {
  Y = fetch();
  setFlag(Z, Y == 0);
  setFlag(N, Y & 0x80);
  return 1;
}
uint8_t M6502::STA() {
  write(addr_abs, A);
  return 0;
}
uint8_t M6502::STX() {
  write(addr_abs, X);
  return 0;
}
uint8_t M6502::STY() {
  write(addr_abs, Y);
  return 0;
}

// Register Transfers
uint8_t M6502::TAX() {
  X = A;
  setFlag(Z, X == 0);
  setFlag(N, X & 0x80);
  return 0;
}
uint8_t M6502::TAY() {
  Y = A;
  setFlag(Z, Y == 0);
  setFlag(N, Y & 0x80);
  return 0;
}

uint8_t M6502::TXA() {
  A = X;
  setFlag(Z, A == 0);
  setFlag(N, A & 0x80);
  return 0;
}

uint8_t M6502::TYA() {
  A = Y;
  setFlag(Z, A == 0);
  setFlag(N, A & 0x80);
  return 0;
}

// Stack Operations
uint8_t M6502::TSX() {
  X = SP;
  setFlag(Z, X == 0);
  setFlag(N, X & 0x80);
  return 0;
}
uint8_t M6502::TXS() {
  SP = X;
  return 0;
}
uint8_t M6502::PHA() {
  StackPush(A);
  return 0;
}
uint8_t M6502::PHP() {
  StackPush(status | B | U);
  return 0;
}
uint8_t M6502::PLA() {
  A = StackPull();
  setFlag(Z, A == 0);
  setFlag(N, A & 0x80);
  return 0;
}
uint8_t M6502::PLP() {
  status = StackPull();

  status &= ~B;
  status |= U;

  return 0;
}

// Logical
uint8_t M6502::AND() {
  A &= fetch();
  setFlag(Z, A == 0);
  setFlag(N, A & 0x80);
  return 1;
}

uint8_t M6502::EOR() {
  A ^= fetch();
  setFlag(Z, A == 0);
  setFlag(N, A & 0x80);
  return 1;
}
uint8_t M6502::ORA() {
  A |= fetch();
  setFlag(Z, A == 0);
  setFlag(N, A & 0x80);
  return 1;
}
uint8_t M6502::BIT() {
  fetch();
  uint16_t temp = A & fetched;
  setFlag(Z, (temp & 0x00FF) == 0x00);
  setFlag(N, fetched & (1 << 7));
  setFlag(V, fetched & (1 << 6));
  return 0;
}

// Arithmetic

uint8_t M6502::ADC() {
  fetch();
  uint16_t value = fetched;
  uint16_t result = (uint16_t)A + value + getFlag(C);

  setFlag(Z, (result & 0x00FF) == 0);
  setFlag(C, result & 0xFF00);
  setFlag(V, (~(A ^ value) & (A ^ result)) & 0x80);
  setFlag(N, result & 0x80);
  A = result & 0x00FF;

  return 1;
}

uint8_t M6502::SBC() {
  fetch();
  int carry_in = getFlag(C);

  uint16_t value = fetched ^ 0x00FF;
  uint16_t result = (uint16_t)A + value + carry_in;

  setFlag(C, result & 0xFF00);
  setFlag(Z, (result & 0x00FF) == 0);
  setFlag(V, (result ^ A) & (result ^ value) & 0x80);
  setFlag(N, result & 0x80);

  A = result & 0x00FF;

  return 1;
}

uint8_t M6502::CMP() {
  fetch();
  uint16_t result = A - fetched;
  setFlag(C, A >= fetched);
  setFlag(Z, A == fetched);
  setFlag(N, result & 0x80);

  return 1;
}
uint8_t M6502::CPX() {
  fetch();
  uint16_t result = X - fetched;
  setFlag(C, X >= fetched);
  setFlag(Z, X == fetched);
  setFlag(N, result & 0x80);

  return 0;
}
uint8_t M6502::CPY() {
  fetch();
  uint16_t result = Y - fetched;
  setFlag(C, Y >= fetched);
  setFlag(Z, Y == fetched);
  setFlag(N, result & 0x80);

  return 0;
}

// Increments & Decrements
uint8_t M6502::INC() {
  fetch();
  uint16_t temp = fetched + 1;
  uint8_t result = temp & 0x00FF;
  write(addr_abs, result);
  setFlag(Z, result == 0);
  setFlag(N, result & 0x80);
  return 0;
}
uint8_t M6502::INX() {
  X++;
  setFlag(Z, X == 0);
  setFlag(N, X & 0x80);
  return 0;
}
uint8_t M6502::INY() {
  Y++;
  setFlag(Z, Y == 0);
  setFlag(N, Y & 0x80);
  return 0;
}
uint8_t M6502::DEC() {
  fetch();
  uint16_t temp = fetched - 1;
  uint8_t result = temp & 0x00FF;
  write(addr_abs, result);
  setFlag(Z, result == 0);
  setFlag(N, result & 0x80);
  return 0;
}
uint8_t M6502::DEX() {
  X--;
  setFlag(Z, X == 0);
  setFlag(N, X & 0x80);
  return 0;
}
uint8_t M6502::DEY() {
  Y--;
  setFlag(Z, Y == 0);
  setFlag(N, Y & 0x80);
  return 0;
}

// Shifts

uint8_t M6502::ASL() {
  fetch();
  uint16_t temp = (uint16_t)fetched << 1;
  setFlag(C, (temp & 0xFF00) > 0);
  setFlag(Z, (temp & 0x00FF) == 0x00);
  setFlag(N, temp & 0x80);
  if (lookup[opcode].addrmode == &M6502::IMP)
    A = temp & 0x00FF;
  else
    write(addr_abs, temp & 0x00FF);
  return 0;
}

uint8_t M6502::LSR() {
  fetch();
  setFlag(C, fetched & 0x0001);
  uint16_t temp = fetched >> 1;
  setFlag(Z, (temp & 0x00FF) == 0x0000);
  setFlag(N, temp & 0x0080);
  if (lookup[opcode].addrmode == &M6502::IMP)
    A = temp & 0x00FF;
  else
    write(addr_abs, temp & 0x00FF);
  return 0;
}

uint8_t M6502::ROL() {
  fetch();
  uint16_t temp = (uint16_t)fetched << 1 | getFlag(C);
  setFlag(C, temp & 0xFF00);
  setFlag(Z, (temp & 0x00FF) == 0x0000);
  setFlag(N, temp & 0x0080);
  if (lookup[opcode].addrmode == &M6502::IMP)
    A = temp & 0x00FF;
  else
    write(addr_abs, temp & 0x00FF);
  return 0;
}

uint8_t M6502::ROR() {
  fetch();
  uint16_t temp = (uint16_t)(getFlag(C) << 7) | (fetched >> 1);
  setFlag(C, fetched & 0x01);
  setFlag(Z, (temp & 0x00FF) == 0x00);
  setFlag(N, temp & 0x0080);
  if (lookup[opcode].addrmode == &M6502::IMP)
    A = temp & 0x00FF;
  else
    write(addr_abs, temp & 0x00FF);
  return 0;
}

// Jumps & Calls
uint8_t M6502::JMP() {
  PC = addr_abs;
  return 0;
}
uint8_t M6502::JSR() {
  PC--;
  StackPush((PC >> 8) & 0xFF);
  StackPush(PC & 0xFF);

  PC = addr_abs;
  return 0;
}
uint8_t M6502::RTS() {

  uint8_t lo = StackPull();
  uint8_t hi = StackPull();

  PC = ((hi << 8) | lo) + 1;

  return 0;
}

// Branches
uint8_t M6502::BCC() {
  if (getFlag(C) == 0) {
    cycles++;
    addr_abs = PC + addr_rel;

    if ((addr_abs & 0xFF00) != (PC & 0xFF00)) {
      cycles++;
    }

    PC = addr_abs;
  }
  return 0;
}
uint8_t M6502::BCS() {
  if (getFlag(C) == 1) {
    cycles++;
    addr_abs = PC + addr_rel;

    if ((addr_abs & 0xFF00) != (PC & 0xFF00)) {
      cycles++;
    }

    PC = addr_abs;
  }
  return 0;
}

uint8_t M6502::BEQ() {
  if (getFlag(Z) == 1) {
    cycles++;
    addr_abs = PC + addr_rel;

    if ((addr_abs & 0xFF00) != (PC & 0xFF00)) {
      cycles++;
    }

    PC = addr_abs;
  }
  return 0;
}

uint8_t M6502::BNE() {
  if (getFlag(Z) == 0) {
    cycles++;
    addr_abs = PC + addr_rel;

    if ((addr_abs & 0xFF00) != (PC & 0xFF00)) {
      cycles++;
    }

    PC = addr_abs;
  }
  return 0;
}
uint8_t M6502::BMI() {
  if (getFlag(N) == 1) {
    cycles++;
    addr_abs = PC + addr_rel;

    if ((addr_abs & 0xFF00) != (PC & 0xFF00)) {
      cycles++;
    }

    PC = addr_abs;
  }
  return 0;
}
uint8_t M6502::BPL() {
  if (getFlag(N) == 0) {
    cycles++;
    addr_abs = PC + addr_rel;

    if ((addr_abs & 0xFF00) != (PC & 0xFF00)) {
      cycles++;
    }

    PC = addr_abs;
  }
  return 0;
}
uint8_t M6502::BVC() {
  if (getFlag(V) == 0) {
    cycles++;
    addr_abs = PC + addr_rel;

    if ((addr_abs & 0xFF00) != (PC & 0xFF00)) {
      cycles++;
    }

    PC = addr_abs;
  }
  return 0;
}
uint8_t M6502::BVS() {
  if (getFlag(V) == 1) {
    cycles++;
    addr_abs = PC + addr_rel;

    if ((addr_abs & 0xFF00) != (PC & 0xFF00)) {
      cycles++;
    }

    PC = addr_abs;
  }
  return 0;
}

// Status Flag Changes
uint8_t M6502::CLC() {
  setFlag(C, false);
  return 0;
}

uint8_t M6502::CLD() {
  setFlag(D, false);
  return 0;
}

uint8_t M6502::CLI() {
  setFlag(I, false);
  return 0;
}

uint8_t M6502::CLV() {
  setFlag(V, false);
  return 0;
}

uint8_t M6502::SEC() {
  setFlag(C, true);
  return 0;
}

uint8_t M6502::SED() {
  setFlag(D, true);
  return 0;
}

uint8_t M6502::SEI() {
  setFlag(I, true);
  return 0;
}

// System Functions
uint8_t M6502::BRK() {
  PC++;
  StackPush((PC >> 8) & 0xFF);
  StackPush(PC & 0xFF);

  setFlag(B, true);
  setFlag(U, true);

  StackPush(status);

  setFlag(I, true);

  uint8_t lo = read(irqVectorL);
  uint8_t hi = read(irqVectorH);

  PC = (hi << 8) | lo;

  return 0;
}
uint8_t M6502::NOP() {
  return 0;
}
uint8_t M6502::RTI() {
  status = StackPull();
  status &= ~B;
  status |= U;

  uint8_t lo = StackPull();
  uint8_t hi = StackPull();
  PC = (hi << 8) | lo;

  return 0;
}

// System Interrupts

void M6502::IRQ() {
  if (getFlag(I) == 0) {

    StackPush((PC >> 8) & 0xFF);
    StackPush(PC & 0xFF);

    setFlag(B, false);
    setFlag(U, true);
    StackPush(status);

    setFlag(I, true);

    uint8_t lo = read(irqVectorL);
    uint8_t hi = read(irqVectorH);
    PC = (hi << 8) | lo;

    cycles = 7;
  }
}

void M6502::NMI() {
  StackPush((PC >> 8) & 0xFF);
  StackPush(PC & 0xFF);

  setFlag(B, false);
  setFlag(U, true);
  StackPush(status);

  setFlag(I, true);

  uint8_t lo = read(nmiVectorL);
  uint8_t hi = read(nmiVectorH);
  PC = (hi << 8) | lo;

  cycles = 7;
}

// Illegal Opcodes
uint8_t M6502::XXX() {
  return 0;
}

bool M6502::complete() {
  return cycles == 0;
}

std::map<uint16_t, std::string> M6502::disassemble(uint16_t nStart,
                                                   uint16_t nStop) {
  uint32_t addr = nStart;
  uint8_t value = 0x00, lo = 0x00, hi = 0x00;
  std::map<uint16_t, std::string> mapLines;
  uint16_t line_addr = 0;

  auto hex = [](uint32_t n, uint8_t d) {
    std::string s(d, '0');
    for (int i = d - 1; i >= 0; i--, n >>= 4)
      s[i] = "0123456789ABCDEF"[n & 0xF];
    return s;
  };

  // Starting at the specified address we read an instruction
  // byte, which in turn yields information from the lookup table
  // as to how many additional bytes we need to read and what the
  // addressing mode is. I need this info to assemble human readable
  // syntax, which is different depending upon the addressing mode

  // As the instruction is decoded, a std::string is assembled
  // with the readable output
  while (addr <= (uint32_t)nStop) {
    line_addr = addr;

    // Prefix line with instruction address
    std::string sInst = "$" + hex(addr, 4) + ": ";

    // Read instruction, and get its readable name
    uint8_t opcode = bus->cpuRead(addr, true);
    addr++;
    sInst += lookup[opcode].name + " ";

    // Get oprands from desired locations, and form the
    // instruction based upon its addressing mode. These
    // routines mimmick the actual fetch routine of the
    // 6502 in order to get accurate data as part of the
    // instruction
    if (lookup[opcode].addrmode == &M6502::IMP) {
      sInst += " {IMP}";
    } else if (lookup[opcode].addrmode == &M6502::IMM) {
      value = bus->cpuRead(addr, true);
      addr++;
      sInst += "#$" + hex(value, 2) + " {IMM}";
    } else if (lookup[opcode].addrmode == &M6502::ZP0) {
      lo = bus->cpuRead(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + " {ZP0}";
    } else if (lookup[opcode].addrmode == &M6502::ZPX) {
      lo = bus->cpuRead(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + ", X {ZPX}";
    } else if (lookup[opcode].addrmode == &M6502::ZPY) {
      lo = bus->cpuRead(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + ", Y {ZPY}";
    } else if (lookup[opcode].addrmode == &M6502::IZX) {
      lo = bus->cpuRead(addr, true);
      addr++;
      hi = 0x00;
      sInst += "($" + hex(lo, 2) + ", X) {IZX}";
    } else if (lookup[opcode].addrmode == &M6502::IZY) {
      lo = bus->cpuRead(addr, true);
      addr++;
      hi = 0x00;
      sInst += "($" + hex(lo, 2) + "), Y {IZY}";
    } else if (lookup[opcode].addrmode == &M6502::ABS) {
      lo = bus->cpuRead(addr, true);
      addr++;
      hi = bus->cpuRead(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + " {ABS}";
    } else if (lookup[opcode].addrmode == &M6502::ABX) {
      lo = bus->cpuRead(addr, true);
      addr++;
      hi = bus->cpuRead(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", X {ABX}";
    } else if (lookup[opcode].addrmode == &M6502::ABY) {
      lo = bus->cpuRead(addr, true);
      addr++;
      hi = bus->cpuRead(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", Y {ABY}";
    } else if (lookup[opcode].addrmode == &M6502::IND) {
      lo = bus->cpuRead(addr, true);
      addr++;
      hi = bus->cpuRead(addr, true);
      addr++;
      sInst += "($" + hex((uint16_t)(hi << 8) | lo, 4) + ") {IND}";
    } else if (lookup[opcode].addrmode == &M6502::REL) {
      value = bus->cpuRead(addr, true);
      addr++;
      sInst += "$" + hex(value, 2) + " [$" + hex(addr + (int8_t)value, 4) +
               "] {REL}";
    }

    mapLines[line_addr] = sInst;
  }

  return mapLines;
}
