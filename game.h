#pragma once

#include <list>
using namespace std;

#include "player.h"
#include "base.h"

class Game
{
public:
	Game(void);
	~Game(void);

	void CleanObjs();

	list<base*> objects;
	player players[6];
	bool over;
	int playerNum;
	int turn;
};
