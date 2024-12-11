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
#include <glm/gtx/matrix_decompose.hpp>

#include "mgl/mgl.hpp"

#define NUM_OF_PIECES 7
#define NUM_OF_TRI 5

////////////////////////////////////////////////////////////////////////// MYAPP

class SceneNode {
public:
    const GLuint UBO_BP = 0;
    mgl::Mesh* mesh;
    std::vector<SceneNode*> children;
    glm::mat4 TranslateMatrixCube = glm::mat4(1.0f);
    glm::mat4 TranslateMatrixCrab = glm::mat4(1.0f);
    glm::mat4 RotateMatrixCube = glm::mat4(1.0f);
    glm::mat4 RotateMatrixCrab = glm::mat4(1.0f);
    glm::mat4 ScaleMatrix = glm::mat4(1.0f);
    mgl::ShaderProgram* Shader = nullptr;
    SceneNode* parent = nullptr;
    GLint ModelMatrixId;

    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

    void setColor(glm::vec3 newColor) {
        color = newColor;
    }

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

    void translateCrab(glm::vec3 vector) {
        TranslateMatrixCrab = glm::translate(TranslateMatrixCrab, vector);
    };
    void rotateCrab(float angle, glm::vec3 axis) {
        RotateMatrixCrab = glm::rotate(RotateMatrixCrab, glm::radians(angle), axis);
    };

    void translateCube(glm::vec3 vector) {
        TranslateMatrixCube = glm::translate(TranslateMatrixCube, vector);
    };
    void rotateCube(float angle, glm::vec3 axis) {
        RotateMatrixCube = glm::rotate(RotateMatrixCube, glm::radians(angle), axis);
    };
    void scale(glm::vec3 vector) {
        ScaleMatrix = glm::scale(ScaleMatrix, vector);
    };

    void draw(float progress) {
        if (Shader != nullptr) {
            glm::mat4 MatrixCrab = TranslateMatrixCrab * RotateMatrixCrab * ScaleMatrix;
            glm::mat4 MatrixCube = TranslateMatrixCube * RotateMatrixCube * ScaleMatrix;
            glm::mat4 worldMatrix;

            if (parent != nullptr) {
                glm::mat4 parentMatrixCrab = parent->TranslateMatrixCrab * parent->RotateMatrixCrab * parent->ScaleMatrix;
                glm::mat4 parentMatrixCube = parent->TranslateMatrixCube * parent->RotateMatrixCube * parent->ScaleMatrix;
                worldMatrix = interpolateMatrix(parentMatrixCrab, parentMatrixCube, progress) * interpolateMatrix(MatrixCrab, MatrixCube, progress);
            }
            else {
                worldMatrix = interpolateMatrix(MatrixCrab, MatrixCube, progress);
            }

            Shader->bind();  // Bind the shader program
            glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(worldMatrix));
            glUniform3fv(Shader->Uniforms["meshColor"].index, 1, glm::value_ptr(color));
            // Draw the mesh
            mesh->draw();
            Shader->unbind();  // Unbind the shader program
        }

        if (children.size() != 0) {
            // Recursively draw all children nodes
            for (SceneNode* child : children) {
                child->draw(progress);
            }
        }
        TranslateMatrixCrab = glm::mat4(1.0f);
        TranslateMatrixCube = glm::mat4(1.0f);
        RotateMatrixCrab = glm::mat4(1.0f);
        RotateMatrixCube = glm::mat4(1.0f);
        ScaleMatrix = glm::mat4(1.0f);
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

            Shader->addUniform("meshColor");
            Shader->addUniform(mgl::MODEL_MATRIX);
            Shader->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
            Shader->create();
            GLuint ModelMatrixId = Shader->Uniforms[mgl::MODEL_MATRIX].index;
        }

    }

    glm::mat4 interpolateMatrix(const glm::mat4& CrabMat, const glm::mat4& CubeMat, float progress) {
        // Decompose the start and end matrices into translation, rotation, and scale
        glm::vec3 startTranslation, endTranslation, startScale, endScale, startSkew, endSkew;
        glm::vec4 startPerspective, endPerspective;
        glm::quat startRotation, endRotation;

        // Decompose both matrices
        glm::decompose(CrabMat, startScale, startRotation, startTranslation, startSkew, endPerspective);
        glm::decompose(CubeMat, endScale, endRotation, endTranslation, endSkew, startPerspective);

        // Interpolate translation, rotation, and scale
        glm::vec3 interpolatedTranslation = glm::mix(startTranslation, endTranslation, progress);
        glm::quat interpolatedRotation = glm::slerp(startRotation, endRotation, progress);
        glm::vec3 interpolatedScale = glm::mix(startScale, endScale, progress);

        // Reconstruct the final matrix with the interpolated components
        glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), interpolatedTranslation);
        glm::mat4 rotationMat = glm::mat4_cast(interpolatedRotation);
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), interpolatedScale);

        return translationMat * rotationMat * scaleMat;
    }


};

