"""
pyda.pygame: Pure C-accelerated native game engine.
"""
from ._core import (
    init,
    quit,
    set_mode,
    flip,
    pump_events,
    get_ticks,
    delay
)

# Event constants matching standard Pygame/SDL
QUIT = 256
KEYDOWN = 768
KEYUP = 769
MOUSEBUTTONDOWN = 1025
MOUSEBUTTONUP = 1026
MOUSEMOTION = 1024

__all__ = [
    "init", "quit", "set_mode", "flip", "pump_events", "get_ticks", "delay",
    "QUIT", "KEYDOWN", "KEYUP", "MOUSEBUTTONDOWN", "MOUSEBUTTONUP", "MOUSEMOTION"
]
