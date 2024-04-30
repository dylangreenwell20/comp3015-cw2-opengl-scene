#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include "helper/texture.h"
#include <glm/gtc/matrix_transform.hpp>
#include "helper/particleutils.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

SceneBasic_Uniform::SceneBasic_Uniform() :
    tPrev(0), angle(0.0f), rotSpeed(glm::pi<float>()/2.0f), sky(450.0f), lightPos(250.0f, 250.0f, 250.0f, 1.0f), particleLifetime(5.5f), nParticles(8000), emitterPos(1,0,0), emitterDir(-1,2,0) {
    rock_arch = ObjMesh::load("media/CaveRock_L_Obj.obj", false, true);
}

void SceneBasic_Uniform::initScene()
{
    compile();

    
    flatProg.use();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glEnable(GL_BLEND);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    GLuint particleTex = Texture::loadTexture("media/texture/fire.png");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, particleTex);

    initBuffers();

    

    

    flatProg.setUniform("ParticleTex", 0);
    flatProg.setUniform("ParticleLifetime", particleLifetime);
    flatProg.setUniform("ParticleSize", 0.05f);
    flatProg.setUniform("Gravity", vec3(0.0f, -0.2f, 0.0f));
    flatProg.setUniform("EmitterPos", emitterPos);
    
    prog.use();
    prog.setUniform("Colour", glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));

    //vec3 cameraPos = vec3(110.0f * cos(angle), 5.0f, 7.0f * sin(angle));
    vec3 cameraPos = vec3(130.0f, 5.0f, 0.0f);
    view = glm::lookAt(cameraPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    
    projection = glm::perspective(glm::radians(70.0f), (float)width / height, 0.3f, 1000.0f);

    /*
    view = glm::lookAt(
        glm::vec3(0.0f, 4.0f, 7.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    */

    lightAngle = 0.0f;
    lightRotationSpeed = 1.5f;

    prog.setUniform("PBRLight[0].L", vec3(45.0f));
    prog.setUniform("PBRLight[0].Position", view * lightPos);
    prog.setUniform("PBRLight[1].L", vec3(0.3f));
    prog.setUniform("PBRLight[1].Position", vec4(0.0f, 0.15f, -1.0f, 0.0f));
    prog.setUniform("PBRLight[2].L", vec3(45.0f));
    prog.setUniform("PBRLight[2].Position", view * vec4(-7.0f, 3.0f, 7.0f, 1.0f));

    GLuint cubeTex = Texture::loadHdrCubeMap("media/texture/cube/skybox-hdr/skybox");

    model = mat4(1.0f);

    angle = 0.0f;

    prog.setUniform("Light.L", vec3(1.0f));
    prog.setUniform("Light.La", vec3(0.05f));

    GLuint diffTex = Texture::loadTexture("media/texture/CaveRock_L_Base_Diffuse.png");
    GLuint normalTex = Texture::loadTexture("media/texture/CaveRock_L_Base_Normal.png");
    GLuint moss = Texture::loadTexture("media/texture/moss.png");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffTex);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTex);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, moss);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
}

void SceneBasic_Uniform::initBuffers() {
    glGenBuffers(1, &initVel);
    glGenBuffers(1, &startTime);

    int size = nParticles * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glBufferData(GL_ARRAY_BUFFER, size * 3, 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

    glm::mat3 emitterBasis = ParticleUtils::makeArbitraryBasis(emitterDir);
    vec3 v(0.0f);
    float velocity, theta, phi;
    std::vector<GLfloat> data(nParticles * 3);

    for (uint32_t i = 0; i < nParticles; i++) {
        theta = glm::mix(0.0f, glm::pi<float>() / 20.0f, randFloat());
        phi = glm::mix(0.0f, glm::two_pi<float>(), randFloat());

        v.x = sinf(theta) * cosf(phi);
        v.y = cosf(theta);
        v.z = sinf(theta) * sinf(phi);

        velocity = glm::mix(1.25f, 1.5f, randFloat());
        v = glm::normalize(emitterBasis * v) * velocity;

        data[3 * i] = v.x;
        data[3 * i + 1] = v.y;
        data[3 * i + 2] = v.z;
    }

    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size * 3, data.data());

    float rate = particleLifetime / nParticles;

    for (int i = 0; i < nParticles; i++) {
        data[i] = rate * i;
    }

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), data.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &particles);
    glBindVertexArray(particles);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
}

