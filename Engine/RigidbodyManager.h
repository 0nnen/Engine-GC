#pragma once
#include "GameObject.h"

#include <vector>

class RigidbodyManager
{

private:
	std::vector<GameObject*> rigidbody;

public:
	RigidbodyManager() {}

	~RigidbodyManager() {}

	void addRigidbody(GameObject* gameObject)
	{
		rigidbody.push_back(gameObject);
	}

	void removeRigidbody(GameObject* gameObject) {} // enlever le rigidbody cibl�

	void Update()
	{
		for (GameObject* gameObject : rigidbody)
		{

		}
	}

};

