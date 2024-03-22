#pragma once

#include "lve_window.h"
#include "lve_pipeline.h"
#include "lve_device.h"
#include "lve_swap_chain.h"
#include "lve_model.h"

//std
#include <memory>
#include <vector>

namespace lve {

    /**
     * @brief Classe repr�sentant la premi�re application utilisant Vulkan.
     *
     * Cette classe g�re l'ex�cution de la premi�re application Vulkan, incluant la cr�ation de la fen�tre.
     */
    class FirstApp {
    public:
        static constexpr int WIDTH = 800; ///< Largeur de la fen�tre par d�faut.
        static constexpr int HEIGHT = 600; ///< Hauteur de la fen�tre par d�faut.

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp&) = delete;
        FirstApp operator=(const FirstApp&) = delete;

        /**
         * @brief Lance l'ex�cution de l'application.
         *
         * Cette fonction d�marre l'ex�cution de l'application Vulkan, y compris la cr�ation de la fen�tre.
         */
        void Run();

    private:
        void LoadModels();
        void CreatePipelineLayout();
        void CreatePipeline();
        void CreateCommandBuffers();
        void DrawFrame();
        void FreeCommandBuffers();
        void RecreateSwapChain();
        void RecordCommandBuffer(int _imageIndex);

        LveWindow lveWindow{ WIDTH, HEIGHT, "Hello Vulkan!" }; /// Fen�tre de l'application.
        LveDevice lveDevice{ lveWindow };
        std::unique_ptr<LveSwapChain> lveSwapChain;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<LveModel> lveModel;
    };

} // namespace lve
