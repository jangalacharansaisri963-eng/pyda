"""
pyda.pygame.transform: Surface scaling and transformation module.
"""
from ._transform import scale as _c_scale
from .surface import Surface

def scale(surface, size):
    """Resize a surface to a new width and height tuple."""
    width, height = size
    new_capsule = _c_scale(surface._capsule, (int(width), int(height)))
    return Surface(new_capsule)
  
