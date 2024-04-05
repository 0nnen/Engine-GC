#pragma once

#include <functional>

#include "Module.h"
//#include "TimeModule.h"
#include "Modules/WindowModule.h"


class ImGuiModule final : public Module
{
	/**
		* @brief Initialise le module.
		*/
	void Init() override;

	/**
	 * @brief D�marre le module.
	 */
	void Start() override;

	/**
	 * @brief Effectue une mise � jour fixe du module.
	 */
	void FixedUpdate() override;

	/**
	 * @brief Met � jour le module.
	 */
	void Update() override;

	/**
	 * @brief Fonction pr�-rendu du module.
	 */
	void PreRender() override;

	/**
	 * @brief Rendu du module.
	 */
	void Render() override;

	/**
	 * @brief Rendu de l'interface graphique du module.
	 */
	void RenderGui() override;

	/**
	 * @brief Fonction post-rendu du module.
	 */
	void PostRender() override;

	/**
	 * @brief Lib�re les ressources utilis�es par le module.
	 */
	void Release() override;

	/**
	 * @brief Finalise le module.
	 */
	void Finalize() override;

	void GetGUI();

	//TimeModule* timeModule = nullptr;

	vk::Fence         _immFence;
	vk::CommandBuffer _immCommandBuffer;
	vk::CommandPool   _immCommandPool;

	void immediate_submit(std::function<void(vk::CommandBuffer cmd)>&& function);

	protected:
		vk::Device device;
		vk::Queue  graphicsQueue;

		WindowModule* windowModule = nullptr;

		~ImGuiModule() = default;
};
