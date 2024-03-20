#include "first_app.h"

namespace lve {

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

} // namespace lve
