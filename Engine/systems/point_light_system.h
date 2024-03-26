#pragma once

#include "lve_camera.h"
#include "lve_pipeline.h"
#include "lve_device.h"
#include "lve_game_object.h"
#include "lve_frame_info.h"

//std
#include <memory>
#include <vector>

namespace lve {
    /**
     * @brief Classe repr�sentant la premi�re application utilisant Vulkan.
     *
     * Cette classe g�re l'ex�cution de la premi�re application Vulkan, incluant la cr�ation de la fen�tre.
     */
    class PointLightSystem {
    public:

        PointLightSystem(LveDevice& _device, VkRenderPass _renderPass, VkDescriptorSetLayout _globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem operator=(const PointLightSystem&) = delete;


        void Render(FrameInfo &_frameInfo);

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout _globalSetLayout);
        void CreatePipeline(VkRenderPass _renderPass);

        LveDevice &lveDevice;


        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };

} // namespace lve