class SceneGraph {
public:
    SceneNode* root = nullptr;
    mgl::Camera* camera;
    uint8_t cameraPos = 0;
    uint8_t orto = 0;
    uint8_t left = 0;
    uint8_t right = 0;

    void setRootNode(SceneNode* rootNode) {
        root = rootNode;
    }
    void addCamera(mgl::Camera* Camera) {
        this->camera = Camera;
    }
    void draw(float progress) {
        if (root != nullptr) {
            root->draw(progress);
        }
    }
    void createShaderProgram() {
        root->createShaderProgram();
    }
};

class MyApp : public mgl::App {
public:
    void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) override;
    void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) override;
    void cursorCallback(GLFWwindow* win, double xpos, double ypos) override;
    void initCallback(GLFWwindow* win) override;
    void displayCallback(GLFWwindow* win, double elapsed) override;
    void windowSizeCallback(GLFWwindow* win, int width, int height) override;
    void scrollCallback(GLFWwindow* win, double xpos, double ypos) override;
    float progress = 0.0f;

private:
    bool pressing = false;
    double cursor_x_pos;
    double cursor_y_pos;
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

    Triangle1->setColor(glm::vec3(1.0f, 0.0f, 0.0f)); // Red
    Triangle2->setColor(glm::vec3(0.0f, 1.0f, 0.0f)); // Green
    Triangle3->setColor(glm::vec3(0.0f, 0.0f, 1.0f)); // Blue
    Triangle4->setColor(glm::vec3(1.0f, 1.0f, 0.0f)); // Yellow
    Triangle5->setColor(glm::vec3(1.0f, 0.5f, 0.0f)); // Orange
    Para->setColor(glm::vec3(0.5f, 0.0f, 0.5f)); // Purple
    Square->setColor(glm::vec3(0.0f, 1.0f, 1.0f)); // Cyan

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
glm::mat4 ViewMatrix1 =
glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(-5,-5,-5) Center(0,0,0) Up(0,1,0)
glm::mat4 ViewMatrix2 =
glm::lookAt(glm::vec3(-5.0f, -5.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

// Orthographic LeftRight(-2,2) BottomTop(-2,2) NearFar(1,10)
glm::mat4 ProjectionMatrix1 =
glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 10.0f);

// Perspective Fovy(30) Aspect(640/480) NearZ(1) FarZ(10)
glm::mat4 ProjectionMatrix2 =
glm::perspective(glm::radians(30.0f), 640.0f / 480.0f, 1.0f, 10.0f);

void MyApp::createCameras() {
    mgl::Camera* Camera = new mgl::Camera(0);
    Camera->setViewMatrix(ViewMatrix1);
    Camera->setProjectionMatrix(ProjectionMatrix1);
    scene.addCamera(Camera);
}

/////////////////////////////////////////////////////////////////////////// DRAW

glm::mat4 ModelMatrix(1.0f);
glm::vec3 RotateAxisX = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 RotateAxisY = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 RotateAxisZ = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 TriangleBigScale = glm::vec3(1.0f, 2.0f, 2.0f);
glm::vec3 TriangleMidScale = glm::vec3(1.0f, glm::sqrt(2), glm::sqrt(2));

/////////////////////////////////////////////////////////////////////////// CRAB

//Big Triangles
glm::vec3 Triangle1Translate = glm::vec3(0.0f, -0.75f, 0.25f);
glm::vec3 Triangle2Translate = glm::vec3(0.0f, -0.25f, -0.25f);
//Mid Triangle
glm::vec3 TriangleMidTranslate = glm::vec3(0.0f, 0.25f, -0.75f);
//Tiny Triangles
glm::vec3 Triangle4Translate = glm::vec3(0.0f, 0.50f, 1.0f);
glm::vec3 Triangle5Translate = glm::vec3(0.0f, -1.0f, -0.50f);
//Para
glm::vec3 ParaTranslate = glm::vec3(0.0f, 0.0f, 0.70f);

////////////////////////////////////////////////////////////////////////// Tangram Cube

//Big Triangles
glm::vec3 Triangle1Translate2 = glm::vec3(-1.05f, 0.0f, 0.0f);
glm::vec3 Triangle2Translate2 = glm::vec3(-0.35f, 0.0f, -0.71f);
//Mid Triangles
glm::vec3 TriangleMidTranslate2 = glm::vec3(0.0f,0.0f,0.35f);
//Tiny Triangles
glm::vec3 Triangle4Translate2 = glm::vec3(0.355f, 0.0f, -0.355f);
glm::vec3 Triangle5Translate2 = glm::vec3(-0.35f, 0.0f, 0.35f);
//Para
glm::vec3 ParaTranslate2 = glm::vec3(-0.485f, 0.0f, 0.49f);

void MyApp::drawScene() {
    //Big Triangles
    tangram.getChild(0)->scale(TriangleBigScale);
    tangram.getChild(1)->scale(TriangleBigScale);
    //Crab
    tangram.getChild(0)->rotateCrab(-90.0f, RotateAxisX);
    tangram.getChild(1)->rotateCrab(90.0f, RotateAxisX);
    tangram.getChild(0)->translateCrab(Triangle1Translate);
    tangram.getChild(1)->translateCrab(Triangle2Translate);
    //Cube
    tangram.getChild(0)->rotateCube(45.0f, RotateAxisY);
    tangram.getChild(0)->rotateCube(90.0f, RotateAxisZ);
    tangram.getChild(1)->rotateCube(-45.0f, RotateAxisY);
    tangram.getChild(1)->rotateCube(90.0f, RotateAxisZ);
    tangram.getChild(0)->translateCube(Triangle1Translate2);
    tangram.getChild(1)->translateCube(Triangle2Translate2);
    ////Mid Triangle
    tangram.getChild(2)->scale(TriangleMidScale);
    //Crab
    tangram.getChild(2)->rotateCrab(135.0f, RotateAxisX);
    tangram.getChild(2)->translateCrab(TriangleMidTranslate);
    //Cube
    tangram.getChild(2)->rotateCube(90.0f, RotateAxisZ);
    tangram.getChild(2)->translateCube(TriangleMidTranslate2);
    ////Tiny Triangles
    // Crab
    tangram.getChild(3)->rotateCrab(90.0f, RotateAxisX);
    tangram.getChild(3)->translateCrab(Triangle4Translate);
    tangram.getChild(4)->rotateCrab(180.0f, RotateAxisX);
    tangram.getChild(4)->translateCrab(Triangle5Translate);
    // Cube
    tangram.getChild(3)->rotateCube(-135.0f, RotateAxisY);
    tangram.getChild(3)->rotateCube(90.0f, RotateAxisZ);
    tangram.getChild(3)->translateCube(Triangle4Translate2);
    tangram.getChild(4)->rotateCube(135.0f, RotateAxisY);
    tangram.getChild(4)->rotateCube(90.0f, RotateAxisZ);
    tangram.getChild(4)->translateCube(Triangle5Translate2);
    ////Para
    //Crab
    tangram.getChild(5)->translateCrab(ParaTranslate);
    //Cube
    tangram.getChild(5)->rotateCube(-45.0f, RotateAxisY);
    tangram.getChild(5)->rotateCube(90.0f, RotateAxisZ);
    tangram.getChild(5)->translateCube(ParaTranslate2);
    //Square
    tangram.getChild(6)->rotateCube(45.0f, RotateAxisY);

    if (scene.left) {
        progress += 0.01f;
    } else if (scene.right) {
        progress -= 0.01f;
    }

    if (progress < 0.0f) {
        progress = 0.0f;
    } else if (progress > 1.0f) {
        progress = 1.0f;
    }
    scene.draw(progress);
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_C) {
            if (scene.cameraPos == 0) {
                scene.camera->setViewMatrix(ViewMatrix2);
                scene.cameraPos = 1;
            }
            else {
                scene.camera->setViewMatrix(ViewMatrix1);
                scene.cameraPos = 0;
            }
        }
        if (key == GLFW_KEY_P) {
            if (scene.orto == 1) {
                scene.camera->setProjectionMatrix(ProjectionMatrix2);
                scene.orto = 0;
            }
            else {
                scene.camera->setProjectionMatrix(ProjectionMatrix1);
                scene.orto = 1;
            }
        }
        if (key == GLFW_KEY_LEFT) {
            if (!scene.right) {
                scene.left = 1;
            }
        }
        if (key == GLFW_KEY_RIGHT) {
            if (!scene.left) {
                scene.right = 1;
            }
        }
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT) {
            scene.left = 0;
        }
        if (key == GLFW_KEY_RIGHT) {
            scene.right = 0;
        }
    }
}

