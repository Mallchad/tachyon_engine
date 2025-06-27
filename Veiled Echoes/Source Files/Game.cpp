#include "../Header Files/Game.h"
#include "../Header Files/TextureManager.h"
#include "../Header Files/TileMap.h"
#include "../ECS/Components.h"
#include "../Header Files/Collision.h"


TileMap* tileMap;
EntityManager manager;

SDL_Renderer* Game::renderer = nullptr;

SDL_Event Game::event;


std::vector<ColliderComponent*> Game::colliders;

auto& player(manager.addEntity());
//auto& wall(manager.addEntity());


Game::Game()
{

}

Game::~Game()
{

}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
{
	int flags = 0;

	if (fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) //Check if shit initialized properly
	{
		std::cout << "SDL has initialized successfully! YIPPIE!" << std::endl;
		
		window = SDL_CreateWindow(
			title,
			xpos,
			ypos,
			width,
			height,
			flags
		);
		
		if (window)
		{
			std::cout << "Window created successfully! YIPPIE!" << std::endl;
		}

		renderer = SDL_CreateRenderer(
			window,
			0,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		if (renderer)
		{
			SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
			std::cout << "Renderer created successfully! YIPPIE!" << std::endl;
		}

		isRunning = true;
	
		//Create Tilemap
		tileMap = new TileMap();

		//ECS Implementation

		TileMap::LoadTileMap("Assets/p32.map", 32, 32);

		player.addComponent<TransformComponent>(1);
		player.addComponent<SpriteComponent>("Assets/player.png");
		player.addComponent<ColliderComponent>("player");
		player.addComponent<KeyboardController>();

		//wall.addComponent<TransformComponent>(150.0f, 150.0f, 32, 32, 1);
		//wall.addComponent<SpriteComponent>("Assets/dirt.png");
		//wall.addComponent<ColliderComponent>("wall");
	}
	else { isRunning = false; }
}

void Game::HandleEvents()
{
	
	SDL_PollEvent(&event);
	
	switch (event.type)
	{
	case SDL_QUIT:
		isRunning = false;
		break;

	default:
			break;
	}
}

void Game::Update()
{
	manager.refresh();
	manager.update();

	for (auto cc : colliders)
	{
		Collision::AABB(player.getComponent<ColliderComponent>(), *cc); // Debug collision check
	}
}

void Game::Render()
{
	SDL_RenderClear(renderer);
	// Render shit here
	manager.draw();
	
	SDL_RenderPresent(renderer);
}

void Game::Clean()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	std::cout << "Cleaned shit up" << std::endl;
}

void Game::AddTile(int id, int x, int y)
{
	auto& tile(manager.addEntity());
	tile.addComponent<TileComponent>(x, y, 32, 32, id);
}