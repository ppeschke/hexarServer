#include "game.h"

Game::Game(unsigned int pN)
{
	playerNum = pN;
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