#include "base.h"

base::base(float _x, float _z, color _c)
{
	x = _x;
	z = _z;
	c = _c;
}

base::~base(void)
{
}

bool base::coords(int _i, int _p)
{
	return i == _i && p == _p;
}

bool base::rcoords(int, int)
{
	return false;
}