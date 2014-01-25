#ifndef __PSYSTEM_HPP__
#define __PSYSTEM_HPP__

#include "defs.hpp"
#include "mesh.hpp"

#include <map>
#include <vector>

/* Used by the std::map in PSystem::initialize() for finding duplicate
 * vertices.
 */
class Vec3Less
{
public:
    bool operator()(const dlib::vec3& v1, const dlib::vec3& v2) const
    {
        // Need to satisfy strict weak ordering for C++ containers.
        // Start by comparing the first elements, then second, then third...
        for (int i = 0; i < 3; ++i)
        {
            if (v1(i) < v2(i)) return true;
            if (v2(i) < v1(i)) return false;
        }

        // They're equal
        return false;
    }
};

/* Straight forward implementation of the paper
 *     'Meshless Deformations Based on Shape Matching' 
 * http://dl.acm.org/citation.cfm?id=1073216
 */
class PSystem
{
public:
    /* Mesh must stay allocated for at least as long as this object.
     */
    PSystem(Mesh& mesh);

    /* Cleans up all allocations.
     */
    ~PSystem();

    /* Performs the integration step. Does not upload the data to
     * the video card, must call EndUpdate() before rendering.
     *
     * Params:
     *   dt - timestep in seconds (usually 0.016)
     *   force - Any forces accumulated, like gravity
     */
    void Update(real dt, const dlib::vec3& force);

    /* Update mesh data and update the OpenGL buffer object.
     */
    void EndUpdate();

    /* Calls the internal mesh's render method.
     */
    void Render();

    /* Get the positions array for modifying, usually for collision
     * detection. Use GetNumParticles() to get the length.
     */
    dlib::vec3* GetPositions()
    {
        return m_current_pos;
    }

    /* Get the velocities array for modifying. Use GetNumParticles()
     * to get the length.
     */
    dlib::vec3* GetVelocities()
    {
        return m_current_vel;
    }

    /* Returns the current center of mass of the particle system.
     */
    dlib::vec3 GetCOM() const
    {
        return m_current_com;
    }

    /* Return the number of particles in the system.
     */
    size_t GetNumParticles() const
    {
        return m_data_length;
    }

    real GetAlpha()
    {
        return m_alpha;
    }

    /* The alpha value partially controls the stiffness, valid range
     * is 0.0 - 1.0. Controls how quickly particles go back to their
     * equilibrium position. Lower values create a 'goopier' system
     * and higher values a more rigid system.
     */
    void SetAlpha(real alpha)
    {
        if (alpha > 1.0) m_alpha = 1.0;
        else if (alpha < 0.0) m_alpha = 0.0;
        else m_alpha = alpha;
    }

    real GetBeta()
    {
        return m_beta;
    }

    /* The beta value is an interpolation parameter between the
     * undeformed shape and the deformed shape. This controls
     * where the particles go back to. Lower values make the
     * particle system act 'squishier' and higher values make
     * the system stiffer.
     */
    void SetBeta(real beta)
    {
        if (beta > 1.0) m_beta = 1.0;
        else if (beta < 0.0) m_beta = 0.0;
        else m_beta = beta;
    }

    /* Reset the particle system to its initial state.
     */
    void Reset();

private:
    /* Perform all one time setup and allocations.
     */
    void initialize();

    /* Helper for calculating the center of mass.
     */
    dlib::vec3 calc_com(const dlib::vec3* data);

    /* Helper for calculating the relative positions of the particles from
     * their original positions.
     */
    void calc_rel_pos(const dlib::vec3* pos, dlib::vec3* rel_pos, dlib::vec3 com);

private:
    Mesh& m_mesh; // Underlying mesh that this particles system is based on
    real m_alpha; // Alpha parameter (explained above in SetAlpha())
    real m_beta; // Beta parameter (explained above in SetBeta())
    size_t m_data_length; // The number of particles
    std::vector<std::vector<int> > m_vec_to_index; // Mapping of particles to mesh indices
    dlib::vec3 m_current_com; // Current particle system center of mass
    dlib::vec3 m_initial_com; // Initial particle system center of mass

    dlib::vec3* m_current_vel; // Array of each particles current velocity
    dlib::vec3* m_current_pos; // Array of each particles position
    dlib::vec3* m_current_rel; // Array of cur_pos - cur_COM
    dlib::vec3* m_old_pos; // Temporary array used during Update()

    dlib::vec3 const* m_initial_pos; // Array of initial particle positions
    dlib::vec3 const* m_initial_rel; // Array of init_pos - init_COM

    // These matrices follow the paper
    dlib::mat3x9 mat_Apq_tilde; // Stores the Apq~ matrix
    dlib::mat3x9 mat_A_tilde; // Stores the A matrix
    dlib::mat9x1 const* m_q_tilde; // Stores q~ array, calculated once
    dlib::mat9x9 mat_Aqq_tilde; // Stores the Aqq~ matrix, calculated once

    // Matrices from the paper
    dlib::mat3 mat_Apq; // Apq matrix
    dlib::mat3 mat_Aqq; // Aqq matrix, calculated once
    dlib::mat3 mat_A; // A matrix
    dlib::mat3 mat_R; // R matrix, rotation matrix
};

#endif
