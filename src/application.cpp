#include "defs.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "psystem.hpp"
#include "objloader.hpp"
#include "application.hpp"

#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ctime>
#include <cstdlib>
#include <iostream>

using namespace dlib;
using std::cout;
using std::cerr;
using std::endl;

// Application title
const char* title = "Meshless";

float width = 0;
float height = 0;
double dt_multiplier = 1.0; // slow motion

Camera camera;

Mesh g_obj_mesh; // Loaded OBJ model
Mesh g_ground_mesh; // Floor plane
PSystem* g_psystem; // Particle system, does all the work

real g_t_min = 0.0; // Used for closest particle (during picking)
bool g_object_selected = false; // Checks if the object has been picked up

// Tracks if the mouse pointer is currently visible
bool g_mouse_pointer_enabled = false;
bool g_mouse_down = false;

// Small macro to check for a single keydown event
bool __glfw_keys[GLFW_KEY_LAST + 1];
#define glfwKeyPressed(KEY)\
    (glfwGetKey(KEY) == GLFW_PRESS ?\
     (__glfw_keys[KEY] ? false : (__glfw_keys[KEY] = true)) :\
     (__glfw_keys[KEY] = false))

// Current mouse position
int g_mouse_x = 0;
int g_mouse_y = 0;

// Shaders
Shader ground_shader;
Shader object_shader;

// Forces
dlib::vec3 g_gravity;

//=============================================================================
// initialize
//=============================================================================

static void print_help();

bool initialize(int argc, char* argv[])
{
    glEnable(GL_DEPTH_TEST);

    // Load the model
    ObjLoader obj;
    const char* file = "sphere.obj";
    if (argc > 1)
    {
        file = argv[1];
    }

    if (!obj.LoadFile(file) || !obj.ToMesh(g_obj_mesh, Mesh::VERTICES))
    {
        cerr << "Failed to load OBJ file" << endl;
        return false;
    }

    size_t size = g_obj_mesh.GetDataSize();
    real* data = g_obj_mesh.GetData();
    for (size_t i = 0; i < size; i += 3)
    {
        data[i+0] += 0;
        data[i+1] += 5;
        data[i+2] += 0;
    }
    g_obj_mesh.UpdateData();

    g_psystem = new PSystem(g_obj_mesh);

	std::cout << g_psystem->GetNumParticles() << " number of particles\n";

    // Create the ground plane
    g_ground_mesh.NewMesh();

    using dlib::vec3;
    using dlib::vec2;
    float norm[3] = { 0, 1, 0 };
    const float plane_half_size = 20.0f;
    float v[4][3] = { {-plane_half_size, 0, -plane_half_size},
                      {-plane_half_size, 0,  plane_half_size},
                      { plane_half_size, 0,  plane_half_size},
                      { plane_half_size, 0, -plane_half_size},
                    };
    float t[4][2] = { {0, 0}, {0, 1}, {1, 1}, {1, 0} };
    vec3 normal(norm);
    g_ground_mesh.AddQuad(vec3(v[0]), vec2(t[0]), normal,
                        vec3(v[1]), vec2(t[1]), normal,
                        vec3(v[2]), vec2(t[2]), normal,
                        vec3(v[3]), vec2(t[3]), normal);

    g_ground_mesh.Finish();

    glfwDisable(GLFW_MOUSE_CURSOR);
    glfwGetMousePos(&g_mouse_x, &g_mouse_y);

    // Defaults for now
    camera.LookAt(glm::vec3(0, 10, -20), glm::vec3(0), glm::vec3(0, 1, 0));

    // Set the forces
    g_gravity = 0, -9.8, 0;

    srand(time(NULL));

    const bool shaders_loaded = 
            ground_shader.LoadShaders("glsl/ground.vert", "glsl/ground.frag") &&
            object_shader.LoadShaders("glsl/object.vert", "glsl/object.frag",
                                     "glsl/object.geom");
    if (shaders_loaded)
    {
        print_help();
        return true;
    }

    return false;
}

//=============================================================================
// cleanup
//=============================================================================

void cleanup()
{
    delete g_psystem;
}

