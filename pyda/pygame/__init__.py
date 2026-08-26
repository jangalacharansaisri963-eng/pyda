"""
pyda.pygame: Full native rewrite of Pygame built on SDL2.
"""
from ._core import init, quit, get_ticks, delay
from .rect import Rect
from .surface import Surface
from .display import DisplayModule, display
from .event import EventModule, event, QUIT, KEYDOWN, KEYUP, MOUSEBUTTONDOWN, MOUSEBUTTONUP, MOUSEMOTION
from .time import Clock
from . import draw
from . import key

__all__ = [
    "init", "quit", "Rect", "Surface", "display", "event", "Clock", "draw", "key",
    "QUIT", "KEYDOWN", "KEYUP", "MOUSEBUTTONDOWN", "MOUSEBUTTONUP", "MOUSEMOTION"
]
