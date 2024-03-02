#pragma once
#include "Game.h"

class TileMap
{


public:

	TileMap();
	~TileMap();

	void LoadTileMap(int arr[20][25]);
	void DrawTileMap();

private:

	SDL_Rect srcRect, destRect; 
	SDL_Texture* dirt;
	SDL_Texture* grass;
	SDL_Texture* water;

	int tileMap[20][25];
};