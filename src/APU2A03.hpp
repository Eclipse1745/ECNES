#pragma once

#include <cstdint>

// NES 2A03 audio processing unit: two pulse channels, triangle and noise,
// driven by the frame counter (4-step mode) and mixed with the hardware's
// non-linear formulas.
//
// Everything a caller must know:
//   - clock() is called once per PPU clock; the APU divides by 3 itself.
//   - cpuWrite() takes $4000-$4013 and $4015. cpuRead() only returns something
//     for $4015 (length-counter status); every other address reads 0.
//   - GetOutputSample() returns the mix averaged over the time since the last
//     call, with DC removed, nominally within [-1, 1]. Call it once per output
//     sample (the Bus does this at the audio sample rate).
//   - Not implemented yet: DMC ($4010-$4013 are ignored), the 5-step frame
//     counter mode and frame IRQ ($4017).
class APU2A03 {
public:
  void cpuWrite(uint16_t addr, uint8_t data);
  uint8_t cpuRead(uint16_t addr);
  void clock();
  void reset();
  double GetOutputSample();

private:
  struct LengthCounter {
    uint8_t value = 0;
    bool halt = false;
    bool enabled = false;

    void Load(uint8_t index); // ignored while the channel is disabled
    void SetEnabled(bool on); // disabling clears the counter
    void Clock();
  };

  struct Envelope {
    bool start = false;
    bool loop = false;
    bool constant = false;
    uint8_t period = 0;
    uint8_t divider = 0;
    uint8_t decay = 0;

    void Write(uint8_t data); // bits: --LC VVVV
    void Clock();
    uint8_t Volume() const {
      return constant ? period : decay;
    }
  };

  struct Pulse {
    explicit Pulse(bool isPulse1) : isPulse1(isPulse1) {
    }

    bool isPulse1; // the two pulse channels negate their sweep differently
    uint8_t duty = 0;
    uint8_t seq = 0;
    uint16_t timer = 0;
    uint16_t counter = 0;
    Envelope env;
    LengthCounter length;

    bool sweepEnabled = false;
    bool sweepNegate = false;
    bool sweepReload = false;
    uint8_t sweepPeriod = 0;
    uint8_t sweepShift = 0;
    uint8_t sweepDivider = 0;

    void Write(uint8_t reg, uint8_t data);
    void ClockTimer(); // once per APU cycle (every second CPU cycle)
    void ClockSweep(); // half-frame
    int TargetPeriod() const;
    bool Muted() const;
    uint8_t Output() const; // 0..15
  };

  struct Triangle {
    bool control = false;
    bool reloadFlag = false;
    uint8_t linearReload = 0;
    uint8_t linearCounter = 0;
    uint8_t seq = 0;
    uint16_t timer = 0;
    uint16_t counter = 0;
    LengthCounter length;

    void Write(uint8_t reg, uint8_t data);
    void ClockTimer();      // once per CPU cycle
    void ClockLinear();     // quarter-frame
    uint8_t Output() const; // 0..15
  };

  struct Noise {
    uint16_t shift = 1;
    bool mode = false;
    uint16_t period = 4;
    uint16_t counter = 0;
    Envelope env;
    LengthCounter length;

    void Write(uint8_t reg, uint8_t data);
    void ClockTimer();      // once per CPU cycle
    uint8_t Output() const; // 0..15
  };

  void ClockCpuCycle();
  void QuarterFrame();
  void HalfFrame();
  double Mix() const;

  Pulse pulse1_{true};
  Pulse pulse2_{false};
  Triangle triangle_;
  Noise noise_;

  uint8_t phase_ = 0;       // counts PPU clocks, 3 per CPU cycle
  bool apuCycle_ = false;   // toggles every CPU cycle
  uint32_t frameCycle_ = 0; // CPU cycles into the frame counter sequence

  double mixSum_ = 0.0; // box-filter accumulator for downsampling
  uint32_t mixCount_ = 0;
  double lastMix_ = 0.0;
  double dcPrevIn_ = 0.0; // DC-blocking high-pass state
  double dcPrevOut_ = 0.0;
};