void MyApp::mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        glfwGetCursorPos(win, &cursor_x_pos, &cursor_y_pos);
        pressing = true;
    }
    else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT){
        glfwGetCursorPos(win, &cursor_x_pos, &cursor_y_pos);
        pressing = false;
    }
}

void MyApp::cursorCallback(GLFWwindow* win, double xpos, double ypos) {
    float rotationSpeed = 0.5f; // Adjust for sensitivity

    if (pressing) {
        double delta_x = xpos - cursor_x_pos;
        double delta_y = ypos - cursor_y_pos;

        cursor_x_pos = xpos;
        cursor_y_pos = ypos;

        // Calculate rotation angles
        float horizontalAngle = glm::radians(static_cast<float>(delta_x * rotationSpeed));
        float verticalAngle = glm::radians(static_cast<float>(-delta_y * rotationSpeed));

        glm::mat4 ViewMatrix = scene.camera->getViewMatrix();
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);

        //Update Camera Position
        glm::vec3 CameraPos = glm::vec3(glm::inverse(ViewMatrix)[3]);

        glm::vec3 direction = glm::normalize(CameraPos - target);

        // Compute rotation quaternions
        glm::quat horizontalQuat = glm::angleAxis(horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // Around Y-axis
        glm::vec3 rightAxis = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f))); // Perpendicular to Y and direction
        glm::quat verticalQuat = glm::angleAxis(verticalAngle, rightAxis); // Around the right axis

        // Combine the rotations
        glm::quat combinedQuat = horizontalQuat * verticalQuat;

        if (ViewMatrix == ViewMatrix1) {
            scene.camera->setViewMatrix(glm::lookAt(CameraPos * combinedQuat, target, glm::vec3(0.0f, 1.0f, 0.0f)));
            ViewMatrix1 = scene.camera->getViewMatrix();
        }
        else {
            scene.camera->setViewMatrix(glm::lookAt(CameraPos * combinedQuat, target, glm::vec3(0.0f, 1.0f, 0.0f)));
            ViewMatrix2 = scene.camera->getViewMatrix();
        }
    }
}

