from setuptools import setup, Extension

c_extension = Extension(
    'pyda.pygame._core',
    sources=['pyda/pygame/core.c'],
    libraries=['SDL2'],
    extra_compile_args=['-O3'],
)

setup(
    name="pyda",
    version="1.0.0",
    packages=['pyda', 'pyda.pygame', 'pyda.dangame'],
    ext_modules=[c_extension],
    zip_safe=False,
)
