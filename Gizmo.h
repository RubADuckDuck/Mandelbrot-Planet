#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include "GameObject.h"
#include "Transform.h"
#include "Light.h" // For camera object

// Base Gizmo Class
class Gizmo {
public:
    GLuint VAO;
    GLuint VBO;
    glm::vec3 color;
    Transform transform;

    virtual void CreateandLoad() = 0;
    virtual void Update() = 0;
    virtual void Draw(CameraObject& cameraObj, GLuint shaderProgram) = 0;

    virtual ~Gizmo() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};

// Grid Class Derived from Gizmo
class Grid : public Gizmo {
public:
    float size;
    int divisions;
    GameObject* targetObject;

    Grid(float gridSize, int gridDivisions, GameObject* target = nullptr, glm::vec3 gridColor = glm::vec3(0.5f))
        : size(gridSize), divisions(gridDivisions), targetObject(target) {
        color = gridColor;
        CreateandLoad();
    }

    void CreateandLoad() override {
        std::vector<glm::vec3> gridLines;
        float halfSize = size / 2.0f;
        float step = size / divisions;

        // Generate grid lines
        for (int i = 0; i <= divisions; ++i) {
            float position = -halfSize + i * step;

            // Lines parallel to X-axis
            gridLines.push_back(glm::vec3(-halfSize, 0.0f, position));
            gridLines.push_back(glm::vec3(halfSize, 0.0f, position));

            // Lines parallel to Z-axis
            gridLines.push_back(glm::vec3(position, 0.0f, -halfSize));
            gridLines.push_back(glm::vec3(position, 0.0f, halfSize));
        }

        // Generate buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, gridLines.size() * sizeof(glm::vec3), gridLines.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }

    void Update() override {
        if (targetObject != nullptr) {
            // Follow the target object's position
            glm::vec3 translation = targetObject->ptrTransform->GetTranslation();
            transform.SetTranslation(glm::vec3(translation.x, 0.0f, translation.z)); // Stay at Y=0
        }
    }

    void Draw(CameraObject& cameraObj, GLuint shaderProgram) override {
        glUseProgram(shaderProgram);

        // Pass uniform data
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewProjLoc = glGetUniformLocation(shaderProgram, "viewProj");
        GLuint colorLoc = glGetUniformLocation(shaderProgram, "color");

        // Calculate matrices
        glm::mat4 model = transform.GetTransformMatrix();
        glm::mat4 viewProj = cameraObj.GetviewProjMat();

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, glm::value_ptr(viewProj));
        glUniform3fv(colorLoc, 1, glm::value_ptr(color));

        // Draw grid
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, (divisions + 1) * 4);
        glBindVertexArray(0);

        glUseProgram(0);
    }
};