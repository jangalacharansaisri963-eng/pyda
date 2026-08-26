"""
pyda.pygame.time: Clock and timing module.
"""
from ._core import get_ticks, delay

class Clock:
    def __init__(self):
        self._last_time = get_ticks()
        self._framerate = 0

    def tick(self, framerate=0):
        current_time = get_ticks()
        elapsed = current_time - self._last_time
        self._last_time = current_time

        if framerate > 0:
            target_delay = (1000.0 / framerate) - elapsed
            if target_delay > 0:
                delay(int(target_delay))
                current_time = get_ticks()
                elapsed = current_time - (self._last_time - elapsed)

        if elapsed > 0:
            self._framerate = 1000.0 / elapsed
        else:
            self._framerate = 0.0

        return elapsed

    def get_fps(self):
        return round(self._framerate, 1)
      
