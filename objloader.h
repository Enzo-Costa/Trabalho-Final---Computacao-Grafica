#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <string>
#include <vector>
#include <GL/glut.h>

struct Vec3 
{
    float x, y, z;
};

struct Vec2 
{
    float u, v;
};

struct Material 
{
    std::string name;
    float Ka[3] = {0.2f, 0.2f, 0.2f};
    float Kd[3] = {0.8f, 0.8f, 0.8f};
    float Ks[3] = {0.0f, 0.0f, 0.0f};
    float Ns = 10.0f;
    std::string map_Kd;
    GLuint textureID = 0;
};

struct Face 
{
    std::vector<int> v_idx;
    std::vector<int> vt_idx;
    std::vector<int> vn_idx;
    int material_index = -1;
};

class OBJModel 
{
public:
    std::vector<Vec3> vertices;
    std::vector<Vec2> texcoords;
    std::vector<Vec3> normals;
    std::vector<Face> faces;
    std::vector<Material> materials;

    bool load(const std::string& path);
    bool loadMTL(const std::string& path);
    GLuint loadTexture(const std::string& filename);

    void computeBoundingBox(Vec3 &min, Vec3 &max);
    void draw();
};

#endif