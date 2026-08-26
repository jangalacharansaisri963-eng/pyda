"""
pyda.pygame.draw: Shape rendering functions.
"""
from ._core import draw_rect

def rect(surface, color, rect_tuple, width=0):
    # color can be RGB tuple or hex
    if isinstance(color, (tuple, list)):
        r, g, b = color[:3]
        hex_val = (r << 16) | (g << 8) | b
    else:
        hex_val = int(color)
    
    x, y, w, h = rect_tuple
    draw_rect(x, y, w, h, hex_val)
  
