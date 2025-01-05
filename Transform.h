#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp> // For quaternion support

class Transform {
public:
    Transform();
    Transform(const Transform& other);

    glm::mat4 GetTransformMatrix();

    void SetTranslation(const glm::vec3& t);
    void SetRotation(float radians, const glm::vec3& axis);
    void AddRotation(float radians, const glm::vec3& axis); // calls addrotationleft
    void AddRotationLeft(float radians, const glm::vec3& axis);  // New method
    void AddRotationRight(float radians, const glm::vec3& axis); // Updated AddRotation
    void SetScale(const glm::vec3& s);

    // Optional: Additional setters/getters if you want finer control
    void SetRotationQuat(const glm::quat& q);
    glm::vec3 GetTranslation() const;
    glm::quat GetRotationQuat() const;
    glm::vec3 GetScale() const;

private:
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;

    bool dirty_flag = true; 

    glm::mat4 transformMatrix;
};