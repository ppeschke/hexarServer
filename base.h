#pragma once
#include "global.h"
#include <string>
using namespace std;

class base
{
public:
	base(float x, float z, color c);
	virtual ~base(void);

	virtual void onStep() = 0;
	virtual bool coords(int, int);
	virtual bool rcoords(int, int);
	virtual string type() = 0;

	float x, z;
	color c;
	int i;
	int p;
	string _type;
};
