#pragma once
class TextureManager
{
public:
	static TextureManager& get()
	{
		// la fonction static va �tre cr�er lors du premi�re appel de la fonction
		static TextureManager sSingleton;
		return sSingleton;
	}
private:
	TextureManager() {}

	~TextureManager() {}

	void start() {}

	void stop() {}
};

