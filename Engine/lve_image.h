#pragma once

#include "lve_device.h"
#include "lve_buffer.h"

#include <cassert>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

namespace lve {
	class LveImage
	{
	public:

		/**
		 * @brief Constructeur de la classe LveImage.
		 *
		 * @param _lveDevice R�f�rence vers l'objet LveDevice associ� � cette image.
		 */
		LveImage(LveDevice& _lveDevice);

		/** @brief Destructeur de la classe LveImage. */
		~LveImage();

		/**
		 * @brief Op�rateur de copie supprim� pour la classe LveImage.
		 *
		 * @return L'objet LveImage r�sultant de l'op�ration.
		 */
		LveImage(const LveImage&) = delete;

		/**
		 * @brief Op�rateur d'affectation supprim� pour la classe LveImage.
		 *
		 * @return L'objet LveImage r�sultant de l'op�ration.
		 */
		LveImage operator=(const LveImage&) = delete;

	private:
		/**
		 * @brief Cr�e une image de texture.
		 */
		void createTextureImage();

		/**
		 * @brief Cr�e une image Vulkan avec les param�tres sp�cifi�s.
		 *
		 * @param width La largeur de l'image.
		 * @param height La hauteur de l'image.
		 * @param format Le format de l'image.
		 * @param tiling Le mode de pavage de l'image.
		 * @param usage Les indicateurs d'utilisation de l'image.
		 * @param properties Les propri�t�s de m�moire de l'image.
		 * @param image R�f�rence o� stocker l'image Vulkan cr��e.
		 * @param imageMemory R�f�rence o� stocker la m�moire allou�e pour l'image.
		 */
		void createImage(uint32_t width,
			uint32_t height,
			vk::Format format,
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties,
			vk::Image& image,
			vk::DeviceMemory& imageMemory);

		/** R�f�rence vers l'objet LveDevice associ�. */
		LveDevice& lveDevice;

		/** Image de texture Vulkan cr��e. */
		vk::Image textureImage;

		/** M�moire allou�e pour l'image de texture. */
		vk::DeviceMemory textureImageMemory;

	};
}
