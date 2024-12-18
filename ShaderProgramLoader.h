#pragma once
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <string> 
#include <iostream>

// Function to load shader source code from file
std::string LoadShaderFromFile(const std::string& filePath);

GLuint CompileShader(GLenum type, const char* source);

GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource);

// Function to load, compile, and link shaders into a program
GLuint LoadAndCreateShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);