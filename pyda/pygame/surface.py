"""
pyda.pygame.surface: Surface pixel pipeline wrapper.
"""
from ._core import fill

class Surface:
    def __init__(self, surface_capsule):
        self._capsule = surface_capsule

    def fill(self, color):
        if isinstance(color, (tuple, list)):
            r, g, b = color[:3]
            hex_val = (r << 16) | (g << 8) | b
        else:
            hex_val = int(color)
        fill(hex_val)
      
