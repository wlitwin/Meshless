#include "psystem.hpp"

#include <cstring>
#include <cassert>

// Get rid of some boiler plate when freeing arrays
#define FREE_ARRAY(X) if ((X) != NULL) { delete [] (X); (X) = NULL; }

//=============================================================================
// Constructor
//=============================================================================

PSystem::PSystem(Mesh& mesh) :
    m_mesh(mesh),
    m_alpha(0.4),
    m_beta(0.7)
{
    // We only want to deal with vertex meshes
    assert(mesh.GetIncludedData() == Mesh::VERTICES);

    initialize();
}

//=============================================================================
// Destructor
//=============================================================================

PSystem::~PSystem()
{
    // Defined above
    FREE_ARRAY(m_initial_pos);
    FREE_ARRAY(m_initial_rel);
    FREE_ARRAY(m_current_pos);
    FREE_ARRAY(m_current_rel);
    FREE_ARRAY(m_current_vel);
    FREE_ARRAY(m_q_tilde);
    FREE_ARRAY(m_old_pos);
}

//=============================================================================
// initialize
//=============================================================================

void PSystem::initialize()
{
    // Calculate how much space we need, we won't use the number
    // of vertices in the mesh because there are many duplicate
    // values. We need to figure out how many unique vertices
    // there are, and the mapping to each duplicated vertex in
    // the original mesh.
    
    // This map stores all the unique vectors and their corresponding
    // index in the m_mesh data array, so they can be copied over after
    // calculating the new positions.
    std::map<dlib::vec3, std::vector<int>, Vec3Less> vec_to_index_map;
    const dlib::vec3* data = reinterpret_cast<dlib::vec3*>(m_mesh.GetData());
    const size_t data_size = (m_mesh.GetDataSize()*sizeof(real)) / sizeof(dlib::vec3);
    for (size_t i = 0; i < data_size; ++i)
    {
        const dlib::vec3& val(data[i]);
        if (vec_to_index_map.find(val) == vec_to_index_map.end())
        {
            std::vector<int> duplicates;
            duplicates.push_back(i);
            vec_to_index_map[val] = duplicates;
        }
        else
        {
            vec_to_index_map[val].push_back(i);
        }
    }
    m_data_length = vec_to_index_map.size();

    // We need to make our own buffer because we have fewer
    // particles than vertices. They are the keys in the
    // m_vec_to_index map.
    int i = 0;
    dlib::vec3* temp = new dlib::vec3[m_data_length];
    std::map<dlib::vec3, std::vector<int>, Vec3Less>::const_iterator iter;
    for (iter = vec_to_index_map.begin(); iter != vec_to_index_map.end(); ++iter)
    {
        temp[i] = iter->first;
        m_vec_to_index.push_back(iter->second);
        ++i;
    }
    m_initial_pos = temp;

    // Temporary space for use during the Update() method.
    m_old_pos = new dlib::vec3[m_data_length];

    // Velocity space
    m_current_vel = new dlib::vec3[m_data_length];

    // Current position space
    m_current_pos = new dlib::vec3[m_data_length];

    // Calculate the initial center of mass
    m_initial_com = calc_com(m_initial_pos);

    // Calculate the relative positions
    temp = new dlib::vec3[m_data_length];   
    calc_rel_pos(m_initial_pos, temp, m_initial_com);
    m_initial_rel = temp;

    // Space the the q_i values
    m_current_rel = new dlib::vec3[m_data_length];

    // Make the q~ matrix for quadratic deformation, we only
    // have to do this once
    dlib::mat9x1* mat_temp = new dlib::mat9x1[m_data_length];
    for (size_t i = 0; i < m_data_length; ++i)
    {
        const dlib::vec3& q(m_initial_rel[i]);
        const real q_x = q(0);
        const real q_y = q(1);
        const real q_z = q(2);
        mat_temp[i] = q_x, q_y, q_z,
                q_x*q_x, q_y*q_y, q_z*q_z,
                q_x*q_y, q_y*q_z, q_z*q_x;
    }
    m_q_tilde = mat_temp;

    // Calculate the A_qq matrix, this only has to be done once
    mat_Aqq = dlib::zeros_matrix<real>(3L, 3L);
    for (size_t i = 0; i < m_data_length; ++i)
    {
        // A_qq += q_i * q_i^T
        mat_Aqq += m_initial_rel[i] * dlib::trans(m_initial_rel[i]);
    }
    mat_Aqq = dlib::inv(mat_Aqq);

    // Also calculate A_qq~
    mat_Aqq_tilde = dlib::zeros_matrix<real>(9L, 9L);
    for (size_t i = 0; i < m_data_length; ++i)
    {
        mat_Aqq_tilde += m_q_tilde[i] * dlib::trans(m_q_tilde[i]);
    }
    mat_Aqq_tilde = dlib::inv(mat_Aqq_tilde);

    // Perform the rest of the initialization
    Reset();
}

//=============================================================================
// Reset
//=============================================================================

void PSystem::Reset()
{
    memset(m_current_vel, 0, m_data_length*sizeof(dlib::vec3));
    memcpy(m_current_pos, m_initial_pos, m_data_length*sizeof(dlib::vec3)); 
    memcpy(m_current_rel, m_initial_rel, m_data_length*sizeof(dlib::vec3));
    m_current_com = m_initial_com;
}

//=============================================================================
// calc_com
//=============================================================================

