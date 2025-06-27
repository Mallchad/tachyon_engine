#pragma once
#include <string>

class TileMap
{


public:

	TileMap();
	~TileMap();

	static void LoadTileMap(std::string path, int sizeX, int sizeY);
	void DrawTileMap();

private:
};
