#include "Scene/BaseScene.h"
#include "GameObject/GameObject.h"
#include <fstream>
#include <string>
#include <iostream>

/**
 * @brief Destructeur de la sc�ne.
 * R�initialise les indicateurs de chargement et d'initialisation, et d�truit tous les objets racine de la sc�ne.
 */
void BaseScene::Destroy()
{
	m_bLoaded = false;
	m_bInitialized = false;

	for (GameObject* rootObject : m_RootObjects)
	{
		if (rootObject != nullptr)
		{
			for (Component* component : rootObject->GetComponents())
			{
				rootObject->RemoveComponent(component);
			}
			delete rootObject;
		}
	}
	m_RootObjects.clear();
}

/**
 * @brief Ajoute un objet de jeu en tant qu'objet racine � la sc�ne.
 * @param gameObject Pointeur vers l'objet de jeu � ajouter.
 * @return Pointeur vers l'objet de jeu ajout�.
 */
GameObject* BaseScene::AddRootObject(GameObject* gameObject)
{
	if (gameObject == nullptr)
	{
		return nullptr;
	}

	m_PendingAddObjects.push_back(gameObject);

	return gameObject;
}

/**
 * @brief Supprime tous les objets de la sc�ne � la fin du frame.
 */
void BaseScene::RemoveAllObjects()
{
	for (GameObject* rootObject : m_RootObjects)
	{
		m_PendingDestroyObjects.push_back(rootObject->GetId());
	}
}

/**
 * @brief V�rifie si la sc�ne est initialis�e.
 * @return true si la sc�ne est initialis�e, sinon false.
 */
bool BaseScene::IsInitialized() const
{
	return m_bInitialized;
}

/**
 * @brief D�finit le nom de la sc�ne.
 * @param name Nouveau nom de la sc�ne.
 */
void BaseScene::SetName(const std::string& name)
{
	m_Name = name;
}

/**
 * @brief R�cup�re le nom de la sc�ne.
 * @return Le nom de la sc�ne.
 */
std::string BaseScene::GetName() const
{
	return m_Name;
}

/**
 * @brief V�rifie si la sc�ne utilise un fichier de sauvegarde.
 * @return true si la sc�ne utilise un fichier de sauvegarde, sinon false.
 */
bool BaseScene::IsUsingSaveFile() const
{
	return false;
}

/**
 * @brief R�cup�re les objets racine de la sc�ne.
 * @return Une r�f�rence vers le vecteur contenant les objets racine de la sc�ne.
 */
std::vector<GameObject*>& BaseScene::GetRootObjects()
{
	return m_RootObjects;
}


/**
 * @brief R�cup�re le nom du fichier de la sc�ne.
 * @return Le nom du fichier de la sc�ne.
 */
std::string BaseScene::GetFileName() const
{
	return m_FileName;
}

/**
 * @brief D�finit le nom du fichier de la sc�ne.
 * @param fileName Nouveau nom du fichier de la sc�ne.
 * @param bDeletePreviousFiles Indique s'il faut supprimer les fichiers pr�c�dents.
 * @return true si le nom du fichier est d�fini avec succ�s, sinon false.
 */
bool BaseScene::SetFileName(const std::string& fileName, bool bDeletePreviousFiles)
{
	bool success = false;

	if (fileName == m_FileName)
	{
		return true;
	}
}

/**
 * @brief R�cup�re le chemin de fichier par d�faut de la sc�ne.
 * @return Le chemin de fichier par d�faut de la sc�ne.
 */
std::string BaseScene::GetDefaultRelativeFilePath() const
{
	//A MODIFIIIIEEEEEEEEEERRRRRR
	const std::string DEFAULT_SCENE_DIRECTORY = "assets/scenes/";
	return DEFAULT_SCENE_DIRECTORY + m_FileName;
}

/**
 * @brief V�rifie si un fichier existe.
 * @param filePath Chemin du fichier � v�rifier.
 * @return true si le fichier existe, sinon false.
 */

bool FileExists(const std::string& filePath) {
	std::ifstream file(filePath);
	return file.good();
}

/**
 * @brief Supprime les fichiers de sauvegarde de la sc�ne.
 */
void BaseScene::DeleteSaveFiles()
{
	const std::string defaultSaveFilePath = "assets/scenes/" + m_FileName;
	const std::string savedSaveFilePath = "saved/scenes/" + m_FileName;

	bool bDefaultFileExists = FileExists(defaultSaveFilePath);
	bool bSavedFileExists = FileExists(savedSaveFilePath);

	if (bSavedFileExists || bDefaultFileExists)
	{
		std::cout << "Deleting scene's save files from " << m_FileName << std::endl;

		if (bDefaultFileExists)
		{
			if (remove(defaultSaveFilePath.c_str()) != 0)
			{
				std::cerr << "Error deleting default save file: " << defaultSaveFilePath << std::endl;
			}
			else
			{
				std::cout << "Default save file deleted successfully." << std::endl;
			}
		}

		if (bSavedFileExists)
		{
			if (remove(savedSaveFilePath.c_str()) != 0)
			{
				std::cerr << "Error deleting saved save file: " << savedSaveFilePath << std::endl;
			}
			else
			{
				std::cout << "Saved save file deleted successfully." << std::endl;
			}
		}
	}
}

/**
 * @brief V�rifie si un �l�ment est contenu dans un vecteur.
 * @param container Vecteur dans lequel chercher.
 * @param value Valeur � chercher.
 * @return true si la valeur est trouv�e dans le vecteur, sinon false.
 */