dlib::vec3 PSystem::calc_com(const dlib::vec3* data)
{
    dlib::vec3 pos_sum;
    pos_sum = 0, 0, 0;

    for (size_t i = 0; i < m_data_length; ++i)
    {
        pos_sum += data[i];
    }

    pos_sum /= static_cast<real>(m_data_length);

    return pos_sum;
}

//=============================================================================
// calc_rel_pos
//=============================================================================

void PSystem::calc_rel_pos(const dlib::vec3* pos, dlib::vec3* rel_pos, dlib::vec3 com)
{
    for (size_t i = 0; i < m_data_length; ++i)
    {
        rel_pos[i] = pos[i] - com;
    }
}

//=============================================================================
// Update
//=============================================================================

/* Helper method for Update(), calculates num^(1.0/exp).
 */
static real root(real num, real exp)
{
    if (num < 0)
    {
        return -std::pow(-num, 1.0/exp);
    }
    else if (num == 0)
    {
        return 1.0;
    }
    else
    {
        return std::pow(num, 1.0/exp);
    }
}

// Probably not necessary
static void verlet(dlib::vec3& pos, dlib::vec3& vel, 
		const dlib::vec3 force, const real dt)
{
	const dlib::vec3 old_vel = vel;
	vel = vel + force * dt;
	pos = pos + (old_vel + vel) * 0.5 * dt;
}

void PSystem::Update(real dt, const dlib::vec3& force)
{
    // Do a partial integration
    const dlib::vec3 force_dt = force * dt;
    for (size_t i = 0; i < m_data_length; ++i)
    {
        m_old_pos[i] = m_current_pos[i];
        //m_current_vel[i] += force_dt;
        //m_current_pos[i] += dt * m_current_vel[i];
		verlet(m_current_pos[i], m_current_vel[i], force, dt);
    }

    // Update the center of mass and relative positions
    m_current_com = calc_com(m_current_pos);
    calc_rel_pos(m_current_pos, m_current_rel, m_current_com);

    // Calculate the A_pq matrix
    mat_Apq = dlib::zeros_matrix<real>(3L, 3L);
    for (size_t i = 0; i < m_data_length; ++i)
    {
        mat_Apq += m_current_rel[i] * dlib::trans(m_initial_rel[i]);
    }

    mat_A = mat_Apq * mat_Aqq;
    mat_A *= (1.0 / root(dlib::det(mat_A), 3.0));

    // Calculate the R matrix
    dlib::mat3 mat_S = dlib::sqrt_db(dlib::trans(mat_Apq) * mat_Apq);
    mat_R = mat_Apq * dlib::inv(mat_S);

    // Calculate R~, a 3x9 matrix with [R 0 0]
    dlib::mat3x9 mat_R_tilde;
    mat_R_tilde = mat_R(0,0), mat_R(0,1), mat_R(0,2), 0, 0, 0, 0, 0, 0,
                  mat_R(1,0), mat_R(1,1), mat_R(1,2), 0, 0, 0, 0, 0, 0,
                  mat_R(2,0), mat_R(2,1), mat_R(2,2), 0, 0, 0, 0, 0, 0;

    // Calculate Apq~
    mat_Apq_tilde = dlib::zeros_matrix<real>(3L, 9L);
    for (size_t i = 0; i < m_data_length; ++i)
    {
        mat_Apq_tilde += m_current_rel[i] * dlib::trans(m_q_tilde[i]);
    }

    // Calculate A~
    mat_A_tilde = mat_Apq_tilde * mat_Aqq_tilde;

    // Fix the A~ matrix by doing some volume preservation
    dlib::mat9x9 mat_A_tilde_sq = dlib::identity_matrix<real>(9L);
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 9; ++c)
        {
            mat_A_tilde_sq(r, c) = mat_A_tilde(r, c);
        }
    }
    mat_A_tilde *= (1.0 / root(dlib::det(mat_A_tilde_sq), 9.0));

    // Finish the integration
    const dlib::mat3x9 mat_goal = (m_beta*mat_A_tilde) + ((1.0 - m_beta)*mat_R_tilde);
	const real dt_inv = 1.0 / dt;

#ifdef SLOW_MO
	// Slow motion doesn't work all that great, but this is what the paper
	// suggested for varying time steps, it doesn't work all that well.
	const real alpha_term = m_alpha * (dt / 0.01);
#else
	const real alpha_term = m_alpha;
#endif

    for (size_t i = 0; i < m_data_length; ++i)
    {
        const dlib::vec3 goal = (mat_goal*m_q_tilde[i]) + m_current_com;
        const dlib::vec3 alpha = alpha_term * (goal - m_current_pos[i]) * dt_inv;

        m_current_vel[i] += alpha;
        m_current_pos[i] = m_old_pos[i] + dt*m_current_vel[i];
    }
}

//=============================================================================
// EndUpdate
//=============================================================================

void PSystem::EndUpdate()
{
    // Update the mesh by copying over the new positions using the
    // duplicate mappings in m_vec_to_index.
    dlib::vec3* data = reinterpret_cast<dlib::vec3*>(m_mesh.GetData()); 
    for (size_t i = 0; i < m_vec_to_index.size(); ++i)
    {
        const std::vector<int>& duplicates(m_vec_to_index[i]);
        for (size_t j = 0; j < duplicates.size(); ++j)
        {
            data[duplicates[j]] = m_current_pos[i];
        }
    }

    // Upload the new positions to the video card.
    m_mesh.UpdateData();
}

//=============================================================================
// Render
//=============================================================================

void PSystem::Render()
{
    m_mesh.Render();
}

//=============================================================================
//
//=============================================================================
