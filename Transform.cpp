#include "Transform.h"

// Constructor to initialize the transformation matrices
Transform::Transform()
    : translation(glm::mat4(1.0f)),
    rotation(glm::mat4(1.0f)),
    scale(glm::mat4(1.0f)) {}

// Method to compute the final transformation matrix
glm::mat4 Transform::GetTranformMatrix() {
    return translation * rotation * scale; // Apply transformations in order
}

// Set the translation matrix using a translation vector
void Transform::SetTranslation(glm::vec3 x) {
    translation = glm::translate(glm::mat4(1.0f), x);
}

// Set the rotation matrix using an angle (radians) and an axis
void Transform::SetRotation(float theta, glm::vec3 axis) {
    rotation = glm::rotate(glm::mat4(1.0f), theta, axis);
}

// Set the scaling matrix using a scaling vector
void Transform::SetScale(glm::vec3 x) {
    scale = glm::scale(glm::mat4(1.0f), x);
}
