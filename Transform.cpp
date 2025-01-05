#include "Transform.h"

Transform::Transform()
    : translation(0.0f, 0.0f, 0.0f),
    rotation(glm::quat_identity<float, glm::defaultp>()),
    scale(1.0f, 1.0f, 1.0f)
{
    // Initialize translation to zero, rotation to identity quaternion, scale to 1
}

Transform::Transform(const Transform& other)
    : translation(other.translation),
    rotation(other.rotation),
    scale(other.scale), 
    dirty_flag(other.dirty_flag), 
    transformMatrix(other.transformMatrix){
    // No additional initialization needed; this simply copies the values.
}


glm::mat4 Transform::GetTransformMatrix() {
    if (dirty_flag) {
        // Construct transformation matrix in order: Translate * Rotate * Scale
        glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 R = glm::mat4_cast(rotation);   // Converts quaternion to rotation matrix
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

        transformMatrix = T * R * S;

        this->dirty_flag = false; 

        return T * R * S;
    }
    else {
        return transformMatrix;
    }
    
}

void Transform::SetTranslation(const glm::vec3& t) {
    this->translation = t;

    dirty_flag = true;
}

void Transform::SetRotation(float radians, const glm::vec3& axis) {
    // Create a quaternion representing a rotation of 'radians' around 'axis'
    // Ensure 'axis' is normalized
    glm::vec3 normAxis = glm::normalize(axis);
    this->rotation = glm::angleAxis(radians, normAxis);

    dirty_flag = true;
}


void Transform::AddRotation(float radians, const glm::vec3& axis) {
    AddRotationLeft(radians, axis); // Default behavior as right 

    dirty_flag = true;
}

void Transform::AddRotationLeft(float radians, const glm::vec3& axis) {
    glm::vec3 normAxis = glm::normalize(axis);
    glm::quat additionalRotation = glm::angleAxis(radians, normAxis);
    rotation = additionalRotation * rotation; // Pre-multiply 

    dirty_flag = true;
}

void Transform::AddRotationRight(float radians, const glm::vec3& axis) {
    glm::vec3 normAxis = glm::normalize(axis);
    glm::quat additionalRotation = glm::angleAxis(radians, normAxis);
    rotation = rotation * additionalRotation; // Post-multiply 

    dirty_flag = true;
}

void Transform::SetScale(const glm::vec3& s) {
    this->scale = s; 

    dirty_flag = true;
}

void Transform::SetRotationQuat(const glm::quat& q) {
    this->rotation = q; 

    dirty_flag = true;
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
