#pragma once
#include "CollisionManager.h"
#include "RenderManager.h"

class SubSystem
{
	SubSystem() {
		// do nothing
	}

	~SubSystem() {
		// do nothing
	}

	void startUp()
	{
		// l'ordre dans lequel on va start les diff�rents manager
	}

	void shutDown()
	{
		// l'ordre dans lequel on va shutDown les diff�rents manager
	}
};