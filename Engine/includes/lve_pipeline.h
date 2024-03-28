#pragma once

#include "lve_device.h"

#include <vulkan/vulkan.hpp>

#include <string>
#include <vector>

namespace lve {

	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
		std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
		vk::PipelineViewportStateCreateInfo viewportInfo;
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
		vk::PipelineMultisampleStateCreateInfo multisampleInfo;
		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
		vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<vk::DynamicState> dynamicStateEnables;
		vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
		vk::PipelineLayout pipelineLayout = nullptr;
		vk::RenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};




	class LvePipeline
	{
	public:

		/**
		 * @brief Constructeur de la classe LvePipeline.
		 *
		 * Initialise un pipeline graphique en cr�ant les shaders � partir des fichiers sp�cifi�s.
		 *
		 * @param _vertFilepath Le chemin d'acc�s au fichier contenant le code source du shader de vertex.
		 * @param _fragFilepath Le chemin d'acc�s au fichier contenant le code source du shader de fragment.
		 */
		LvePipeline(LveDevice & _device, const std::string& _vertFilepath, const std::string& _fragFilepath, const PipelineConfigInfo& _configInfo);

		~LvePipeline();

		LvePipeline(const LvePipeline&) = delete;
		LvePipeline &operator=(const LvePipeline&) = delete;


		void Bind(vk::CommandBuffer commandBuffer);
		static void DefaultPipelineConfigInfo(PipelineConfigInfo& _configInfo);
		static void EnableAlphaBlending(PipelineConfigInfo& _configInfo);

	private:

		/**
		* @brief Lit le contenu d'un fichier binaire et le retourne sous forme de vecteur de caract�res.
		 *
		 * Cette fonction ouvre un fichier binaire, lit son contenu dans un vecteur de caract�res et le retourne.
		 *
		* @param _filepath Le chemin d'acc�s au fichier � lire.
		* @return Un vecteur de caract�res contenant le contenu du fichier.
		* @throws std::runtime_error si le fichier ne peut pas �tre ouvert.
		*/
		static std::vector<char> ReadFile(const std::string& _filepath);

		/**
		* @brief Cr�e un pipeline graphique en chargeant les shaders depuis les fichiers sp�cifi�s.
		*
		* Cette fonction charge le code source des shaders de vertex et de fragment depuis les fichiers sp�cifi�s,
		* puis affiche la taille du code source des shaders.
		*
		* @param _vertFilepath Le chemin d'acc�s au fichier contenant le code source du shader de vertex.
		* @param _fragFilepath Le chemin d'acc�s au fichier contenant le code source du shader de fragment.
		*/
		void CreateGraphicsPipeline(const std::string& _vertFilepath, const std::string& _fragFilepath, const PipelineConfigInfo& _configInfo);

		void CreateShaderModule(const std::vector<char>& _code, vk::ShaderModule* _shaderModule);

		LveDevice& lveDevice;
		vk::Pipeline graphicsPipeline;
		vk::ShaderModule vertShaderModule;
		vk::ShaderModule fragShaderModule;
	};

} //namespace lve

