"""
pyda.pygame.rect: Advanced 2D bounding box and geometry math.
"""

class Rect:
    def __init__(self, x, y, width, height):
        self._x = float(x)
        self._y = float(y)
        self._w = float(width)
        self._h = float(height)

    @property
    def x(self): return int(self._x)
    @x.setter
    def x(self, val): self._x = float(val)

    @property
    def y(self): return int(self._y)
    @y.setter
    def y(self, val): self._y = float(val)

    @property
    def width(self): return int(self._w)
    @width.setter
    def width(self, val): self._w = float(val)

    @property
    def height(self): return int(self._h)
    @height.setter
    def height(self, val): self._h = float(val)

    @property
    def left(self): return self.x
    @left.setter
    def left(self, val): self._x = float(val)

    @property
    def right(self): return self.x + self.width
    @right.setter
    def right(self, val): self._x = float(val) - self._w

    @property
    def top(self): return self.y
    @top.setter
    def top(self, val): self._y = float(val)

    @property
    def bottom(self): return self.y + self.height
    @bottom.setter
    def bottom(self, val): self._y = float(val) - self._h

    def colliderect(self, other):
        return (
            self.x < other.x + other.width and
            self.x + self.width > other.x and
            self.y < other.y + other.height and
            self.y + self.height > other.y
        )

    def inflate(self, dx, dy):
        """Returns a new rect inflated by given pixel amounts."""
        return Rect(self._x - dx / 2, self._y - dy / 2, self._w + dx, self._h + dy)

    def move(self, dx, dy):
        """Returns a translated rect."""
        return Rect(self._x + dx, self._y + dy, self._w, self._h)

    def __repr__(self):
        return f"<Rect(x={self.x}, y={self.y}, w={self.width}, h={self.height})>"
      
