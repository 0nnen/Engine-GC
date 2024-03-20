#pragma once

#include "lve_window.h"
#include "lve_pipeline.h"

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

        /**
         * @brief Lance l'ex�cution de l'application.
         *
         * Cette fonction d�marre l'ex�cution de l'application Vulkan, y compris la cr�ation de la fen�tre.
         */
        void Run();

    private:
        LveWindow lveWindow{ WIDTH, HEIGHT, "Hello Vulkan!" }; /// Fen�tre de l'application.
        LvePipeline lvePipeline{ "Shaders/simple_shader.vert.spv", "Shaders/simple_shader.frag.spv" };
    };

} // namespace lve
