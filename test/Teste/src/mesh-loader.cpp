////////////////////////////////////////////////////////////////////////////////
//
//  Loading meshes from external files
//
// Copyright (c) 2023-24 by Carlos Martinho
//
// INTRODUCES:
// MODEL DATA, ASSIMP, mglMesh.hpp
//
////////////////////////////////////////////////////////////////////////////////

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "mgl/mgl.hpp"

#define NUM_OF_PIECES 7
#define NUM_OF_TRI 5

////////////////////////////////////////////////////////////////////////// MYAPP

class SceneNode {
private:
    const GLuint UBO_BP = 0;
    mgl::Mesh* mesh;
    std::vector<SceneNode*> children;
    glm::mat4 TranslateMatrix = glm::mat4(1.0f);
    glm::mat4 RotateMatrix = glm::mat4(1.0f);
    glm::mat4 ScaleMatrix = glm::mat4(1.0f);
    mgl::ShaderProgram* Shader = nullptr;
    SceneNode* parent = nullptr;
    GLint ModelMatrixId;

public:

    void setMesh(mgl::Mesh* mesh) {
        this->mesh = mesh;
    }

    void addChild(SceneNode* child) {
        child->parent = this;
        children.push_back(child);
    };

    SceneNode* getChild(int index) {
        return children[index];
    }

    void translate(glm::vec3 vector) {
        TranslateMatrix = glm::translate(glm::mat4(1.0f), vector);
    };
    void rotate(float angle, glm::vec3 axis) {
        RotateMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    };
    void scale(glm::vec3 vector) {
        ScaleMatrix = glm::scale(glm::mat4(1.0f), vector);
    };

    void draw() {
        if (Shader != nullptr) {

            glm::mat4 worldMatrix = TranslateMatrix * RotateMatrix * ScaleMatrix;

            
            Shader->bind();  // Bind the shader program
            glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(worldMatrix));
            // Draw the mesh
            mesh->draw();
            Shader->unbind();  // Unbind the shader program
        }

        if (children.size() != 0) {
            // Recursively draw all children nodes
            for (SceneNode* child : children) {
                child->draw();
            }
        }
    }

    void createShaderProgram() {
        if (children.size() != 0) {
            for (SceneNode* child : children) {
                child->createShaderProgram();
            }
        }
        if (mesh != nullptr) {
            Shader = new mgl::ShaderProgram();
            Shader->addShader(GL_VERTEX_SHADER, "cube-vs.glsl");
            Shader->addShader(GL_FRAGMENT_SHADER, "cube-fs.glsl");

            Shader->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
            if (mesh->hasNormals()) {
                Shader->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
            }
            if (mesh->hasTexcoords()) {
                Shader->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);
            }
            if (mesh->hasTangentsAndBitangents()) {
                Shader->addAttribute(mgl::TANGENT_ATTRIBUTE, mgl::Mesh::TANGENT);
            }

            Shader->addUniform(mgl::MODEL_MATRIX);
            Shader->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
            Shader->create();
            GLuint ModelMatrixId = Shader->Uniforms[mgl::MODEL_MATRIX].index;
        }

    }

};

class SceneGraph {
public:
    SceneNode* root = nullptr;
    std::vector<mgl::Camera*> cameras;
    uint8_t currentCamera = 0;

    void setRootNode(SceneNode* rootNode) {
        root = rootNode;
    }
    void addCamera(mgl::Camera* camera) {
        cameras.push_back(camera);
    }
    void draw() {
        if (root != nullptr) {
            root->draw();
        }
    }
    void createShaderProgram() {
        root->createShaderProgram();
    }
};

class MyApp : public mgl::App {
public:
    void initCallback(GLFWwindow* win) override;
    void displayCallback(GLFWwindow* win, double elapsed) override;
    void windowSizeCallback(GLFWwindow* win, int width, int height) override;

private:
    mgl::ShaderProgram* Shaders = nullptr;
    std::vector<mgl::Camera*> Cameras;
    mgl::Mesh* Mesh = nullptr;
    std::vector<mgl::Mesh*> tangramMeshes;
    SceneGraph scene;
    SceneNode tangram;

    void createMeshes();
    void createShaderPrograms();
    void createCameras();
    void drawScene();
};

///////////////////////////////////////////////////////////////////////// MESHES

