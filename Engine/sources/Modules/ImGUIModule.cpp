#include "Modules/ImGUIModule.h"
#include "lve_renderer.h"
#include "ModuleManager.h"
#include "Modules/WindowModule.h"
#include "Modules/ImGUIModule.h"

#include "ImGUIInterface.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "rhi.h"

#include "Transform.h"
#include "Scene/SceneManager.h"
#include "TCP/Errors.h"

class BaseScene;
// ----------========== IMGUI SETTINGS ==========---------- //

void ImGuiModule::Init()
{
	Module::Init();

	windowModule = moduleManager->GetModule<WindowModule>();
	rhiModule = moduleManager->GetModule<RHIModule>();
	sceneManager = moduleManager->GetModule<SceneManager>();
	device                                             = rhiModule->GetDevice()->Device();
	graphicsQueue                                      = rhiModule->GetDevice()->GraphicsQueue();
	const lve::QueueFamilyIndices queue_family_indices = rhiModule->GetDevice()->FindPhysicalQueueFamilies();

	// Cr�ation du pool de commandes
	const vk::CommandPoolCreateInfo command_pool_info(
		vk::CommandPoolCreateFlags(),       // Flags de cr�ation
		queue_family_indices.graphicsFamily // Indice de la famille de file d'attente de commandes
	);
	immCommandPool = device.createCommandPool(command_pool_info);

	// Allocation du tampon de commande pour les soumissions imm�diates
	const vk::CommandBufferAllocateInfo cmd_alloc_info(
		immCommandPool,                   // Pool de commandes
		vk::CommandBufferLevel::ePrimary, // Niveau du tampon de commande
		1                                 // Nombre de tampons � allouer
	);
	immCommandBuffer = device.allocateCommandBuffers(cmd_alloc_info)[0];

	// Ajout de la fonction de suppression du pool de commandes � la file de suppression principale
	//_mainDeletionQueue.push_back([=]() {
	//	device.destroyCommandPool(_immCommandPool);
	//});
}

void ImGuiModule::Start()
{
	Module::Start();
	//timeModule = moduleManager->GetModule<TimeModule>();

	//ImGui::CreateContext();

	const vk::DescriptorPoolSize pool_sizes[] = {
		{vk::DescriptorType::eSampler, 1000},
		{vk::DescriptorType::eCombinedImageSampler, 1000},
		{vk::DescriptorType::eSampledImage, 1000},
		{vk::DescriptorType::eStorageImage, 1000},
		{vk::DescriptorType::eUniformTexelBuffer, 1000},
		{vk::DescriptorType::eStorageTexelBuffer, 1000},
		{vk::DescriptorType::eUniformBuffer, 1000},
		{vk::DescriptorType::eStorageBuffer, 1000},
		{vk::DescriptorType::eUniformBufferDynamic, 1000},
		{vk::DescriptorType::eStorageBufferDynamic, 1000},
		{vk::DescriptorType::eInputAttachment, 1000}
	};

	vk::DescriptorPoolCreateInfo pool_info = {};
	pool_info.sType                        = vk::StructureType::eDescriptorPoolCreateInfo;
	pool_info.flags                        = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	pool_info.maxSets                      = 1000;
	pool_info.poolSizeCount                = static_cast<uint32_t>(std::size(pool_sizes));
	pool_info.pPoolSizes                   = pool_sizes;

	vk::DescriptorPool im_gui_pool;
	if (rhiModule->GetDevice()->Device().createDescriptorPool(&pool_info, nullptr, &im_gui_pool) !=
	    vk::Result::eSuccess)
		throw std::runtime_error("Impossible de creer la pool de imgui!");

	// 2: initialize imgui library

	// this initializes the core structures of imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// this initializes imgui for SDL
	ImGui_ImplGlfw_InitForVulkan(windowModule->GetWindow()->GetGlfwWindow(), true);

	// this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance                  = rhiModule->GetDevice()->GetInstance();
	init_info.PhysicalDevice            = rhiModule->GetDevice()->GetPhysicalDevice();
	init_info.Device                    = device;
	init_info.Queue                     = graphicsQueue;
	init_info.DescriptorPool            = im_gui_pool;
	init_info.MinImageCount             = 3;
	init_info.ImageCount                = 3;
	init_info.RenderPass                = rhiModule->GetRenderer()->GetSwapChainRenderPass();
	//init_info.UseDynamicRendering = VK_TRUE;
	//init_info.ColorAttachmentFormat = _swapchainImageFormat;

	init_info.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);

	ImGui_ImplVulkan_Init(&init_info);

	// execute a gpu command to upload imgui font textures
	//immediate_submit([&](vk::CommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(); });

	// clear font textures from cpu data
	//ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiModule::FixedUpdate()
{
	Module::FixedUpdate();
}

