#include "../Header Files/Game.h"
#include "../Header Files/TextureManager.h"
#include "../Header Files/TileMap.h"
#include "../ECS/Components.h"


TileMap* tileMap;
EntityManager manager;

SDL_Renderer* Game::renderer = nullptr;

SDL_Event Game::event;


auto& player(manager.addEntity());

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
		player.addComponent<TransformComponent>();
		player.addComponent<SpriteComponent>("Assets/player.png");
		player.addComponent<ColliderComponent>();
		player.addComponent<KeyboardController>();

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
}

void Game::Render()
{
	SDL_RenderClear(renderer);
	// Render shit here
	tileMap->DrawTileMap();
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