//=============================================================================
// resolution_changed
//=============================================================================

void resolution_changed(int w, int h)
{
    width = w; 
    height = h;
    glViewport(0, 0, w, h);

    camera.SetPerspective(75.0f, (float)w/h, 0.1f, 1000.0f);
}

//=============================================================================
// randf
//=============================================================================

static float randf()
{
    return static_cast<float>(::rand() / static_cast<float>(RAND_MAX));
}

static float randf(float min, float max)
{
    return randf()*(max-min) + min;
}

//=============================================================================
// print_help
//=============================================================================

static void print_help()
{
    std::cout << "\nControls\n"
              << "W,S,A,D - Move camera in typical FPS fashion\n"
              << "Q,E - Move camera up/down respectively\n"
              << "M - Enable mouse pointer (for throwing the object)\n"
              << "O,L - Increase/decrease alpha value respectively\n"
              << "I,K - Increase/decrease beta value respectively\n"
              << "Y - Print current alpha, beta, and time speed values\n"
#ifdef SLOW_MO
              << "T,G - Increase/decrease time speed respectively\n"
              << "B - Instant slow motion\n"
              << "N - Instant normal motion\n"
#endif
              << "H - Print this help\n"
              << "[ - Add random force to particle system\n"
              << "P - Pause/unpause the simulation\n"
              << "(spacebar) - When simulation is paused, take one step\n"
              << "R - Reset mesh to original location and deformation\n"
			  << "ESC - Quit\n"
			  << std::endl;
}

//=============================================================================
// update
//=============================================================================

static void integrate(double dt);

