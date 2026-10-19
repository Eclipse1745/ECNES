# ECNES

A work-in-progress NES emulator, built for fun and to learn how the hardware actually works. Not cycle-accurate.

## Status

- **CPU (Ricoh 2A03 / 6502 core)** — passes Klaus Dormann's functional and interrupt test suites.
- **PPU (Ricoh 2C02)** — registers, VRAM/OAM/palette memory, VBlank/NMI timing, background rendering (nametables, attribute tables, scrolling) and sprite rendering, sprite-0-hit implemented.
- **Mappers** — only Mapper 0 (NROM) is implemented, so only NROM-based ROMs will load correctly.
- **Renderer** — built on [raylib](https://www.raylib.com/).

## Building

```
make
```

## Usage

```
./ECNES <rom.nes> [scale]
```

- `rom.nes` — path to an iNES ROM file. Defaults to `TestRoms/nestest.nes` if omitted.
- `scale` — integer window scale factor. Defaults to `1` if omitted.

```
./ECNES smb.nes 2
```

## Controls

### Game (Controller 1)

| Key | NES Button |
|---|---|
| `W` | D-pad Up |
| `S` | D-pad Down |
| `A` | D-pad Left |
| `D` | D-pad Right |
| `O` | A |
| `P` | B |
| `Right Shift` | Select |
| `Enter` | Start |

### Debug demo

| Key | Action |
|---|---|
| `Space` | Toggle emulation running / paused |
| `C` | Step one CPU instruction |
| `F` | Step one full frame |
| `R` | Reset the CPU |
| `TAB` | Cycle the selected palette |

## Credits

- CPU/PPU architecture follows [javidx9](https://github.com/OneLoneCoder)'s NES emulator tutorial series.
- Uses [raylib](https://www.raylib.com/) (zlib/libpng License).

## License

MIT — see [LICENSE](LICENSE).
