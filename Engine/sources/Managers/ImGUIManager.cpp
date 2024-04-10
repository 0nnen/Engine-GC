#include "Managers/ImGUIManager.h"
#include "ModuleManager.h"
#include "GameObject/GameObject.h"
#include "Transform.h"
#include "lve_renderer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

void ImGuiManager::Init()
{
	windowManager = moduleManager->GetModule<WindowManager>();
	sceneManager = moduleManager->GetModule<SceneManager>();

	device = windowManager->GetDevice()->device();
	graphicsQueue = windowManager->GetDevice()->graphicsQueue();
	lve::QueueFamilyIndices queueFamilyIndices = windowManager->GetDevice()->findPhysicalQueueFamilies();

	// Cr�ation du pool de commandes
	vk::CommandPoolCreateInfo commandPoolInfo(
		vk::CommandPoolCreateFlags(), // Flags de cr�ation
		queueFamilyIndices.graphicsFamily // Indice de la famille de file d'attente de commandes
	);
	_immCommandPool = device.createCommandPool(commandPoolInfo);

	// Allocation du tampon de commande pour les soumissions imm�diates
	vk::CommandBufferAllocateInfo cmdAllocInfo(
		_immCommandPool, // Pool de commandes
		vk::CommandBufferLevel::ePrimary, // Niveau du tampon de commande
		1 // Nombre de tampons � allouer
	);
	_immCommandBuffer = device.allocateCommandBuffers(cmdAllocInfo)[0];

	// Ajout de la fonction de suppression du pool de commandes � la file de suppression principale
	//_mainDeletionQueue.push_back([=]() {
	//	device.destroyCommandPool(_immCommandPool);
	//});
}

void ImGuiManager::Start()
{
	Module::Start();

	//timeModule = moduleManager->GetModule<TimeModule>();

	//ImGui::CreateContext();

	vk::DescriptorPoolSize pool_sizes[] = { { vk::DescriptorType::eSampler, 1000 },
	{ vk::DescriptorType::eCombinedImageSampler, 1000 },
	{ vk::DescriptorType::eSampledImage, 1000 },
	{ vk::DescriptorType::eStorageImage, 1000 },
	{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
	{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
	{ vk::DescriptorType::eUniformBuffer, 1000 },
	{ vk::DescriptorType::eStorageBuffer, 1000 },
	{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
	{ vk::DescriptorType::eStorageBufferDynamic , 1000 },
	{ vk::DescriptorType::eInputAttachment, 1000 } };

	vk::DescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = vk::StructureType::eDescriptorPoolCreateInfo;
	pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	vk::DescriptorPool imguiPool;
	if (windowManager->GetDevice()->device().createDescriptorPool(&pool_info, nullptr, &imguiPool) != vk::Result::eSuccess) {
		throw std::runtime_error("Impossible de creer la pool de imgui!");
	}

	// 2: initialize imgui library

	// this initializes the core structures of imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// this initializes imgui for SDL
	ImGui_ImplGlfw_InitForVulkan(windowManager->GetWindow()->GetGLFWwindow(), true);

	// this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = windowManager->GetDevice()->GetInstance();
	init_info.PhysicalDevice = windowManager->GetDevice()->GetPhysicalDevice();
	init_info.Device = device;
	init_info.Queue = graphicsQueue;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.RenderPass = windowManager->GetRenderer()->GetSwapChainRenderPass();
	//init_info.UseDynamicRendering = VK_TRUE;
	//init_info.ColorAttachmentFormat = _swapchainImageFormat;

	init_info.MSAASamples = (VkSampleCountFlagBits)vk::SampleCountFlagBits::e1;

	ImGui_ImplVulkan_Init(&init_info);

	// execute a gpu command to upload imgui font textures
	//immediate_submit([&](vk::CommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(); });

	// clear font textures from cpu data
	//ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiManager::immediate_submit(std::function<void(vk::CommandBuffer cmd)>&& function)
{
	vk::Device device = windowManager->GetDevice()->device();
	vk::Queue graphicsQueue = windowManager->GetDevice()->graphicsQueue();

	device.resetFences(_immFence);
	_immCommandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

	vk::CommandBuffer cmd = _immCommandBuffer;

	vk::CommandBufferBeginInfo cmdBeginInfo{};
	cmdBeginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
	cmdBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	cmd.begin(cmdBeginInfo);

	function(cmd);

	cmd.end();

	vk::CommandBufferSubmitInfo cmdinfo{};
	cmdinfo.sType = vk::StructureType::eCommandBufferSubmitInfo;
	cmdinfo.pNext = nullptr;
	cmdinfo.commandBuffer = cmd;
	cmdinfo.deviceMask = 0;

	vk::SubmitInfo2 submitInfo{};
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdinfo;
	vk::DispatchLoaderDynamic dispatcher = vk::DispatchLoaderDynamic();
	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	graphicsQueue.submit2KHR(submitInfo, _immFence, dispatcher);

	device.waitForFences(_immFence, VK_TRUE, 9999999999);
}

void ImGuiManager::Update()
{
	Module::Update();

	//ImGui_ImplGlfw_ProcessEvent(&e);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();
	GetGUI();

	//imgui commands
}

void ImGuiManager::Render()
{
	Module::Render();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), windowManager->GetRenderer()->GetCurrentCommandBuffer());

}

void ImGuiManager::Finalize()
{
	Module::Finalize();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();
}

void ImGuiManager::GetGUI() {
	DrawHierarchy();
	DrawInspector();
}



void ImGuiManager::DrawInspector() {
	ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Inspector");

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

	ImGui::End();
}

void ImGuiManager::DisplayTransform(Transform* _transform) {
	if (!_transform) return;

	// Position
	glm::vec3 position = _transform->GetPosition();
	if (ImGui::DragFloat3("Position", &position[0])) {
		_transform->SetPosition(position);
	}

	// Rotation
	float rotation = _transform->GetRotation();
	if (ImGui::DragFloat("Rotation", &rotation)) {
		_transform->SetRotation(rotation);
	}

	// Scale
	glm::vec3 scale = _transform->GetScale();
	if (ImGui::DragFloat3("Scale", &scale[0])) {
		_transform->SetScale(scale);
	}
}

void ImGuiManager::DrawHierarchy() {
	ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Hierarchy");

	// Bouton pour cr�er un nouveau GameObject
	if (ImGui::Button("New GameObject")) {
		BaseScene* currentScene = sceneManager->GetCurrentScene();
		if (currentScene) {
			currentScene->CreateGameObject();  // Ajoute un GameObject � la sc�ne actuelle
			std::cout << "Added new GameObject to current scene." << std::endl;
		}
		else {
			std::cout << "No active scene found." << std::endl;
		}
	}
	ImGui::SameLine();
	// Bouton pour ajouter une nouvelle sc�ne
	if (ImGui::Button("Add New Scene")) {
		sceneManager->CreateScene("New Scene", false);
	}

	// Barre de recherche
	static char searchBuffer[100];
	ImGui::InputText("Search", searchBuffer, IM_ARRAYSIZE(searchBuffer));

	// Affichage des sc�nes et de leurs GameObjects
	const auto& scenes = sceneManager->GetScenes();
	for (size_t i = 0; i < scenes.size(); ++i) {
		const auto& scene = scenes[i];
		bool isCurrentScene = (sceneManager->GetCurrentScene() == scene);

		ImGui::PushID(i);

		// Affichage du nom de la sc�ne avec un bouton "Set Active" si n�cessaire
		if (ImGui::TreeNode(scene->GetName().c_str())) {
			// Affichage des GameObjects
			const auto& gameObjects = scene->GetRootObjects();
			for (size_t j = 0; j < gameObjects.size(); ++j) {
				const auto& gameObject = gameObjects[j];
				if (strstr(gameObject->GetName().c_str(), searchBuffer)) {
					ImGui::PushID(j);
					if (ImGui::Selectable(gameObject->GetName().c_str(), selectedGameObject == gameObject)) {
						selectedGameObject = gameObject;
					}

					// Menu contextuel pour GameObject
					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Rename")) {
							isRenamePopupOpen = true;
							entityToRename = j;
							strncpy_s(renameBuffer, gameObject->GetName().c_str(), sizeof(renameBuffer) - 1);
							renameBuffer[sizeof(renameBuffer) - 1] = '\0';
							ImGui::OpenPopup("Rename Entity");
						}

						ImGui::Separator();
						if (ImGui::MenuItem("Delete")) { DeleteGameObject(selectedGameObject); }

						ImGui::Separator();
						if (ImGui::MenuItem("Duplicate")) { DuplicateGameObject(j); }

						ImGui::EndPopup();
					}
					ImGui::PopID();
				}
			}
			ImGui::TreePop();
		}

		if (!isCurrentScene) {
			ImGui::SameLine();
			std::string buttonLabel = "Set Active##" + std::to_string(i);
			if (ImGui::Button(buttonLabel.c_str())) {
				sceneManager->SetCurrentScene(static_cast<int>(i));
			}
		}

		// Menu contextuel pour chaque sc�ne
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Set as Main")) {
				sceneManager->SetMainScene(scene->GetName());
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
				sceneManager->DestroyScene(scene->GetName());
			}
			ImGui::EndPopup();
		}

		ImGui::PopID(); // Restaure l'ID pr�c�dent
	}

	ImGui::End(); // Ferme la fen�tre ImGUI
}


