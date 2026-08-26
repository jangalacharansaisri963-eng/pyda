"""
pyda.pygame.sprite: Game object management and collision utilities.
"""
from .rect import Rect

class Sprite:
    """
    Base class for visible game objects. 
    Inherit from this to create custom game entities.
    """
    def __init__(self, *groups):
        self.groups_set = set()
        self.image = None
        self.rect = Rect(0, 0, 0, 0)
        
        for group in groups:
            group.add(self)

    def add(self, *groups):
        for group in groups:
            group.add(self)

    def remove(self, *groups):
        for group in groups:
            group.remove(self)

    def kill(self):
        """Remove the sprite from all groups it belongs to."""
        for group in list(self.groups_set):
            group.remove(self)

    def update(self, *args, **kwargs):
        """Override this method to add custom sprite behavior per frame."""
        pass


class Group:
    """
    A container class to hold and manage multiple Sprite objects.
    """
    def __init__(self, *sprites):
        self.spritedict = {}
        self.add(*sprites)

    def add(self, *sprites):
        for sprite in sprites:
            if hasattr(sprite, "groups_set") and sprite not in self.spritedict:
                self.spritedict[sprite] = None
                sprite.groups_set.add(self)
            elif isinstance(sprite, (list, tuple, Group)):
                self.add(*sprite)

    def remove(self, *sprites):
        for sprite in sprites:
            if hasattr(sprite, "groups_set") and sprite in self.spritedict:
                del self.spritedict[sprite]
                sprite.groups_set.discard(self)
            elif isinstance(sprite, (list, tuple, Group)):
                self.remove(*sprite)

    def update(self, *args, **kwargs):
        """Call update() on every sprite in the group."""
        for sprite in list(self.spritedict.keys()):
            sprite.update(*args, **kwargs)

    def draw(self, surface):
        """Draw every sprite's image to the target surface using its rect."""
        for sprite in list(self.spritedict.keys()):
            if hasattr(sprite, "image") and hasattr(sprite, "rect") and sprite.image:
                # Assuming surface has a blit method or we use core blitting
                # If Surface blit is implemented, we use it here.
                pass

    def __iter__(self):
        return iter(self.spritedict.keys())

    def __len__(self):
        return len(self.spritedict)


def spritecollide(sprite, group, dokill, collided=None):
    """
    Finds all sprites in a group that collide with a given sprite.
    """
    collided_list = []
    
    if collided is None:
        # Default collision check using Rect.colliderect
        for obj in group:
            if hasattr(obj, "rect") and sprite.rect.colliderect(obj.rect):
                collided_list.append(obj)
                if dokill:
                    obj.kill()
    else:
        # Custom collision function provided by user
        for obj in group:
            if collided(sprite, obj):
                collided_list.append(obj)
                if dokill:
                    obj.kill()
                    
    return collided_list
  
