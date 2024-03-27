#include "lve_model.h"

#include "lve_utils.h"

//lives
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

//std
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif


namespace std {
	template<>
	struct hash<lve::LveModel::Vertex> {
		size_t operator()(lve::LveModel::Vertex const& _vertex) const {
			size_t seed = 0;
			lve::HashCombine(seed, _vertex.position, _vertex.color, _vertex.normal, _vertex.uv);
			return seed;
		}
	};
}

namespace lve {
	/*struct Vertex {
		glm::vec2 position;

		static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
	};*/

	LveModel::LveModel(LveDevice& _lveDevice, const LveModel::Builder& _builder) : lveDevice(_lveDevice){
		CreateVertexBuffer(_builder.vertices);
		CreateIndexBuffer(_builder.indices);
	}

	LveModel::~LveModel() {}


	std::unique_ptr<LveModel> LveModel::CreateModelFromFile(LveDevice& _device, const std::string& _filepath){
		Builder builder{};
		builder.LoadModel(ENGINE_DIR + _filepath);
		std::cout << "Vertex Count" << builder.vertices.size() << std::endl;

		return std::make_unique<LveModel>(_device, builder);
	}


	void LveModel::Bind(VkCommandBuffer _commandBuffer){
		VkBuffer buffers[] = { vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(_commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}

	}
	void LveModel::Draw(VkCommandBuffer _commandBuffer){
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(_commandBuffer, indexCount, 1,0,0,0);
		} else {
			vkCmdDraw(_commandBuffer, vertexCount, 1, 0, 0);
		}
		
	}

	void LveModel::CreateVertexBuffer(const std::vector<Vertex>& _vertices) {
		vertexCount = static_cast<uint32_t>(_vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(_vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(_vertices[0]);

		// D�claration et initialisation d'un objet de type LveBuffer nomm� stagingBuffer
		LveBuffer stagingBuffer{
			// Param�tre repr�sentant le p�riph�rique graphique associ� au tampon
			lveDevice,
			// Taille des donn�es de vertex � stocker dans le tampon
			vertexSize,
			// Nombre total de vertices dans les donn�es
			vertexCount,
			// Indique que le tampon sera utilis� comme une source lors des op�rations de transfert de donn�es
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			// Propri�t�s de la m�moire dans laquelle le tampon sera allou� :
			// - VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : La m�moire est visible pour le CPU
			// - VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : Les �critures CPU seront visibles par le GPU sans n�cessiter d'op�rations de synchronisation explicites
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)_vertices.data());

		// Allocation dynamique d'un objet de type unique_ptr pointant vers un LveBuffer et initialisation avec make_unique
		vertexBuffer = std::make_unique<LveBuffer>(
			// Param�tre repr�sentant le p�riph�rique graphique associ� au tampon
			lveDevice,
			// Taille des donn�es de vertex � stocker dans le tampon
			vertexSize,
			// Nombre total de vertices dans les donn�es
			vertexCount,
			// Combinaison de drapeaux d'utilisation du tampon :
			// - VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : Le tampon est utilis� comme tampon de vertex
			// - VK_BUFFER_USAGE_TRANSFER_DST_BIT : Le tampon peut �tre utilis� comme destination lors des op�rations de transfert de donn�es
			// - VK_BUFFER_USAGE_INDEX_BUFFER_BIT : Le tampon est utilis� comme tampon d'index
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			// Propri�t�s de la m�moire dans laquelle le tampon sera allou� :
			// - VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : La m�moire est locale au p�riph�rique et n'est pas visible pour le CPU
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);


		lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void LveModel::CreateIndexBuffer(const std::vector<uint32_t>& _indices) {
		indexCount = static_cast<uint32_t>(_indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}

		VkDeviceSize bufferSize = sizeof(_indices[0]) * indexCount;
		uint32_t indexSize = sizeof(_indices[0]);


		// Cr�ation d'un objet de type LveBuffer nomm� stagingBuffer en utilisant une initialisation directe
		LveBuffer stagingBuffer{
			// Param�tre repr�sentant le p�riph�rique graphique associ� au tampon
			lveDevice,
			// Taille des donn�es d'index � stocker dans le tampon
			indexSize,
			// Nombre total d'indices dans les donn�es
			indexCount,
			// Indique que le tampon sera utilis� comme une source lors des op�rations de transfert de donn�es
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			// Propri�t�s de la m�moire dans laquelle le tampon sera allou� :
			// - VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : La m�moire est visible pour le CPU
			// - VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : Les �critures CPU seront visibles par le GPU sans n�cessiter d'op�rations de synchronisation explicites
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)_indices.data());

		// Allocation dynamique d'un objet unique_ptr pointant vers un LveBuffer et initialisation avec make_unique
		indexBuffer = std::make_unique<LveBuffer>(
			// Param�tre repr�sentant le p�riph�rique graphique associ� au tampon
			lveDevice,
			// Taille des donn�es d'index � stocker dans le tampon
			indexSize,
			// Nombre total d'indices dans les donn�es
			indexCount,
			// Combinaison de drapeaux d'utilisation du tampon :
			// - VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : Le tampon est utilis� comme tampon de vertex
			// - VK_BUFFER_USAGE_TRANSFER_DST_BIT : Le tampon peut �tre utilis� comme destination lors des op�rations de transfert de donn�es
			// - VK_BUFFER_USAGE_INDEX_BUFFER_BIT : Le tampon est utilis� comme tampon d'index
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			// Propri�t�s de la m�moire dans laquelle le tampon sera allou� :
			// - VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : La m�moire est locale au p�riph�rique et n'est pas visible pour le CPU
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);


	}

	std::vector<VkVertexInputBindingDescription> LveModel::Vertex::GetBindingDescriptions() {
		std::vector< VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride =sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	 }
	std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::GetAttributeDescriptions() {
		std::vector< VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
		attributeDescriptions.push_back({ 1,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
		attributeDescriptions.push_back({ 2,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
		attributeDescriptions.push_back({ 1,0,VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

		return attributeDescriptions;
	 }

	void LveModel::Builder::LoadModel(const std::string& _filepath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, _filepath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				if (index.vertex_index >= 0) {
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
					};

					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2],
					};
				}

				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2],
					};
				}

				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1],
					};
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}


} //namespace lve