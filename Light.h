#pragma once

#include <glm/glm.hpp>

struct BaseLight {
    glm::vec3 color;
    float ambientIntensity;
    float diffuseIntensity;

    BaseLight() { // default light  
        // Set BaseLight properties
        color = glm::vec3(1.0f, 1.0f, 1.0f);  // White light
        ambientIntensity = 0.3f;             // 30% ambient intensity
        diffuseIntensity = 1.0f;             // Full diffuse intensity
    }
};

struct Attenuation {
    float constant;
    float linear;
    float exp;
};

struct PointLight {
    BaseLight base;
    glm::vec3 localPos;
    Attenuation atten;
};

struct SpotLight {
    PointLight base;
    glm::vec3 direction;
    float cutoff;
};

class DirectionalLight {
private:
    // Private constructor to ensure no external instantiation
    DirectionalLight() : base(), direction(glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f))) {}

    // Delete copy constructor and assignment operator
    DirectionalLight(const DirectionalLight&) = delete;
    DirectionalLight& operator=(const DirectionalLight&) = delete;

public:
    BaseLight base;
    glm::vec3 direction;

    // Static method to access the singleton instance
    static DirectionalLight& getInstance() {
        static DirectionalLight instance;  // Guaranteed to be created once and thread-safe
        return instance;
    }

    // Public methods to access and modify the DirectionalLight properties
    BaseLight& getBaseLight() { return base; }
    glm::vec3& getDirection() { return direction; }
    void setDirection(const glm::vec3& dir) { direction = glm::normalize(dir); }
};
