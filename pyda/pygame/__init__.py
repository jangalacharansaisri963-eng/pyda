"""
pyda.pygame: High-performance, standalone native rewrite of Pygame.
Powered directly by SDL2, SDL2_image, SDL2_mixer, and SDL2_ttf C extensions.
"""

# Core window, display, and event subsystems
from ._core import (
    init,
    quit,
    set_mode,
    flip,
    fill,
    pump_events,
    get_pressed,
    get_ticks,
    delay,
    draw_rect
)

# Native or optimized modules
from ._rect import Rect
from ._time import Clock
from . import display
from . import event
from . import draw
from . import key
from . import mouse
from . import transform
from . import image
from . import mixer
from . import font
from .sprite import Sprite, Group, spritecollide

# Event Constants matching SDL / Pygame standards
QUIT = 256
KEYDOWN = 768
KEYUP = 769
MOUSEBUTTONDOWN = 1025
MOUSEBUTTONUP = 1026
MOUSEMOTION = 1024

__all__ = [
    "init", "quit", "set_mode", "flip", "fill", "pump_events", "get_pressed",
    "get_ticks", "delay", "draw_rect",
    "Rect", "Clock",
    "display", "event", "draw", "key", "mouse", "transform", "image", "mixer", "font",
    "Sprite", "Group", "spritecollide",
    "QUIT", "KEYDOWN", "KEYUP", "MOUSEBUTTONDOWN", "MOUSEBUTTONUP", "MOUSEMOTION"
]
