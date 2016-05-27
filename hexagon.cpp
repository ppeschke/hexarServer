#include "hexagon.h"

hexagon::hexagon(float _x, float _z, color _c, int _i, int _p) : base(_x, _z, _c)
{
	i = _i;
	p = _p;
	_type = "hexagon";
}

hexagon::~hexagon(void)
{
}


void hexagon::onStep()
{
}

bool hexagon::coords(int _i, int _p)
{
	return (_i == i && _p == p);
}