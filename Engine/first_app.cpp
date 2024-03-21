#include "first_app.h"

#include <stdexcept>

namespace lve {

    FirstApp::FirstApp() {
        CreatePipelineLayout();
        CreatePipeline();
        CreateCommandBuffers();
    }

    FirstApp::~FirstApp() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void FirstApp::Run() {
        /**
         * @brief Boucle principale de l'application.
         *
         * Cette boucle s'ex�cute tant que la fen�tre de l'application n'a pas �t� ferm�e.
         */
        while (!lveWindow.ShouldClose()) {
            glfwPollEvents(); // Traite tous les �v�nements de la fen�tre
        }
    }




    void FirstApp::CreatePipelineLayout(){
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }
;    }

    void FirstApp::CreatePipeline() {
        auto pipelineConfig = LvePipeline::DefaultPipelineConfigInfo(lveSwapChain.width(), lveSwapChain.height());

        pipelineConfig.renderPass = lveSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LvePipeline>(
            lveDevice,
            "Shaders/simple_shader.vert.spv",
            "Shaders/simple_shader.frag.spv",
            pipelineConfig
        );
    }


    void FirstApp::CreateCommandBuffers() {
    
    }


    void FirstApp::DrawFrame() {
    
    }


} // namespace lve
