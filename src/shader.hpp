/* Shader.hpp
 *
 * Provides a utility class that handles create GLSL programs
 * from shader source files. Specify the two source filenames
 * in the constructor and then check isValid() before using
 * the shader class.
 */

#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include <GL/gl.h>

#include <map>
#include <string>

class Shader
{
public:
    /* Construct a new shader object. It will create a new OpenGL program
     * object using the contents of the vertex and fragment shader files
     * provided as arguments.
     *
     * Params:
     *   vert_file - Path to the vertex shader source file.
     *   frag_file - Path to the fragment shader source file.
     *   geom_file - An optional geometry shader source file.
     *
     * Errors will automatically be printed to standard error if any
     * problems were encountered.
     *
     * Before using this object call isValid() and make sure it returns
     * true. If it does not then something went wrong compiling the
     * shaders or linking the final program.
     */
    Shader(const std::string& vert_file, 
           const std::string& frag_file,
           const std::string& geom_file = "");

    /* Allows creation of a shader object without any loaded OpenGL shaders.
     * Before using LoadShaders() must be called. It is recommended to call
     * IsValid() before using this shader to make sure everything is okay.
     */
    Shader();

    /* Destroy the shader object. This will delete the OpenGL  program 
     * object as well.
     */
    ~Shader();

    /* Creates an OpenGL program object. It stores the result in m_program.
     *
     * Params:
     *   vert_file - The path to the vertex shader source file.
     *   frag_file - The path to the fragment shader source file.
     *   geom_file - The path to the geometry shader source file (or "" if none)
     *
     * Returns:
     *   True if the OpenGL program was successfully created, false otherwise.
     *
     *   If any problems are encountered during the OpenGL program creation
     *   they are printed to standard error and m_program is set to 0.
     */
    bool LoadShaders(const std::string& vert_file,
                     const std::string& frag_file,
                     const std::string& geom_file = "");

    /* Get the location of the uniform variable for this shader program.
     * It caches the look ups so calling this method multiple times isn't
     * as expensive as calling glGetUniformLocation(). Upon loading a new
     * program all cached references are cleared.
     *
     * Params:
     *   name - The name of the uniform variable to lookup.
     *
     * Returns:
     *   The location of the uniform or a negative value indicating the
     *   specified uniform was not found. Also prints a message to
     *   standard error if the specified uniform could not be found.
     */
    GLint GetUniformLocation(const std::string& name);

    /* Check if the shader program was successfully created.
     *
     * Returns:
     *   True if everything worked, false if something went wrong.
     */
    bool IsValid() const;

    /* Wrapper for glUseProgram(m_program).
     */
    void Bind() const;

    /* Wrapper for glUseProgram(0).
     */
    void Unbind() const;

    /* Overloads the index operator to make accessing uniforms easier. This is
     * a wrapper around GetUniformLocation().
     */
    inline GLint operator[](const std::string& uniform) 
    {
        return GetUniformLocation(uniform);
    }

    /* Returns the OpenGL program object. Use in the call to glUseProgram().
     * If unable to successfully create a program object this method will
     * return 0.
     */
    GLuint GetProgram() const;

// Private class functions
private: 

    /* Helper function that creates a shader object given a shader type and
     * a path to a shader file.
     *
     * Params:
     *   type     - The type of shader, must be GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
     *              or GL_GEOMETRY_SHADER
     *   filename - The path to the shader's source file.
     *   out_var  - An out parameter. Upon successful creation it is set to the
     *              OpenGL shader object's handle. Upon failure it has the value 0.
     *
     * Returns:
     *   True if successfully created the shader object. False otherwise.
     *
     *   All problems encountered during creation are printed to standard error.
     */
    static bool create_shader(GLenum type, 
                const std::string& filename, GLuint& out_var);

// Class Variables
private: 
    GLuint m_program; // The OpenGL program name
    // Caches uniform location lookups
    std::map<const std::string, GLint> m_uniforms;
};

#endif
