#include "APU2A03.hpp"

namespace {

constexpr uint8_t kLengthTable[32] = {
    10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
    12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};

constexpr uint8_t kDuty[4][8] = {{0, 1, 0, 0, 0, 0, 0, 0},
                                 {0, 1, 1, 0, 0, 0, 0, 0},
                                 {0, 1, 1, 1, 1, 0, 0, 0},
                                 {1, 0, 0, 1, 1, 1, 1, 1}};

constexpr uint8_t kTriangleSeq[32] = {15, 14, 13, 12, 11, 10, 9,  8,  7,  6, 5,
                                      4,  3,  2,  1,  0,  0,  1,  2,  3,  4, 5,
                                      6,  7,  8,  9,  10, 11, 12, 13, 14, 15};

// NTSC noise periods, in CPU cycles
constexpr uint16_t kNoisePeriod[16] = {
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068};

// Frame counter, 4-step mode, in CPU cycles
constexpr uint32_t kStep1 = 7457;
constexpr uint32_t kStep2 = 14913;
constexpr uint32_t kStep3 = 22371;
constexpr uint32_t kStep4 = 29829;
constexpr uint32_t kFrameLength = 29830;

constexpr double kDcBlockR = 0.996; // high-pass, about 35 Hz at 44.1 kHz

} // namespace

// ---- LengthCounter ----

void APU2A03::LengthCounter::Load(uint8_t index) {
  if (enabled)
    value = kLengthTable[index & 0x1F];
}

void APU2A03::LengthCounter::SetEnabled(bool on) {
  enabled = on;
  if (!on)
    value = 0;
}

void APU2A03::LengthCounter::Clock() {
  if (!halt && value > 0)
    value--;
}

// ---- Envelope ----

void APU2A03::Envelope::Write(uint8_t data) {
  loop = (data & 0x20) != 0;
  constant = (data & 0x10) != 0;
  period = data & 0x0F;
}

void APU2A03::Envelope::Clock() {
  if (start) {
    start = false;
    decay = 15;
    divider = period;
  } else if (divider == 0) {
    divider = period;
    if (decay > 0)
      decay--;
    else if (loop)
      decay = 15;
  } else {
    divider--;
  }
}

// ---- Pulse ----

void APU2A03::Pulse::Write(uint8_t reg, uint8_t data) {
  switch (reg) {
  case 0: // DDLC VVVV
    duty = data >> 6;
    length.halt = (data & 0x20) != 0; // same bit as the envelope loop flag
    env.Write(data);
    break;
  case 1: // EPPP NSSS
    sweepEnabled = (data & 0x80) != 0;
    sweepPeriod = (data >> 4) & 0x07;
    sweepNegate = (data & 0x08) != 0;
    sweepShift = data & 0x07;
    sweepReload = true;
    break;
  case 2: // timer low
    timer = (timer & 0x0700) | data;
    break;
  case 3: // LLLL LHHH
    timer = (timer & 0x00FF) | ((data & 0x07) << 8);
    length.Load(data >> 3);
    seq = 0;
    env.start = true;
    break;
  }
}

void APU2A03::Pulse::ClockTimer() {
  if (counter == 0) {
    counter = timer;
    seq = (seq + 1) & 7;
  } else {
    counter--;
  }
}

int APU2A03::Pulse::TargetPeriod() const {
  const int change = timer >> sweepShift;
  if (sweepNegate)
    return timer - change - (isPulse1 ? 1 : 0);
  return timer + change;
}

bool APU2A03::Pulse::Muted() const {
  return timer < 8 || TargetPeriod() > 0x7FF;
}

void APU2A03::Pulse::ClockSweep() {
  if (sweepDivider == 0 && sweepEnabled && sweepShift != 0 && !Muted())
    timer = static_cast<uint16_t>(TargetPeriod());

  if (sweepDivider == 0 || sweepReload) {
    sweepDivider = sweepPeriod;
    sweepReload = false;
  } else {
    sweepDivider--;
  }
}

uint8_t APU2A03::Pulse::Output() const {
  if (length.value == 0 || Muted() || kDuty[duty][seq] == 0)
    return 0;
  return env.Volume();
}

// ---- Triangle ----

void APU2A03::Triangle::Write(uint8_t reg, uint8_t data) {
  switch (reg) {
  case 0: // CRRR RRRR
    control = (data & 0x80) != 0;
    length.halt = control;
    linearReload = data & 0x7F;
    break;
  case 2:
    timer = (timer & 0x0700) | data;
    break;
  case 3:
    timer = (timer & 0x00FF) | ((data & 0x07) << 8);
    length.Load(data >> 3);
    reloadFlag = true;
    break;
  }
}

void APU2A03::Triangle::ClockTimer() {
  if (counter == 0) {
    counter = timer;
    // Periods below 2 are ultrasonic and only cause aliasing, so hold instead.
    if (length.value > 0 && linearCounter > 0 && timer >= 2)
      seq = (seq + 1) & 31;
  } else {
    counter--;
  }
}

