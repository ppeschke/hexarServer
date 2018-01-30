#pragma once

#include <list>
using namespace std;

#include "player.h"
#include "base.h"
#include "NetworkServer.h"

class Game
{
public:
	Game(unsigned int pN);
	~Game(void);

	void CleanObjs();

	list<base*> objects;
	player players[CLIENTS];
	bool over;
	int playerNum;
	int turn;
};
