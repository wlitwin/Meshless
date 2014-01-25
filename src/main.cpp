#include <GL/glew.h>
#include <GL/glfw.h>

#include <cstdlib>  // For EXIT_SUCCESS/FAILURE
#include <iostream> // For cout/cerr

#include <sstream>

#include "application.hpp"

using std::cerr;
using std::cout;

// Window size
const int window_width  = 500;
const int window_height = 500;

// Simulation variables
const double SIM_FPS = 60.0;
const double SIM_DT  = 1.0 / SIM_FPS;
const double SIM_MAX_DT = 10.0 * SIM_DT;

// Change these to change what version of OpenGL to use
const int desired_major_version = 3;
const int desired_minor_version = 3;

/* Called when the window is resized. We need to tell the
 * application that the window size has changed so it can
 * update some internal information, like FBOs.
 */
void GLFWCALL window_resize(int width, int height)
{
    resolution_changed(width, height);
}

/* Setup the OpenGL context and window using GLFW. GLFW
 * is a very useful cross-platform library that will do
 * all the dirty work of setting up a window and context
 * for you. It also abstracts a lot of basic functions
 * like keyboard and mouse input for you.
 */
int main(int argc, char* argv[])
{
    if (!glfwInit()) 
    {
        cerr << "Failed to initialize GLFW\n";
        exit(EXIT_FAILURE);
    }

    // Set the version of OpenGL we want to use
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, desired_major_version);   
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, desired_minor_version);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Try to create the context and open the window
    if (!glfwOpenWindow(window_width, window_height, 8, 8, 8, 8, 24, 0, GLFW_WINDOW))
    {
        cerr << "Failed to create an OpenGL context\n";
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // We need to also initialize GLEW because it loads the entry
    // points for the extension functions, like glGenVertexArray()
    // and other functions that are required by a newer OpenGL
    // context.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) 
    {
        cerr << "Failed to initialize GLEW\n";
        exit(EXIT_FAILURE);
    }

    if (!GLEW_VERSION_3_3)
    {
        cerr << "Insufficient GLEW version\n";
        exit(EXIT_FAILURE);
    }

    // Clear the OpenGL error flag. GLEW apparently will still sometimes
    // cause an INVALID_ENUM error after initialization. This can be ignored
    // according to the GLEW documentation.
    glGetError();

    // Make sure we got the version of OpenGL we asked for
    const int v_major = glfwGetWindowParam(GLFW_OPENGL_VERSION_MAJOR);
    const int v_minor = glfwGetWindowParam(GLFW_OPENGL_VERSION_MINOR);

    if (v_major < desired_major_version || 
        v_minor < desired_minor_version) 
    {
        cerr << "Failed to get appropriate OpenGL Context, got: "
             << v_major << "." << v_minor << "\n";
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Check what we ended up with
    cout << "OpenGL Context: " << v_major << "." << v_minor << "\n";

    const int forward_compat = glfwGetWindowParam(GLFW_OPENGL_FORWARD_COMPAT);
    const int opengl_context = glfwGetWindowParam(GLFW_OPENGL_PROFILE);
    const int debug_context  = glfwGetWindowParam(GLFW_OPENGL_DEBUG_CONTEXT);

    if (forward_compat == GL_TRUE) 
    {
        cout << "Forward Compatible\n";
    }

    switch (opengl_context)
    {
        case GLFW_OPENGL_CORE_PROFILE:
            cout << "Core Profile\n";
            break;
        case GLFW_OPENGL_COMPAT_PROFILE:
            cout << "Compat Profile\n";
            break;
    }

    if (debug_context == GL_TRUE)
    {
        cout << "Debug Context\n";
    }

    // Do some more minor setup things
    glfwSetWindowTitle(title);

    // Center the window
    GLFWvidmode video_mode;
    glfwGetDesktopMode(&video_mode);
    glfwSetWindowPos((video_mode.Width - window_width) / 2,
                     (video_mode.Height - window_height) / 2);
    
    // Create the application
    if (!initialize(argc, argv))
    {
        std::cerr << "Application failed to initialize\n";
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Make sure the application knows the current window size
    resolution_changed(window_width, window_height);

    // Called when the user changes the window size
    glfwSetWindowSizeCallback(window_resize);

    // Enter the actual update/render loop
    glfwSwapInterval(1); // Limit to 60Hz
    int frames = 0;
    double time_passed = 0;

    bool quit = false;
    double sim_time = 0.0;

    double previous_time = glfwGetTime();
    while (!quit && glfwGetWindowParam(GLFW_OPENED) && !glfwGetKey(GLFW_KEY_ESC))
    {
        // Get the elapsed time since the last update
        double current_time = glfwGetTime();
        const double delta_time = std::min(current_time - previous_time, SIM_MAX_DT);
        previous_time = current_time;

        sim_time += delta_time;
        if (sim_time > SIM_MAX_DT)
        {
            sim_time = SIM_MAX_DT;
        }

        while (sim_time >= SIM_DT)
        {
            glfwPollEvents();
            sim_time -= SIM_DT;
            if (!update(SIM_DT))
            {
                quit = true;
            }
        }

        // Render the application's state
        render();

        // Put the render on the screen
        glfwSwapBuffers();

        // Frame rate calculation
        time_passed += delta_time;
        if (time_passed >= 1.0f)
        {
            std::stringstream fps;          
            fps << title << " (";
            fps << frames << " fps)";
            glfwSetWindowTitle(fps.str().c_str());

            time_passed = 0;
            frames = 0;
        }
        ++frames;
    }

    // Tell the application to cleanup
    cleanup();

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
