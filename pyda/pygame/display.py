"""
pyda.pygame.display: Window and event pipeline management.
"""
from ._core import set_mode as _c_set_mode, flip as _c_flip, pump_events as _c_pump_events
from .surface import Surface

class DisplayModule:
    def set_mode(self, resolution, flags=0):
        capsule = _c_set_mode(resolution, flags)
        return Surface(capsule)

    def flip(self):
        return _c_flip()

class EventModule:
    def get(self):
        return _c_pump_events()

# Constants for event handling
QUIT = 256
KEYDOWN = 768
KEYUP = 769
