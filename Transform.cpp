#include "Transform.h"

Transform::Transform()
    : translation(0.0f, 0.0f, 0.0f),
    rotation(glm::quat_identity<float, glm::defaultp>()),
    scale(1.0f, 1.0f, 1.0f)
{
    // Initialize translation to zero, rotation to identity quaternion, scale to 1
}

glm::mat4 Transform::GetTransformMatrix() const {
    // Construct transformation matrix in order: Translate * Rotate * Scale
    glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
    glm::mat4 R = glm::mat4_cast(rotation);   // Converts quaternion to rotation matrix
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

    return T * R * S;
}

void Transform::SetTranslation(const glm::vec3& t) {
    this->translation = t;
}

void Transform::SetRotation(float radians, const glm::vec3& axis) {
    // Create a quaternion representing a rotation of 'radians' around 'axis'
    // Ensure 'axis' is normalized
    glm::vec3 normAxis = glm::normalize(axis);
    this->rotation = glm::angleAxis(radians, normAxis);
}

void Transform::AddRotation(float radians, const glm::vec3& axis) {
    // Add to the current rotation
    glm::vec3 normAxis = glm::normalize(axis);
    glm::quat additionalRotation = glm::angleAxis(radians, normAxis);

    // Combine the current rotation with the new rotation
    this->rotation = additionalRotation * this->rotation; // Note: Order matters
}

void Transform::SetScale(const glm::vec3& s) {
    this->scale = s;
}

void Transform::SetRotationQuat(const glm::quat& q) {
    this->rotation = q;
}

glm::vec3 Transform::GetTranslation() const {
    return translation;
}

glm::quat Transform::GetRotationQuat() const {
    return rotation;
}

glm::vec3 Transform::GetScale() const {
    return scale;
}
