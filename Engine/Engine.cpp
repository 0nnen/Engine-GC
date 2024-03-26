#include "first_app.h"

#include <iostream>
#include <cstdlib>
#include <stdexcept>

int main() {
    /**
     * @brief Fonction principale du programme.
     *
     * Cette fonction cr�e une instance de l'application FirstApp et lance son ex�cution en appelant la m�thode Run().
     *
     * @return EXIT_SUCCESS si l'application s'est termin�e avec succ�s, EXIT_FAILURE sinon.
     */
    lve::FirstApp app{};

    try {
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}