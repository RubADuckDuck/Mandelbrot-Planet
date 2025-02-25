#include "Rendering/ShaderProgramLoader.h"

// Function to load shader source code from file
std::string LoadShaderFromFile(const std::string& filePath) {
	std::ifstream file(filePath);
	std::stringstream buffer;

	if (!file.is_open()) {
		std::cerr << "Error: Could not open shader file: " << filePath << std::endl;
		exit(1);
	}

	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

GLuint CompileShader(GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	// Check for compilation errors
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cout << "Shader Compilation Error:\n" << infoLog << std::endl;
		exit(1);
	}

	std::cout << "Successully compiled shader at: " << shader << std::endl;

	return shader;
}

GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// Check for linking errors
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cout << "Shader Linking Error:\n" << infoLog << std::endl;
		exit(1);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

// Function to load, compile, and link shaders into a program
GLuint LoadAndCreateShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
	// Load shader source code from files
	std::string vertexCode = LoadShaderFromFile(vertexPath);
	std::string fragmentCode = LoadShaderFromFile(fragmentPath);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	// Create shader program using existing functions
	GLuint shaderProgram = CreateShaderProgram(vertexSource, fragmentSource);

	std::cout << "Shader program created successfully: " << shaderProgram << std::endl;

	return shaderProgram;
}