void ImGuiManager::ShowRenamePopup() {
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
				auto& scenes = sceneManager->GetScenes();
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

void ImGuiManager::RenameGameObject(GameObject* _gameObject, const std::string& _newName) {
	if (_gameObject) {
		std::cout << "Renamed GameObject: " << _gameObject->GetName() << " to " << _newName << std::endl;
		_gameObject->SetName(_newName);
	}
}

void ImGuiManager::DeleteGameObject(GameObject* _gameObject) {
	//if (_gameObject) {
	//	// R�cup�ration de la sc�ne active
	//	BaseScene* currentScene = sceneManager->GetCurrentScene();
	//	if (currentScene) {
	//		// Trouve l'index du GameObject dans la sc�ne active
	//		auto& gameObjects = currentScene->GetRootObjects();
	//		auto it = std::find(gameObjects.begin(), gameObjects.end(), _gameObject);
	//		if (it != gameObjects.end()) {
	//			// Supprime le GameObject et lib�re la m�moire
	//			delete* it;
	//			gameObjects.erase(it);

	//			// R�initialiser la s�lection si c'�tait l'objet s�lectionn�
	//			if (selectedGameObject == _gameObject) {
	//				selectedGameObject = nullptr;
	//			}
	//			std::cout << "Deleted GameObject: " << _gameObject->GetName() << std::endl;
	//		}
	//	}
	//}
}




void ImGuiManager::DuplicateGameObject(int _index) {
	//auto& gameObjects = sceneManager->GetMainScene()->GetRootObjects();
	//if (_index < gameObjects.size()) {
	//	GameObject* original = gameObjects[_index];
	//	GameObject* clone = original->Clone();
	//	
	//	// Ajoute le clone � la sc�ne
	//	sceneManager->GetMainScene()->AddRootObject(clone);
	//}
}

