#include "utils.h"

// Overload the << operator for glm::mat4
std::ostream& operator<<(std::ostream& os, const glm::mat4& matrix)
{
    os << std::fixed << std::setprecision(2); // Set precision as needed
    for (int row = 0; row < 4; ++row)
    {
        os << "| ";
        for (int col = 0; col < 4; ++col)
        {
            os << std::setw(6) << matrix[col][row] << " ";
        }
        os << "|\n";
    }
    return os;
}

// Overload the << operator for glm::vec3
std::ostream& operator<<(std::ostream& os, const glm::vec3& vector)
{
    os << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
    return os;
}

// Overload the << operator for glm::vec4
std::ostream& operator<<(std::ostream& os, const glm::vec4& vector)
{
    os << "(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
    return os;
}