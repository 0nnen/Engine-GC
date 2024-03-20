#pragma once

#include "lve_window.h"

// std lib headers
#include <string>
#include <vector>

namespace lve {

    // Structure pour stocker les d�tails de support de la cha�ne d'�change
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities; // Capacit�s de la surface Vulkan
        std::vector<VkSurfaceFormatKHR> formats; // Formats de surface Vulkan pris en charge
        std::vector<VkPresentModeKHR> presentModes; // Modes de pr�sentation Vulkan pris en charge
    };

    // Structure pour stocker les indices des files d'attente requises
    struct QueueFamilyIndices {
        uint32_t graphicsFamily; // Indice de la famille de files d'attente graphiques
        uint32_t presentFamily; // Indice de la famille de files d'attente de pr�sentation
        bool graphicsFamilyHasValue = false; // Indique si l'indice de la famille de files d'attente graphiques est d�fini
        bool presentFamilyHasValue = false; // Indique si l'indice de la famille de files d'attente de pr�sentation est d�fini

        // V�rifie si les indices des files d'attente requis sont complets
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class LveDevice {
    public:
#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif

        /**
        * @brief Constructeur de la classe LveDevice.
        *
        * Initialise un p�riph�rique Vulkan en prenant une r�f�rence � une fen�tre Vulkan LveWindow en tant que param�tre.
        *
        * @param window Une r�f�rence � la fen�tre Vulkan utilis�e pour initialiser le p�riph�rique.
        */
        LveDevice(LveWindow& window);

        /**
         * @brief Destructeur de la classe LveDevice.
         *
         * Nettoie les ressources associ�es au p�riph�rique Vulkan lorsqu'il est d�truit.
         */
        ~LveDevice();

        // Not copyable or movable
        /**
        * @brief Constructeur de copie supprim�.
        *
        * Emp�che la cr�ation d'une nouvelle instance de LveDevice en copiant une instance existante.
        */
        LveDevice(const LveDevice&) = delete;

        /**
        * @brief Op�rateur d'affectation de copie supprim�.
        *
        * Emp�che la copie des membres d'une instance de LveDevice vers une autre instance existante.
        */
        void operator=(const LveDevice&) = delete;

        /**
        * @brief Constructeur de d�placement supprim�.
        *
        * Emp�che la cr�ation d'une nouvelle instance de LveDevice en d�pla�ant une instance existante.
        */
        LveDevice(LveDevice&&) = delete;

        /**
        * @brief Op�rateur d'affectation de d�placement supprim�.
        *
        * Emp�che le d�placement des membres d'une instance de LveDevice vers une autre instance existante.
        */
        LveDevice& operator=(LveDevice&&) = delete;

        /**
        * @brief R�cup�re le pool de commandes Vulkan associ� au p�riph�rique.
        *
        * @return Le pool de commandes Vulkan associ� au p�riph�rique.
        */
        VkCommandPool getCommandPool() { return commandPool; }

        /**
         * @brief R�cup�re l'objet de p�riph�rique Vulkan.
         *
         * @return L'objet de p�riph�rique Vulkan.
         */
        VkDevice device() { return device_; }

        /**
         * @brief R�cup�re la surface Vulkan associ�e au p�riph�rique.
         *
         * @return La surface Vulkan associ�e au p�riph�rique.
         */
        VkSurfaceKHR surface() { return surface_; }

        /**
         * @brief R�cup�re la file de commandes graphiques du p�riph�rique.
         *
         * @return La file de commandes graphiques du p�riph�rique.
         */
        VkQueue graphicsQueue() { return graphicsQueue_; }

        /**
         * @brief R�cup�re la file de commandes de pr�sentation du p�riph�rique.
         *
         * @return La file de commandes de pr�sentation du p�riph�rique.
         */
        VkQueue presentQueue() { return presentQueue_; }


        /**
         * @brief R�cup�re les d�tails de prise en charge de la cha�ne de balisage pour le p�riph�rique.
         *
         * @return Les d�tails de prise en charge de la cha�ne de balisage pour le p�riph�rique.
         */
        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }

        /**
         * @brief Trouve un type de m�moire appropri� pour une utilisation sp�cifi�e.
         *
         * @param typeFilter Le type de filtre de m�moire.
         * @param properties Les propri�t�s de la m�moire.
         * @return Le type de m�moire appropri�.
         */
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        /**
         * @brief Trouve les familles de files de commandes physiques disponibles sur le p�riph�rique.
         *
         * @return Les familles de files de commandes physiques disponibles sur le p�riph�rique.
         */
        QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }

        /**
         * @brief Trouve un format d'image support� par le p�riph�rique.
         *
         * @param candidates Les formats d'image candidats.
         * @param tiling L'inclinaison de l'image.
         * @param features Les fonctionnalit�s du format de l'image.
         * @return Le format d'image support� par le p�riph�rique.
         */
        VkFormat findSupportedFormat(
            const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);


        // Buffer Helper Functions
        /**
         * @brief Cr�e un tampon Vulkan avec les param�tres sp�cifi�s.
         *
         * @param size La taille du tampon.
         * @param usage Les indicateurs d'utilisation du tampon.
         * @param properties Les propri�t�s de la m�moire du tampon.
         * @param buffer R�f�rence � l'objet tampon Vulkan cr��.
         * @param bufferMemory R�f�rence � l'objet m�moire du tampon Vulkan cr��.
         */
        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);

        /**
         * @brief D�marre une s�quence de commandes Vulkan temporaires.
         *
         * Cette fonction alloue un tampon de commandes Vulkan et le d�marre pour une utilisation unique.
         *
         * @return Le tampon de commandes Vulkan allou�.
         */
        VkCommandBuffer beginSingleTimeCommands();

        /**
         * @brief Termine une s�quence de commandes Vulkan temporaires.
         *
         * Cette fonction termine l'ex�cution d'une s�quence de commandes Vulkan temporaires et lib�re les ressources associ�es.
         *
         * @param commandBuffer Le tampon de commandes Vulkan � terminer.
         */
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        /**
         * @brief Copie les donn�es d'un tampon source vers un tampon de destination.
         *
         * @param srcBuffer Le tampon source � partir duquel copier les donn�es.
         * @param dstBuffer Le tampon de destination o� copier les donn�es.
         * @param size La taille des donn�es � copier.
         */
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void copyBufferToImage(
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

        void createImageWithInfo(
            const VkImageCreateInfo& imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory);

        VkPhysicalDeviceProperties properties;

    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char*> getRequiredExtensions();
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        LveWindow& window;
        VkCommandPool commandPool;

        VkDevice device_;
        VkSurfaceKHR surface_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;

        const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    };

}  // namespace lve