#pragma once
#include "Component.h"

class Light final : public Component
{
public:
	Light()
	{
		name = "Light";
	}

	/*void Start() overrideoverride ;
	void Physics(const float& _delta) overrideoverride ;
	void Update(const float& _delta) overrideoverride ;

	//Render Methods
	void Render(lve::LveWindow* _window) overrideoverride ;
	void RenderGui(lve::LveWindow* _window) overrideoverride ;
	void RenderBackground(lve::LveWindow* _window) overrideoverride ;
	*/
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
	float lightIntensity = 1.0f;
} ;
