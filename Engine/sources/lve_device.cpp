#include "lve_device.h"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace lve {

	// local callback functions
	/**
	 * @brief Fonction de rappel de d�bogage pour la gestion des messages de validation Vulkan.
	 *
	 * Cette fonction est appel�e par les couches de validation Vulkan pour g�rer les messages de d�bogage.
	 * Elle affiche les messages de d�bogage dans la console d'erreur standard.
	 *
	 * @param messageSeverity La s�v�rit� du message de d�bogage.
	 * @param messageType Le type de message de d�bogage.
	 * @param pCallbackData Les donn�es de rappel du messager de d�bogage.
	 * @param pUserData Des donn�es utilisateur optionnelles.
	 * @return VK_FALSE indiquant que le traitement du message est termin�, et aucune action suppl�mentaire n'est n�cessaire.
	 */
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		vk::DebugUtilsMessageTypeFlagsEXT messageType,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		// Affiche le message de d�bogage dans la console d'erreur standard
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		// Indique que le traitement du message est termin�, et qu'aucune action suppl�mentaire n'est n�cessaire
		return VK_FALSE;
	}


	/**
	 * @brief Cr�e un gestionnaire de messager de d�bogage Vulkan en utilisant l'extension VK_EXT_debug_utils.
	 *
	 * Cette fonction cr�e un gestionnaire de messager de d�bogage Vulkan en utilisant l'extension VK_EXT_debug_utils.
	 * Elle utilise la fonction vkCreateDebugUtilsMessengerEXT, si elle est disponible.
	 *
	 * @param instance L'instance Vulkan.
	 * @param pCreateInfo Les informations de cr�ation du gestionnaire de messager de d�bogage.
	 * @param pAllocator L'allocation de rappels, permettant la personnalisation de la gestion de la m�moire.
	 * @param pDebugMessenger Un pointeur vers l'objet de gestionnaire de messager de d�bogage � cr�er.
	 * @return VK_SUCCESS si le gestionnaire de messager de d�bogage a �t� cr�� avec succ�s, VK_ERROR_EXTENSION_NOT_PRESENT si l'extension n'est pas prise en charge.
	 */
	vk::Result CreateDebugUtilsMessengerEXT(vk::Instance instance, const vk::DebugUtilsMessengerCreateInfoEXT* pCreateInfo, const vk::AllocationCallbacks* pAllocator, vk::DebugUtilsMessengerEXT* pDebugMessenger)
    {
        PFN_vkVoidFunction function = instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
        PFN_vkGetInstanceProcAddr messenger_ext = reinterpret_cast<PFN_vkGetInstanceProcAddr>(function);
        vk::DispatchLoaderDynamic dispatch(instance, messenger_ext);
        vk::Result result = instance.createDebugUtilsMessengerEXT(pCreateInfo, pAllocator, pDebugMessenger, dispatch);
        return result;
    }


	/**
	 * @brief D�truit un gestionnaire de messager de d�bogage Vulkan cr�� avec l'extension VK_EXT_debug_utils.
	 *
	 * Cette fonction d�truit un gestionnaire de messager de d�bogage Vulkan cr�� avec l'extension VK_EXT_debug_utils.
	 * Elle utilise la fonction vkDestroyDebugUtilsMessengerEXT, si elle est disponible.
	 *
	 * @param instance L'instance Vulkan.
	 * @param debugMessenger L'objet de gestionnaire de messager de d�bogage � d�truire.
	 * @param pAllocator L'allocation de rappels, permettant la personnalisation de la gestion de la m�moire.
	 */
	void DestroyDebugUtilsMessengerEXT(
		vk::Instance instance,
		vk::DebugUtilsMessengerEXT debugMessenger,
		const vk::AllocationCallbacks* pAllocator) {

		// R�cup�re le pointeur de fonction pour vkDestroyDebugUtilsMessengerEXT
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));

		// V�rifie si la fonction est disponible
		if (func != nullptr) {
			// Appelle la fonction vkDestroyDebugUtilsMessengerEXT pour d�truire le gestionnaire de messager de d�bogage
			func(static_cast<vk::Instance > (instance), static_cast<VkDebugUtilsMessengerEXT>(debugMessenger), reinterpret_cast<const VkAllocationCallbacks*>(pAllocator));
		}
	}


	// class member functions

	// Constructeur de la classe LveDevice
	LveDevice::LveDevice(LveWindow& window) : window{ window } {
		createInstance(); // Cr�e une instance Vulkan
		setupDebugMessenger(); // Configure le messager de d�bogage Vulkan
		createSurface(); // Cr�e la surface de la fen�tre Vulkan
		pickPhysicalDevice(); // S�lectionne le p�riph�rique physique adapt�
		createLogicalDevice(); // Cr�e le p�riph�rique logique Vulkan
		createCommandPool(); // Cr�e le pool de commandes Vulkan
	}

	// Destructeur de la classe LveDevice
	LveDevice::~LveDevice() {
		// D�truit le pool de commandes Vulkan
		device_.destroyCommandPool(commandPool, nullptr);

		// D�truit le p�riph�rique logique Vulkan
		device_.destroy(nullptr);

		// Si les couches de validation sont activ�es, d�truit le messager de d�bogage Vulkan
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		// D�truit la surface Vulkan associ�e � la fen�tre
		instance.destroySurfaceKHR(surface_, nullptr);

		// D�truit l'instance Vulkan
		instance.destroy(nullptr);
	}


	/**
	 * @brief Cr�e une instance Vulkan pour l'application.
	 *
	 * Cette fonction configure et cr�e une instance Vulkan, qui est la premi�re �tape dans l'utilisation de l'API Vulkan.
	 * Elle configure les informations d'application, les extensions requises, les couches de validation, et cr�e l'instance Vulkan correspondante.
	 *
	 * @throws std::runtime_error si la cr�ation de l'instance �choue ou si les couches de validation sont activ�es mais non disponibles.
	 */
	void LveDevice::createInstance() {
		// V�rifie si les couches de validation sont activ�es et si elles sont support�es
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		// Configuration des informations d'application pour l'instance Vulkan
		vk::ApplicationInfo appInfo(
			"LittleVulkanEngine App",
			VK_MAKE_VERSION(1, 0, 0),
			"No Engine",
			VK_MAKE_VERSION(1, 0, 0),
			VK_API_VERSION_1_0
		);

		// Obtention des extensions Vulkan requises pour l'application
		auto extensions = getRequiredExtensions();

		// Configuration de la cr�ation de l'instance Vulkan
		vk::InstanceCreateInfo createInfo(
			{},
			&appInfo, // Initialisation directe avec l'adresse de appInfo
			0, nullptr, // Aucune couche de validation
			static_cast<uint32_t>(extensions.size()), extensions.data() // Extensions activ�es
		);

		// Configuration du messager de d�bogage Vulkan si les couches de validation sont activ�es
		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		// Cr�ation de l'instance Vulkan
		instance = vk::createInstance(createInfo);

		// V�rification des extensions GLFW requises pour l'instance Vulkan
		hasGflwRequiredInstanceExtensions();
	}

	/**
	 * @brief S�lectionne le p�riph�rique physique adapt� pour l'application.
	 *
	 * Cette fonction r�cup�re la liste des p�riph�riques physiques Vulkan disponibles sur le syst�me,
	 * puis elle it�re � travers chaque p�riph�rique pour trouver celui qui convient le mieux � l'application.
	 * Le p�riph�rique s�lectionn� est stock� dans la variable physicalDevice.
	 *
	 * @throws Une exception si aucun p�riph�rique physique appropri� n'est trouv�.
	 */
	void LveDevice::pickPhysicalDevice() {
		// R�cup�re la liste des p�riph�riques physiques Vulkan disponibles sur le syst�me
		std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

		// V�rifie si des p�riph�riques Vulkan ont �t� trouv�s
		if (devices.empty()) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		// Affiche le nombre de p�riph�riques Vulkan trouv�s
		std::cout << "Device count: " << devices.size() << std::endl;

		// Parcourt chaque p�riph�rique pour trouver celui qui convient le mieux � l'application
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		// V�rifie si un p�riph�rique adapt� a �t� trouv�
		if (!physicalDevice) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}

		// R�cup�re les propri�t�s du p�riph�rique s�lectionn� et les affiche
		vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
		std::cout << "Physical device: " << properties.deviceName << std::endl;
	}



	/**
	* @brief Cr�e un p�riph�rique logique Vulkan.
	*
	* Cette fonction cr�e un p�riph�rique logique Vulkan � partir du p�riph�rique physique s�lectionn�.
	* Elle configure les files de commandes pour les op�rations graphiques et de pr�sentation, ainsi que les fonctionnalit�s du p�riph�rique.
	*
	* @throws Une exception si la cr�ation du p�riph�rique logique �choue.
	*/
	void LveDevice::createLogicalDevice() {
		// Recherche des familles de files de commandes pour le p�riph�rique physique s�lectionn�
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		// Configuration des files de commandes pour les op�rations graphiques et de pr�sentation
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			vk::DeviceQueueCreateInfo queueCreateInfo(
				{},
				queueFamily,
				1,
				&queuePriority
			);
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Configuration des fonctionnalit�s du p�riph�rique
		vk::PhysicalDeviceFeatures deviceFeatures;
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		// Configuration de la cr�ation du p�riph�rique logique
		vk::DeviceCreateInfo createInfo;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// Configuration des couches de validation si elles sont activ�es
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		// Cr�ation du p�riph�rique logique Vulkan
		device_ = physicalDevice.createDevice(createInfo);

		// Obtention des files de commandes graphiques et de pr�sentation du p�riph�rique logique
		graphicsQueue_ = device_.getQueue(indices.graphicsFamily, 0);
		presentQueue_ = device_.getQueue(indices.presentFamily, 0);
	}


	/**
	 * @brief Cr�e un pool de commandes Vulkan.
	 *
	 * Cette fonction cr�e un pool de commandes Vulkan pour le p�riph�rique logique actuel.
	 * Le pool de commandes est associ� � la famille de files de commandes graphiques trouv�e pour ce p�riph�rique.
	 * Les tampons de commandes cr��s � partir de ce pool auront les drapeaux VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
	 * et VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, ce qui permet des op�rations transitoires et de r�initialiser
	 * les tampons de commandes.
	 *
	 * @throw std::runtime_error si la cr�ation du pool de commandes �choue.
	 */
	void LveDevice::createCommandPool() {
		// Recherche les indices des familles de files de commandes support�es par le p�riph�rique physique actuel.
		QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

		// Initialise la structure d'informations du pool de commandes Vulkan.
		vk::CommandPoolCreateInfo poolInfo(
			vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
			queueFamilyIndices.graphicsFamily
		);

		// Cr�e le pool de commandes Vulkan en utilisant les informations fournies.
		try {
			commandPool = device_.createCommandPool(poolInfo);
		}
		catch (const vk::SystemError& e) {
			// Lance une exception si la cr�ation du pool de commandes �choue.
			throw std::runtime_error("failed to create command pool!");
		}
	}


	/**
	 * @brief Cr�e une surface Vulkan associ�e � la fen�tre de l'application.
	 *
	 * Cette fonction utilise la fen�tre de l'application pour cr�er une surface Vulkan, qui est n�cessaire
	 * pour interagir avec la fen�tre et effectuer le rendu graphique.
	 *
	 * @throw std::runtime_error si la cr�ation de la surface �choue.
	 */
	void LveDevice::createSurface() { window.CreateWindowSurface(instance, &surface_); }

	/**
	 * @brief V�rifie si le p�riph�rique physique Vulkan sp�cifi� convient aux besoins de l'application.
	 *
	 * Cette fonction examine les propri�t�s et les capacit�s du p�riph�rique pour d�terminer s'il convient � l'application.
	 *
	 * @param device Le p�riph�rique physique Vulkan � �valuer.
	 * @return true si le p�riph�rique convient, sinon false.
	 */
	bool LveDevice::isDeviceSuitable(vk::PhysicalDevice _device) {
		// Recherche des indices des familles de files de commandes support�es par le p�riph�rique physique.
		QueueFamilyIndices indices = findQueueFamilies(_device);

		// V�rification de la prise en charge des extensions de p�riph�rique n�cessaires.
		bool extensionsSupported = checkDeviceExtensionSupport(_device);

		// V�rification de l'ad�quation de la cha�ne d'�change.
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			// Interrogation du p�riph�rique pour obtenir les d�tails de support de la cha�ne d'�change.
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		// R�cup�ration des fonctionnalit�s support�es par le p�riph�rique.
		vk::PhysicalDeviceFeatures supportedFeatures = _device.getFeatures();

		// V�rification des fonctionnalit�s requises.
		return indices.isComplete() && extensionsSupported && swapChainAdequate &&
			supportedFeatures.samplerAnisotropy;
	}

	/**
	 * @brief Remplit une structure de donn�es pour la cr�ation d'un gestionnaire de d�bogage Vulkan.
	 *
	 * Cette fonction initialise une structure de donn�es pour la cr�ation d'un gestionnaire de d�bogage Vulkan avec les informations n�cessaires.
	 *
	 * @param createInfo La structure de donn�es � remplir pour la cr�ation du gestionnaire de d�bogage.
	 */
	void LveDevice::populateDebugMessengerCreateInfo(
		vk::DebugUtilsMessengerCreateInfoEXT& _createInfo) {
		// Sp�cifie le type de la structure.
		_createInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;

		// Sp�cifie les niveaux de s�v�rit� des messages � intercepter.
		_createInfo.setMessageSeverity(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		// Sp�cifie les types de messages � intercepter.
		_createInfo.setMessageType(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);

		// Sp�cifie la fonction de rappel � appeler pour chaque message de d�bogage.
		_createInfo.setPfnUserCallback((PFN_vkDebugUtilsMessengerCallbackEXT)debugCallback);

		// Sp�cifie des donn�es utilisateur facultatives.
		_createInfo.setPUserData(nullptr);  // Optionnel
	}



	/**
	 * @brief Configure le gestionnaire de d�bogage Vulkan.
	 *
	 * Cette fonction configure le gestionnaire de d�bogage Vulkan si les couches de validation sont activ�es.
	 *
	 * Elle utilise la fonction populateDebugMessengerCreateInfo() pour remplir les informations n�cessaires pour
	 * la cr�ation du gestionnaire de d�bogage.
	 *
	 * @throw std::runtime_error si la configuration du gestionnaire de d�bogage �choue.
	 */
	void LveDevice::setupDebugMessenger()
	{
		// V�rifie si les couches de validation sont activ�es.
		if (!enableValidationLayers)
			return;

		// Initialise une structure pour la cr�ation du gestionnaire de d�bogage.
		vk::DebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		// Cr�e le gestionnaire de d�bogage avec les informations fournies.
		try
		{
			vk::DispatchLoaderDynamic dispatcher = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
			if (vk::Result result = instance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger, dispatcher); result != vk::Result::eSuccess)
				throw std::runtime_error("failed to set up debug messenger!");
		}
		catch (const vk::SystemError& e)
		{
			throw std::runtime_error("failed to set up debug messenger: " + std::string(e.what()));
		}
	}


	/**
	 * @brief V�rifie la prise en charge des couches de validation Vulkan.
	 *
	 * Cette fonction v�rifie si toutes les couches de validation sp�cifi�es dans la variable `validationLayers`
	 * sont prises en charge par Vulkan sur le syst�me.
	 *
	 * @return true si toutes les couches de validation sont prises en charge, sinon false.
	 */
	bool LveDevice::checkValidationLayerSupport() {
		// R�cup�re les propri�t�s de toutes les couches de validation disponibles.
		auto availableLayers = vk::enumerateInstanceLayerProperties();

		// Parcourt toutes les couches de validation requises.
		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			// V�rifie si la couche de validation requise est pr�sente parmi les couches disponibles.
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			// Si la couche de validation requise n'est pas trouv�e, retourne false.
			if (!layerFound) {
				return false;
			}
		}

		// Si toutes les couches de validation requises sont trouv�es, retourne true.
		return true;
	}


	/**
	 * @brief R�cup�re les extensions Vulkan requises pour l'instance Vulkan.
	 *
	 * Cette fonction r�cup�re les extensions Vulkan requises pour l'instance Vulkan en utilisant la biblioth�que GLFW.
	 * Elle inclut les extensions requises par GLFW et, si les couches de validation sont activ�es, l'extension de
	 * gestionnaire de d�bogage Vulkan.
	 *
	 * @return Un vecteur de pointeurs vers les noms des extensions Vulkan requises.
	 */
	std::vector<const char*> LveDevice::getRequiredExtensions() {
		// R�cup�re le nombre d'extensions requises par GLFW.
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// Convertit les noms des extensions GLFW en un vecteur de pointeurs vers les noms des extensions Vulkan.
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// Ajoute l'extension de gestionnaire de d�bogage Vulkan si les couches de validation sont activ�es.
		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Retourne le vecteur contenant les noms des extensions Vulkan requises.
		return extensions;
	}



	/**
	  * @brief V�rifie la disponibilit� des extensions Vulkan requises par GLFW.
	  *
	  * Cette fonction v�rifie si toutes les extensions Vulkan requises par GLFW sont disponibles sur le syst�me.
	  * Elle r�cup�re d'abord toutes les extensions Vulkan disponibles, puis compare avec les extensions requises
	  * obtenues � partir de la fonction getRequiredExtensions(). Si une extension requise est manquante, une exception
	  * est lev�e.
	  *
	  * @throw std::runtime_error si une extension requise par GLFW est manquante.
	  */
	void LveDevice::hasGflwRequiredInstanceExtensions() {
		// R�cup�re le nombre d'extensions Vulkan disponibles.
		auto availableExtensions = vk::enumerateInstanceExtensionProperties();

		// Affiche les extensions Vulkan disponibles et les stocke dans un ensemble pour une recherche rapide.
		std::cout << "available extensions:" << std::endl;
		std::unordered_set<std::string> available;
		for (const auto& extension : availableExtensions) {
			std::cout << "\t" << extension.extensionName << std::endl;
			available.insert(extension.extensionName);
		}

		// Affiche les extensions Vulkan requises par GLFW et v�rifie leur disponibilit�.
		std::cout << "required extensions:" << std::endl;
		auto requiredExtensions = getRequiredExtensions();
		for (const auto& required : requiredExtensions) {
			std::cout << "\t" << required << std::endl;
			if (available.find(required) == available.end()) {
				// Lance une exception si une extension requise est manquante.
				throw std::runtime_error("Missing required glfw extension");
			}
		}
	}


	/**
	 * @brief V�rifie la prise en charge des extensions de p�riph�rique Vulkan par le p�riph�rique physique sp�cifi�.
	 *
	 * Cette fonction v�rifie si toutes les extensions de p�riph�rique Vulkan requises par l'application sont prises en charge
	 * par le p�riph�rique physique Vulkan sp�cifi�.
	 *
	 * @param device Le p�riph�rique physique Vulkan � v�rifier.
	 * @return true si toutes les extensions de p�riph�rique requises sont prises en charge, sinon false.
	 */
	bool LveDevice::checkDeviceExtensionSupport(vk::PhysicalDevice _device) {
		// R�cup�re les propri�t�s de toutes les extensions de p�riph�rique Vulkan disponibles pour le p�riph�rique sp�cifi�.
		auto availableExtensions = _device.enumerateDeviceExtensionProperties(nullptr);

		// Cr�e un ensemble contenant toutes les extensions de p�riph�rique requises par l'application.
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		// Parcourt toutes les extensions de p�riph�rique disponibles.
		for (const auto& extension : availableExtensions) {
			// Supprime les extensions disponibles de l'ensemble des extensions requises.
			requiredExtensions.erase(extension.extensionName);
		}

		// Si l'ensemble des extensions requises est vide, cela signifie que toutes les extensions requises sont prises en charge.
		return requiredExtensions.empty();
	}


	/**
	 * @brief Recherche et retourne les indices des files d'attente de p�riph�riques pour le p�riph�rique physique sp�cifi�.
	 *
	 * Cette fonction recherche les indices des files d'attente de p�riph�riques pour les op�rations de rendu graphique et de pr�sentation
	 * pour le p�riph�rique physique Vulkan sp�cifi�.
	 *
	 * @param device Le p�riph�rique physique Vulkan pour lequel les files d'attente doivent �tre recherch�es.
	 * @return Une structure QueueFamilyIndices contenant les indices des files d'attente pour les op�rations de rendu graphique et de pr�sentation.
	 */
	QueueFamilyIndices LveDevice::findQueueFamilies(vk::PhysicalDevice device) {
		// Initialise la structure QueueFamilyIndices pour stocker les indices des files d'attente.
		QueueFamilyIndices indices;

		// R�cup�re le nombre de familles de files d'attente de p�riph�riques disponibles pour le p�riph�rique sp�cifi�.
		auto queueFamilies = device.getQueueFamilyProperties();

		// Parcourt toutes les familles de files d'attente pour trouver celles qui prennent en charge les op�rations de rendu graphique et de pr�sentation.
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			// V�rifie si la famille de files d'attente prend en charge les op�rations de rendu graphique.
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
				indices.graphicsFamily = i;
				indices.graphicsFamilyHasValue = true;
			}
			// V�rifie si la famille de files d'attente prend en charge la pr�sentation sur la surface associ�e.
			bool presentSupport = device.getSurfaceSupportKHR(i, surface_);
			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamily = i;
				indices.presentFamilyHasValue = true;
			}
			// Si les indices requis ont �t� trouv�s, arr�te la recherche.
			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		// Retourne les indices des files d'attente trouv�es.
		return indices;
	}



	/**
	 * @brief Interroge et retourne les d�tails de prise en charge de la cha�ne d'�change de swap (SwapChain) pour le p�riph�rique physique sp�cifi�.
	 *
	 * Cette fonction interroge le p�riph�rique physique Vulkan sp�cifi� pour obtenir les d�tails de prise en charge de la cha�ne d'�change de swap, tels que
	 * les capacit�s de surface, les formats de surface pris en charge et les modes de pr�sentation pris en charge.
	 *
	 * @param device Le p�riph�rique physique Vulkan pour lequel les d�tails de prise en charge de la cha�ne d'�change de swap doivent �tre interrog�s.
	 * @return Une structure SwapChainSupportDetails contenant les d�tails de prise en charge de la cha�ne d'�change de swap.
	 */
	SwapChainSupportDetails LveDevice::querySwapChainSupport(vk::PhysicalDevice device) {
		// Initialise la structure SwapChainSupportDetails pour stocker les d�tails de prise en charge de la cha�ne d'�change de swap.
		SwapChainSupportDetails details;

		// Interroge le p�riph�rique physique Vulkan pour obtenir les capacit�s de la surface associ�e.
		details.capabilities = device.getSurfaceCapabilitiesKHR(surface_);

		// R�cup�re les formats de surface pris en charge par le p�riph�rique.
		details.formats = device.getSurfaceFormatsKHR(surface_);

		// R�cup�re les modes de pr�sentation pris en charge par le p�riph�rique.
		details.presentModes = device.getSurfacePresentModesKHR(surface_);

		// Retourne les d�tails de prise en charge de la cha�ne d'�change de swap.
		return details;
	}



	/**
	 * @brief Recherche un format d'image support� parmi une liste de formats candidats.
	 *
	 * Cette fonction recherche un format d'image support� parmi une liste de formats candidats,
	 * en tenant compte du mode de tiling sp�cifi� (lin�aire ou optimal) et des fonctionnalit�s requises.
	 *
	 * @param candidates La liste des formats candidats � v�rifier.
	 * @param tiling Le mode de tiling de l'image (VK_IMAGE_TILING_LINEAR ou VK_IMAGE_TILING_OPTIMAL).
	 * @param features Les fonctionnalit�s requises pour le format.
	 * @return Le format d'image support� trouv�.
	 * @throws std::runtime_error si aucun format d'image support� n'est trouv�.
	 */
	vk::Format LveDevice::findSupportedFormat(
		const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
		// Parcourt chaque format d'image candidat
		for (vk::Format format : candidates) {
			vk::FormatProperties props;
			// Obtient les propri�t�s du format
			props = physicalDevice.getFormatProperties(format);

			// V�rifie si le format est adapt� au mode de tiling sp�cifi� (lin�aire ou optimal)
			if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
				// Si le format est adapt� au mode lin�aire et prend en charge toutes les fonctionnalit�s requises, le retourne
				return format;
			}
			else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
				// Si le format est adapt� au mode optimal et prend en charge toutes les fonctionnalit�s requises, le retourne
				return format;
			}
		}
		// Si aucun format d'image trouv� ne r�pond aux crit�res sp�cifi�s, lance une exception
		throw std::runtime_error("failed to find supported format!");
	}



	/**
	 * @brief Recherche un type de m�moire adapt� pour les allocations de m�moire graphique.
	 *
	 * Cette fonction recherche un type de m�moire adapt� pour les allocations de m�moire graphique,
	 * en tenant compte du filtre de type sp�cifi� et des propri�t�s de m�moire requises.
	 *
	 * @param typeFilter Le filtre de type de m�moire sp�cifi�.
	 * @param properties Les propri�t�s de m�moire requises.
	 * @return L'index du type de m�moire adapt� trouv�.
	 * @throws std::runtime_error si aucun type de m�moire adapt� n'est trouv�.
	 */
	uint32_t LveDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
		vk::PhysicalDeviceMemoryProperties memProperties;
		memProperties = physicalDevice.getMemoryProperties();

		// Parcourt chaque type de m�moire
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			// V�rifie si le type de m�moire est compatible avec le filtre sp�cifi� et poss�de toutes les propri�t�s requises
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				// Si le type de m�moire convient, retourne son index
				return i;
			}
		}
		// Si aucun type de m�moire adapt� n'est trouv�, lance une exception
		throw std::runtime_error("failed to find suitable memory type!");
	}



	/**
	 * @brief Cr�e un tampon Vulkan avec la taille, l'utilisation et les propri�t�s sp�cifi�es.
	 *
	 * Cette fonction cr�e un tampon Vulkan avec la taille, l'utilisation et les propri�t�s sp�cifi�es,
	 * et associe la m�moire appropri�e au tampon.
	 *
	 * @param size La taille du tampon en octets.
	 * @param usage Les indicateurs d'utilisation du tampon.
	 * @param properties Les propri�t�s de m�moire du tampon.
	 * @param buffer Une r�f�rence � l'objet tampon � cr�er.
	 * @param bufferMemory Une r�f�rence � la m�moire allou�e pour le tampon.
	 * @throws std::runtime_error en cas d'�chec de la cr�ation ou de l'allocation de m�moire pour le tampon.
	 */
	void LveDevice::createBuffer(
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties,
		vk::Buffer& buffer,
		vk::DeviceMemory& bufferMemory) {

		// D�finit les informations du tampon
		vk::BufferCreateInfo bufferInfo(
			{},
			size,
			usage,
			vk::SharingMode::eExclusive
		);

		// Cr�e le tampon
		buffer = device_.createBuffer(bufferInfo);

		// Obtient les exigences de m�moire pour le tampon
		vk::MemoryRequirements memRequirements;
		memRequirements = device_.getBufferMemoryRequirements(buffer);

		// Alloue la m�moire pour le tampon
		vk::MemoryAllocateInfo allocInfo(
			memRequirements.size,
			findMemoryType(memRequirements.memoryTypeBits, properties)
		);

		bufferMemory = device_.allocateMemory(allocInfo);

		// Associe la m�moire allou�e au tampon
		device_.bindBufferMemory(buffer, bufferMemory, 0);
	}



	/**
	 * @brief D�marre une s�rie de commandes Vulkan pour une utilisation unique.
	 *
	 * Cette fonction alloue et d�marre un tampon de commandes Vulkan pour une utilisation unique.
	 * Le tampon de commandes retourn� peut �tre utilis� pour ex�cuter des commandes Vulkan
	 * qui doivent �tre ex�cut�es une seule fois.
	 *
	 * @return Le tampon de commandes Vulkan allou� et d�marr�.
	 * @throws std::runtime_error en cas d'�chec de l'allocation du tampon de commandes.
	 */
	vk::CommandBuffer LveDevice::beginSingleTimeCommands() {
		// Alloue un tampon de commandes
		vk::CommandBufferAllocateInfo allocInfo(
			commandPool,
			vk::CommandBufferLevel::ePrimary,
			1
		);

		vk::CommandBuffer commandBuffer = device_.allocateCommandBuffers(allocInfo)[0];

		// D�marre l'enregistrement des commandes dans le tampon
		vk::CommandBufferBeginInfo beginInfo(
			vk::CommandBufferUsageFlagBits::eOneTimeSubmit
		);

		commandBuffer.begin(beginInfo);

		return commandBuffer;
	}



	/**
	 * @brief Termine une s�rie de commandes Vulkan pour une utilisation unique.
	 *
	 * Cette fonction termine l'enregistrement des commandes dans le tampon de commandes sp�cifi�,
	 * soumet les commandes � la file de commandes graphiques pour ex�cution, attend la fin de l'ex�cution
	 * des commandes, puis lib�re le tampon de commandes.
	 *
	 * @param commandBuffer Le tampon de commandes Vulkan � terminer et � soumettre.
	 * @throws std::runtime_error en cas d'�chec de soumission des commandes ou d'attente de la fin de l'ex�cution.
	 */
	void LveDevice::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
		// Termine l'enregistrement des commandes dans le tampon de commandes
		commandBuffer.end();

		// Soumet les commandes � la file de commandes graphiques pour ex�cution
		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		graphicsQueue_.submit(submitInfo, nullptr);

		// Attend la fin de l'ex�cution des commandes
		graphicsQueue_.waitIdle();

		// Lib�re le tampon de commandes
		device_.freeCommandBuffers(commandPool, 1, &commandBuffer);
	}



	/**
	 * @brief Copie les donn�es d'un tampon source vers un tampon de destination.
	 *
	 * Cette fonction utilise un tampon de commandes temporaire pour copier les donn�es
	 * du tampon source vers le tampon de destination. Elle garantit que l'op�ration de
	 * copie se produit de mani�re synchrone, c'est-�-dire que le tampon de commandes temporaire
	 * est soumis � la file de commandes graphiques, attend que toutes les op�rations en cours
	 * sur cette file soient termin�es, puis est lib�r�.
	 *
	 * @param srcBuffer Le tampon source � partir duquel copier les donn�es.
	 * @param dstBuffer Le tampon de destination vers lequel copier les donn�es.
	 * @param size La taille des donn�es � copier, en octets.
	 * @throws std::runtime_error en cas d'�chec de l'allocation ou de l'enregistrement des commandes.
	 */
	void LveDevice::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
		// D�marre l'enregistrement des commandes dans un tampon de commandes temporaire
		vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

		// D�finit la r�gion de copie
		vk::BufferCopy copyRegion{};
		copyRegion.srcOffset = 0;  // Facultatif
		copyRegion.dstOffset = 0;  // Facultatif
		copyRegion.size = size;

		// Effectue la copie des donn�es entre les tampons
		commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

		// Termine l'enregistrement des commandes et soumet le tampon de commandes � la file de commandes graphiques
		endSingleTimeCommands(commandBuffer);
	}



	/**
	 * @brief Copie les donn�es d'un tampon vers une image Vulkan.
	 *
	 * Cette fonction utilise un tampon de commandes temporaire pour copier les donn�es
	 * du tampon vers l'image Vulkan. Elle garantit que l'op�ration de copie se produit de mani�re
	 * synchrone, c'est-�-dire que le tampon de commandes temporaire est soumis � la file de commandes
	 * graphiques, attend que toutes les op�rations en cours sur cette file soient termin�es, puis est lib�r�.
	 *
	 * @param buffer Le tampon contenant les donn�es � copier vers l'image.
	 * @param image L'image Vulkan de destination vers laquelle copier les donn�es.
	 * @param width La largeur de l'image en pixels.
	 * @param height La hauteur de l'image en pixels.
	 * @param layerCount Le nombre de couches de l'image.
	 * @throws std::runtime_error en cas d'�chec de l'allocation ou de l'enregistrement des commandes.
	 */
	void LveDevice::copyBufferToImage(
		vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount) {
		// D�marre l'enregistrement des commandes dans un tampon de commandes temporaire
		vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

		// Configure la r�gion de copie
		vk::BufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = vk::Offset3D{ 0, 0, 0 };
		region.imageExtent = vk::Extent3D{ width, height, 1 };

		// Copie les donn�es du tampon vers l'image Vulkan
		commandBuffer.copyBufferToImage(
			buffer,
			image,
			vk::ImageLayout::eTransferDstOptimal,
			1,
			&region);

		// Termine l'enregistrement des commandes et soumet le tampon de commandes � la file de commandes graphiques
		endSingleTimeCommands(commandBuffer);
	}



	/**
	 * @brief Cr�e une image Vulkan avec les informations sp�cifi�es.
	 *
	 * Cette fonction cr�e une image Vulkan en utilisant les informations fournies dans
	 * l'objet VkImageCreateInfo. Elle alloue �galement la m�moire n�cessaire pour cette
	 * image et associe cette m�moire � l'image cr��e.
	 *
	 * @param imageInfo Les informations n�cessaires � la cr�ation de l'image.
	 * @param properties Les propri�t�s de la m�moire de l'image.
	 * @param image R�f�rence o� l'objet image cr�� sera stock�.
	 * @param imageMemory R�f�rence o� la m�moire de l'image sera stock�e.
	 * @throws std::runtime_error en cas d'�chec de la cr�ation de l'image ou de l'allocation de m�moire.
	 */
	void LveDevice::createImageWithInfo(
		const vk::ImageCreateInfo& imageInfo,
		vk::MemoryPropertyFlags properties,
		vk::Image& image,
		vk::DeviceMemory& imageMemory) {
		// Cr�e l'image Vulkan
		try {
			image = device_.createImage(imageInfo);
		}
		catch (const vk::SystemError& e) {
			throw std::runtime_error("failed to create image!");
		}

		// R�cup�re les exigences de m�moire de l'image
		vk::MemoryRequirements memRequirements;
		memRequirements = device_.getImageMemoryRequirements(image);

		// Alloue la m�moire pour l'image
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.setAllocationSize(memRequirements.size);
		allocInfo.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

		try {
			imageMemory = device_.allocateMemory(allocInfo);
		}
		catch (const vk::SystemError& e) {
			// Lib�re l'image si l'allocation de m�moire �choue
			device_.destroyImage(image);
			throw std::runtime_error("failed to allocate image memory!");
		}

		// Associe la m�moire allou�e � l'image
		try {
			device_.bindImageMemory(image, imageMemory, 0);
		}
		catch (const vk::SystemError& e) {
			// Lib�re l'image et la m�moire si la liaison �choue
			device_.destroyImage(image);
			device_.freeMemory(imageMemory);
			throw std::runtime_error("failed to bind image memory!");
		}
	}



}  // namespace lve