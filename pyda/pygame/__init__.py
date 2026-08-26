"""
pyda.pygame: Standalone native SDL2-powered game module.
"""

from ._core import (
    init,
    quit,
    set_mode,
    flip,
)

class Rect:
    def __init__(self, x, y, width, height):
        self.x = x
        self.y = y
        self.width = width
        self.height = height

    def colliderect(self, other):
        return (
            self.x < other.x + other.width and
            self.x + self.width > other.x and
            self.y < other.y + other.height and
            self.y + self.height > other.y
        )

    def __repr__(self):
        return f"<Rect(x={self.x}, y={self.y}, w={self.width}, h={self.height})>"

class DisplayModule:
    def set_mode(self, resolution):
        return set_mode(resolution)
    
    def flip(self):
        return flip()

display = DisplayModule()
