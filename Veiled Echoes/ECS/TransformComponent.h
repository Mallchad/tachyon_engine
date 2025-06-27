#pragma once
#include "../ECS/Components.h"
#include "../Header Files/Vector2D.h"


class TransformComponent : public Component
{
public:

	Vector2D position;
	Vector2D velocity;

	int height = 72; //Height of single sprite on sprite sheet (eg. see player.png)
	int width = 48; //Width of single sprite on sprite sheet
	int scale = 1;

	float speed = 5.0f;

	void init() override
	{
		velocity.Zero();

	}

	void update() override
	{
		position.x += velocity.x * speed;
		position.y += velocity.y * speed;
	}

	TransformComponent(float x, float y)
	{
		position.Zero();
	}

	TransformComponent(float x, float y, int h, int w, int s)
	{
		position.x = x;
		position.y = y;
		height = h;
		width = w;
		scale = s;
	}

	TransformComponent(int sc)
	{
		position.Zero();
		scale = sc;
	}

	TransformComponent()
	{
		position.Zero();
	}


};