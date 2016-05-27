#pragma once
#include "base.h"

class rotator : public base
{
public:
	rotator(float _x, float _z, color _c, int _i, int _p, string _t);
	~rotator(void);

	virtual void onStep();
	virtual bool rcoords(int, int);
	virtual string type()
	{
		return _type;
	}
};
