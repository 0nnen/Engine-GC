#pragma once

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace lve {

    class LveWindow {
    public:

        enum GLFW_CURSOR_TYPE{
            ARROW,
            IBEAM,
            CROSSHAIR,
            POINTING_HAND,
            RESIZE_EW,
            RESIZE_NS,
            RESIZE_NWSE,
            RESIZE_NESW,
            RESIZE_ALL,
            NOT_ALLOWED
        };

        enum GLFW_CURSOR_MODE {
            DISABLED,
            HIDDEN,
            NORMAL,
            CAPTURED
        };


        /**
         * @brief Constructeur de la classe LveWindow.
         *
         * Initialise une instance de LveWindow avec les dimensions sp�cifi�es et le nom de la fen�tre.
         *
         * @param _width La largeur de la fen�tre.
         * @param _height La hauteur de la fen�tre.
         * @param _name Le nom de la fen�tre.
         * @return Rien.
         */
        LveWindow(int _width, int _height, std::string _name);

        /**
         * @brief Destructeur de la classe LveWindow.
         *
         * D�truit la fen�tre GLFW.
         */
        ~LveWindow();

        LveWindow(const LveWindow&) = delete;
        LveWindow& operator=(const LveWindow&) = delete;

        /**
         * @brief V�rifie si la fen�tre doit �tre ferm�e.
         *
         * Cette fonction renvoie true si la fen�tre doit �tre ferm�e, false sinon.
         *
         * @return true si la fen�tre doit �tre ferm�e, false sinon.
         */
        bool ShouldClose() { return glfwWindowShouldClose(window); }

        vk::Extent2D GetExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

        bool WasWindowResized() { return frameBufferResize;  }

        void ResetWindowResizedFlag() { frameBufferResize = false;  }

        /**
        * @brief Cr�e une surface Vulkan associ�e � une fen�tre.
        *
        * Cette fonction cr�e une surface Vulkan associ�e � une fen�tre, permettant � Vulkan de dessiner dans cette fen�tre.
        *
        * @param _instance L'instance Vulkan utilis�e pour cr�er la surface.
        * @param _surface Un pointeur vers l'objet de surface Vulkan � cr�er. Ce pointeur sera mis � jour pour contenir la surface cr��e.
        * @throws Une exception en cas d'�chec lors de la cr�ation de la surface.
        */
        void CreateWindowSurface(vk::Instance _instance, vk::SurfaceKHR* _surface);

        GLFWwindow* GetGLFWwindow() const { return window; }

        void ResetCursorWindow() { glfwSetCursor(window, NULL); }

        void CubeCursorWindow(int color = 255);

        void StandarCursorWindow(GLFW_CURSOR_TYPE CursorType);

        void SetInputCursorMode(GLFW_CURSOR_MODE mode);

    private:

        static void FramebufferResizeCallBack(GLFWwindow* _window, int _width, int _height);

        /**
         * @brief Initialise la fen�tre GLFW.
         *
         * Cette fonction initialise GLFW, configure la fen�tre pour �tre non redimensionnable et cr�e une fen�tre GLFW avec la taille et le nom sp�cifi�s.
         */
        void InitWindow();

        int width; // Largeur de la fen�tre
        int height; // Hauteur de la fen�tre
        bool frameBufferResize = false;

        std::string windowName; // Nom de la fen�tre
        GLFWwindow* window; // Fen�tre GLFW
    };

} // namespace lve
