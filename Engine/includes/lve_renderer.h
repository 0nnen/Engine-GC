#pragma once

#include "lve_device.h"
#include "lve_swap_chain.h"
#include "lve_window.h"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace lve {
    class LveRenderer {
    public:
        LveRenderer(LveWindow& _window, LveDevice& _device);
        ~LveRenderer();

        LveRenderer(const LveRenderer&) = delete;
        LveRenderer& operator=(const LveRenderer&) = delete;

        /**
         * @brief Obtient l'objet de rendu associ� � la cha�ne de swap.
         *
         * Cette fonction retourne l'objet de rendu associ� � la cha�ne de swap.
         *
         * @return VkRenderPass L'objet de rendu associ� � la cha�ne de swap.
         */
        VkRenderPass GetSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }

        /**
         * @brief Obtient le ratio de l'aspect de la fen�tre associ�e � la cha�ne de swap.
         *
         * Cette fonction retourne le ratio de l'aspect de la fen�tre associ�e � la cha�ne de swap.
         *
         * @return float Le ratio de l'aspect de la fen�tre.
         */
        float GetAspectRatio() const { return lveSwapChain->extentAspectRatio(); }

        /**
         * @brief V�rifie si un cadre est en cours.
         *
         * Cette fonction retourne vrai si un cadre est actuellement en cours, sinon elle retourne faux.
         *
         * @return bool Vrai si un cadre est en cours, sinon faux.
         */
        bool IsFrameInProgress() const { return isFrameStarted; }

        /**
         * @brief Obtient le tampon de commandes actuel.
         *
         * Cette fonction retourne le tampon de commandes actuel. Elle suppose qu'un cadre est en cours.
         *
         * @return VkCommandBuffer Le tampon de commandes actuel.
         * @throws std::runtime_error si aucun cadre n'est en cours.
         */
        VkCommandBuffer GetCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        /**
         * @brief Obtient l'index du cadre en cours.
         *
         * Cette fonction retourne l'index du cadre en cours. Elle suppose qu'un cadre est en cours.
         *
         * @return int L'index du cadre en cours.
         * @throws std::runtime_error si aucun cadre n'est en cours.
         */
        int GetFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        /**
         * @brief D�marre un nouveau cadre.
         *
         * Cette fonction d�marre un nouveau cadre en acqu�rant l'image suivante de la cha�ne de swaps.
         *
         * @return VkCommandBuffer Le tampon de commandes actuel.
         * @throws std::runtime_error si l'acquisition de l'image �choue ou si le tampon de commandes ne peut pas �tre commenc�.
         */
        VkCommandBuffer BeginFrame();

        /**
         * @brief Termine le cadre en cours.
         *
         * Cette fonction termine le cadre en cours en arr�tant l'enregistrement du tampon de commandes, en soumettant
         * le tampon de commandes � la cha�ne de swaps pour pr�sentation et en g�rant les �v�nements de redimensionnement de la fen�tre.
         *
         * @throws std::runtime_error si l'arr�t de l'enregistrement du tampon de commandes �choue, si la pr�sentation de l'image de la cha�ne de swaps �choue
         * ou si une redimensionnement de la fen�tre est n�cessaire.
         */
        void EndFrame();

        /**
         * @brief D�marre la passe de rendu de la cha�ne de swaps.
         *
         * Cette fonction d�marre la passe de rendu de la cha�ne de swaps en commen�ant par initialiser
         * les informations sur le rendu, en d�finissant la zone de rendu et en d�finissant les valeurs de nettoyage.
         *
         * @param commandBuffer Le tampon de commandes sur lequel commencer la passe de rendu.
         *
         * @throws std::runtime_error si la commande de d�but de la passe de rendu �choue.
         */
        void BeginSwapChainRenderPass(VkCommandBuffer _commandBuffer);

        /**
         * @brief Termine la passe de rendu de la cha�ne de swaps.
         *
         * Cette fonction termine la passe de rendu de la cha�ne de swaps en appelant la commande de fin
         * de la passe de rendu sur le tampon de commandes sp�cifi�.
         *
         * @param commandBuffer Le tampon de commandes sur lequel terminer la passe de rendu.
         */
        void EndSwapChainRenderPass(VkCommandBuffer _commandBuffer);

    private:

        /**
         * @brief Cr�e les tampons de commandes.
         *
         * Cette m�thode alloue les tampons de commandes pour chaque image de la cha�ne de swaps.
         *
         * @throw std::runtime_error si l'allocation des tampons de commandes �choue.
         */
        void CreateCommandBuffers();

        /**
         * @brief Lib�re les tampons de commandes.
         *
         * Cette m�thode lib�re les tampons de commandes allou�s pr�c�demment.
         */
        void FreeCommandBuffers();

        /**
         * @brief Recreate la cha�ne de swaps.
         *
         * Cette m�thode recr�e la cha�ne de swaps en fonction de la nouvelle �tendue de la fen�tre.
         * Elle attend �galement que la fen�tre ait une �tendue valide avant de proc�der � la recr�ation.
         * Si une cha�ne de swaps existait d�j�, elle est d�truite et remplac�e par la nouvelle cha�ne de swaps.
         * Si les formats d'image (ou de profondeur) de la nouvelle cha�ne de swaps diff�rent de ceux de l'ancienne,
         * une erreur est lev�e.
         */
        void RecreateSwapChain();

        LveWindow& lveWindow;
        LveDevice& lveDevice;
        std::unique_ptr<LveSwapChain> lveSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex = 0;
        int currentFrameIndex = 0;
        bool isFrameStarted = false;
    };
}  // namespace lve