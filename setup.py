from setuptools import setup, Extension
import sys

# Common compile flags
extra_compile_args = ['-O3']
if sys.platform == 'win32':
    # Windows may need different flags; SDL2 path often via env
    extra_compile_args = ['/O2']

# Modules that need SDL2 linking
sdl_libs = ['SDL2']
if sys.platform == 'darwin':
    # macOS often needs frameworks; adjust if using Homebrew
    pass

def make_ext(name, sources, needs_sdl=False):
    kwargs = {
        'name': name,
        'sources': sources,
        'extra_compile_args': extra_compile_args,
    }
    if needs_sdl:
        kwargs['libraries'] = sdl_libs
        # Help find headers on common layouts
        kwargs['include_dirs'] = ['/usr/include', '/usr/include/SDL2', '/usr/local/include', '/usr/local/include/SDL2']
        kwargs['library_dirs'] = ['/usr/lib', '/usr/local/lib', '/usr/lib/x86_64-linux-gnu']
    return Extension(**kwargs)

setup(
    name="pyda",
    version="1.0.0",
    packages=['pyda'],
    ext_modules=[
        # Core (C defines PyInit__core / m_name="_core")
        make_ext('pyda._core', ['pyda/core.c'], needs_sdl=True),
        make_ext('pyda.gameplay', ['pyda/gameplay.c']),

        # Engines
        make_ext('pyda.physics_engine', ['pyda/physics_engine.c']),
        make_ext('pyda.geometry_engine', ['pyda/geometry_engine.c']),
        make_ext('pyda.collision_engine', ['pyda/collision_engine.c']),
        make_ext('pyda.game_utils', ['pyda/game_utils.c']),

        # Sub-systems (C uses leading underscore in module name)
        make_ext('pyda._draw', ['pyda/draw.c'], needs_sdl=True),
        make_ext('pyda._font', ['pyda/font.c'], needs_sdl=True),
        make_ext('pyda._image', ['pyda/image.c'], needs_sdl=True),
        make_ext('pyda._mixer', ['pyda/mixer.c'], needs_sdl=True),
        make_ext('pyda._mouse', ['pyda/mouse.c'], needs_sdl=True),
        make_ext('pyda._key', ['pyda/key.c'], needs_sdl=True),
        make_ext('pyda._rect', ['pyda/rect.c']),
        make_ext('pyda._sprite', ['pyda/sprite.c']),
        make_ext('pyda._time', ['pyda/time.c'], needs_sdl=True),
        make_ext('pyda._transform', ['pyda/transform.c'], needs_sdl=True),

        # Low-level helpers
        make_ext('pyda._math', ['pyda/_math.c']),
        make_ext('pyda._text', ['pyda/_text.c']),
        make_ext('pyda._bytes', ['pyda/_bytes.c']),
        make_ext('pyda._datastruct', ['pyda/_datastruct.c']),
    ],
    zip_safe=False,
)
