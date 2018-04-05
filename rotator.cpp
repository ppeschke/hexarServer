#include "rotator.h"

const float PI = 3.14159f;

rotator::rotator(float _x, float _z, color _c, int _i, int _p, string _t) : base(_x, _z, _c)
{
	_type = _t;
	i = _i;
	p = _p;
}

rotator::~rotator(void)
{
}

void rotator::onStep()
{
}

bool rotator::rcoords(int _i, int _p)
{
	return (_i == i && _p == p);
}