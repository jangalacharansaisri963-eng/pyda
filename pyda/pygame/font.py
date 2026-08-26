"""
pyda.pygame.font: TrueType font loading and text rendering module.
"""
from ._font import (
    init as _ttf_init,
    quit as _ttf_quit,
    load_font,
    render_text
)
from .surface import Surface

def init():
    return _ttf_init()

def quit():
    return _ttf_quit()

class Font:
    def __init__(self, filename, size):
        self._capsule = load_font(filename, size)

    def render(self, text, antialias, color):
        if isinstance(color, (tuple, list)):
            r, g, b = color[:3]
            hex_val = (r << 16) | (g << 8) | b
        else:
            hex_val = int(color)
        
        surface_capsule = render_text(self._capsule, str(text), int(antialias), hex_val)
        return Surface(surface_capsule)
      
