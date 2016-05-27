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

bool base::coords(int, int)
{
	return false;
}

bool base::rcoords(int, int)
{
	return false;
}