////////////////////////////////////////////////////////////////////////////////
//
// Drawing a Crab Tangram.
// A "Group 9 Crab Tangram" of Modern OpenGL.
//
// Copyright (c) 2013-24 by Carlos Martinho
//
// INTRODUCES:
// GL PIPELINE, mglShader.hpp, mglConventions.hpp
//
////////////////////////////////////////////////////////////////////////////////

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <vector>
#include <iostream>

#include "mgl/mgl.hpp"

////////////////////////////////////////////////////////////////////////// MYAPP

class MyApp : public mgl::App {
 public:
  void initCallback(GLFWwindow *win) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowCloseCallback(GLFWwindow *win) override;
  void windowSizeCallback(GLFWwindow *win, int width, int height) override;

 private:
  const GLuint POSITION = 0, COLOR = 1;
  GLuint VaoId[7], VboId[2];
  std::unique_ptr<mgl::ShaderProgram> Shaders;
  GLint MatrixId;

  void createShaderProgram();
  void createBufferObjects();
  void destroyBufferObjects();
  void createGeometry();
  void drawScene();
};

//////////////////////////////////////////////////////////////////////// SHADERs

void MyApp::createShaderProgram() {
  Shaders = std::make_unique<mgl::ShaderProgram>();
  Shaders->addShader(GL_VERTEX_SHADER, "clip-vs.glsl");
  Shaders->addShader(GL_FRAGMENT_SHADER, "clip-fs.glsl");

  Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, POSITION);
  Shaders->addAttribute(mgl::COLOR_ATTRIBUTE, COLOR);
  Shaders->addUniform("Matrix");

  Shaders->create();

  MatrixId = Shaders->Uniforms["Matrix"].index;
}

//////////////////////////////////////////////////////////////////// VAOs & VBOs

typedef struct {
  GLfloat XYZW[4];
  GLfloat RGBA[4];
} Vertex;


class Geometry {
public: 
    Geometry() = default;
    std::vector<Vertex> vertices;
    std::vector<GLubyte> indices;
    std::string geo;
    void setColor(float r, float g, float b) {
        for (auto& vertex : vertices) {
            vertex.RGBA[0] = r;
            vertex.RGBA[1] = g;
            vertex.RGBA[2] = b;
            vertex.RGBA[3] = 1.0f;
        }
    }
};


class Triangle : public Geometry {
public:
    Triangle() {
        vertices = {
        {{0.0f, 0.0f, 0.0f, 1.0f}, {0.933f, 0.380f, 0.2f, 1.0f}},
        {{0.25f, 0.0f, 0.0f, 1.0f}, {0.933f, 0.380f, 0.2f, 1.0f}},
        {{0.25, 0.25f, 0.0f, 1.0f}, {0.933f, 0.380f, 0.2f, 1.0f}}
        };
        indices = { 0, 1, 2 };
        geo = "t";
    }
};

class Square : public Geometry {
public:
    Square() {
        vertices = {
        {{-0.125f,  -0.125f, 0.0f, 1.0f}, {0.0f, 0.8f, 0.0f, 1.0f}},
        {{0.125f,  -0.125f, 0.0f, 1.0f}, {0.0f, 0.8f, 0.0f, 1.0f}},
        {{0.125f, 0.125f, 0.0f, 1.0f}, {0.0f, 0.8f, 0.0f, 1.0f}},
        {{ -0.125f, 0.125f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}}
        };
        indices = { 0, 1, 2,
                    0, 2, 3 };
        geo = "s";
    }
};

class Parallelogram : public Geometry {
public:
    Parallelogram() {
        vertices = {
        {{0.0f, 0.0f, 0.0f, 1.0f}, {0.992f, 0.549f, 0.0f, 1.0f}},
        {{0.25f, 0.0, 0.0f, 1.0f}, {0.992f, 0.549f, 0.0f, 1.0f}},
        {{0.0f, 0.25f, 0.0f, 1.0f}, {0.992f, 0.549f, 0.0f, 1.0f}},
        {{-0.25f, 0.25f, 0.0f, 1.0f}, {0.992f, 0.549f, 0.0f, 1.0f}}
        };
        indices = { 0, 1, 2,
                    0, 2, 3 };
        geo = "p";
    }
};

std::vector<Geometry> geometryList;

