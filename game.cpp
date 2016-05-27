#include "game.h"

Game::Game(void)
{
	over = true;
	turn = 1;
}

Game::~Game(void)
{
	CleanObjs();
}


void Game::CleanObjs()
{
	for(list<base*>::iterator index = objects.begin(); index != objects.end(); ++index)
	{
		delete (*index);
	}
}