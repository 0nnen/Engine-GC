#include "Managers/SceneManager.h"
#include "Managers/WindowManager.h"
#include "ModuleManager.h"
#include <fstream>
#include <sstream>


/**
 * @brief D�marre le gestionnaire de sc�nes.
 * Initialise le gestionnaire de fen�tres.
 */
void SceneManager::Start() {
	Module::Start();

	windowManager = moduleManager->GetModule<WindowManager>();

	mainScene = CreateScene("Default", true);
}

/**
 * @brief Cr�e une nouvelle sc�ne.
 * @param _name Nom de la nouvelle sc�ne.
 * @param _isActive Indique si la nouvelle sc�ne est active.
 */
BaseScene* SceneManager::CreateScene(std::string _name, bool _isActive)
{
	BaseScene* newScene = new BaseScene(_name);
	scenes.push_back(newScene);

	return newScene;
	//listScenes.insert(std::make_pair(_name, _isActive));
}

/**
 * @brief R�cup�re le nombre de sc�nes.
 * @return Le nombre de sc�nes.
 */
int SceneManager::SceneCount()
{
	int count = 0;
	if (!SceneManager::listScenes.empty())
	{
		count = SceneManager::listScenes.size();
	}
	return count;
}

/**
 * @brief R�cup�re le nom de la sc�ne active.
 * @return Le nom de la sc�ne active, ou "None" si aucune sc�ne n'est active.
 */
std::string SceneManager::GetActiveScene()
{
	std::string _name;
	bool isActive;
	for (const auto scene : listScenes)
	{
		_name = scene.first;
		isActive = scene.second;
		if (isActive)
		{
			return _name;
		}
	}
	return "None";
}

/**
 * @brief R�cup�re le nom de la premi�re sc�ne dans la liste des sc�nes.
 * @return Le nom de la premi�re sc�ne, ou "None" si la liste des sc�nes est vide.
 */
std::string SceneManager::GetListScenes()
{
	std::string _name;
	for (const auto scene : listScenes)
	{
		_name = scene.first;
		return _name;
	}
	return "None";
}

/**
 * @brief R�cup�re la sc�ne � un index donn� dans la liste des sc�nes.
 * @param _index Index de la sc�ne � r�cup�rer.
 * @return Une paire contenant le nom de la sc�ne et son �tat d'activit�, ou une paire "None" si l'index est invalide.
 */
std::pair<std::string, bool> SceneManager::GetSceneAt(int _index)
{
	if (!listScenes.empty())
	{
		auto findName = listScenes.begin();
		std::advance(findName, _index);
		return *findName;
	}
	return { "None", false };
}


/**
 * @brief Supprime une sc�ne de la liste des sc�nes.
 * @param sceneName Nom de la sc�ne � supprimer.
 */
void SceneManager::DestroyScene(const std::string& sceneName)
{
	auto it = listScenes.find(sceneName);
	if (it != listScenes.end())
	{
		listScenes.erase(it);
	}
}

/**
 * @brief D�finit la sc�ne courante par son index dans la liste des sc�nes.
 * @param sceneIndex Index de la sc�ne � d�finir comme sc�ne courante.
 */
void SceneManager::SetCurrentScene(int sceneIndex) {
	if (sceneIndex >= 0 && sceneIndex < static_cast<int>(scenes.size())) {
		currentSceneIndex = sceneIndex;
	}
}

/**
 * @brief Active la sc�ne suivante dans la liste des sc�nes.
 * Si la liste des sc�nes n'est pas vide, l'index de la sc�ne courante est mis � jour pour passer � la sc�ne suivante circulairement.
 */
void SceneManager::SetNextSceneActive() {
	if (!scenes.empty()) {
		currentSceneIndex = (currentSceneIndex + 1) % scenes.size();
	}
}

/**
 * @brief Active la sc�ne pr�c�dente dans la liste des sc�nes.
 * Si la liste des sc�nes n'est pas vide, l'index de la sc�ne courante est mis � jour pour passer � la sc�ne pr�c�dente circulairement.
 */
void SceneManager::SetPreviousSceneActive() {
	if (!scenes.empty()) {
		currentSceneIndex = (currentSceneIndex - 1 + scenes.size()) % scenes.size();
	}
}


/**
 * @brief R�cup�re la sc�ne courante.
 * @return Un pointeur vers la sc�ne courante si l'index de la sc�ne courante est valide, sinon nullptr.
 */
BaseScene* SceneManager::GetCurrentScene() const {
	if (currentSceneIndex >= 0 && currentSceneIndex < static_cast<int>(scenes.size())) {
		return scenes[currentSceneIndex];
	}
	return nullptr;//(currentSceneIndex >= 0 && currentSceneIndex < static_cast<int>(scenes.size())) ? scenes[currentSceneIndex].get() : nullptr;
}

/**
 * @brief D�truit toutes les sc�nes et vide la liste des sc�nes.
 * L'index de la sc�ne courante est r�initialis� � -1.
 */
