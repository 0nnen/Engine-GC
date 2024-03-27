#pragma once

#include "lve_device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace lve {

    class LveSwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        LveSwapChain(LveDevice& deviceRef, VkExtent2D windowExtent);
        LveSwapChain(LveDevice& deviceRef, VkExtent2D extent, std::shared_ptr<LveSwapChain> previous);
        ~LveSwapChain();
        void Init();

        LveSwapChain(const LveSwapChain&) = delete;
        LveSwapChain &operator=(const LveSwapChain&) = delete;

        /**
         * @brief Obtient le framebuffer correspondant � l'indice sp�cifi�.
         *
         * Cette fonction retourne le framebuffer de la swap chain qui correspond � l'indice sp�cifi�.
         *
         * @param index L'indice du framebuffer � r�cup�rer.
         * @return VkFramebuffer Le framebuffer correspondant � l'indice sp�cifi�.
         */
        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }

        /**
         * @brief Obtient le passe de rendu.
         *
         * Cette fonction retourne le passe de rendu associ� � la swap chain.
         *
         * @return VkRenderPass Le passe de rendu associ� � la swap chain.
         */
        VkRenderPass getRenderPass() { return renderPass; }

        /**
         * @brief Obtient la vue d'image correspondant � l'indice sp�cifi�.
         *
         * Cette fonction retourne la vue d'image de la swap chain qui correspond � l'indice sp�cifi�.
         *
         * @param index L'indice de la vue d'image � r�cup�rer.
         * @return VkImageView La vue d'image correspondant � l'indice sp�cifi�.
         */
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }

        /**
         * @brief Obtient le nombre d'images dans la swap chain.
         *
         * Cette fonction retourne le nombre d'images actuellement pr�sentes dans la swap chain.
         *
         * @return size_t Le nombre d'images dans la swap chain.
         */
        size_t imageCount() { return swapChainImages.size(); }

        /**
         * @brief Obtient le format d'image de la swap chain.
         *
         * Cette fonction retourne le format d'image utilis� par la swap chain.
         *
         * @return VkFormat Le format d'image de la swap chain.
         */
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }

        /**
         * @brief Obtient l'�tendue de la swap chain.
         *
         * Cette fonction retourne l'�tendue (largeur et hauteur) de la swap chain.
         *
         * @return VkExtent2D L'�tendue de la swap chain.
         */
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }

        /**
         * @brief Obtient la largeur de la swap chain.
         *
         * Cette fonction retourne la largeur de la swap chain.
         *
         * @return uint32_t La largeur de la swap chain.
         */
        uint32_t width() { return swapChainExtent.width; }

        /**
         * @brief Obtient la hauteur de la swap chain.
         *
         * Cette fonction retourne la hauteur de la swap chain.
         *
         * @return uint32_t La hauteur de la swap chain.
         */
        uint32_t height() { return swapChainExtent.height; }

        /**
         * @brief Calcule le rapport d'aspect de l'�tendue de la swap chain.
         *
         * Cette fonction calcule le rapport d'aspect (largeur/hauteur) de l'�tendue de la swap chain.
         *
         * @return float Le rapport d'aspect de l'�tendue de la swap chain.
         */
        float extentAspectRatio() {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }

        /**
         * @brief Recherche le format de profondeur adapt�.
         *
         * Cette fonction recherche le format de profondeur adapt� parmi une liste de formats donn�e, en tenant compte des caract�ristiques sp�cifi�es.
         *
         * @return VkFormat Le format de profondeur adapt�.
         */
        VkFormat findDepthFormat();

        /**
         * @brief Acquiert l'index de l'image suivante dans la cha�ne de swaps.
         *
         * Cette fonction acquiert l'index de l'image suivante dans la cha�ne de swaps.
         *
         * @param imageIndex Pointeur vers la variable o� stocker l'index de l'image acquise.
         * @return VkResult Le r�sultat de l'op�ration.
         */
        VkResult acquireNextImage(uint32_t* imageIndex);

        /**
         * @brief Soumet les command buffers pour ex�cution et pr�sente le r�sultat.
         *
         * Cette fonction soumet les command buffers sp�cifi�s pour ex�cution, attend leur ach�vement, puis pr�sente le r�sultat � l'�cran.
         *
         * @param buffers Tableau des command buffers � soumettre.
         * @param imageIndex Pointeur vers l'index de l'image � pr�senter.
         * @return VkResult Le r�sultat de l'op�ration.
         */
        VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

        /**
         * @brief Compare les formats de profondeur et d'image avec une autre LveSwapChain.
         *
         * Cette fonction compare les formats de profondeur et d'image de cette LveSwapChain avec ceux d'une autre LveSwapChain sp�cifi�e.
         *
         * @param _swapChain La LveSwapChain avec laquelle comparer les formats.
         * @return bool True si les formats de profondeur et d'image sont identiques, sinon False.
         */
        bool CompareSwapFormats(const LveSwapChain& _swapChain) const {
            return  _swapChain.swapChainDepthFormat == swapChainDepthFormat && 
                    _swapChain.swapChainImageFormat == swapChainImageFormat;
        }

    private:

        /**
         * @brief Cr�e la cha�ne de swaps.
         *
         * Cette fonction cr�e la cha�ne de swaps, qui est utilis�e pour la pr�sentation des images � l'�cran.
         * La cr�ation de la cha�ne de swaps implique la s�lection des formats de surface, des modes de pr�sentation et des dimensions appropri�s, ainsi que la configuration de divers autres param�tres li�s � la pr�sentation.
         *
         * @throws std::runtime_error si la cr�ation de la cha�ne de swaps �choue.
         */
        void createSwapChain();

        /**
         * @brief Cr�e les vues d'image de la cha�ne de swaps.
         *
         * Cette fonction cr�e les vues d'image pour chaque image de la cha�ne de swaps.
         * Chaque vue d'image est associ�e � une image de la cha�ne de swaps et sp�cifie comment cette image doit �tre interpr�t�e lorsqu'elle est utilis�e comme texture.
         *
         * @throws std::runtime_error si la cr�ation d'une vue d'image �choue.
         */
        void createImageViews();

        /**
        * @brief Cr�e les ressources de profondeur pour la cha�ne de swaps.
        *
        * Cette fonction cr�e les ressources de profondeur n�cessaires pour chaque image de la cha�ne de swaps.
        * Ces ressources de profondeur sont utilis�es pour le rendu des sc�nes en 3D afin de d�terminer la profondeur des fragments et g�rer la d�tection des collisions.
        *
        * @throws std::runtime_error si la cr�ation d'une ressource de profondeur �choue.
        */
        void createDepthResources();

        /**
         * @brief Cr�e la passe de rendu.
         *
         * Cette fonction cr�e la passe de rendu utilis�e pour d�finir comment les donn�es de g�om�trie sont trait�es lors du rendu d'une image.
         * La passe de rendu d�finit les attachements de couleur et de profondeur, ainsi que les sous-passes qui d�crivent les op�rations de rendu � ex�cuter.
         *
         * @throws std::runtime_error si la cr�ation de la passe de rendu �choue.
         */
        void createRenderPass();

        /**
         * @brief Cr�e les tampons de trame.
         *
         * Cette fonction cr�e les tampons de trame qui sont utilis�s pour stocker les informations de rendu associ�es � chaque image de la cha�ne de swaps.
         * Chaque tampon de trame est associ� � une image de la cha�ne de swaps et est configur� avec les vues d'image et les profondeurs appropri�es pour le rendu.
         *
         * @throws std::runtime_error si la cr�ation d'un tampon de trame �choue.
         */
        void createFramebuffers();

        /**
         * @brief Cr�e les objets de synchronisation.
         *
         * Cette fonction cr�e les objets de synchronisation utilis�s pour synchroniser l'ex�cution des op�rations de rendu avec la pr�sentation des images.
         * Ces objets comprennent des s�maphores pour la synchronisation entre les diff�rentes �tapes du pipeline graphique, ainsi que des cl�tures de barri�re pour la synchronisation entre le CPU et le GPU.
         *
         * @throws std::runtime_error si la cr�ation d'un objet de synchronisation �choue.
         */
        void createSyncObjects();

        // Helper functions

        /**
         * @brief Choisissez le format de surface de la cha�ne de swaps.
         *
         * Cette fonction choisit le format de surface de la cha�ne de swaps en parcourant les formats disponibles et en recherchant un format sp�cifique (B8G8R8A8_SRGB) avec un espace colorim�trique compatible (VK_COLOR_SPACE_SRGB_NONLINEAR_KHR).
         * Si un tel format est trouv�, il est renvoy�. Sinon, le premier format disponible est renvoy� par d�faut.
         *
         * @param availableFormats Les formats de surface disponibles.
         * @return Le format de surface choisi.
         */
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);

        /**
         * @brief Choisissez le mode de pr�sentation de la cha�ne de swaps.
         *
         * Cette fonction choisit le mode de pr�sentation de la cha�ne de swaps en parcourant les modes de pr�sentation disponibles et en recherchant un mode sp�cifique (VK_PRESENT_MODE_IMMEDIATE_KHR) qui offre une pr�sentation imm�diate sans attente de synchronisation verticale (V-Sync).
         * Si un tel mode est trouv�, il est renvoy�. Sinon, le mode de pr�sentation FIFO (VK_PRESENT_MODE_FIFO_KHR) avec synchronisation verticale est renvoy� par d�faut.
         *
         * @param availablePresentModes Les modes de pr�sentation disponibles.
         * @return Le mode de pr�sentation choisi.
         */
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);

        /**
         * @brief Choisissez l'�tendue de la cha�ne de swaps.
         *
         * Cette fonction choisit l'�tendue de la cha�ne de swaps en se basant sur les capacit�s de la surface de rendu.
         * Si l'�tendue actuelle est d�j� d�finie dans les capacit�s de la surface, elle est retourn�e directement.
         * Sinon, une �tendue appropri�e est calcul�e en fonction de la taille de la fen�tre et des contraintes minimales et maximales sp�cifi�es dans les capacit�s de la surface.
         *
         * @param capabilities Les capacit�s de la surface de rendu.
         * @return L'�tendue de la cha�ne de swaps choisie.
         */
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkFormat swapChainImageFormat;
        VkFormat swapChainDepthFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        LveDevice& device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;
        std::shared_ptr<LveSwapChain> oldSwapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };

}  // namespace lve
