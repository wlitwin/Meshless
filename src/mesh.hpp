#ifndef __MESH_HPP__
#define __MESH_HPP__

#include "defs.hpp"

#include <vector>
#include <GL/gl.h>

/* Represents a simple 3D mesh. This class provides a simple wrapper around the
 * creation of OpenGL vertex array objects. This makes it easier to create or
 * load geometry and use it in an OpenGL 3.x+ program.
 *
 * This class provides 3 values to a vertex shader
 *   0 - dlib::vec3 - Vertex position
 *   1 - dlib::vec3 - Normal for the vertex
 *   2 - dlib::vec2 - Texture coordinate of the vertex
 *
 * In the vertex shader the 'layout(location = x)' format should be used to make
 * sure the variable match up with the above buffer locations.
 *
 * All triangles/quads must be specified using counter-clockwise rotation in order
 * for OpenGL to render them when glCullFace(GL_BACK) is enabled.
 * 
 * The basic format for use is:
 *
 *   Mesh myMesh;
 *   myMesh.NewMesh();
 *
 *   myMesh.AddTriangle(...);
 *   myMesh.AddQuad(...);
 *   ...
 *
 *   myMesh.Finish();
 *
 *   ...
 *   
 *   // To Render
 *   myMesh.Render();
 */
class Mesh
{
public:
    /* Constructor, doesn't do much besides small initialization
     */
    Mesh();

    /* Cleans up any allocated space (including OpenGL buffers)
     */
    virtual ~Mesh();

    /* Create a new mesh. If there was a mesh already created those resources
     * are freed. This method must be called before adding geometry to the mesh.
     */
    void NewMesh();

    /* Add a triangle to the mesh given the three vertices of the triangle. Vertices
     * should be specified in counter-clockwise order in order to prevent OpenGL
     * from culling them when glCullFace(GL_BACK) is enabled. Also allows each vertex
     * to have texture coordinates specified.
     *
     * Params:
     *   p1 - The first vertex
     *   t1 - The first vertex's texture coordinates
     *   n1 - The first vertex's normal vector
     *   p2 - The second vertex
     *   t2 - The second vertex's texture coordinates
     *   n2 - The second vertex's normal vector
     *   p3 - The third vertex
     *   t3 - The third vertex's texture coordinates
     *   n3 - The third vertex's normal vector
     *
     * Note: Must have called NewMesh() at some point before this method.
     */
    void AddTriangle(const dlib::vec3& p1, const dlib::vec3& p2, const dlib::vec3& p3);

    void AddTriangle(const dlib::vec3& p1, const dlib::vec2& t1,
                     const dlib::vec3& p2, const dlib::vec2& t2,
                     const dlib::vec3& p3, const dlib::vec2& t3);

    void AddTriangle(const dlib::vec3& p1, const dlib::vec2& t1, const dlib::vec3& n1,
                     const dlib::vec3& p2, const dlib::vec2& t2, const dlib::vec3& n2,
                     const dlib::vec3& p3, const dlib::vec2& t3, const dlib::vec3& n3);

    void AddTriangle(const dlib::vec3& p1, const dlib::vec3& n1,
                     const dlib::vec3& p2, const dlib::vec3& n2,
                     const dlib::vec3& p3, const dlib::vec3& n3);

    /* Add a quad to the mesh. These four points are assumed to lie on a plane.
     * The points should be specified in counter-clockwise order in order to
     * prevent OpenGL from culling the resulting triangles when glCullFace(GL_BACK)
     * is enabled.
     *
     * Params:
     *   p1 - The vertex of the first corner of the quad
     *   t1 - The texture coordinate of the first vertex
     *   n1 - The normal vector of the first vertex
     *   p2 - The vertex of the second corner of the quad
     *   t2 - The texture coordinate of the second vertex
     *   n2 - The normal vector of the second vertex
     *   p3 - The vertex of the third corner of the quad
     *   t3 - The texture coordinate of the third vertex
     *   n3 - The normal vector of the third vertex
     *   p4 - The vertex of the fourth corner of the quad
     *   t4 - The texture coordinate of the fourth vertex
     *   n4 - The normal vector of the fourth vertex
     *
     * Note: The quad is actually tesselated into two triangles of the form:
     *         Triangle 1 - p1, p2, p4
     *         Triangle 2 - p2, p3, p4
     */
    void AddQuad(const dlib::vec3& p1, const dlib::vec3& p2, 
                 const dlib::vec3& p3, const dlib::vec3& p4);

