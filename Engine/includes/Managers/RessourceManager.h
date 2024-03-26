#pragma once
#include <iostream>
#include "Managers/TextureManager.h"
#include <unordered_map>

class RessourceManager
{
public:
	std::shared_ptr<TextureManager> LoadTexture(const std::string& _filePath)
	{
		// check si la texture est charg�
		if (textures.find(_filePath) != textures.end()) {
			std::cout << "La texture '" << _filePath << "' est d�j� charg�e." << std::endl;
			return textures[_filePath];
		}

		// si la texture est pas charg� alors on la charge
		std::shared_ptr<TextureManager> texture = std::make_shared<TextureManager>(_filePath);
		textures[_filePath] = texture;
		return texture;
	}

	void Clear()
	{
		textures.clear();
	}

private:
	std::unordered_map<std::string, std::shared_ptr<TextureManager>> textures;
};