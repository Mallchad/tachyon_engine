#pragma once
#include "ECS.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "SDL.h"

class TileComponent : public Component
{

public:
	TransformComponent *transform;
	SpriteComponent *sprite;

	SDL_Rect tileRect;
	int tileID;
	const char* path;


	TileComponent() = default;

	TileComponent(int x, int y, int w, int h, int id)
	{
		tileRect.x = x;
		tileRect.y = y;
		tileRect.w = w;
		tileRect.h = h;
		tileID = id;

		switch (tileID)
		{
		case 0:
			path = "Assets/water.png";
				break;
		case 1:
			path = "Assets/dirt.png";
			break;
		case 2:
			path = "Assets/grass.png";
			break;
		case 3:
			path = "Assets/water.png";
			break;
		default:
			break;
		}
	};

	void init() override
	{
		entity->addComponent<TransformComponent>(static_cast<float>(tileRect.x), static_cast<float>(tileRect.y), static_cast<float>(tileRect.w), static_cast<float>(tileRect.h), 1);
		transform = &entity->getComponent<TransformComponent>();

		entity->addComponent<SpriteComponent>(path);
		sprite = &entity->getComponent<SpriteComponent>();
	}
};