void SceneManager::Destroy()
{
	if (currentSceneIndex >= 0 && currentSceneIndex < static_cast<int>(scenes.size()))
	{
		scenes[currentSceneIndex]->Destroy();
	}

	scenes.clear();
	currentSceneIndex = -1;
}

/**
 * @brief V�rifie si un fichier de sc�ne existe dans la liste des sc�nes.
 * @param fileName Nom du fichier de sc�ne � rechercher.
 * @return true si le fichier de sc�ne existe, sinon false.
 */
bool SceneManager::SceneFileExists(const std::string& fileName) const
{
	for (const auto& scene : scenes)
	{
		if (scene->GetFileName() == fileName)
		{
			return true;
		}
	}

	return false;
}


//--------------------------A MODIFIER TRES SIMPLIFIE------------------------------------
static GameObject::id_t nextGameObjectID = 0;
GameObject* CreateGameObjectFromSceneData(const std::string& sceneObjectName) {

	GameObject* gameObject = new GameObject();

	return gameObject;
}

/**
 * @brief Charge une sc�ne � partir d'un fichier.
 * @param fileName Nom du fichier de sc�ne � charger.
 * @return true si la sc�ne a �t� charg�e avec succ�s, sinon false.
 */
bool SceneManager::LoadSceneFromFile(const std::string& fileName) {
	// V�rifier si le fichier existe
	if (!SceneFileExists(fileName)) {
		std::cerr << "Erreur : Le fichier de sc�ne '" << fileName << "' n'existe pas." << std::endl;
		return false;
	}

	// Ouvrir le fichier de sc�ne en lecture
	std::ifstream file(fileName);
	if (!file.is_open()) {
		std::cerr << "Erreur : Impossible d'ouvrir le fichier de sc�ne '" << fileName << "'." << std::endl;
		return false;
	}

	// Lire le contenu du fichier de sc�ne
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string sceneData = buffer.str();

	// Fermer le fichier
	file.close();

	// Analyser les donn�es de la sc�ne
	std::istringstream sceneStream(sceneData);
	std::string sceneObjectName;
	while (sceneStream >> sceneObjectName) {
		GameObject* gameObject = CreateGameObjectFromSceneData(sceneObjectName);
		if (gameObject) {
			// Ajouter l'objet de sc�ne � la sc�ne courante
			GetCurrentScene()->AddRootObject(gameObject);
		}
		else {
			std::cerr << "Erreur : Impossible de cr�er l'objet de sc�ne '" << sceneObjectName << "'." << std::endl;
		}
	}

	std::cout << "Chargement de la sc�ne depuis le fichier '" << fileName << "' termin�." << std::endl;
	return true;
}

/**
 * @brief D�finit la sc�ne principale � utiliser.
 * @param sceneName Nom de la sc�ne � d�finir comme sc�ne principale.
 */
void SceneManager::SetMainScene(const std::string& sceneName) {
	auto it = listScenes.find(sceneName);
	if (it != listScenes.end()) {
		currentSceneIndex = std::distance(listScenes.begin(), it);
	}
}


/**
 * @brief R�cup�re une sc�ne par son nom.
 * @param sceneName Nom de la sc�ne � r�cup�rer.
 * @return Un pointeur vers la sc�ne si elle existe, sinon nullptr.
 */
BaseScene* SceneManager::GetScene(const std::string& sceneName) {
	for (BaseScene* scene : scenes)
	{
		if (scene->GetName() == sceneName)
		{
			return scene;
		}
	}

	return nullptr;
}

/**
 * @brief R�cup�re la sc�ne principale.
 * @return Un pointeur vers la sc�ne principale.
 */
std::vector<BaseScene*>& SceneManager::GetScenes() {
	return scenes;
}

/**
 * @brief Renomme une sc�ne dans la liste des sc�nes.
 * @param oldName Ancien nom de la sc�ne � renommer.
 * @param newName Nouveau nom de la sc�ne.
 */
void SceneManager::RenameScene(const std::string& oldName, const std::string& newName) {
	auto it = listScenes.find(oldName);
	if (it != listScenes.end()) {
		listScenes[newName] = it->second;
		listScenes.erase(it);
	}
}

/**
 * @brief Met � jour la sc�ne principale.
 * Appelle la m�thode Update de la sc�ne principale avec un delta time fixe.
 */
void SceneManager::UpdateMainScene()
{
	float deltaTime = 0.016f;
	mainScene->Update(deltaTime); // Appel de la m�thode Update de BaseScene
}

/**
 * @brief Effectue le rendu de la sc�ne principale.
 * Appelle la m�thode Render de la sc�ne principale en passant le pointeur de la fen�tre.
 */
void SceneManager::RenderMainScene()
{
	mainScene->Render(windowManager->GetWindow()); // Passe le pointeur de la fen�tre � la fonction Render
}

void SceneManager::MarkGameObjectForDeletion(GameObject* _gameObject) {
	// Ajoute le GameObject � une liste de suppression
	objectsToDelete.push_back(_gameObject);
}

void SceneManager::DeleteMarkedGameObjects() {
	for (auto* gameObject : objectsToDelete) {
		// Supprime l'objet ici
		delete gameObject;
	}
	objectsToDelete.clear();
}