void MyApp::createBufferObjects() {
    glGenVertexArrays(geometryList.size(), VaoId);
    for (int i = 0; i < geometryList.size(); i++) {
        glBindVertexArray(VaoId[i]);
        glGenBuffers(2, VboId);

        glBindBuffer(GL_ARRAY_BUFFER, VboId[0]);
        const std::vector<Vertex>& vertices = geometryList[i].vertices;

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(POSITION);
        glVertexAttribPointer(POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(0));
        glEnableVertexAttribArray(COLOR);
        glVertexAttribPointer(COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(sizeof(vertices[0].XYZW)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VboId[1]);
        const std::vector<GLubyte>& indices = geometryList[i].indices;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLubyte), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void MyApp::destroyBufferObjects() {
    for (int i = 0; i < geometryList.size(); i++) {
        glBindVertexArray(VaoId[i]);

        glDisableVertexAttribArray(POSITION);
        glDisableVertexAttribArray(COLOR);
        glDeleteVertexArrays(1, &VaoId[i]);
        glBindVertexArray(0);
    }
    glDeleteBuffers(2, VboId);
}

void MyApp::createGeometry() {
    geometryList.clear();

    Triangle triangle1;
    geometryList.push_back(triangle1);

    Triangle triangle2;
    triangle2.setColor(0.804f, 0.055f, 0.4f);
    geometryList.push_back(triangle2);

    Triangle triangle3;
    triangle3.setColor(0.059f, 0.510f, 0.949f);
    geometryList.push_back(triangle3);

    Triangle triangle4;
    triangle4.setColor(0.43f, 0.23f, 0.75f);
    geometryList.push_back(triangle4);

    Triangle triangle5;
    triangle5.setColor(0.0f, 0.62f, 0.65f);
    geometryList.push_back(triangle5);

    Square square;
    geometryList.push_back(square);

    Parallelogram parallelogram;
    geometryList.push_back(parallelogram);

}

////////////////////////////////////////////////////////////////////////// SCENE

int numTrian;
const glm::mat4 I(1.0f);
const glm::mat4 M = glm::translate(glm::vec3(-0.375f, -0.125f, 0.0f));
const glm::mat4 T1 = glm::translate(glm::vec3(-0.625f,0.125, 0.0f));
const glm::mat4 T2 = glm::translate(glm::vec3(0.125f, -0.125f, 0.0f))*glm::rotate(glm::radians(180.0f), glm::vec3(0.0f,0.0f,1.0f))*glm::scale(glm::vec3(2.0f,2.0f,2.0f));
const glm::mat4 T3 = glm::translate(glm::vec3(-0.125f, -0.375f, 0.0f)) * glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
const glm::mat4 T4 = glm::translate(glm::vec3(0.375f, -0.125f, 0.0f)) * glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f))*glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
const glm::mat4 T5 = glm::translate(glm::vec3(0.375f, -0.625f, 0.0f)) * glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

std::vector<glm::mat4> trans = { T1, T2, T3, T4, T5 };

void MyApp::drawScene() {
    numTrian = 0;
  // Drawing directly in clip space
    for (int i = 0; i < geometryList.size(); i++) {
        glBindVertexArray(VaoId[i]);
        Shaders->bind();

        if (geometryList[i].geo == "s") {
            glUniformMatrix4fv(MatrixId, 1, GL_FALSE, glm::value_ptr(I));
        }
        else if (geometryList[i].geo == "p") {
            glUniformMatrix4fv(MatrixId, 1, GL_FALSE, glm::value_ptr(M));
        }
        else {
            glUniformMatrix4fv(MatrixId, 1, GL_FALSE, glm::value_ptr(trans[numTrian]));
            numTrian++;
        }
        const std::vector<GLubyte>& indices = geometryList[i].indices;
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_BYTE,
                        reinterpret_cast<GLvoid *>(0));

        Shaders->unbind();
    }
  glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win) {
  createGeometry();
  createBufferObjects();
  createShaderProgram();
}

void MyApp::windowCloseCallback(GLFWwindow *win) { destroyBufferObjects(); }

void MyApp::windowSizeCallback(GLFWwindow *win, int winx, int winy) {
  glViewport(0, 0, winx, winy);
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { drawScene(); }

/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char *argv[]) {
  mgl::Engine &engine = mgl::Engine::getInstance();
  engine.setApp(new MyApp());
  engine.setOpenGL(4, 6);
  engine.setWindow(600, 600, "Group 9 Crab Tangram", 0, 1);
  engine.init();
  engine.run();
  exit(EXIT_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////// END