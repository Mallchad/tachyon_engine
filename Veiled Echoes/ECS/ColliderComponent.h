#pragma once

#include <string>
#include "SDL.h"
#include "Components.h"

class ColliderComponent : public Component
{

public:

	void init() override
	{
		if (!entity->hasComponent<TransformComponent>())
		{
			entity->addComponent<TransformComponent>();
		}
		transform = &entity->getComponent<TransformComponent>();
	}

	void update() override
	{
		collider.x = transform->position.x;
		collider.y = transform->position.y;
		collider.h = transform->height * transform->scale;
		collider.w = transform->width * transform->scale;
	}

private:
	SDL_Rect collider;
	std::string tag;
	TransformComponent* transform;

};