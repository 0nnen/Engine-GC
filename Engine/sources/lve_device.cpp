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
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, // S�v�rit� du message de d�bogage
		VkDebugUtilsMessageTypeFlagsEXT messageType, // Type de message de d�bogage
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, // Donn�es de rappel du messager de d�bogage
		void* pUserData) { // Donn�es utilisateur optionnelles

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
	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance, // L'instance Vulkan
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, // Les informations de cr�ation du gestionnaire de messager de d�bogage
		const VkAllocationCallbacks* pAllocator, // L'allocation de rappels, permettant la personnalisation de la gestion de la m�moire
		VkDebugUtilsMessengerEXT* pDebugMessenger) { // Un pointeur vers l'objet de gestionnaire de messager de d�bogage � cr�er

		// R�cup�re le pointeur de fonction pour vkCreateDebugUtilsMessengerEXT
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");

		// V�rifie si la fonction est disponible
		if (func != nullptr) {
			// Appelle la fonction vkCreateDebugUtilsMessengerEXT pour cr�er le gestionnaire de messager de d�bogage
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			// L'extension VK_EXT_debug_utils n'est pas prise en charge
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
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
		VkInstance instance, // L'instance Vulkan
		VkDebugUtilsMessengerEXT debugMessenger, // L'objet de gestionnaire de messager de d�bogage � d�truire
		const VkAllocationCallbacks* pAllocator) { // L'allocation de rappels, permettant la personnalisation de la gestion de la m�moire

		// R�cup�re le pointeur de fonction pour vkDestroyDebugUtilsMessengerEXT
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugUtilsMessengerEXT");

		// V�rifie si la fonction est disponible
		if (func != nullptr) {
			// Appelle la fonction vkDestroyDebugUtilsMessengerEXT pour d�truire le gestionnaire de messager de d�bogage
			func(instance, debugMessenger, pAllocator);
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
		vkDestroyCommandPool(device_, commandPool, nullptr);

		// D�truit le p�riph�rique logique Vulkan
		vkDestroyDevice(device_, nullptr);

		// Si les couches de validation sont activ�es, d�truit le messager de d�bogage Vulkan
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		// D�truit la surface Vulkan associ�e � la fen�tre
		vkDestroySurfaceKHR(instance, surface_, nullptr);

		// D�truit l'instance Vulkan
		vkDestroyInstance(instance, nullptr);
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
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "LittleVulkanEngine App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Configuration de la cr�ation de l'instance Vulkan
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Obtention des extensions Vulkan requises pour l'application
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Configuration du messager de d�bogage Vulkan si les couches de validation sont activ�es
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		// Cr�ation de l'instance Vulkan
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}

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
		// R�cup�re le nombre de p�riph�riques physiques Vulkan disponibles sur le syst�me
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		// Affiche le nombre de p�riph�riques Vulkan trouv�s
		std::cout << "Device count: " << deviceCount << std::endl;

		// R�cup�re la liste des p�riph�riques physiques Vulkan disponibles sur le syst�me
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// Parcourt chaque p�riph�rique pour trouver celui qui convient le mieux � l'application
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		// V�rifie si un p�riph�rique adapt� a �t� trouv�
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}

		// R�cup�re les propri�t�s du p�riph�rique s�lectionn� et les affiche
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		std::cout << "physical device: " << properties.deviceName << std::endl;
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
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Configuration des fonctionnalit�s du p�riph�rique
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		// Configuration de la cr�ation du p�riph�rique logique
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
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
		else {
			createInfo.enabledLayerCount = 0;
		}

		// Cr�ation du p�riph�rique logique Vulkan
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		// Obtention des files de commandes graphiques et de pr�sentation du p�riph�rique logique
		vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
		vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
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
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; // Indique le type de la structure.
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily; // Sp�cifie la famille de files de commandes graphiques associ�e au pool.
		poolInfo.flags =
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // D�finit les drapeaux du pool de commandes, permettant des op�rations transitoires et la r�initialisation des tampons de commandes.

		// Cr�e le pool de commandes Vulkan en utilisant les informations fournies.
		if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
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
	bool LveDevice::isDeviceSuitable(VkPhysicalDevice device) {
		// Recherche des indices des familles de files de commandes support�es par le p�riph�rique physique.
		QueueFamilyIndices indices = findQueueFamilies(device);

		// V�rification de la prise en charge des extensions de p�riph�rique n�cessaires.
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		// V�rification de l'ad�quation de la cha�ne d'�change.
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			// Interrogation du p�riph�rique pour obtenir les d�tails de support de la cha�ne d'�change.
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		// R�cup�ration des fonctionnalit�s support�es par le p�riph�rique.
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

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
		VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		// Initialise la structure avec des valeurs par d�faut.
		createInfo = {};

		// Sp�cifie le type de la structure.
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		// Sp�cifie les niveaux de s�v�rit� des messages � intercepter.
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		// Sp�cifie les types de messages � intercepter.
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		// Sp�cifie la fonction de rappel � appeler pour chaque message de d�bogage.
		createInfo.pfnUserCallback = debugCallback;

		// Sp�cifie des donn�es utilisateur facultatives.
		createInfo.pUserData = nullptr;  // Optionnel
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
	void LveDevice::setupDebugMessenger() {
		// V�rifie si les couches de validation sont activ�es.
		if (!enableValidationLayers) return;

		// Initialise une structure pour la cr�ation du gestionnaire de d�bogage.
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		// Cr�e le gestionnaire de d�bogage avec les informations fournies.
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
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
		// R�cup�re le nombre de couches de validation disponibles.
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// R�cup�re les propri�t�s de toutes les couches de validation disponibles.
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		// Affiche les extensions Vulkan disponibles et les stocke dans un ensemble pour une recherche rapide.
		std::cout << "available extensions:" << std::endl;
		std::unordered_set<std::string> available;
		for (const auto& extension : extensions) {
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
	bool LveDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
		// R�cup�re le nombre d'extensions de p�riph�rique Vulkan disponibles pour le p�riph�rique sp�cifi�.
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		// R�cup�re les propri�t�s de toutes les extensions de p�riph�rique Vulkan disponibles.
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(
			device,
			nullptr,
			&extensionCount,
			availableExtensions.data());

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
	QueueFamilyIndices LveDevice::findQueueFamilies(VkPhysicalDevice device) {
		// Initialise la structure QueueFamilyIndices pour stocker les indices des files d'attente.
		QueueFamilyIndices indices;

		// R�cup�re le nombre de familles de files d'attente de p�riph�riques disponibles pour le p�riph�rique sp�cifi�.
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		// R�cup�re les propri�t�s de toutes les familles de files d'attente de p�riph�riques.
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// Parcourt toutes les familles de files d'attente pour trouver celles qui prennent en charge les op�rations de rendu graphique et de pr�sentation.
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			// V�rifie si la famille de files d'attente prend en charge les op�rations de rendu graphique.
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
				indices.graphicsFamilyHasValue = true;
			}
			// V�rifie si la famille de files d'attente prend en charge la pr�sentation sur la surface associ�e.
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
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
	SwapChainSupportDetails LveDevice::querySwapChainSupport(VkPhysicalDevice device) {
		// Initialise la structure SwapChainSupportDetails pour stocker les d�tails de prise en charge de la cha�ne d'�change de swap.
		SwapChainSupportDetails details;

		// Interroge le p�riph�rique physique Vulkan pour obtenir les capacit�s de la surface associ�e.
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

		// R�cup�re le nombre de formats de surface pris en charge par le p�riph�rique.
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

		// Si des formats sont pris en charge, r�cup�re les d�tails de ces formats.
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
		}

		// R�cup�re le nombre de modes de pr�sentation pris en charge par le p�riph�rique.
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

		// Si des modes de pr�sentation sont pris en charge, r�cup�re les d�tails de ces modes.
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				surface_,
				&presentModeCount,
				details.presentModes.data());
		}

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
	VkFormat LveDevice::findSupportedFormat(
		const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		// Parcourt chaque format d'image candidat
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			// Obtient les propri�t�s du format
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			// V�rifie si le format est adapt� au mode de tiling sp�cifi� (lin�aire ou optimal)
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				// Si le format est adapt� au mode lin�aire et prend en charge toutes les fonctionnalit�s requises, le retourne
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
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
	uint32_t LveDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

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
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory) {
		// D�finit les informations du tampon
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Cr�e le tampon
		if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}

		// Obtient les exigences de m�moire pour le tampon
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

		// Alloue la m�moire pour le tampon
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			// En cas d'�chec de l'allocation de m�moire, d�truit le tampon cr��
			vkDestroyBuffer(device_, buffer, nullptr);
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		// Associe la m�moire allou�e au tampon
		vkBindBufferMemory(device_, buffer, bufferMemory, 0);
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
	VkCommandBuffer LveDevice::beginSingleTimeCommands() {
		// Alloue un tampon de commandes
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer!");
		}

		// D�marre l'enregistrement des commandes dans le tampon
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			// En cas d'�chec du d�marrage de l'enregistrement des commandes, lib�re le tampon de commandes allou�
			vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
			throw std::runtime_error("failed to begin recording command buffer!");
		}

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
	void LveDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		// Termine l'enregistrement des commandes dans le tampon de commandes
		vkEndCommandBuffer(commandBuffer);

		// Soumet les commandes � la file de commandes graphiques pour ex�cution
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit command buffer to graphics queue!");
		}

		// Attend la fin de l'ex�cution des commandes
		if (vkQueueWaitIdle(graphicsQueue_) != VK_SUCCESS) {
			throw std::runtime_error("failed to wait for graphics queue to idle!");
		}

		// Lib�re le tampon de commandes
		vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
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
	void LveDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		// D�marre l'enregistrement des commandes dans un tampon de commandes temporaire
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		// D�finit la r�gion de copie
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;  // Facultatif
		copyRegion.dstOffset = 0;  // Facultatif
		copyRegion.size = size;

		// Effectue la copie des donn�es entre les tampons
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

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
		VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
		// D�marre l'enregistrement des commandes dans un tampon de commandes temporaire
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		// Configure la r�gion de copie
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		// Copie les donn�es du tampon vers l'image Vulkan
		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
		const VkImageCreateInfo& imageInfo,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory) {
		// Cr�e l'image Vulkan
		if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		// R�cup�re les exigences de m�moire de l'image
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device_, image, &memRequirements);

		// Alloue la m�moire pour l'image
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			// Lib�re l'image si l'allocation de m�moire �choue
			vkDestroyImage(device_, image, nullptr);
			throw std::runtime_error("failed to allocate image memory!");
		}

		// Associe la m�moire allou�e � l'image
		if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS) {
			// Lib�re l'image et la m�moire si la liaison �choue
			vkDestroyImage(device_, image, nullptr);
			vkFreeMemory(device_, imageMemory, nullptr);
			throw std::runtime_error("failed to bind image memory!");
		}
	}

}  // namespace lve