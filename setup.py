from setuptools import setup, Extension
import os

# Define the C extension module and link SDL2
# (Note: On Linux/Termux you link against -lSDL2, on Windows/macOS you link appropriately)
c_extension = Extension(
    'pyda._core',
    sources=['pyda/core.c'],
    libraries=['SDL2'],
    extra_compile_args=['-O3'],  # Maximum optimization for raw speed
)

setup(
    name="pyda",
    version="1.0.0",
    description="A high-performance C+Python game engine built from scratch",
    author="Sampath",
    packages=['pyda'],
    ext_modules=[c_extension],
    zip_safe=False,
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C",
        "Topic :: Games/Entertainment",
    ],
)
