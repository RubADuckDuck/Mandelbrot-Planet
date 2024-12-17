#pragma once
#include <iomanip> // For std::setw and std::setprecision
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Overload the << operator for glm::mat4
std::ostream& operator<<(std::ostream& os, const glm::mat4& matrix);

// Overload the << operator for glm::vec3
std::ostream& operator<<(std::ostream& os, const glm::vec3& vector);

// Overload the << operator for glm::vec4
std::ostream& operator<<(std::ostream& os, const glm::vec4& vector);