bool update(double dt)
{
    // Camera movement
    const float CAM_SPEED = 10.0f;
    if (glfwGetKey('W') == GLFW_PRESS) { camera.MoveForward(CAM_SPEED * dt); }
    if (glfwGetKey('A') == GLFW_PRESS) { camera.Strafe(-CAM_SPEED * dt); }
    if (glfwGetKey('D') == GLFW_PRESS) { camera.Strafe(CAM_SPEED * dt); }
    if (glfwGetKey('S') == GLFW_PRESS) { camera.MoveForward(-CAM_SPEED * dt); }
    if (glfwGetKey('Q') == GLFW_PRESS) { camera.MoveUp(CAM_SPEED * dt); }
    if (glfwGetKey('E') == GLFW_PRESS) { camera.MoveUp(-CAM_SPEED * dt); }

    // Move the camera with the mouse
    if (!g_mouse_pointer_enabled)
    {
        int new_mouse_x, new_mouse_y;
        glfwGetMousePos(&new_mouse_x, &new_mouse_y);
        
        const int delta_mouse_x = new_mouse_x - g_mouse_x;
        const int delta_mouse_y = new_mouse_y - g_mouse_y;

        camera.Yaw(delta_mouse_x * 4.0f * dt);
        camera.Pitch(delta_mouse_y * 4.0f * dt);

        g_mouse_x = new_mouse_x;
        g_mouse_y = new_mouse_y;
    }
    else
    {
        glfwGetMousePos(&g_mouse_x, &g_mouse_y);
    }

    g_mouse_down = glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // Take one simulation step, useful when simulation is paused
    static bool run_sim = false;
    if (glfwKeyPressed(' '))
    {
        if (!run_sim)
        {
            integrate(dt);
        }
    }

    // Increase alpha value
    if (glfwKeyPressed('O'))
    {
        g_psystem->SetAlpha(g_psystem->GetAlpha() + 0.1);
        std::cout << "Alpha is now " << g_psystem->GetAlpha() << "\n";
    }

    // Decrease alpha value
    if (glfwKeyPressed('L'))
    {
        g_psystem->SetAlpha(g_psystem->GetAlpha() - 0.1);
        std::cout << "Alpha is now " << g_psystem->GetAlpha() << "\n";
    }

    // Increase beta value
    if (glfwKeyPressed('I'))
    {
        g_psystem->SetBeta(g_psystem->GetBeta() + 0.1);
        std::cout << "Beta is now " << g_psystem->GetBeta() << "\n";
    }

    // Decrease beta value
    if (glfwKeyPressed('K'))
    {
        g_psystem->SetBeta(g_psystem->GetBeta() - 0.1);
        std::cout << "Beta is now " << g_psystem->GetBeta() << "\n";
    }

    // Dump the current settings
    if (glfwKeyPressed('Y'))
    {
        std::cout << "Alpha: " << g_psystem->GetAlpha() << "\n"
                << "Beta: " << g_psystem->GetBeta() << "\n"
                << "Time Speed: " << dt_multiplier << "x\n";
    }

    // Print help
    if (glfwKeyPressed('H'))
    {
        print_help();
    }

    // Reset the mesh
    if (glfwKeyPressed('R'))
    {
        g_psystem->Reset();
    }

    // Enable the mouse pointer for throwing
    if (glfwKeyPressed('M'))
    {
        g_mouse_pointer_enabled = !g_mouse_pointer_enabled;
        if (g_mouse_pointer_enabled)
        {
            glfwEnable(GLFW_MOUSE_CURSOR);
            g_object_selected = false;
            g_t_min = 0.0;
        }
        else
        {
            glfwDisable(GLFW_MOUSE_CURSOR);
            glfwGetMousePos(&g_mouse_x, &g_mouse_y);
        }
    }

#ifdef SLOW_MO
    // Increase time multiplier
    if (glfwKeyPressed('T'))
    {
        dt_multiplier += 0.1;
        if (dt_multiplier > 1.0)
        {
            dt_multiplier = 1.0;
        }
        std::cout << "Speed: " << dt_multiplier << "x\n";
    }

    // Decrease time multiplier
    if (glfwKeyPressed('G'))
    {
        dt_multiplier -= 0.1;
        if (dt_multiplier < 0.1)
        {
            dt_multiplier = 0.1;
        }
        std::cout << "Speed: " << dt_multiplier << "x\n";
    }

    // Instant slow motion
    if (glfwKeyPressed('B'))
    {
        dt_multiplier = 0.1;
        std::cout << "Speed: " << dt_multiplier << "x\n";
    }

    // Instant normal speed
    if (glfwKeyPressed('N'))
    {
        dt_multiplier = 1.0;
        std::cout << "Speed: " << dt_multiplier << "x\n";
    }
#endif

    // Add a random force to the mesh
    if (glfwKeyPressed('['))
    {
        dlib::vec3 dv;
        dv = randf(-8, 8), randf(2, 8), randf(-8, 8);
        dlib::vec3* vel = g_psystem->GetVelocities();
        size_t size = g_psystem->GetNumParticles();
        for (size_t i = 0; i < size; ++i)
        {
            vel[i] += dv;
        }
    }

    // Pause the simulation
    if (glfwKeyPressed('P'))
    {
        run_sim = !run_sim;
    }

    // Run the simulation
    if (run_sim)
    {
        integrate(dt_multiplier*dt);
    }

    return true;
}

//=============================================================================
// ray_sphere_intersect
//=============================================================================

bool ray_sphere_intersect(const glm::vec3& v1, const glm::vec3& v2,
                        const glm::vec3& center, const float radius,
                        float& t)
{
    const glm::vec3 o_to_c = center - v1;
    const glm::vec3 line_dir = glm::normalize(v2 - v1);
    const float line_length = glm::distance(v1, v2);
    t = glm::dot(o_to_c, line_dir);
    glm::vec3 closest;
    if (t <= 0.0f)
    {
        return false;
    }
    else if (t >= line_length)
    {
        closest = v2;
    }
    else
    {
        closest = v1 + line_dir*t;
    }

    return glm::distance(center, closest) <= radius;
}

//=============================================================================
// try_to_select
//=============================================================================

static bool try_to_select(PSystem& psys, const glm::vec3& v1, 
                const glm::vec3& v2, real& t_min)
{
    const size_t data_size = psys.GetNumParticles();
    const dlib::vec3* data = psys.GetPositions();
    for (size_t i = 0; i < data_size; ++i)
    {
        const dlib::vec3& p(data[i]);
        const glm::vec3 center(p(0), p(1), p(2));
        if (ray_sphere_intersect(v1, v2, center, 0.2, t_min))
        {
            return true;
        }
    }

    return false;
}

