#include "shader.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>

using std::string;
using std::fstream;

//=============================================================================
// Constructor(string, string)
//=============================================================================

Shader::Shader(const string& vert_file, 
               const string& frag_file,
               const string& geom_file) :
    m_program(0)
{
    LoadShaders(vert_file, frag_file, geom_file);
}

//=============================================================================
// Constructor()
//=============================================================================

Shader::Shader() :
    m_program(0)
{ }

//=============================================================================
// Destructor
//=============================================================================

Shader::~Shader()
{
    if (m_program != 0)
    {
        glDeleteProgram(m_program);
    }
}

//=============================================================================
// isValid
//=============================================================================

bool Shader::IsValid() const
{
    return m_program > 0;
}

//=============================================================================
// Bind
//=============================================================================

void Shader::Bind() const
{
    assert(IsValid());

    glUseProgram(m_program);
}

//=============================================================================
// Unbind
//=============================================================================

void Shader::Unbind() const
{
    glUseProgram(0);
}

//=============================================================================
// getProgram
//=============================================================================

GLuint Shader::GetProgram() const
{
    return m_program;
}

//=============================================================================
// GetUniformLocation
//=============================================================================

GLint Shader::GetUniformLocation(const std::string& name)
{
    if (!IsValid()) 
    {
        return -1;
    }

    // m_program is guarenteed to be valid after this point
    // (Unless someone called glDeleteProgram(shader.getProgram()),
    //  but we assume that hasn't happend)
    if (m_uniforms.find(name) == m_uniforms.end())
    {
        // Try too look it up
        GLint loc = glGetUniformLocation(m_program, name.c_str());
        if (loc >= 0) 
        {
            m_uniforms[name] = loc;
        } 
        else 
        {
            std::cerr << "Warning: " << name << " uniform does not exist\n";
        }

        return loc;
    } 
    else 
    {
        return m_uniforms[name];
    }
}

//=============================================================================
// LoadShaders
//=============================================================================

bool Shader::LoadShaders(const string& vert_file, 
                         const string& frag_file,
                         const string& geom_file)
{
    if (m_program != 0)
    {
        // Clear the uniforms as well
        m_uniforms.erase(m_uniforms.begin(), m_uniforms.end());
        glDeleteProgram(m_program);
        m_program = 0;
    }

    GLuint vert_shader = 0;
    if (!create_shader(GL_VERTEX_SHADER, vert_file, vert_shader))
    {
        return false;
    }

    GLuint geom_shader = 0;
    if (geom_file != "" 
        && !create_shader(GL_GEOMETRY_SHADER, geom_file, geom_shader))
    {
        // Cleanup vertex shader object
        glDeleteShader(vert_shader);
        return false;
    }

    GLuint frag_shader = 0;
    if (!create_shader(GL_FRAGMENT_SHADER, frag_file, frag_shader))
    {
        // Cleanup the vertex shader object & geometry shader object
        glDeleteShader(vert_shader);
        if (geom_shader != 0)
        {
            glDeleteShader(geom_shader);
        }
        return false;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vert_shader);
    // Check for optional geometry shader
    if (geom_shader != 0)
    {
        glAttachShader(m_program, geom_shader);
    }
    glAttachShader(m_program, frag_shader);

    glLinkProgram(m_program);

    GLint status;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint log_length;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &log_length);

        GLchar* log = new GLchar[log_length+1];
        glGetProgramInfoLog(m_program, log_length, NULL, log);
        std::cerr << "Failed to link program:\n"
            << vert_file << "\n" 
            << frag_file << "\n"
            << log << "\n";
        delete [] log;

        // Cleanup the program and shader variables
        glDetachShader(m_program, vert_shader);
        if (geom_shader != 0)
        {
            glDetachShader(m_program, geom_shader);
        }
        glDetachShader(m_program, frag_shader);
        glDeleteProgram(m_program);
        m_program = 0;
    } 

    // The shader objects are no longer needed after they
    // have been compiled into the program.
    glDeleteShader(vert_shader);
    if (geom_shader != 0)
    {
        glDeleteShader(geom_shader);
    }
    glDeleteShader(frag_shader);
    return status != GL_FALSE && m_program > 0;
}

//=============================================================================
// create_shader
//=============================================================================

bool Shader::create_shader(GLenum type, const string& filename, GLuint& out_var)
{
    // Clear the output variable just in case this doesn't work
    out_var = 0;

    fstream file(filename.c_str(), std::ios::in);
    string source;
    
    // See if we could open the file
    if (!file.is_open())
    {
        std::cerr << "Failed to open " << filename << "\n";
        return false;
    }

    // Read in the source
    std::stringstream ss;
    ss << file.rdbuf();
    source = ss.str();

    // Create the shader object after successfully reading the shader source    
    GLuint shader = glCreateShader(type);   

    const char* c_source = source.c_str();
    glShaderSource(shader, 1, &c_source, NULL);

    glCompileShader(shader);

    // Check if compilation worked
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        GLchar* log = new GLchar[log_length+1];
        glGetShaderInfoLog(shader, log_length, NULL, log);

        std::cerr << "Failed to compile shader:\n"
            << filename << "\n"
            << log << "\n";

        delete [] log;

        glDeleteShader(shader);
        return false;
    }

    // Set the output variable appropriately
    out_var = shader;
    return true;
}

//=============================================================================
//
//=============================================================================