void ImGuiModule::Update()
{
	Module::Update();

	// Mise � jour d'ImGui
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	GetGui();
}

void ImGuiModule::PreRender()
{
	Module::PreRender();
}

void ImGuiModule::Render()
{
	Module::Render();

	// Rendu d'ImGui
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), rhiModule->GetRenderer()->GetCurrentCommandBuffer());
}

void ImGuiModule::RenderGui()
{
	Module::RenderGui();
}

void ImGuiModule::PostRender()
{
	Module::PostRender();
}

void ImGuiModule::Release()
{
	Module::Release();
}

void ImGuiModule::Finalize()
{
	Module::Finalize();


	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();
}

void ImGuiModule::ImmediateSubmit(std::function<void(vk::CommandBuffer _cmd)>&& _function) const
{
	const vk::Device device = rhiModule->GetDevice()->Device();
	const vk::Queue  graphics_queue = rhiModule->GetDevice()->GraphicsQueue();

	device.resetFences(immFence);
	immCommandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

	const vk::CommandBuffer cmd = immCommandBuffer;

	vk::CommandBufferBeginInfo cmd_begin_info{};
	cmd_begin_info.sType = vk::StructureType::eCommandBufferBeginInfo;
	cmd_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	cmd.begin(cmd_begin_info);

	_function(cmd);

	cmd.end();

	vk::CommandBufferSubmitInfo cmd_info;
	cmd_info.sType = vk::StructureType::eCommandBufferSubmitInfo;
	cmd_info.pNext = nullptr;
	cmd_info.commandBuffer = cmd;
	cmd_info.deviceMask = 0;

	vk::SubmitInfo2 submitInfo{};
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmd_info;
	constexpr auto dispatcher = vk::DispatchLoaderDynamic();
	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	graphics_queue.submit2KHR(submitInfo, immFence, dispatcher);

	device.waitForFences(immFence, VK_TRUE, 9999999999);
}

// ----------========== IMGUI SHOWN ==========---------- //

void ImGuiModule::GetGui()
{
	ImVec2 mainWindowSize = ImGui::GetMainViewport()->Size;

	// Flags pour les fen�tres d�pla�ables et non-redimensionnables
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

	// Dessin de la fen�tre "Hierarchy" - Gauche
	ImGui::SetNextWindowSize(ImVec2(300, mainWindowSize.y), ImGuiCond_Always); // Hauteur fixe et non-redimensionnable
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always); // Ancrage en haut � gauche
	ImGui::Begin("Hierarchy", nullptr, window_flags);
	DrawHierarchyWindow();
	ImGui::End();

	// Dessin de la fen�tre "Inspector" - Droite
	ImGui::SetNextWindowSize(ImVec2(300, mainWindowSize.y), ImGuiCond_Always); // Hauteur fixe et non-redimensionnable
	ImGui::SetNextWindowPos(ImVec2(mainWindowSize.x - 300, 0), ImGuiCond_Always); // Ancrage en haut � droite
	ImGui::Begin("Inspector", nullptr, window_flags);
	DrawInspectorWindow();
	ImGui::End();

	DrawSettingsWindow();
}

void ImGuiModule::AnchorWindow(const std::string& _windowName)
{
	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 mainWindowPos = ImGui::GetMainViewport()->Pos;
	ImVec2 mainWindowSize = ImGui::GetMainViewport()->Size;

	// Ancrage � droite avec mise � jour dynamique lors du redimensionnement
	ImGui::SetWindowPos(_windowName.c_str(), ImVec2(mainWindowSize.x - 300, windowPos.y));

	// Ancrage � gauche ou � droite si n�cessaire
	if (windowPos.x < mainWindowPos.x + 50) {
		ImGui::SetWindowPos(_windowName.c_str(), ImVec2(mainWindowPos.x, windowPos.y));
	}
	else if (windowPos.x > mainWindowPos.x + mainWindowSize.x - 350) {
		// S'assurer que la fen�tre reste coll�e au bord droit m�me apr�s redimensionnement
		ImGui::SetWindowPos(_windowName.c_str(), ImVec2(mainWindowPos.x + mainWindowSize.x - 300, windowPos.y));
	}

	// Correction pour �viter le d�bordement par le haut ou par le bas
	if (windowPos.y < mainWindowPos.y) {
		ImGui::SetWindowPos(_windowName.c_str(), ImVec2(windowPos.x, mainWindowPos.y));
	}
	else if (windowPos.y + mainWindowSize.y > mainWindowPos.y + mainWindowSize.y) {
		ImGui::SetWindowPos(_windowName.c_str(), ImVec2(windowPos.x, mainWindowPos.y + mainWindowSize.y - mainWindowSize.y));
	}
}

// ----------========== DRAW WINDOWS ==========---------- //

