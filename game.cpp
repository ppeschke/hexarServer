#include <iostream>
using namespace std;
#include "game.h"

Game::Game(unsigned int pN)
{
	playerNum = pN;
	over = true;
	turn = 1;
	players = new player[playerNum];
	if (players == nullptr)
	{
		cout << "Could not allocate memory for players... aborting." << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

Game::~Game(void)
{
	if (players != nullptr)
		delete[] players;
	CleanObjs();
}


void Game::CleanObjs()
{
	for(list<base*>::iterator index = objects.begin(); index != objects.end(); ++index)
	{
		delete (*index);
	}
}