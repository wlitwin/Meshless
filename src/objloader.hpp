#ifndef __OBJ_LOADER_HPP__
#define __OBJ_LOADER_HPP__

#include "defs.hpp"
#include "mesh.hpp"

#include <string>
#include <vector>

/* Stores 0-based indexes in each of the arrays.
 *
 * vert -> m_verts
 * norm -> m_norms
 * tex  -> m_texs
 */
struct Face
{
    union {
        struct {
            int vert[3];
            int norm[3];
            int tex[3];
        } val;
        int all[3][3];
    };

    Face() 
    {
        for (int i = 0; i < 3; ++i)
        {
            val.vert[i] = -1;
            val.norm[i] = -1;
            val.tex[i]  = -1;
        }
    }
};

class ObjLoader
{
public:
    /* Doesn't do much */
    ObjLoader();

    /* Load data from an OBJ file.
     *
     * Fills in the member vectors with the data
     * from the file. Ignores everything but v, vt,
     * vn, and f lines. Also only reads three 
     * values and doesn't work with quads.
     *
     * Returns:
     *   True if the file exists and is well formed,
     *   false otherwise.
     */
    bool LoadFile(const std::string& file);

    /* Fill in a mesh object with the current data.
     *
     * This will erase the current mesh and build a new one.
     *
     * Returns:
     *   True if successfully created the mesh, false if
     *   no data to fill or other issue.
     */
    bool ToMesh(Mesh& mesh, unsigned char flags);

    /* Load data from an obj stream.
     *
     * Returns:
     *   True if the stream contained well formatted
     *   data and false otherwise.
     */
    bool Load(std::istream& stream);

protected:
    
    /* Clear all data buffers.
     */
    void cleanup(void);

private:
    // True if the data in the vectors is valid.
    bool m_is_loaded;
    // Stores the read in vertices, normals and texture
    // coordinates from the obj file.
    std::vector<Face> m_faces;
    std::vector<dlib::vec3> m_verts;
    std::vector<dlib::vec3> m_norms;
    std::vector<dlib::vec2> m_texs;
};

#endif
