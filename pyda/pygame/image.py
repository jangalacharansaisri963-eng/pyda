"""
pyda.pygame.image: Image loading and saving module.
"""
from ._image import load as _c_load
from .surface import Surface

def load(filename):
    """Load a new image from a file path and return a Surface."""
    surface_capsule = _c_load(str(filename))
    return Surface(surface_capsule)
  
