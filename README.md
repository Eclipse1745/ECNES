# NESemu

A work-in-progress NES emulator, built for fun and to learn how the hardware actually works. Not cycle-accurate.

## Status

- **CPU (Ricoh 2A03 / 6502 core)** — passes Klaus Dormann's functional and interrupt test suites.
- **PPU (Ricoh 2C02)** — registers, VRAM/OAM/palette memory, and VBlank/NMI timing implemented. Background and sprite rendering (and sprite-0-hit) not yet implemented — this is the current blocker for running most commercial games.
- **Mappers** — only Mapper 0 (NROM) is implemented, so only NROM-based ROMs will load correctly.
- **Renderer** — originally built on [olcPixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine), migrating to [raylib](https://www.raylib.com/).

## Building

```
make
./NESemu
```

## Controls (debug demo)

| Key | Action |
|---|---|
| `Space` | Toggle emulation running / paused |
| `C` | Step one CPU instruction |
| `F` | Step one full frame |
| `R` | Reset the CPU |
| `P` | Cycle the selected palette |

## Credits

- CPU/PPU architecture follows [javidx9](https://github.com/OneLoneCoder)'s NES emulator tutorial series.
- Uses [raylib](https://www.raylib.com/) (zlib/libpng License).

## License

MIT — see [LICENSE](LICENSE).
