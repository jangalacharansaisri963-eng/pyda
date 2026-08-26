"""
pyda.pygame.key: Keyboard device state queries.
"""
from ._core import get_pressed as _c_get_pressed

def get_pressed():
    return _c_get_pressed()

# Key Constants mapping (subset of standard SDL key codes)
K_BACKSPACE = 8
K_TAB = 9
K_RETURN = 13
K_ESCAPE = 27
K_SPACE = 32
K_LEFT = 1073741904
K_RIGHT = 1073741903
K_UP = 1073741906
K_DOWN = 1073741905
K_a = 97
K_b = 98
K_c = 99
K_d = 100
K_w = 119
K_s = 115
