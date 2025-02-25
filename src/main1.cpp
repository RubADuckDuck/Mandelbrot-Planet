#include <SDL.h> 
#include <iostream>

// Globals 
int gScreenHeight = 640; 
int gScreenWidth = 480; 
SDL_Window* gGraphicsApplicationWindow = nullptr; 
SDL_GLContext gOpenGLContext = nullptr; 


bool gQuit = false; 

void InitializaProgram() {
	if (SDL_Init(SDL_INIT_VIndexEO) < 0) {
		std::cout << "SDL could not init" << std::endl; 
		exit(1); 
	} 

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	gGraphicsApplicationWindow = SDL_CreateWindow("OpenGL", 
		0, 0, 
		gScreenWidth, gScreenHeight, 
		SDL_WINDOW_OPENGL
	); 

	if (gGraphicsApplicationWindow == nullptr) {
		std::cout << "Graphics Window not initialized properly" << std::endl; 
		exit(1);
	} 

	gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow); 

	if (gOpenGLContext == nullptr) {
		std::cout << "OpenGL context not created" << std::endl; 

		exit(1);
	}
}

void Input() {
	SDL_Event e; 

	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			std::cout << "Goodbye" << std::endl; 

		}
	}
}


void PreDraw() {
	 
}

void Draw() {
	
}

void MainLoop() {
	while (!gQuit) {
		Input(); 

		PreDraw(); 
		Draw(); 

		SDL_GL_SwapWindow(gGraphicsApplicationWindow); 
	}
}

void CleanUp() { 
	SDL_DestroyWindow(gGraphicsApplicationWindow);

	SDL_Quit();
}


int main() {
	InitializaProgram(); 

	MainLoop(); 

	CleanUp();

	return 0;
}