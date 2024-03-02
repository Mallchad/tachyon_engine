#pragma once

#include "SDL.h"
#include "Components.h"
#include "../Header Files/TextureManager.h"

class SpriteComponent : public Component
{
public:

	SpriteComponent() = default;
	
	SpriteComponent(const char* path)
	{
		setTexture(path);
	}
	
	~SpriteComponent()
	{
		SDL_DestroyTexture(texture);
	}

	void setTexture(const char* path)
	{
		texture = TextureManager::LoadTexture(path);
	}

	void init() override
	{
		
		transform = &entity->getComponent<TransformComponent>();

		srcRect.x = srcRect.y = 0;
		
		srcRect.w = transform->width;
		srcRect.h = transform->height;

		destRect.w = destRect.h = 64;
	}

	void update() override
	{
		destRect.x = (int)transform->position.x;
		destRect.y = (int)transform->position.y;

		destRect.h = transform->height * transform->scale;
		destRect.w = transform->width * transform->scale;
	}

	void draw() override
	{
		TextureManager::Draw(texture, srcRect, destRect);
	}

private:
	TransformComponent* transform;
	SDL_Texture* texture;
	SDL_Rect srcRect, destRect;
};