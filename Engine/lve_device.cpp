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


    void LveDevice::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
        poolInfo.flags =
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void LveDevice::createSurface() { window.CreateWindowSurface(instance, &surface_); }

    bool LveDevice::isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate &&
            supportedFeatures.samplerAnisotropy;
    }

    void LveDevice::populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;  // Optional
    }

    void LveDevice::setupDebugMessenger() {
        if (!enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    bool LveDevice::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> LveDevice::getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void LveDevice::hasGflwRequiredInstanceExtensions() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "available extensions:" << std::endl;
        std::unordered_set<std::string> available;
        for (const auto& extension : extensions) {
            std::cout << "\t" << extension.extensionName << std::endl;
            available.insert(extension.extensionName);
        }

        std::cout << "required extensions:" << std::endl;
        auto requiredExtensions = getRequiredExtensions();
        for (const auto& required : requiredExtensions) {
            std::cout << "\t" << required << std::endl;
            if (available.find(required) == available.end()) {
                throw std::runtime_error("Missing required glfw extension");
            }
        }
    }

    bool LveDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(
            device,
            nullptr,
            &extensionCount,
            availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices LveDevice::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
                indices.graphicsFamilyHasValue = true;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
                indices.presentFamilyHasValue = true;
            }
            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    SwapChainSupportDetails LveDevice::querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                surface_,
                &presentModeCount,
                details.presentModes.data());
        }
        return details;
    }

    VkFormat LveDevice::findSupportedFormat(
        const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (
                tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    uint32_t LveDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void LveDevice::createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(device_, buffer, bufferMemory, 0);
    }

    VkCommandBuffer LveDevice::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void LveDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue_);

        vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
    }

    void LveDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void LveDevice::copyBufferToImage(
        VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

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

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);
        endSingleTimeCommands(commandBuffer);
    }

    void LveDevice::createImageWithInfo(
        const VkImageCreateInfo& imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory) {
        if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device_, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS) {
            throw std::runtime_error("failed to bind image memory!");
        }
    }

}  // namespace lve