void APU2A03::Triangle::ClockLinear() {
  if (reloadFlag)
    linearCounter = linearReload;
  else if (linearCounter > 0)
    linearCounter--;

  if (!control)
    reloadFlag = false;
}

uint8_t APU2A03::Triangle::Output() const {
  return kTriangleSeq[seq];
}

// ---- Noise ----

void APU2A03::Noise::Write(uint8_t reg, uint8_t data) {
  switch (reg) {
  case 0: // --LC VVVV
    length.halt = (data & 0x20) != 0;
    env.Write(data);
    break;
  case 2: // M--- PPPP
    mode = (data & 0x80) != 0;
    period = kNoisePeriod[data & 0x0F];
    break;
  case 3: // LLLL L---
    length.Load(data >> 3);
    env.start = true;
    break;
  }
}

void APU2A03::Noise::ClockTimer() {
  if (counter == 0) {
    counter = period - 1;
    const uint16_t feedback = (shift & 1) ^ ((shift >> (mode ? 6 : 1)) & 1);
    shift >>= 1;
    shift |= feedback << 14;
  } else {
    counter--;
  }
}

uint8_t APU2A03::Noise::Output() const {
  if ((shift & 1) || length.value == 0)
    return 0;
  return env.Volume();
}

void APU2A03::cpuWrite(uint16_t addr, uint8_t data) {
  if (addr >= 0x4000 && addr <= 0x4003) {
    pulse1_.Write(addr & 3, data);
  } else if (addr >= 0x4004 && addr <= 0x4007) {
    pulse2_.Write(addr & 3, data);
  } else if (addr >= 0x4008 && addr <= 0x400B) {
    triangle_.Write(addr & 3, data);
  } else if (addr >= 0x400C && addr <= 0x400F) {
    noise_.Write(addr & 3, data);
  } else if (addr == 0x4015) {
    pulse1_.length.SetEnabled((data & 0x01) != 0);
    pulse2_.length.SetEnabled((data & 0x02) != 0);
    triangle_.length.SetEnabled((data & 0x04) != 0);
    noise_.length.SetEnabled((data & 0x08) != 0);
  }
}

uint8_t APU2A03::cpuRead(uint16_t addr) {
  if (addr != 0x4015)
    return 0x00;

  uint8_t status = 0x00;
  if (pulse1_.length.value > 0)
    status |= 0x01;
  if (pulse2_.length.value > 0)
    status |= 0x02;
  if (triangle_.length.value > 0)
    status |= 0x04;
  if (noise_.length.value > 0)
    status |= 0x08;
  return status;
}

void APU2A03::clock() {
  if (++phase_ < 3)
    return;
  phase_ = 0;
  ClockCpuCycle();
}

void APU2A03::ClockCpuCycle() {
  frameCycle_++;
  switch (frameCycle_) {
  case kStep1:
    QuarterFrame();
    break;
  case kStep2:
    QuarterFrame();
    HalfFrame();
    break;
  case kStep3:
    QuarterFrame();
    break;
  case kStep4:
    QuarterFrame();
    HalfFrame();
    break;
  case kFrameLength:
    frameCycle_ = 0;
    break;
  }

  triangle_.ClockTimer();
  noise_.ClockTimer();

  apuCycle_ = !apuCycle_;
  if (apuCycle_) {
    pulse1_.ClockTimer();
    pulse2_.ClockTimer();
  }

  mixSum_ += Mix();
  mixCount_++;
}

void APU2A03::QuarterFrame() {
  pulse1_.env.Clock();
  pulse2_.env.Clock();
  noise_.env.Clock();
  triangle_.ClockLinear();
}

void APU2A03::HalfFrame() {
  pulse1_.length.Clock();
  pulse2_.length.Clock();
  triangle_.length.Clock();
  noise_.length.Clock();
  pulse1_.ClockSweep();
  pulse2_.ClockSweep();
}

// The hardware's non-linear DAC, approximated (DMC input is always 0 for now).
double APU2A03::Mix() const {
  const double p = pulse1_.Output() + pulse2_.Output();
  const double pulseOut = p > 0.0 ? 95.88 / (8128.0 / p + 100.0) : 0.0;

  const double t = triangle_.Output() / 8227.0 + noise_.Output() / 12241.0;
  const double tndOut = t > 0.0 ? 159.79 / (1.0 / t + 100.0) : 0.0;

  return pulseOut + tndOut;
}

double APU2A03::GetOutputSample() {
  // Average everything since the last call (box filter) to limit aliasing.
  const double x = mixCount_ > 0 ? mixSum_ / mixCount_ : lastMix_;
  lastMix_ = x;
  mixSum_ = 0.0;
  mixCount_ = 0;

  // Remove DC so held channels and the 0..1 mixer range don't offset the wave.
  const double y = x - dcPrevIn_ + kDcBlockR * dcPrevOut_;
  dcPrevIn_ = x;
  dcPrevOut_ = y;
  return y;
}

void APU2A03::reset() {
  *this = APU2A03();
}
