# pyda 🚀

A custom, high-performance 2D game engine built with C (SDL2 backend) and Python. Zero Pygame dependency—pure raw speed and modular flexibility.

## Requirements

- Python >= 3.8
- SDL2 development libraries (headers + lib)
  - Linux/Debian/Ubuntu/Termux: `libsdl2-dev`
  - macOS (Homebrew): `brew install sdl2`
  - Windows: install SDL2 and set include/lib paths or use vcpkg

## Installation (Editable Mode)

```bash
pip install --editable .
```

Or from source:

```bash
python setup.py build_ext --inplace
pip install .
```

## Quick usage

```python
import pyda

pyda.init()
# ... use pyda._core, pyda.gameplay, pyda.physics_engine, etc.
pyda.quit()
```

Core entry points live in `pyda._core` (re-exported from the top-level package). Other modules are available as:

- `pyda._draw`, `pyda._font`, `pyda._image`, `pyda._mixer`, `pyda._mouse`, `pyda._key`
- `pyda._rect`, `pyda._sprite`, `pyda._time`, `pyda._transform`
- `pyda.gameplay`, `pyda.physics_engine`, `pyda.geometry_engine`, `pyda.collision_engine`, `pyda.game_utils`
- Helpers: `pyda._math`, `pyda._text`, `pyda._bytes`, `pyda._datastruct`

## Build notes

The C extensions must match the `PyInit_*` / module names defined in the `.c` sources. SDL2-dependent modules are linked with `-lSDL2`.

## License

See repository for license information.