void MyApp::createMeshes() {
    std::string mesh_dir = "models/";

    std::vector<std::string> mesh_files = { "Cube.obj",
                                            "Para.obj",
                                            "TriangleFix.obj" };

    for (const std::string& file : mesh_files) {
        std::string mesh_fullname = mesh_dir + file;
        Mesh = new mgl::Mesh();
        Mesh->joinIdenticalVertices();
        Mesh->create(mesh_fullname);
        tangramMeshes.push_back(Mesh);
    }

    SceneNode* Triangle1 = new SceneNode();
    SceneNode* Triangle2 = new SceneNode();
    SceneNode* Triangle3 = new SceneNode();
    SceneNode* Triangle4 = new SceneNode();
    SceneNode* Triangle5 = new SceneNode();
    SceneNode* Para = new SceneNode();
    SceneNode* Square = new SceneNode();

    Triangle1->setMesh(tangramMeshes[2]);
    Triangle2->setMesh(tangramMeshes[2]);
    Triangle3->setMesh(tangramMeshes[2]);
    Triangle4->setMesh(tangramMeshes[2]);
    Triangle5->setMesh(tangramMeshes[2]);
    Para->setMesh(tangramMeshes[1]);
    Square->setMesh(tangramMeshes[0]);

    tangram.addChild(Triangle1);
    tangram.addChild(Triangle2);
    tangram.addChild(Triangle3);
    tangram.addChild(Triangle4);
    tangram.addChild(Triangle5);
    tangram.addChild(Para);
    tangram.addChild(Square);

}

///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
    scene.createShaderProgram();
}

///////////////////////////////////////////////////////////////////////// CAMERA

// Eye(5,5,5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix1 =
glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(-5,-5,-5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix2 =
glm::lookAt(glm::vec3(-5.0f, -5.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

// Orthographic LeftRight(-2,2) BottomTop(-2,2) NearFar(1,10)
const glm::mat4 ProjectionMatrix1 =
glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 10.0f);

// Perspective Fovy(30) Aspect(640/480) NearZ(1) FarZ(10)
const glm::mat4 ProjectionMatrix2 =
glm::perspective(glm::radians(30.0f), 640.0f / 480.0f, 1.0f, 10.0f);

void MyApp::createCameras() {

    mgl::Camera *Camera1 = new mgl::Camera(0);
    Camera1->setViewMatrix(ViewMatrix2);
    Camera1->setProjectionMatrix(ProjectionMatrix1);
    Cameras.push_back(Camera1);

    mgl::Camera* Camera2 = new mgl::Camera(0);
    Camera2->setViewMatrix(ViewMatrix2);
    Camera2->setProjectionMatrix(ProjectionMatrix2);
    Cameras.push_back(Camera2);
}

/////////////////////////////////////////////////////////////////////////// DRAW

glm::mat4 ModelMatrix(1.0f);
glm::vec3 RotateAxis = glm::vec3(1.0f, 0.0f, 0.0f);
//Big Triangles
glm::vec3 TriangleBigScale = glm::vec3(1.0f, 2.0f, 2.0f);
glm::vec3 Triangle1Translate = glm::vec3(0.0f, -0.75f, 0.25f);
glm::vec3 Triangle2Translate = glm::vec3(0.0f, -0.25f, -0.25f);
//Mid Triangle
glm::vec3 TriangleMidScale = glm::vec3(1.0f, 1.5f, 1.5f);
glm::vec3 TriangleMidTranslate = glm::vec3(0.0f, 0.25f, -0.75f);
//Tiny Triangles
glm::vec3 Triangle4Translate = glm::vec3(0.0f, 0.50f, 1.01f);
glm::vec3 Triangle5Translate = glm::vec3(0.0f, -1.0f, -0.50f);
//Para
glm::vec3 ParaTranslate = glm::vec3(0.0f, 0.0f, 0.70f);

void MyApp::drawScene() {
    //Big Triangles
    tangram.getChild(0)->rotate(-90.0f, RotateAxis);
    tangram.getChild(1)->rotate(90.0f, RotateAxis);
    tangram.getChild(0)->translate(Triangle1Translate);
    tangram.getChild(1)->translate(Triangle2Translate);
    tangram.getChild(0)->scale(TriangleBigScale);
    tangram.getChild(1)->scale(TriangleBigScale);
    //Mid Triangle
    tangram.getChild(2)->rotate(135.0f, RotateAxis);
    tangram.getChild(2)->translate(TriangleMidTranslate);
    tangram.getChild(2)->scale(TriangleMidScale);
    //Tiny Triangles
    tangram.getChild(3)->rotate(90.0f, RotateAxis);
    tangram.getChild(3)->translate(Triangle4Translate);
    tangram.getChild(4)->rotate(180.0f, RotateAxis);
    tangram.getChild(4)->translate(Triangle5Translate);
    //Para
    tangram.getChild(5)->translate(ParaTranslate);
    scene.draw();
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow* win) {
    scene.setRootNode(&tangram);

    createMeshes();
    createShaderPrograms();  // after mesh;
    createCameras();

}

void MyApp::windowSizeCallback(GLFWwindow* win, int winx, int winy) {
    glViewport(0, 0, winx, winy);
    // change projection matrices to maintain aspect ratio
}

void MyApp::displayCallback(GLFWwindow* win, double elapsed) { drawScene(); }

/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char* argv[]) {
    mgl::Engine& engine = mgl::Engine::getInstance();
    engine.setApp(new MyApp());
    engine.setOpenGL(4, 6);
    engine.setWindow(800, 600, "Mesh Loader", 0, 1);
    engine.init();
    engine.run();
    exit(EXIT_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////
