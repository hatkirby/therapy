#include "static_image.h"

StaticImageComponent::StaticImageComponent(const char* filename) : sprite(Texture(filename))
{
  
}

void StaticImageComponent::render(Game&, Entity& entity, Texture& buffer)
{
  buffer.blit(sprite, sprite.entirety(), {(int) entity.position.first, (int) entity.position.second, entity.size.first, entity.size.second});
}