void MyApp::initCallback(GLFWwindow* win) {
    scene.setRootNode(&tangram);

    createMeshes();
    createShaderPrograms();  // after mesh;
    createCameras();
}

void MyApp::windowSizeCallback(GLFWwindow* win, int width, int height) {
    int size = std::min(width, height);

    int x_offset = (width - size) / 2;
    int y_offset = (height - size) / 2;

    glViewport(x_offset, y_offset, size, size);
}

void MyApp::displayCallback(GLFWwindow* win, double elapsed) { 
    drawScene();
}

void MyApp::scrollCallback(GLFWwindow* win, double xpos, double ypos) {
    const float zoomSpeed = 0.5f;
    glm::mat4 ViewMatrix = scene.camera->getViewMatrix();
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);


    // Update the camera's position
    glm::vec3 CameraPos = glm::vec3(glm::inverse(ViewMatrix)[3]);

    glm::vec3 direction = glm::normalize(target - CameraPos);

    glm::vec3 newCameraPos = CameraPos + direction * zoomSpeed * static_cast<float>(ypos);

    if (ViewMatrix == ViewMatrix1) {
        scene.camera->setViewMatrix(glm::lookAt(newCameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f)));
        ViewMatrix1 = scene.camera->getViewMatrix();
    }
    else {
        scene.camera->setViewMatrix(glm::lookAt(newCameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f)));
        ViewMatrix2 = scene.camera->getViewMatrix();
    }
}


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
