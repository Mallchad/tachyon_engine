#include "../Header Files/GameObject.h"
#include "../Header Files/TextureManager.h"

GameObject::GameObject(const char* texturesheet)
{;
	objTexture = TextureManager::LoadTexture(texturesheet);
}

void GameObject::Update()
{
	xpos = 0;
	ypos = 0;

	srcRect.h = 72;
	srcRect.w = 48;
	srcRect.x = 0;
	srcRect.y = 0;

	destRect.x = xpos;
	destRect.y = ypos;
	destRect.w = srcRect.w * 1;
	destRect.h = srcRect.h * 1;
}

void GameObject::Render()
{

	SDL_RenderCopy(Game::renderer, objTexture, &srcRect, &destRect);
}