void ImGuiModule::DrawInspectorWindow() {
	// V�rifier si un GameObject est s�lectionn�
	if (selectedGameObject) {
		// Affichage du nom du GameObject
		ImGui::Text("Name: ");
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.2f);
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), selectedGameObject->GetName().c_str());
		ImGui::SetWindowFontScale(1.0f);

		// Bouton pour afficher la popup de renommage
		ImGui::SameLine();
		if (ImGui::Button("Rename")) {
			isRenamePopupOpen = true;
			strncpy_s(renameBuffer, selectedGameObject->GetName().c_str(), sizeof(renameBuffer) - 1);
			renameBuffer[sizeof(renameBuffer) - 1] = '\0';
			ImGui::OpenPopup("Rename Entity");
		}
		ShowRenamePopup();

		// Affichage des propri�t�s de transformation
		if (ImGui::CollapsingHeader("Transform")) {
			DisplayTransform(selectedGameObject->GetTransform());
		}

		// Bouton pour ajouter un composant (logique d'ajout � impl�menter)
		if (ImGui::Button("Add Component")) {
			// TODO: Logique d'ajout de composant
		}
	}
	else {
		ImGui::Text("No GameObject selected");
	}
}

void ImGuiModule::DrawHierarchyWindow() {
	// Bouton pour cr�er un nouveau GameObject
	if (ImGui::Button("New GameObject")) {
		ImGui::OpenPopup("CreateGameObjectPopup");
	}
	ImGui::SameLine();
	// Bouton pour ajouter une nouvelle sc�ne
	if (ImGui::Button("Add New Scene")) {
		sceneManager->CreateScene("New Scene", false);
	}
	if (ImGui::BeginPopup("CreateGameObjectPopup")) {
		if (ImGui::MenuItem("Cube")) {
			CreateSpecificGameObject(GameObjectType::Cube);
			std::cout << "Added new GameObject-Cube to current scene." << std::endl;

		}
		if (ImGui::MenuItem("Light")) {
			CreateSpecificGameObject(GameObjectType::Light);
			std::cout << "Added new GameObject-Light to current scene." << std::endl;

		}
		if (ImGui::MenuItem("Plane")) {
			CreateSpecificGameObject(GameObjectType::Plane);
			std::cout << "Added new GameObject-Plane to current scene." << std::endl;

		}
		ImGui::EndPopup();
	}

	// Barre de recherche
	static char searchBuffer[100];
	ImGui::InputText("Search", searchBuffer, IM_ARRAYSIZE(searchBuffer));

	// Affichage des sc�nes et de leurs GameObjects
	const auto& scenes = sceneManager->GetScenes();
	for (size_t i = 0; i < scenes.size(); ++i) {
		ImGui::Spacing();
		ImGui::Separator();

		const auto& scene = scenes[i];
		bool isCurrentScene = (sceneManager->GetCurrentScene() == scene.get());

		ImGui::PushID(i); // Identifiant unique pour chaque sc�ne

		auto SceneTree = ImGui::TreeNode(scene->GetName().c_str());

		// Menu contextuel pour chaque sc�ne
		if (ImGui::BeginPopupContextItem("Scene Menu")) {
			if (ImGui::MenuItem("Set Active")) {
				sceneManager->SetCurrentScene(static_cast<int>(i)); // D�finit la sc�ne courante
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Rename")) {
				sceneToRename = i;
				strncpy_s(renameSceneBuffer, scene->GetName().c_str(), sizeof(renameSceneBuffer));
				renameSceneBuffer[sizeof(renameSceneBuffer) - 1] = '\0';
				ImGui::OpenPopup("Rename Scene");
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Delete")) {
				sceneManager->DestroyScene(scene->GetName()); // Supprime la sc�ne
			}

			ImGui::EndPopup();
		}

		// Affichage du nom de la sc�ne avec un bouton "Set Active" si n�cessaire
		if (SceneTree) {
			// Bouton pour d�finir la sc�ne active
			if (!isCurrentScene) {
				ImGui::SameLine(ImGui::GetWindowWidth() - 100); // D�calage � droite
				std::string buttonLabel = "Set Active##" + std::to_string(i);
				if (ImGui::Button(buttonLabel.c_str())) {
					sceneManager->SetCurrentScene(static_cast<int>(i));
				}
			}

			// Affichage des GameObjects
			const auto& gameObjects = scene->GetRootObjects();
			for (size_t j = 0; j < gameObjects.size(); ++j) {
				const auto& gameObject = gameObjects[j];

				ImGui::PushID(j); // Identifiant unique pour chaque GameObject

				if (strstr(gameObject->GetName().c_str(), searchBuffer)) {
					if (ImGui::Selectable(gameObject->GetName().c_str(), selectedGameObject == gameObject)) {
						selectedGameObject = gameObject;
					}
					if (j == gameObjects.size() - 1) {
						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::Spacing();
					}

					// Menu contextuel pour GameObject
					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Rename")) {
							isRenamePopupOpen = true;
							entityToRename = j;
							strncpy_s(renameBuffer, gameObject->GetName().c_str(), sizeof(renameBuffer) - 1);
							renameBuffer[sizeof(renameBuffer) - 1] = '\0';
							ImGui::OpenPopup("Rename Entity");
							selectedGameObject = gameObject;
						}

						ImGui::Separator();
						if (ImGui::MenuItem("Delete")) { DeleteGameObject(selectedGameObject); }

						ImGui::Separator();
						if (ImGui::MenuItem("Duplicate")) {}

						ImGui::EndPopup();
					}
					ImGui::PopID();  // Restaure l'ID pr�c�dent pour les GameObjects
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();  // Restaure l'ID pr�c�dent pour les sc�nes
	}
}

void ImGuiModule::DrawSettingsWindow() {
	if (ImGui::Begin("Settings")) {
		ImGUIInterface::EditTheme();
		if (ImGui::CollapsingHeader("Input")) {
			// Input settings
		}
		if (ImGui::CollapsingHeader("Graphics")) {
			// Graphics settings
		}
		if (ImGui::CollapsingHeader("Audio")) {
			// Audio settings
		}
		if (ImGui::CollapsingHeader("Network")) {
			//Network settings
		}
	}
	ImGui::End();
}

void ImGuiModule::DisplayTransform(Transform* _transform) {
	if (!_transform) return;

	// Position
	glm::vec3 position = _transform->GetPosition();
	if (ImGui::DragFloat3("Position", &position[0])) {
		_transform->SetPosition(position);
	}

	// Rotation
	glm::vec3 rotation = _transform->GetRotation();
	if (ImGui::DragFloat3("Rotation", &rotation[0])) {
		_transform->SetRotation(rotation);
	}

	// Scale
	glm::vec3 scale = _transform->GetScale();
	if (ImGui::DragFloat3("Scale", &scale[0])) {
		_transform->SetScale(scale);
	}
}

// ----------========== POPUPS ==========---------- //

void ImGuiModule::ShowRenamePopup() {
	// Gestion de la fen�tre popup pour renommer un gameobject
	if (isRenamePopupOpen && selectedGameObject) {
		ImGui::OpenPopup("Rename Entity");
		if (ImGui::BeginPopup("Rename Entity")) {
			ImGui::InputText("##edit", renameBuffer, IM_ARRAYSIZE(renameBuffer));
			if (ImGui::Button("OK##Rename")) {
				RenameGameObject(selectedGameObject, std::string(renameBuffer));
				isRenamePopupOpen = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel##Rename")) {
				isRenamePopupOpen = false;
			}
			ImGui::EndPopup();
		}
	}

	// Gestion de la fen�tre popup pour renommer la sc�ne
	if (sceneToRename >= 0) {
		ImGui::OpenPopup("Rename Scene");
		if (ImGui::BeginPopup("Rename Scene")) {
			ImGui::InputText("New Name", renameSceneBuffer, IM_ARRAYSIZE(renameSceneBuffer));
			if (ImGui::Button("OK")) {
				const auto& scenes = sceneManager->GetScenes();
				if (sceneToRename < scenes.size()) {
					sceneManager->RenameScene(scenes[sceneToRename]->GetName(), renameSceneBuffer);
				}
				sceneToRename = -1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				sceneToRename = -1;
			}
			ImGui::EndPopup();
		}
	}
}

// ----------========== RENAME / DELETE / DUPLICATE ==========---------- //

void ImGuiModule::RenameGameObject(GameObject* _gameObject, const std::string& _newName) {
	if (_gameObject) {
		std::cout << "Renamed GameObject: " << _gameObject->GetName() << " to " << _newName << std::endl;
		_gameObject->SetName(_newName);
	}
}
void ImGuiModule::DeleteGameObject(GameObject* _gameObject) {
	if (_gameObject) {
		BaseScene* currentScene = sceneManager->GetCurrentScene();
		if (currentScene) {
			currentScene->RemoveObject(_gameObject, true); // Suppression de l'objet
		}
	}
}
void ImGuiModule::DuplicateGameObject(GameObject* _gameObject) {

}

void ImGuiModule::CreateSpecificGameObject(GameObjectType _type) {
	BaseScene* currentScene = sceneManager->GetCurrentScene();
	if (currentScene) {
		GameObject* newGameObject = nullptr;
		switch (_type) {
		case GameObjectType::Cube:
			newGameObject = currentScene->CreateCubeGameObject();
			break;
		case GameObjectType::Light:
			newGameObject = currentScene->CreateLightGameObject();
			break;
		case GameObjectType::Plane:
			newGameObject = currentScene->CreatePlaneGameObject();
			break;
		}

		if (newGameObject) {
			currentScene->AddRootObject(newGameObject);
		}
	}
}