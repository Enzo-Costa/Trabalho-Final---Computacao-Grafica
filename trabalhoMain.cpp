#include <GL/glut.h>
#include <iostream>
#include "objloader.h"
#include <cmath>
#include <algorithm> 
#include <vector> 
#include <string> 

OBJModel model;

float rotX = 0, rotY = 0;
int lastX, lastY;
bool mouseDown = false;

Vec3 bbMin, bbMax;
Vec3 model_center = {0.0f, 0.0f, 0.0f}; 
float scale_factor = 1.0f;
std::string currentModelFile; 

void display();
void reshape(int w, int h);
void mouseMotion(int x, int y);
void mouseButton(int button, int state, int x, int y);


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0, 0, 3.0,  
              0, 0, 0,  
              0, 1, 0); 

    glTranslatef(-model_center.x, -model_center.y, -model_center.z);

    glRotatef(rotX, 1.0f, 0.0f, 0.0f); 
    
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);

    glScalef(scale_factor, scale_factor, scale_factor);

    model.draw();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(45, (float)w/h, 0.1, 100.0); 
    glMatrixMode(GL_MODELVIEW);
}

void mouseMotion(int x, int y) {
    if (mouseDown) {
        float dx = (float)(x - lastX);
        float dy = (float)(y - lastY);
        
        rotY += dx * 0.5f; 
        
        rotX += dy * 0.5f;
    }
    lastX = x;
    lastY = y;
    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {
    mouseDown = (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
    lastX = x;
    lastY = y;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Uso: viewer modelo.obj\n";
        return 0;
    }

    currentModelFile = argv[1]; 

    if (!model.load(argv[1])) {
        std::cout << "Erro fatal ao carregar modelo. Encerrando.\n";
        return 0;
    }

    model.computeBoundingBox(bbMin, bbMax);

    float dx = bbMax.x - bbMin.x;
    float dy = bbMax.y - bbMin.y;
    float dz = bbMax.z - bbMin.z;
    
    if (dx <= 0.0001f || dy <= 0.0001f || dz <= 0.0001f) {
        std::cout << "DEBUG: Dimensoes do objeto nulas ou invalidas. Usando escala padrao.\n";
        scale_factor = 1.0f;
        model_center = {0.0f, 0.0f, 0.0f};
    } else {
        float maxdim = std::max({dx, dy, dz});
        scale_factor = 1.5f / maxdim; 

        model_center.x = (bbMin.x + bbMax.x) / 2.0f;
        model_center.y = (bbMin.y + bbMax.y) / 2.0f;
        model_center.z = (bbMin.z + bbMax.z) / 2.0f;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Trabalho de Computacao Grafica");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glDisable(GL_TEXTURE_2D); 
    
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    
    GLfloat light_position[] = { 10.0f, 10.0f, 10.0f, 0.0f };
    GLfloat light_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f }; 
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    
    glEnable(GL_COLOR_MATERIAL); 
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}