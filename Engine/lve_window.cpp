#include "lve_window.h"
#include <stdexcept>    

namespace lve {

    // Constructeur  de la classe LveWindow
    LveWindow::LveWindow(int _width, int _height, std::string _name) : width{ _width }, height{ _height }, windowName{ _name } {
        // Initialise la fen�tre GLFW
        InitWindow();
    }

    void LveWindow::InitWindow() {
        // Initialise GLFW
        glfwInit();

        // Configure la fen�tre pour ne pas �tre redimensionnable
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // Cr�e la fen�tre GLFW avec la taille et le nom sp�cifi�s
        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    }

    void LveWindow::CreateWindowSurface(VkInstance _instance, VkSurfaceKHR* _surface) {
        if (glfwCreateWindowSurface(_instance, window, nullptr, _surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
    }

    // Destructeur de la classe LveWindow
    LveWindow::~LveWindow() {
        // D�truit la fen�tre GLFW
        glfwDestroyWindow(window);
        // Termine GLFW
        glfwTerminate();
    }

} // namespace lve
