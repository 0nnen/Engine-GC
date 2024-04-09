#pragma once
// vulkan_rhi.h
#pragma once

#include <vulkan/vulkan.hpp>
#include "rhi.h"

#include "lve_descriptors.h"
#include "lve_device.h"
#include "lve_model.h"
#include "lve_pipeline.h"
#include "lve_renderer.h"
#include "Systems/simple_render_system.h"

namespace lve
{
	class PointLightSystem;
}

class RHIVulkanModule final : public RHIModule
{
	public:

		RHIVulkanModule();
		~RHIVulkanModule() override;

		vk::CommandBuffer* GetCurrentCommandBuffer() const { return currentCommandBuffer.get(); }

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

	private:
		// Autres m�thodes pour la cr�ation de la surface, des p�riph�riques logiques, etc.

		std::unique_ptr<vk::CommandBuffer> currentCommandBuffer;

		lve::SimpleRenderSystem* simpleRenderSystem;
		lve::PointLightSystem* pointLightSystem;

		lve::LveDevice* lveDevice;
		lve::LveRenderer* lveRenderer;

		lve::LveDescriptorPool::Builder* builder;

		std::unique_ptr<lve::LveDescriptorSetLayout, std::default_delete<lve::LveDescriptorSetLayout>>* globalSetLayout;

		lve::LveCamera* camera;

		GameObject* viewerObject;

		std::vector<vk::DescriptorSet>               globalDescriptorSets;
		std::vector<std::unique_ptr<lve::LveBuffer>> uboBuffers;

		// note : order of declarations matters
		std::unique_ptr<lve::LveDescriptorPool> globalPool{};


		lve::LveDevice*              p_lveDevice;
		lve::LveWindow*              p_lveWindow;
		lve::LveRenderer*            p_lveRenderer;
		lve::LveSwapChain*           p_lveSwapChain;
		lve::LveModel*               p_lveModel;
		lve::LvePipeline*            p_lvePipeline;
		lve::LveBuffer*              p_lveBuffer;
		lve::LveDescriptorPool*      p_lveDescriptorPool;
		lve::LveDescriptorSetLayout* p_lveDescriptorSetLayout;
		lve::LveDescriptorWriter*    p_lveDescriptorWriter;
		// Autres membres sp�cifiques � Vulkan
};
