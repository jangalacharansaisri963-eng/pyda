"""
pyda.pygame.mixer: Audio mixing and playback module.
"""
from ._mixer import (
    init as _mixer_init,
    quit as _mixer_quit,
    load_sound,
    play_sound,
    load_music,
    play_music,
    stop_music
)

def init(frequency=44100, size=-16, channels=2, buffer=1024):
    return _mixer_init(frequency, size, channels, buffer)

def quit():
    return _mixer_quit()

class Sound:
    def __init__(self, filename):
        self._capsule = load_sound(filename)

    def play(self, loops=0):
        return play_sound(self._capsule, loops)

class MusicModule:
    def load(self, filename):
        self._music_capsule = load_music(filename)

    def play(self, loops=-1):
        play_music(self._music_capsule, loops)

    def stop(self):
        stop_music()

music = MusicModule()
