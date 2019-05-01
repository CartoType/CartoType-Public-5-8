#define GLFW_INCLUDE_ES2

#include <GLFW/glfw3.h>
#include <cartotype_framework.h>
#include <cartotype_vector_tile.h>

class MapWindow
    {
    public:
    MapWindow();
    ~MapWindow();
    bool Draw();

    private:
    static void HandleKeyStroke(GLFWwindow* aWindow,int aKey,int aScancode,int aAction,int aMods);

    GLFWwindow* m_window = nullptr;
    std::unique_ptr<CartoType::CFramework> m_framework;
    std::unique_ptr<CartoType::CVectorTileServer> m_vector_tile_server;
    };

MapWindow::MapWindow()
    {
    m_window = glfwCreateWindow(1024,1024,"CartoType for Unix",nullptr,nullptr);
    if (!m_window)
        throw std::runtime_error("could not create GLFW window");

    glfwMakeContextCurrent(m_window);

    CartoType::TResult error = 0;
    m_framework = CartoType::CFramework::New(error,"../../../../src/test/data/ctm1/santa-cruz.ctm1","../../../../style/standard.ctstyle","../../../../font/DejaVuSans.ttf",1024,1024);
    m_vector_tile_server = CartoType::CreateOpenGLESVectorTileServer(*m_framework);
    glfwSetWindowUserPointer(m_window,m_framework.get());
    glfwSetKeyCallback(m_window,HandleKeyStroke);

    //m_framework->SetScaleDenominator(25000);
    }

bool MapWindow::Draw()
    {
    if (glfwWindowShouldClose(m_window))
        return false;
    m_vector_tile_server->Draw();
    /* Swap front and back buffers */
    glfwSwapBuffers(m_window);
    return true;
    }

MapWindow::~MapWindow()
    {
    glfwDestroyWindow(m_window);
    }

void MapWindow::HandleKeyStroke(GLFWwindow* aWindow,int aKey,int aScancode,int aAction,int aMods)
    {
    if (aAction == GLFW_RELEASE)
        return;

    CartoType::CFramework* f = (CartoType::CFramework*)glfwGetWindowUserPointer(aWindow);
    switch (aKey)
        {
        case GLFW_KEY_I: f->Zoom(1.05); break;
        case GLFW_KEY_O: f->Zoom(1.0 / 1.05); break;
        case GLFW_KEY_P: f->SetPerspective(!f->Perspective()); break;
        case GLFW_KEY_RIGHT: f->Pan(32,0); break;
        case GLFW_KEY_LEFT: f->Pan(-32,0); break;
        case GLFW_KEY_UP: f->Pan(0,-32); break;
        case GLFW_KEY_DOWN: f->Pan(0,32); break;
        }

    }

int main(void)
    {
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // Create a Map window.
    MapWindow map_window;

    /* Draw continuously until the user closes the window */
    while (map_window.Draw())
        {

        /* Poll for and process events */
        glfwPollEvents();
        }

    glfwTerminate();
    return 0;
    }