bool Contains(const std::vector<GameObject::id_t>& container, const GameObject::id_t& value)
{
	return std::find(container.begin(), container.end(), value) != container.end();
}

/**
 * @brief Supprime un objet de jeu de la sc�ne.
 * @param gameObjectID Identifiant de l'objet de jeu � supprimer.
 * @param bDestroy Indique s'il faut d�truire l'objet de jeu.
 */
void BaseScene::RemoveObject(const GameObject::id_t& gameObjectID, bool bDestroy)
{
	if (bDestroy)
	{
		if (!Contains(m_PendingDestroyObjects, gameObjectID))
		{
			m_PendingDestroyObjects.push_back(gameObjectID);
		}
	}
	else
	{
		if (!Contains(m_PendingRemoveObjects, gameObjectID))
		{
			m_PendingRemoveObjects.push_back(gameObjectID);
		}
	}
}

/**
 * @brief Supprime un objet de jeu de la sc�ne.
 * @param gameObject Pointeur vers l'objet de jeu � supprimer.
 * @param bDestroy Indique s'il faut d�truire l'objet de jeu.
 */
void BaseScene::RemoveObject(GameObject* gameObject, bool bDestroy)
{
	RemoveObject(gameObject->GetId(), bDestroy);
}


/**
 * @brief Supprime plusieurs objets de jeu de la sc�ne.
 * @param gameObjects Vecteur contenant les identifiants des objets de jeu � supprimer.
 * @param bDestroy Indique s'il faut d�truire les objets de jeu.
 */
void BaseScene::RemoveObjects(const std::vector<GameObject::id_t>& gameObjects, bool bDestroy)
{
	if (bDestroy)
	{
		m_PendingDestroyObjects.insert(m_PendingDestroyObjects.end(), gameObjects.begin(), gameObjects.end());
	}
	else
	{
		m_PendingRemoveObjects.insert(m_PendingRemoveObjects.end(), gameObjects.begin(), gameObjects.end());
	}
}

/**
 * @brief Supprime plusieurs objets de jeu de la sc�ne.
 * @param gameObjects Vecteur contenant les pointeurs vers les objets de jeu � supprimer.
 * @param bDestroy Indique s'il faut d�truire les objets de jeu.
 */
void BaseScene::RemoveObjects(const std::vector<GameObject*>& gameObjects, bool bDestroy)
{
	if (bDestroy)
	{
		m_PendingDestroyObjects.reserve(m_PendingDestroyObjects.size() + gameObjects.size());
		for (GameObject* gameObject : gameObjects)
		{
			if (!Contains(m_PendingDestroyObjects, gameObject->GetId()))
			{
				m_PendingDestroyObjects.push_back(gameObject->GetId());
			}
		}
	}
	else
	{
		m_PendingRemoveObjects.reserve(m_PendingRemoveObjects.size() + gameObjects.size());
		for (GameObject* gameObject : gameObjects)
		{
			if (!Contains(m_PendingRemoveObjects, gameObject->GetId()))
			{
				m_PendingRemoveObjects.push_back(gameObject->GetId());
			}
		}
	}
}


/**
 * @brief Cr�e un nouvel objet de jeu.
 * @return Pointeur vers le nouvel objet de jeu cr��.
 */
GameObject* BaseScene::CreateGameObject() {
	GameObject* gameObject = new GameObject();
	m_PendingAddObjects.push_back(gameObject);
	return gameObject;
}

/**
 * @brief D�truit un objet de jeu.
 * @param gameObject Pointeur vers l'objet de jeu � d�truire.
 */

void BaseScene::DestroyGameObject(GameObject* gameObject) {
	if (gameObject != nullptr) {
		RemoveObject(gameObject, true);
	}
}

/**
 * @brief R�cup�re un objet de jeu par son identifiant.
 * @param gameObjectID Identifiant de l'objet de jeu � r�cup�rer.
 * @return Pointeur vers l'objet de jeu correspondant � l'identifiant.
 */
GameObject* BaseScene::GetGameObjectById(const GameObject::id_t& gameObjectID) {
	for (GameObject* rootObject : m_RootObjects) {
		if (rootObject->GetId() == gameObjectID) {
			return rootObject;
		}
	}
	return nullptr;
}

/**
 * @brief Recherche des objets de jeu par leur nom.
 * @param name Nom des objets de jeu � rechercher.
 * @return Vecteur contenant les pointeurs vers les objets de jeu trouv�s.
 */
std::vector<GameObject*> BaseScene::FindGameObjectsByName(const std::string& name) {
	std::vector<GameObject*> result;
	for (GameObject* rootObject : m_RootObjects) {
		std::vector<GameObject*> found = rootObject->FindChildrenByName(name);
		result.insert(result.end(), found.begin(), found.end());
	}
	return result;
}


/**
 * @brief Effectue le rendu de la sc�ne.
 * @param _window Fen�tre de rendu.
 */
void BaseScene::Render(lve::LveWindow* _window)
{
	// Rendu de chaque objet de la sc�ne
	for (GameObject* rootObject : m_RootObjects)
	{
		//RenderObject(rootObject);
	}
}

/**
 * @brief Met � jour la sc�ne.
 * @param deltaTime Temps �coul� depuis la derni�re mise � jour.
 */
void BaseScene::Update(float deltaTime)
{
	// Mettez � jour chaque objet de la sc�ne avec le delta time
	for (GameObject* rootObject : m_RootObjects)
	{
		rootObject->Update(deltaTime); // Mettez � jour chaque objet avec le delta time
	}
}