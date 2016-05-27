#pragma once
#include "base.h"

class hexagon : public base
{
public:
	hexagon(float _x, float _z, color _c, int _i, int _p);
	~hexagon(void);

	virtual void onStep();
	virtual bool coords(int, int);
	virtual string type()
	{
		return _type;
	}
};