// MainMenuMode.cpp
#include "MainMenuMode.h"
#include "ApplicationConfig.h"
#include "GameEngine.h"

#include <SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Vertex shader source code
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform mat4 uProjection;
    void main() {
        gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    }
)";

// Fragment shader source code
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 uColor;
    void main() {
        FragColor = vec4(uColor, 1.0);
    }
)";

MainMenuMode::MainMenuMode(GameEngine* engine) : GameMode(engine) {
    // Create menu buttons
    int buttonWidth = 200;
    int buttonHeight = 50;
    int startY = 100;
    int spacing = 60;

    buttons = {
        {
            {ApplicationConfig::GetScreenWidth() / 2 - buttonWidth / 2, startY, buttonWidth, buttonHeight},
            "Host Game",
            [this]() { gameEngine->GetModeController()->SwitchMode(GameModeType::HOST_LOBBY); }
        },
        {
            {ApplicationConfig::GetScreenWidth() / 2 - buttonWidth / 2, startY + spacing, buttonWidth, buttonHeight},
            "Join Game",
            [this]() { gameEngine->GetModeController()->SwitchMode(GameModeType::JOIN_LOBBY); }
        },
        {
            {ApplicationConfig::GetScreenWidth() / 2 - buttonWidth / 2, startY + spacing * 2, buttonWidth, buttonHeight},
            "Settings",
            [this]() { gameEngine->GetModeController()->SwitchMode(GameModeType::SETTINGS); }
        },
        {
            {ApplicationConfig::GetScreenWidth() / 2 - buttonWidth / 2, startY + spacing * 3, buttonWidth, buttonHeight},
            "Exit",
            [this]() { /* Implement exit logic */ }
        }
    };
}

MainMenuMode::~MainMenuMode() {
    
    // Clean up OpenGL resources
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
}

void MainMenuMode::Enter() {
    // temporary

    // Compile shaders and link shader program
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Get uniform locations
    projectionUniformLocation = glGetUniformLocation(shaderProgram, "uProjection");
    colorUniformLocation = glGetUniformLocation(shaderProgram, "uColor");

    // Generate vertex array object and vertex buffer object
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
}

void MainMenuMode::Update() {
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    for (const auto& button : buttons) {
        SDL_Point mousePoint = { mouseX, mouseY };
        if (SDL_PointInRect(&mousePoint, &button.rect)) {
            if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                button.onClick();
            }
        }
    }
}

void MainMenuMode::Draw() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up orthographic projection
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(ApplicationConfig::GetScreenWidth()),
        static_cast<float>(ApplicationConfig::GetScreenHeight()), 0.0f, -1.0f, 1.0f);
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, glm::value_ptr(projection));

    // Draw menu buttons
    for (const auto& button : buttons) {
        const SDL_Rect& rect = button.rect;
        const std::string& text = button.text;

        // Set button color
        glUniform3f(colorUniformLocation, 0.8f, 0.8f, 0.8f);

        // Set up vertex data
        float vertices[] = {
            static_cast<float>(rect.x), static_cast<float>(rect.y),
            static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y),
            static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y + rect.h),
            static_cast<float>(rect.x), static_cast<float>(rect.y + rect.h)
        };

        // Draw button background
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Draw button text
        // Use SDL_ttf or other text rendering library to draw the text
        // at the center of the button rectangle
    }

    SDL_GL_SwapWindow(gameEngine->GetWindow());
}

void MainMenuMode::Exit() {}