float SceneBasic_Uniform::randFloat() {
    return rand.nextFloat();
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
        flatProg.compileShader("shader/particle.vert");
        flatProg.compileShader("shader/particle.frag");
        flatProg.link();
        flatProg.use();

	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

//void SceneBasic_Uniform::drawFloor()
//{
//    prog.setUniform("PBRMaterial.Rough", 0.9f);
//    prog.setUniform("PBRMaterial.Metal", 0);
//    prog.setUniform("PBRMaterial.Color", vec3(0.2f));
//
//    model = mat4(1.0f);
//    model = translate(model, vec3(0.0f, -0.75f, 0.0f));
//    setMatrices();
//    plane.render();
//}

void SceneBasic_Uniform::drawScene()
{
    //drawFloor();

    int caveNumber = 3;

    vec3 caveBaseColor(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < caveNumber; i++) {
        float caveX = i * (100.0f / (caveNumber - 1)) - 5.0f;
        //float rough = (i + 1) * (1.0f / caveNumber);
        float rough = 1.5f;
        drawSpot(vec3(caveX, 0.0f, 0.0f), rough, 0, caveBaseColor);
    }

    //float metalRough = 0.6f;

    /*drawSpot(vec3(-100.0f, 0.0f, 3.0f), metalRough, 1, caveBaseColor);
    drawSpot(vec3(-50.0f, 0.0f, 3.0f), metalRough, 1, caveBaseColor);*/
}

void SceneBasic_Uniform::drawSpot(const glm::vec3& pos, float rough, int metal, const glm::vec3& color)
{
    prog.setUniform("PBRMaterial.Rough", rough);
    prog.setUniform("PBRMaterial.Metal", metal);
    prog.setUniform("PBRMaterial.Color", color);

    model = mat4(1.0f);
    model = translate(model, pos);
    model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
    setMatrices(prog);
    rock_arch->render();
}



void SceneBasic_Uniform::update( float t )
{
    float deltaT = t - tPrev;

    if (tPrev == 0.0f) {
        deltaT = 0.0f;
    }

    tPrev = t;

    angle += (rotSpeed / 2) * deltaT;

    //particleAngle = std::fmod(particleAngle + 0.01f, glm::two_pi<float>());

    if (angle > glm::two_pi<float>()) {
        angle -= glm::two_pi<float>();
    }

    if (animating()) {
        lightAngle = glm::mod(lightAngle + deltaT * lightRotationSpeed, glm::two_pi<float>());
        lightPos.x = cos(lightAngle) * 7.0f;
        lightPos.y = 3.0f;
        lightPos.z = sin(lightAngle) * 7.0f;
    }
}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    //prog.setUniform("Light[0].Position", view * lightPos);

    prog.use();

    //sky.render();

    prog.setUniform("PBRLight[0].Position", view * glm::vec4(10.0f * cos(angle), 1.0f, 10.0f * sin(angle), 1.0f));
    //prog.setUniform("PBRLight[0].Position", view * lightPos);

    //prog.setUniform("Light.Position", view * glm::vec4(10.0f * cos(angle), 1.0f, 10.0f * sin(angle), 1.0f));

    drawScene();

    grid.render();
    glDepthMask(GL_FALSE);

    flatProg.use();
    setMatrices(flatProg);
    
    flatProg.setUniform("Time", time);
    glBindVertexArray(particles);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    //glFinish();

    /*
    //prog.setUniform("Light.Position", view * glm::vec4(10.0f * cos(angle), 1.0f, 10.0f * sin(angle), 1.0f));
    
    prog.setUniform("Material.Kd", vec3(0.2f, 0.55f, 0.9f));
    prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
    prog.setUniform("Material.Ka", 0.2f * 0.3f, 0.55f * 0.3f, 0.9f * 0.3f);
    prog.setUniform("Material.Shininess", 100.0f);

    model = mat4(1.0f);

    sky.render();
    * 
    prog.setUniform("Light.Position", view * glm::vec4(10.0f * cos(angle), 1.0f, 10.0f * sin(angle), 1.0f));

    prog.setUniform("Material.Kd", vec3(0.2f, 0.55f, 0.9f));
    prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
    prog.setUniform("Material.Ka", 0.2f * 0.3f, 0.55f * 0.3f, 0.9f * 0.3f);
    prog.setUniform("Material.Shininess", 100.0f);

    model = mat4(1.0f);

    sky.render();

    model = mat4(1.0f);

    model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));

    setMatrices();
    rock_arch->render();

    */
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;

    glViewport(0, 0, w, h);
}

void SceneBasic_Uniform::setMatrices(GLSLProgram& prog) {
    mat4 mv = view * model;

    prog.setUniform("ModelViewMatrix", mv);
    //prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    prog.setUniform("NormalMatrix", mat3(mv));
    prog.setUniform("ProjectionMatrix", projection);
    prog.setUniform("MVP", projection * mv);
}