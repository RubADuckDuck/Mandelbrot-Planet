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

struct DirectionalLight {
    BaseLight base;
    glm::vec3 direction;

    DirectionalLight() {
        this->base = BaseLight();
        // Set Direction
        this->direction = glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f)); // Pointing diagonally downward
    }
};
