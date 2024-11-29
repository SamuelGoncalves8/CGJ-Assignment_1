////////////////////////////////////////////////////////////////////////////////
//
// Drawing two instances of a triangle in Clip Space.
// A "Hello 2D World" of Modern OpenGL.
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

#include "../mgl/mgl.hpp"

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
    void setColor(const glm::vec3 rgba[4]) {
        for (auto& vertex : vertices) {
            vertex.RGBA[0] = rgba[0].r;
            vertex.RGBA[1] = rgba[0].g;
            vertex.RGBA[2] = rgba[0].b;
            vertex.RGBA[3] = 1.0f;
        }
    }
};


class Triangle : public Geometry {
public:
    Triangle() {
        vertices = {
        {{0.0f, 0.0f, 0.0f, 1.0f}, {0.804f, 0.055f, 0.4f, 1.0f}},
        {{0.25f, 0.0f, 0.0f, 1.0f}, {0.804f, 0.055f, 0.4f, 1.0f}},
        {{0.25, 0.25f, 0.0f, 1.0f}, {0.804f, 0.055f, 0.4f, 1.0f}}
        };
        indices = { 0, 1, 2 };
    }
};

class Square : public Geometry {
public:
    Square() {
        vertices = {
        {{0.0f,  0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.25f,  0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.25f, 0.25f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{ 0.0f, 0.25f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}}
        };
        indices = { 0, 1, 2,
                    0, 2, 3 };
    }
};

class Parallelogram : public Geometry {
public:
    Parallelogram() {
        vertices = {
        {{0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.25f, 0.0, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.0f, 0.25f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{-0.25f, 0.25f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}
        };
        indices = { 0, 1, 2,
                    0, 2, 3 };
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
    geometryList.push_back(Triangle());       // Tiny triangle
    geometryList.push_back(Triangle());       // 2nd Tiny triangle
    geometryList.push_back(Triangle());       // Mid triangle
    geometryList.push_back(Triangle());       // Big triangle
    geometryList.push_back(Triangle());       // 2nd Big triangle
    geometryList.push_back(Square());         // Square
    geometryList.push_back(Parallelogram());  // Parallelogram
}

////////////////////////////////////////////////////////////////////////// SCENE

const glm::mat4 I(1.0f);
const glm::mat4 M = glm::translate(glm::vec3(-0.5f, -1.0f, 0.0f));
const glm::mat4 N = glm::translate(glm::vec3(-0.5f, -0.75f, 0.0f));

void MyApp::drawScene() {
  // Drawing directly in clip space
    for (int i = 0; i < geometryList.size(); i++) {
        glBindVertexArray(VaoId[i]);
        Shaders->bind();

        if (i == 0) {
            glUniformMatrix4fv(MatrixId, 1, GL_FALSE, glm::value_ptr(I));
        }
        else {
            glUniformMatrix4fv(MatrixId, 1, GL_FALSE, glm::value_ptr(M));
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
  engine.setWindow(600, 600, "Hello Modern 2D World", 0, 1);
  engine.init();
  engine.run();
  exit(EXIT_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////// END