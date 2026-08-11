#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>

class Bus;

class M6502 {
public:
  M6502();

  void ConnectBus(Bus *n) {
    bus = n;
  }

  std::map<uint16_t, std::string> disassemble(uint16_t nStart, uint16_t nStop);

  bool complete();

  // ===================== Execution =====================
  void reset();
  uint8_t clock();

  // ===================== Interrupts =====================
  void callIRQ() {
    IRQ();
  }
  void callNMI() {
    NMI();
  }
  void syncInterruptState();

  // ===================== Memory Access =====================
  uint8_t read(uint16_t addr);
  void write(uint16_t addr, uint8_t data);

  // ===================== Registers =====================
  uint8_t A = 0x00;
  uint8_t X = 0x00;
  uint8_t Y = 0x00;
  uint8_t SP = 0x00;
  uint16_t PC = 0x0000;
  uint8_t status = 0x00;
  uint8_t cycles = 0x00;

  Bus *bus = nullptr;
  enum FLAGS6502 : uint8_t {
    C = 1 << 0, // Carry
    Z = 1 << 1, // Zero
    I = 1 << 2, // Interrupt Disable
    D = 1 << 3, // Decimal Mode (important for full 6502)
    B = 1 << 4, // Break
    U = 1 << 5, // Unused (always pushed as 1)
    V = 1 << 6, // Overflow
    N = 1 << 7  // Negative
  };

private:
  // ===================== Memory =====================
  std::array<uint8_t, 1024 * 64> ram = {};

  // ===================== Status Flags =====================

  uint8_t getFlag(FLAGS6502 f);
  void setFlag(FLAGS6502 f, bool value);

  // ===================== Interrupt State =====================
  void IRQ();
  void NMI();

  // ===================== Instruction Decode State =====================
  uint8_t opcode = 0x00;
  uint8_t fetched = 0x00;
  uint16_t addr_abs = 0x0000;
  uint16_t addr_rel = 0x0000;

  uint8_t fetch();

  // ===================== Stack Helpers =====================
  void StackPush(uint8_t data);
  uint8_t StackPull();

  // ===================== Addressing Modes =====================
  uint8_t IMP(); // Implied
  uint8_t IMM(); // Immediate
  uint8_t ZP0(); // Zero Page
  uint8_t ZPX(); // Zero Page, X
  uint8_t ZPY(); // Zero Page, Y
  uint8_t REL(); // Relative
  uint8_t ABS(); // Absolute
  uint8_t ABX(); // Absolute, X
  uint8_t ABY(); // Absolute, Y
  uint8_t IND(); // Indirect
  uint8_t IZX(); // Indirect, X
  uint8_t IZY(); // Indirect, Y

  // ===================== Instructions =====================

  // --- Load / Store ---
  uint8_t LDA();
  uint8_t LDX();
  uint8_t LDY();
  uint8_t STA();
  uint8_t STX();
  uint8_t STY();

  // --- Register Transfers ---
  uint8_t TAX();
  uint8_t TAY();
  uint8_t TXA();
  uint8_t TYA();

  // --- Stack Operations ---
  uint8_t TSX();
  uint8_t TXS();
  uint8_t PHA();
  uint8_t PHP();
  uint8_t PLA();
  uint8_t PLP();

  // --- Logical ---
  uint8_t AND();
  uint8_t EOR();
  uint8_t ORA();
  uint8_t BIT();

  // --- Arithmetic ---
  uint8_t ADC();
  uint8_t SBC();
  uint8_t CMP();
  uint8_t CPX();
  uint8_t CPY();

  // --- Increments & Decrements ---
  uint8_t INC();
  uint8_t INX();
  uint8_t INY();
  uint8_t DEC();
  uint8_t DEX();
  uint8_t DEY();

  // --- Shifts ---
  uint8_t ASL();
  uint8_t LSR();
  uint8_t ROL();
  uint8_t ROR();

  // --- Jumps & Calls ---
  uint8_t JMP();
  uint8_t JSR();
  uint8_t RTS();

  // --- Branches ---
  uint8_t BCC();
  uint8_t BCS();
  uint8_t BEQ();
  uint8_t BMI();
  uint8_t BNE();
  uint8_t BPL();
  uint8_t BVC();
  uint8_t BVS();

  // --- Status Flag Changes ---
  uint8_t CLC();
  uint8_t CLD();
  uint8_t CLI();
  uint8_t CLV();
  uint8_t SEC();
  uint8_t SED();
  uint8_t SEI();

  // --- System Functions ---
  uint8_t BRK();
  uint8_t NOP();
  uint8_t RTI();

  // --- Illegal Opcodes ---
  uint8_t XXX();

  // ===================== Instruction Lookup Table =====================
  struct InstructionMap {
    std::string name;
    uint8_t (M6502::*operate)(void) = nullptr;
    uint8_t (M6502::*addrmode)(void) = nullptr;
    uint8_t cycles = 0;
  };
  std::array<InstructionMap, 256> lookup;
  void initLookupTable();

  // ===================== Interrupt / Reset Vectors =====================
  static const uint16_t irqVectorL = 0xFFFE;
  static const uint16_t irqVectorH = 0xFFFF;
  static const uint16_t rstVectorL = 0xFFFC;
  static const uint16_t rstVectorH = 0xFFFD;
  static const uint16_t nmiVectorL = 0xFFFA;
  static const uint16_t nmiVectorH = 0xFFFB;
};
