#pragma once

#include "../Header Files/Game.h"
#include "../ECS/ECS.h"
#include "../ECS/Components.h"

class KeyboardController : public Component
{

public:
	TransformComponent* transform;

	void init() override
	{
		
	}

	void update() override
	{
		if (Game::event.type == SDL_KEYDOWN)
		{

			switch (Game::event.key.keysym.sym)
			{
			case SDLK_w:
				//transforms here
				break;
			
			case SDLK_a:	
				
				break;
			
			case SDLK_s:
				
				break;
			
			case SDLK_d:
				
				break;

			default:
				break;
			}
		}
	}
};



