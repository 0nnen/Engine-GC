#pragma once

#include "lve_device.h"
#include "lve_buffer.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

//std
#include <memory>
#include <vector>
#include <string>

namespace lve {
class LveModel
{

public:

	struct Vertex {
		glm::vec3 position{};
		glm::vec3 color{};
		glm::vec3 normal{};
		glm::vec2 uv{};

		static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

		bool operator==(const Vertex& _other) const {
			return position == _other.position && color == _other.color && normal == _other.normal && uv == _other.uv;
		}
	};

	struct Builder {
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		void LoadModel(const std::string& _filepath);
	};

	LveModel(LveDevice& _lveDevice, const LveModel::Builder &_builder);
	~LveModel();

	LveModel(const LveModel&) = delete;
	LveModel operator=(const LveModel&) = delete;

	static std::unique_ptr<LveModel> CreateModelFromFile(LveDevice& _device, const std::string& _filepath);

	void Bind(VkCommandBuffer _commandBuffer);
	void Draw(VkCommandBuffer _commandBuffer);

private:
	/**
	 * @brief Cr�e le tampon de vertex.
	 *
	 * Cette fonction cr�e le tampon de vertex � partir des donn�es de vertex fournies.
	 * Elle calcule la taille du tampon en fonction du nombre de vertices et de leur taille individuelle, puis cr�e un tampon de transfert de m�moire visible par l'h�te pour copier les donn�es de vertex.
	 * Les donn�es de vertex sont ensuite copi�es dans le tampon de transfert, puis transf�r�es vers un tampon de m�moire local du p�riph�rique pour une utilisation efficace pendant le rendu.
	 *
	 * @param _vertices Les donn�es de vertex � utiliser pour cr�er le tampon.
	 */
	void CreateVertexBuffer(const std::vector<Vertex>& _vertices);


	/**
	 * @brief Cr�e le tampon d'index.
	 *
	 * Cette fonction cr�e le tampon d'index � partir des indices fournis.
	 * Elle d�termine d'abord si des indices sont pr�sents. Si aucun indice n'est pr�sent, la fonction se termine.
	 * Sinon, elle calcule la taille du tampon en fonction du nombre d'indices et de leur taille individuelle, puis cr�e un tampon de transfert de m�moire visible par l'h�te pour copier les donn�es d'indices.
	 * Les donn�es d'indices sont ensuite copi�es dans le tampon de transfert, puis transf�r�es vers un tampon de m�moire local du p�riph�rique pour une utilisation efficace pendant le rendu.
	 *
	 * @param _indices Les indices � utiliser pour cr�er le tampon.
	 */
	void CreateIndexBuffer(const std::vector<uint32_t>& _indices);

	LveDevice& lveDevice;

	std::unique_ptr<LveBuffer> vertexBuffer;
	uint32_t vertexCount;

	bool hasIndexBuffer = false;
	std::unique_ptr<LveBuffer> indexBuffer;
	uint32_t indexCount;

};


} //lve
