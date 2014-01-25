#include "objloader.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

//=============================================================================
// Constructor
//=============================================================================

ObjLoader::ObjLoader() :
    m_is_loaded(false),
    m_faces(),
    m_verts(),
    m_norms(),
    m_texs()
{ }

//=============================================================================
// LoadFile
//=============================================================================

bool ObjLoader::LoadFile(const std::string& file)
{
    std::ifstream in(file.c_str());
    if (!in.is_open())
    {
        return false;
    }

    bool valid = Load(in);  

    in.close();

    return valid;
}

//=============================================================================
// cleanup
//=============================================================================

void ObjLoader::cleanup()
{
    m_faces.clear();
    m_verts.clear();
    m_norms.clear();
    m_texs.clear();

    m_is_loaded = false;
}

//=============================================================================
// starts_with
//=============================================================================

static bool starts_with(const std::string str, const std::string other)
{
    if (str.length() < other.length())
    {
        return false;
    }

    for (size_t i = 0; i < other.length(); ++i)
    {
        if (str[i] != other[i]) 
        {
            return false;
        }
    }

    return true;
}

//=============================================================================
// split
//=============================================================================

static std::vector<std::string> split(const std::string& str, char delim)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string tok;
    while (std::getline(ss, tok, delim))
    {
        tokens.push_back(tok);
    }

    return tokens;
}

//=============================================================================
// Load 
//=============================================================================

bool ObjLoader::Load(std::istream& stream)
{
    using namespace std;

    cleanup();

    string line;
    vector<string> tokens;
    // We ignore the optional fourth parameter
    real r[3];
    real t[2];
    int line_no = 0;
    while (stream.good() && !stream.eof())
    {
        ++line_no;
        getline(stream, line);

        // Skip empty lines and comments
        if (line.empty() || starts_with(line, "#"))
        {
            continue;
        }

        // Should be a useful line
        stringstream ss(line);
        string type;
        ss >> type;

        // Check what type of line it is
        if (type == "v")
        {
            ss >> r[0]; ss >> r[1]; ss >> r[2]; 
            m_verts.push_back(dlib::vec3(r));
        }
        else if (type == "vt")
        {
            ss >> t[0]; ss >> t[1];
            m_texs.push_back(dlib::vec2(t));
        }
        else if (type == "vn")
        {
            ss >> r[0]; ss >> r[1]; ss >> r[2]; 
            m_norms.push_back(dlib::vec3(r));
        }
        else if (type == "f")
        {
            string fdefs[3];    
            ss >> fdefs[0]; ss >> fdefs[1]; ss >> fdefs[2];
            if (!ss) { break; }

            // Face data is specified by
            // vertex/texture/normal
            // where texture and normal are optional

            Face data;
            for (int i = 0; i < 3; ++i)
            {
                vector<string> toks(split(fdefs[i], '/'));
                if (toks.size() < 1)
                {
                    cerr << "Invalid face data:" << line_no << endl;
                    cleanup();
                    return false;
                }


                // Check the fields
                for (size_t j = 0; j < 3 && j < toks.size(); ++j)
                {
                    if (toks[j].empty())
                    {
                        continue;
                    }

                    int index = 0;
                    stringstream ss(toks[j]);
                    ss >> index;

                    if (!ss || index < 1) 
                    {
                        cerr << "Invalid index:" << line_no << endl;
                        cleanup();
                        return false;
                    }

                    --index;
                    data.all[j][i] = index;
                }
            }

            m_faces.push_back(data);
        }
        else
        {
            cerr << "Unrecognized line type:" 
                << line_no 
                << " " 
                << type
                << endl;
        }

        if (!ss)
        {
            cerr << "Bad line:" << line_no << endl;
            cleanup();
            return false;
        }
    }

    m_is_loaded = !stream.bad() && stream.eof();
    return m_is_loaded;
}

//=============================================================================
// check_indices 
//=============================================================================

static bool check_indices(int arr[3])
{
    for (int i = 0; i < 3; ++i)
    {
        if (arr[i] < 0) 
        {
            return false;
        }
    }

    return true;
}

//=============================================================================
// ToMesh
//=============================================================================

bool ObjLoader::ToMesh(Mesh& mesh, unsigned char flags)
{
    if (!m_is_loaded)
    {
        return false;
    }

    mesh.NewMesh();
    mesh.SetIncludedData(flags);

    using namespace std;
    cout << "Loaded: " << m_faces.size() << "\n";

    for (size_t i = 0; i < m_faces.size(); ++i)
    {
        if (!check_indices(m_faces[i].val.vert))
        {
            std::cerr << "Bad vertex index" << std::endl;
            cout << m_faces[i].val.vert[0] << " "; 
            cout << m_faces[i].val.vert[1] << " "; 
            cout << m_faces[i].val.vert[2] << "\n"; 
            return false;
        }

        const dlib::vec3& v1(m_verts[m_faces[i].val.vert[0]]);
        const dlib::vec3& v2(m_verts[m_faces[i].val.vert[1]]);
        const dlib::vec3& v3(m_verts[m_faces[i].val.vert[2]]);

        mesh.AddTriangle(v1, v2, v3);
    }

    mesh.Finish();

    return true;
}

//=============================================================================
// 
//=============================================================================
