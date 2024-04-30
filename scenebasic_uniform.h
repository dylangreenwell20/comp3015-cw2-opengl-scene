#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/random.h"
#include "helper/grid.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glm/glm.hpp"
#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/cube.h"
#include "helper/skybox.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog, flatProg;

    Random rand;
    GLuint initVel, startTime, particles, nParticles;
    Grid grid;
    glm::vec3 emitterPos, emitterDir;
    
    SkyBox sky;

    GLuint moss;

    std::unique_ptr<ObjMesh> rock_arch;

    float rotSpeed;
    float tPrev;
    float angle;

    float particleAngle;
    float particleLifetime;

    float lightAngle;
    float lightRotationSpeed;
    
    glm::vec4 lightPos;

    void initBuffers();
    float randFloat();

    //void drawFloor();
    void drawScene();
    void drawSpot(const glm::vec3& pos, float rough, int metal, const glm::vec3& color);

    void setMatrices(GLSLProgram&);
    void compile();
public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
