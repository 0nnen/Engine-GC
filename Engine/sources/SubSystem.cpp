#include "SubSystem.h"

// premi�re option pour instancier le manager //
CollisionModule* gCollisionsModule; // global variable


// deuxi�me option pour instancier le manager //
std::unordered_map<std::type_index, void*> SubSystem::instances;

template <class T>
void SubSystem::Set(T* instance)
{
	instances[typeid(T)] = instance;
}

template <class T>
T* SubSystem::Get()
{
	auto it = instances.find(typeid(T));
	if (it != instances.end()) return static_cast<T*>(it->second);
	return nullptr;
}

void startUp()
{
	// premi�re option pour instancier le manager //
	gCollisionsModule = new CollisionModule;
	gCollisionsModule->start();

	// deuxi�me option pour instancier le manager //
	SubSystem::Set<CollisionModule>(&CollisionModule::get());
	// ou SubSystem::Set<CollisionModule>(new CollisionModule());
	SubSystem::Get<CollisionModule>()->start();
}