    void AddQuad(const dlib::vec3& p1, const dlib::vec2& t1,
                 const dlib::vec3& p2, const dlib::vec2& t2,
                 const dlib::vec3& p3, const dlib::vec2& t3,
                 const dlib::vec3& p4, const dlib::vec2& t4);

    void AddQuad(const dlib::vec3& p1, const dlib::vec2& t1, const dlib::vec3& n1,
                 const dlib::vec3& p2, const dlib::vec2& t2, const dlib::vec3& n2,
                 const dlib::vec3& p3, const dlib::vec2& t3, const dlib::vec3& n3,
                 const dlib::vec3& p4, const dlib::vec2& t4, const dlib::vec3& n4);

    void AddQuad(const dlib::vec3& p1, const dlib::vec3& n1,
                 const dlib::vec3& p2, const dlib::vec3& n2, 
                 const dlib::vec3& p3, const dlib::vec3& n3,
                 const dlib::vec3& p4, const dlib::vec3& n4);

    /* Add a point to the mesh. Is only valid if the current type is GL_POINTS
     */
    void AddPoint(const dlib::vec3& p);

    void AddPoint(const dlib::vec3& p, const dlib::vec3& norm);

    void AddPoint(const dlib::vec3& p, const dlib::vec3& norm, const dlib::vec2& texCoord);

    /* Finishes creation of the mesh. Any changes after this method will not be saved.
     * In fact none of the Add* methods should be called after this method. To start
     * creation of a new mesh the 'NewMesh()' method should be called, and then the Add*
     * may be called again.
     *
     * After this method has been called the mesh has been setup for rendering with OpenGL
     * and the Render() method may be called.
     */
    virtual void Finish();

    /* Renders the mesh using glDrawArrays(). All of the shaders and other parameters should
     * be set prior to calling this method.
     *
     * The vertex attribute arrays are setup as follows:
     *   0 - dlib::vec3 - The vertex
     *   1 - dlib::vec3 - The normal
     *   2 - dlib::vec2 - The texture coordinate
     */
    void Render();

    /* Change what type of primitives this mesh holds.
     *
     * Params:
     *   mode - What type of primitives this mesh holds
     */
    void SetPrimitiveType(GLenum mode);

    /* Flags for the data included in the buffer.
     */
    static unsigned char VERTICES;
    static unsigned char NORMALS;
    static unsigned char TEXCOORDS;

    /* Change what the generated vertex buffer object contains.
     *
     * By default it contains all three types of data. The different
     * methods for adding data have asserts that check to make sure
     * the correct flags are set.
     */
    void SetIncludedData(unsigned char flags);

    /* Get the current buffer configuration.
     */
    unsigned char GetIncludedData();

    /* Test whether a combination of flags is enabled.
     */
    bool IsFlagEnabled(unsigned char flag)
    {
        return (m_data_types & flag) > 0;
    }

    /* Access the data directly.
     */
    real* GetData()
    {
        return &m_mesh[0];
    }

    /* Get the number of elements in the buffer, these are of
     * real type. NOT the bytes.
     */
    size_t GetDataSize()
    {
        return m_mesh.size();
    }

    /* Upload the current data to the video card.
     */
    void UpdateData();

private:
    /* Resets the mesh to a clean state.
     */
    void cleanup();
    
protected:
    GLuint m_vao; // Vertex array object
    GLuint m_vbo; // Vertex buffer object
    std::vector<real> m_mesh; // The vertices/normals/texture coords
    GLenum m_primitiveType; // The type passed to glDrawArrays()
    unsigned char m_data_types; // Stores the current flags
};

#endif
