#pragma once
#include <functional>
#include <string> 
#include <vector>
#include <SDL.h> 
#include <iostream>

class Event;

// below Code is defining a proxy named (left) for the (right), 
// sort of like assigning variables but with classnames I guess  
using Listener = std::function<void(const std::string&)>;

class InputHandler {
public:
    Event inputEvent; //

    InputHandler() : quit(false) {
        // Initialize SDL
        inputEvent = Event();  


        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to initialize SDL");
        }
    }

    ~InputHandler() {
        SDL_Quit();
    }

    void pollEvents() { // this function will get called every update to check for input
        // Reset key states
        wPressed = aPressed = sPressed = dPressed = spacePressed = false;

        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;

            case SDL_KEYDOWN:
                handleKeyDown(event.key.keysym.sym);
                break;

            case SDL_KEYUP:
                handleKeyUp(event.key.keysym.sym);
                break;

            default:
                break;
            }
        }
    }

    bool isQuit() const { return quit; }
    bool isWPressed() const { return wPressed; }
    bool isAPressed() const { return aPressed; }
    bool isSPressed() const { return sPressed; }
    bool isDPressed() const { return dPressed; }
    bool isSpacePressed() const { return spacePressed; }

    void Subscribe(Listener listener) {
        inputEvent.Subscribe(listener);
    }

private:
    bool quit;
    bool wPressed, aPressed, sPressed, dPressed, spacePressed;

    void handleKeyDown(SDL_Keycode key) { 
        switch (key) {
        case SDLK_w: wPressed = true; inputEvent.Publish("w_down"); break;
        case SDLK_a: aPressed = true; inputEvent.Publish("a_down"); break;
        case SDLK_s: sPressed = true; inputEvent.Publish("s_down"); break;
        case SDLK_d: dPressed = true; inputEvent.Publish("d_down"); break;
        case SDLK_SPACE: spacePressed = true; break;
        default: break;
        }
    }

    void handleKeyUp(SDL_Keycode key) {
        switch (key) {
        case SDLK_w: wPressed = false; inputEvent.Publish("w_up"); break;
        case SDLK_a: aPressed = false; inputEvent.Publish("a_up"); break;
        case SDLK_s: sPressed = false; inputEvent.Publish("s_up"); break;
        case SDLK_d: dPressed = false; inputEvent.Publish("d_up"); break;
        case SDLK_SPACE: spacePressed = false; break;
        default: break;
        }
    }
};

class Event {
public: 
 

	// add Objects that subscribe to this instance of event 
	void Subscribe(Listener listener) {
		listeners.push_back(listener); 
	} 
	
	// propagate Messages 
	void Publish(const std::string& message) {
		for (const auto& listener : listeners) {
			listener(message); 
		}
	}

private: 
	std::vector<Listener> listeners; 
};  




