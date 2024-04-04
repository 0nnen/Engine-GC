#pragma once
#pragma once
#include "Module.h"

class RHIModule : public Module
{
public:
    virtual ~RHIModule() {}

	/**
	 * @brief Initialise le module.
	 */
	virtual void Init() override = 0;

	/**
	 * @brief D�marre le module.
	 */
	virtual void Start() override = 0;

	/**
	 * @brief Effectue une mise � jour fixe du module.
	 */
	virtual void FixedUpdate() override = 0;

	/**
	 * @brief Met � jour le module.
	 */
	virtual void Update() override = 0;

	/**
	 * @brief Fonction pr�-rendu du module.
	 */
	virtual void PreRender()override = 0;

	/**
	 * @brief Rendu du module.
	 */
	virtual void Render()override = 0;

	/**
	 * @brief Rendu de l'interface graphique du module.
	 */
	virtual void RenderGui() override = 0;

	/**
	 * @brief Fonction post-rendu du module.
	 */
	virtual void PostRender()override = 0;

	/**
	 * @brief Lib�re les ressources utilis�es par le module.
	 */
	virtual void Release() override = 0;

	/**
	 * @brief Finalise le module.
	 */
	virtual void Finalize() override = 0;


};
