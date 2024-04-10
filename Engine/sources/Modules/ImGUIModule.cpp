#include "Modules/ImGUIModule.h"
#include "lve_renderer.h"
#include "ModuleManager.h"
#include "Modules/WindowModule.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "rhi.h"

#include "Transform.h"
#include "Scene/SceneManager.h"
#include "TCP/Errors.h"

class BaseScene;

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

	//ImGui_ImplGlfw_ProcessEvent(&e);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();
	GetGui();
	//imgui commands
}

void ImGuiModule::PreRender()
{
	Module::PreRender();
}

void ImGuiModule::Render()
{
	Module::Render();

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


void ImGuiModule::GetGui()
{
	DrawHierarchy();
	DrawInspector();
}



void ImGuiModule::DrawInspector() {
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

void ImGuiModule::DrawHierarchy() {
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
		bool isCurrentScene = (sceneManager->GetCurrentScene() == scene.get());

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

void ImGuiModule::RenameGameObject(GameObject* _gameObject, const std::string& _newName) {
	if (_gameObject) {
		std::cout << "Renamed GameObject: " << _gameObject->GetName() << " to " << _newName << std::endl;
		_gameObject->SetName(_newName);
	}
}

void ImGuiModule::DeleteGameObject(GameObject* _gameObject) {
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




void ImGuiModule::DuplicateGameObject(int _index) {
	//auto& gameObjects = sceneManager->GetMainScene()->GetRootObjects();
	//if (_index < gameObjects.size()) {
	//	GameObject* original = gameObjects[_index];
	//	GameObject* clone = original->Clone();
	//	
	//	// Ajoute le clone � la sc�ne
	//	sceneManager->GetMainScene()->AddRootObject(clone);
	//}
}