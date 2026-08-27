from setuptools import setup, Extension

setup(
    name="pyda",
    version="1.0.0",
    packages=['pyda'],
    ext_modules=[
        # Core & Main Modules
        Extension('pyda.core', sources=['pyda/core.c'], extra_compile_args=['-O3']),
        Extension('pyda.gameplay', sources=['pyda/gameplay.c'], extra_compile_args=['-O3']),
        
        # Engines
        Extension('pyda.physics_engine', sources=['pyda/physics_engine.c'], extra_compile_args=['-O3']),
        Extension('pyda.geometry_engine', sources=['pyda/geometry_engine.c'], extra_compile_args=['-O3']),
        Extension('pyda.collision_engine', sources=['pyda/collision_engine.c'], extra_compile_args=['-O3']),
        Extension('pyda.game_utils', sources=['pyda/game_utils.c'], extra_compile_args=['-O3']),
        
        # Sub-systems & Helpers
        Extension('pyda.draw', sources=['pyda/draw.c'], extra_compile_args=['-O3']),
        Extension('pyda.font', sources=['pyda/font.c'], extra_compile_args=['-O3']),
        Extension('pyda.image', sources=['pyda/image.c'], extra_compile_args=['-O3']),
        Extension('pyda.mixer', sources=['pyda/mixer.c'], extra_compile_args=['-O3']),
        Extension('pyda.mouse', sources=['pyda/mouse.c'], extra_compile_args=['-O3']),
        Extension('pyda.key', sources=['pyda/key.c'], extra_compile_args=['-O3']),
        Extension('pyda.rect', sources=['pyda/rect.c'], extra_compile_args=['-O3']),
        Extension('pyda.sprite', sources=['pyda/sprite.c'], extra_compile_args=['-O3']),
        Extension('pyda.time', sources=['pyda/time.c'], extra_compile_args=['-O3']),
        Extension('pyda.transform', sources=['pyda/transform.c'], extra_compile_args=['-O3']),
        
        # Low-level helpers
        Extension('pyda._math', sources=['pyda/_math.c'], extra_compile_args=['-O3']),
        Extension('pyda._text', sources=['pyda/_text.c'], extra_compile_args=['-O3']),
        Extension('pyda._bytes', sources=['pyda/_bytes.c'], extra_compile_args=['-O3']),
        Extension('pyda._datastruct', sources=['pyda/_datastruct.c'], extra_compile_args=['-O3']),
    ],
    zip_safe=False,
)
