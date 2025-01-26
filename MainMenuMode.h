#pragma once
#include "GameMode.h"
#include <vector>
#include <functional>
#include <SDL_video.h>
#include <string>
#include <glad/glad.h>

// Vertex shader source code
extern const char* vertexShaderSource;

// Fragment shader source code
extern const char* fragmentShaderSource; 



class MainMenuMode : public GameMode {
private:
    struct MenuButton {
        SDL_Rect rect;
        std::string text;
        std::function<void()> onClick;
    };

    std::vector<MenuButton> buttons;
    SDL_Color buttonColor = { 100, 100, 100, 255 };
    SDL_Color hoverColor = { 150, 150, 150, 255 }; 

    //--------------------------------------gl trash
    GLuint shaderProgram;
    GLuint vao, vbo;
    GLint projectionUniformLocation;
    GLint colorUniformLocation;

public:
    MainMenuMode(GameEngine* engine);
    ~MainMenuMode();
    void Enter() override;
    void Update() override;
    void Update(float delta_time) override;
    void Draw() override;
    void Exit() override;
};

