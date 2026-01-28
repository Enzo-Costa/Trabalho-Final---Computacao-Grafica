#include "objloader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <GL/glu.h> 
#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int fixIndex(int idx, size_t size) 
{
    if (idx > 0) return idx - 1;
    if (idx < 0) return (int)size + idx;
    return -1;
}

bool OBJModel::load(const std::string& path) 
{
    std::ifstream file(path);
    if (!file.is_open()) 
    {
        std::cout << "Erro ao abrir OBJ: " << path << "\n";
        return false;
    }

    size_t pos = path.find_last_of("/\\");
    std::string dir = (pos == std::string::npos) ? "" : path.substr(0, pos + 1);

    std::string line, prefix;
    int current_material = -1;

    while (std::getline(file, line)) 
    {
        if (line.empty()) continue; 
        
        std::stringstream ss(line);
        ss >> prefix;

        if (prefix == "mtllib") 
        {
            std::string mtlfile;
            if (ss >> mtlfile) 
            {
                std::cout << "Lido mtllib: '" << mtlfile << "'\n";
                loadMTL(dir + mtlfile); 
            }
        }
        else if (prefix == "usemtl") 
        {
            std::string name_str; 
            ss >> name_str;
            for (int i = 0; i < (int)materials.size(); i++) 
            {
                if (materials[i].name == name_str)
                    current_material = i;
            }
        }
        else if (prefix == "v") 
        {
            Vec3 v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (prefix == "vt") 
        {
            Vec2 t;
            ss >> t.u >> t.v;
            texcoords.push_back(t); 
        }
        else if (prefix == "vn") 
        {
            Vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (prefix == "f") 
        {
            Face f;
            f.material_index = current_material;

            std::string token;
            while (ss >> token) 
            {
                int v = 0, vt = 0, vn = 0;
                int countSlash = (int)std::count(token.begin(), token.end(), '/');

                if (countSlash == 0) 
                {
                    if (sscanf(token.c_str(), "%d", &v) != 1) v = 0;
                }
                else if (token.find("//") != std::string::npos) 
                {
                    if (sscanf(token.c_str(), "%d//%d", &v, &vn) != 2) v = vn = 0;
                }
                else if (countSlash == 1) 
                {
                    if (sscanf(token.c_str(), "%d/%d", &v, &vt) != 2) v = vt = 0;
                }
                else if (countSlash == 2) 
                {
                    if (sscanf(token.c_str(), "%d/%d/%d", &v, &vt, &vn) != 3) v = vt = vn = 0;
                }
                
                f.v_idx.push_back(fixIndex(v, vertices.size()));
                f.vt_idx.push_back(fixIndex(vt, texcoords.size()));
                f.vn_idx.push_back(fixIndex(vn, normals.size()));
            }

            if (f.v_idx.size() >= 3) 
            {
                for(size_t i = 1; i < f.v_idx.size() - 1; ++i) 
                {
                    Face tri;
                    tri.material_index = f.material_index;
                    
                    tri.v_idx.push_back(f.v_idx[0]);
                    tri.v_idx.push_back(f.v_idx[i]);
                    tri.v_idx.push_back(f.v_idx[i+1]);

                    tri.vt_idx.resize(3, -1);
                    tri.vn_idx.resize(3, -1);

                    if (f.vt_idx.size() > i+1) 
                    {
                        tri.vt_idx[0] = f.vt_idx[0];
                        tri.vt_idx[1] = f.vt_idx[i];
                        tri.vt_idx[2] = f.vt_idx[i+1];
                    }

                    if (f.vn_idx.size() > i+1) 
                    {
                        tri.vn_idx[0] = f.vn_idx[0];
                        tri.vn_idx[1] = f.vn_idx[i];
                        tri.vn_idx[2] = f.vn_idx[i+1];
                    }
                    
                    faces.push_back(tri);
                }
            }
        }
    }
    return true; 
}


bool OBJModel::loadMTL(const std::string& path) 
{
    std::ifstream file(path);
    if (!file.is_open()) 
    {
        std::cout << "Erro ao abrir MTL: " << path << "\n";
        return false;
    }

    size_t pos = path.find_last_of("/\\");
    std::string dir = (pos == std::string::npos) ? "" : path.substr(0, pos + 1);

    std::string line, prefix;
    Material mat;
    bool inMaterial = false;

    while (std::getline(file, line)) 
    {
        std::stringstream ss(line);
        ss >> prefix;

        if (prefix == "newmtl") 
        {
            if (inMaterial) materials.push_back(mat);
            
            std::string name_str; 
            ss >> name_str;

            mat = Material(); 
            mat.name = name_str; 
            inMaterial = true;
        }
        else if (prefix == "Ka") ss >> mat.Ka[0] >> mat.Ka[1] >> mat.Ka[2];
        else if (prefix == "Kd") ss >> mat.Kd[0] >> mat.Kd[1] >> mat.Kd[2];
        else if (prefix == "Ks") ss >> mat.Ks[0] >> mat.Ks[1] >> mat.Ks[2];
        else if (prefix == "Ns") ss >> mat.Ns;
        else if (prefix == "map_Kd") 
        {
            ss >> mat.map_Kd;

            mat.textureID = 0; 
        }
    }

    if (inMaterial) materials.push_back(mat);

    return true;
}


GLuint OBJModel::loadTexture(const std::string& filename) 
{
    std::cout << "IGNORADO: Chamada para carregar textura foi desabilitada." << std::endl;
    return 0; 
}

void OBJModel::computeBoundingBox(Vec3 &min, Vec3 &max) 
{
    if (vertices.empty()) return;
    min = vertices[0];
    max = vertices[0];

    for (const auto &v : vertices) 
    {
        if (v.x < min.x) min.x = v.x;
        if (v.y < min.y) min.y = v.y;
        if (v.z < min.z) min.z = v.z;

        if (v.x > max.x) max.x = v.x;
        if (v.y > max.y) max.y = v.y;
        if (v.z > max.z) max.z = v.z;
    }
}

void OBJModel::draw() 
{
    
    for (auto &f : faces) 
    {

        float r = 0.8f, g = 0.8f, b = 0.8f; 

        if (f.material_index >= 0 && f.material_index < (int)materials.size()) 
        {
            Material &m = materials[f.material_index];

            Vec3 &v_pos_tri = vertices[f.v_idx[0]];
            
            float fator_simples = (v_pos_tri.y * 0.5f) + 0.5f; 
            if (fator_simples < 0.0f) fator_simples = 0.0f;
            if (fator_simples > 1.0f) fator_simples = 1.0f;

            if (m.name.find("buddha") != std::string::npos || m.name.find("dourado") != std::string::npos) 
            {

                float sombra_base = 0.1f;
                float max_brilho = 1.9f;
                float delta_contraste = max_brilho - sombra_base;
                float fator_brilho = sombra_base + fator_simples * delta_contraste; 


                r = m.Kd[0] * fator_brilho; 
                g = m.Kd[1] * fator_brilho; 
                b = m.Kd[2] * fator_brilho;
            }
            else if (m.name.find("dragao") != std::string::npos) 
            {

                float sombra_base = 0.05f;
                float max_brilho = 2.2f;
                float delta = max_brilho - sombra_base; 
    
                float fator_brilho = sombra_base + delta * fator_simples; 
    
                r = m.Kd[0] * fator_brilho; 
                g = m.Kd[1] * fator_brilho; 
                b = m.Kd[2] * fator_brilho;
            }
            else if (m.name.find("bunny") != std::string::npos || m.name.find("coelho") != std::string::npos) 
            {
                float sombra_base = 0.1f;
                float max_brilho = 1.1f;   
                float delta = max_brilho - sombra_base; 
                
                float fator_brilho = sombra_base + delta * fator_simples; 
                 
                r = m.Kd[0] * fator_brilho;
                g = m.Kd[1] * fator_brilho;
                b = m.Kd[2] * fator_brilho;
            }
            else 
            {
                r = m.Kd[0]; g = m.Kd[1]; b = m.Kd[2];
            }

            glMaterialfv(GL_FRONT, GL_AMBIENT,  m.Ka);
            glMaterialfv(GL_FRONT, GL_DIFFUSE,  m.Kd);
            glMaterialfv(GL_FRONT, GL_SPECULAR, m.Ks);
            glMaterialf(GL_FRONT, GL_SHININESS, m.Ns);

            glDisable(GL_TEXTURE_2D);
        }
        else 
        {
            glDisable(GL_TEXTURE_2D);
            float color[] = {0.8f, 0.8f, 0.8f, 1.0f}; 
            glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
        }

        glBegin(GL_TRIANGLES); 
        for (int i = 0; i < f.v_idx.size(); i++) 
        {
            
            if (i < f.vn_idx.size() && f.vn_idx[i] >= 0) 
            {
                Vec3 &n = normals[f.vn_idx[i]];
                glNormal3f(n.x, n.y, n.z);
            }

            glColor3f(r, g, b); 

            if (i < f.vt_idx.size()) 
            {
                int vt_idx = f.vt_idx[i];
                if (vt_idx >= 0 && vt_idx < (int)texcoords.size()) 
                {
                    Vec2 &t = texcoords[vt_idx];
                    glTexCoord2f(t.u, t.v);
                } 
                else 
                {
                    glTexCoord2f(0.0f, 0.0f);
                }
            }

            Vec3 &v = vertices[f.v_idx[i]];
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }
}