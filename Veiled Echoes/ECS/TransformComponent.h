#pragma once
#include "../ECS/Components.h"
#include "../Header Files/Vector2D.h"


class TransformComponent : public Component
{
public:

	Vector2D position;
	Vector2D velocity;

	int speed;

	void init() override
	{


	}

	void update() override
	{

	}

	TransformComponent(float x, float y)
	{
		position.x = 0;
		position.y = 0;
	}

	TransformComponent()
	{
		position.x = 0;
		position.y = 0;
	}

};