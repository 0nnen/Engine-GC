#pragma once

#include <string>
#include <vector>

namespace lve {
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
		LvePipeline(const std::string& _vertFilepath, const std::string& _fragFilepath);

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
		void CreateGraphicsPipeline(const std::string& _vertFilepath, const std::string& _fragFilepath);
	};

} //namespace lve

