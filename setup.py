from setuptools import setup, Extension
import sys
import os

# Common compile flags
extra_compile_args = ['-O3']
if sys.platform == 'win32':
    extra_compile_args = ['/O2']

# Common search paths for headers/libs (Linux, Homebrew, etc.)
common_include_dirs = [
    '/usr/include', '/usr/include/SDL2',
    '/usr/local/include', '/usr/local/include/SDL2',
    '/opt/homebrew/include', '/opt/homebrew/include/SDL2',
]
common_library_dirs = [
    '/usr/lib', '/usr/local/lib',
    '/usr/lib/x86_64-linux-gnu', '/usr/lib/aarch64-linux-gnu',
    '/opt/homebrew/lib',
]

def make_ext(name, sources, libraries=None):
    kwargs = {
        'name': name,
        'sources': sources,
        'extra_compile_args': extra_compile_args,
    }
    if libraries:
        kwargs['libraries'] = libraries
        kwargs['include_dirs'] = common_include_dirs
        kwargs['library_dirs'] = common_library_dirs
    return Extension(**kwargs)

setup(
    name="pyda",
    version="1.0.0",
    packages=['pyda'],
    ext_modules=[
        # Core
        make_ext('pyda._core', ['pyda/core.c'], libraries=['SDL2']),
        make_ext('pyda.gameplay', ['pyda/gameplay.c']),

        # Engines (pure C, no SDL)
        make_ext('pyda.physics_engine', ['pyda/physics_engine.c']),
        make_ext('pyda.geometry_engine', ['pyda/geometry_engine.c']),
        make_ext('pyda.collision_engine', ['pyda/collision_engine.c']),
        make_ext('pyda.game_utils', ['pyda/game_utils.c']),

        # Sub-systems
        make_ext('pyda._draw', ['pyda/draw.c'], libraries=['SDL2']),
        make_ext('pyda._font', ['pyda/font.c'], libraries=['SDL2', 'SDL2_ttf']),
        make_ext('pyda._image', ['pyda/image.c'], libraries=['SDL2', 'SDL2_image']),
        make_ext('pyda._mixer', ['pyda/mixer.c'], libraries=['SDL2', 'SDL2_mixer']),
        make_ext('pyda._mouse', ['pyda/mouse.c'], libraries=['SDL2']),
        make_ext('pyda._key', ['pyda/key.c'], libraries=['SDL2']),
        make_ext('pyda._rect', ['pyda/rect.c']),
        make_ext('pyda._sprite', ['pyda/sprite.c']),
        make_ext('pyda._time', ['pyda/time.c'], libraries=['SDL2']),
        make_ext('pyda._transform', ['pyda/transform.c'], libraries=['SDL2']),

        # Low-level helpers
        make_ext('pyda._math', ['pyda/_math.c']),
        make_ext('pyda._text', ['pyda/_text.c']),
        make_ext('pyda._bytes', ['pyda/_bytes.c']),
        make_ext('pyda._datastruct', ['pyda/_datastruct.c']),
    ],
    zip_safe=False,
)