//=============================================================================
// get_mouse_attraction_force
//=============================================================================

static dlib::vec3 get_mouse_attraction_force()
{
    // Create a ray coming out of the camera
    glm::vec3 window_near(g_mouse_x, height-g_mouse_y, 0.0);
    glm::vec3 window_far(g_mouse_x, height-g_mouse_y, 1.0);
    glm::vec4 viewport(0, 0, width, height);
    glm::mat4 view = camera.GetView();
    glm::mat4 proj = camera.GetProj();
    glm::vec3 v1 = glm::unProject(window_near, view, proj, viewport);
    glm::vec3 v2 = glm::unProject(window_far, view, proj, viewport);

    // See if the ray intersects a particle
    if (!g_object_selected)
    {
        g_object_selected = try_to_select(*g_psystem, v1, v2, g_t_min);
    }

    // Return a force that points toward the mouse
    if (g_object_selected)
    {
        const glm::vec3 _dest(v1 + glm::normalize(v2 - v1)*g_t_min);
        dlib::vec3 dest;
        dest = _dest.x, _dest.y, _dest.z;
        return (dest - g_psystem->GetCOM()) * 5.0;
    }

    return dlib::zeros_matrix<real>(3L, 1L);
}

//=============================================================================
// check_for_collisions
//=============================================================================

static void check_for_collisions()
{
    // Keep the particle system contained inside a box
    dlib::vec3* particles = g_psystem->GetPositions();
    dlib::vec3* velocities = g_psystem->GetVelocities();
    size_t num_particles = g_psystem->GetNumParticles();
    for (size_t i = 0; i < num_particles; ++i)
    {
        dlib::vec3& pos(particles[i]);
        dlib::vec3& vel(velocities[i]);
        static real zero[3] = { 0, 0, 0 };
        if (pos(0) < -20) 
        { 
            real r[3] = { -20, pos(1), pos(2) };
            vel = vec3(zero);
            pos = vec3(r);
        }
        else if (pos(0) > 20)
        {
            real r[3] = { 20, pos(1), pos(2) };
            vel = vec3(zero);
            pos = vec3(r);
        }

        if (pos(1) < 0) 
        { 
            real r[3] = { pos(0), 0, pos(2) };
            vel = vec3(zero);
            pos = vec3(r);
        }
        else if (pos(1) > 20)
        {
            real r[3] = { pos(0), 20, pos(2) };
            vel = vec3(zero);
            pos = vec3(r);
        }

        if (pos(2) < -20) 
        { 
            real r[3] = { pos(0), pos(1), -20 };
            vel = vec3(zero);
            pos = vec3(r);
        }
        else if (pos(2) > 20)
        {
            real r[3] = { pos(0), pos(1), 20 };
            vel = vec3(zero);
            pos = vec3(r);
        }
    }
}

//=============================================================================
// integrate
//=============================================================================

static void integrate(double dt)
{
    dlib::vec3 force = g_gravity;

    // Add a force to pull the object towards the mouse
    if (g_mouse_pointer_enabled && g_mouse_down)
    {
        force += get_mouse_attraction_force();
    }

    check_for_collisions();

    g_psystem->Update(dt, force);

    check_for_collisions();
}

//=============================================================================
// render
//=============================================================================

void render()
{
    g_psystem->EndUpdate();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // All vertices are already in world coordinates, don't need model matrix
    ground_shader.Bind();
    glUniformMatrix4fv(ground_shader["proj"], 1, GL_FALSE, glm::value_ptr(camera.GetProj()));
    glUniformMatrix4fv(ground_shader["view"], 1, GL_FALSE, glm::value_ptr(camera.GetView()));
    g_ground_mesh.Render();

    object_shader.Bind();
    glUniformMatrix4fv(object_shader["proj"], 1, GL_FALSE, glm::value_ptr(camera.GetProj()));
    glUniformMatrix4fv(object_shader["view"], 1, GL_FALSE, glm::value_ptr(camera.GetView()));
    g_psystem->Render();
}

//=============================================================================
// 
//=============================================================================
