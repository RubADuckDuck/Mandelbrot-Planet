#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp> // For quaternion support

class Transform {
public:
    Transform();

    glm::mat4 GetTransformMatrix() const;

    void SetTranslation(const glm::vec3& t);
    void SetRotation(float radians, const glm::vec3& axis);
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
};