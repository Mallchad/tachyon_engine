#include "../Header Files/Game.h"

Game *game = nullptr;

int main(int argc, char* argv[])
{

	const int FPS = 60;
	const int frameDelay = 1000 / FPS;

	Uint32 frameStart;
	int frameTime;


	game = new Game();

	game->init("Veiled Echoes", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, false);

	while (game->Running())
	{
		frameStart = SDL_GetTicks();
		
		
		
		game->HandleEvents();
		game->Update();
		game->Render();
		

		frameTime = SDL_GetTicks() - frameStart;
	
		if (frameDelay > frameTime)
		{
			SDL_Delay(frameDelay - frameTime);
		}
	}
	
	game->Clean();

	return EXIT_SUCCESS;
}