#pragma once
#include <functional>
#include <string> 
#include <vector>
#include <SDL.h> 
#include <iostream>
#include <unordered_map>
#include <mutex>
#include "LOG.h"

class Item;

// below Code is defining a proxy named (left) for the (right), 
// sort of like assigning variables but with classnames I guess  
using Listener = std::function<void(const std::string&)>;
using ItemListener = std::function<void(Item*)>;



class EventDispatcher {
public:
    // Public method to get the singleton instance
    static EventDispatcher& GetInstance() {
        static EventDispatcher instance; // Guaranteed to be initialized only once
        return instance;
    }

    void Subscribe(Listener* ptrListener) {
        std::lock_guard<std::mutex> lock(mutex);
        if (std::find(listeners.begin(), listeners.end(), ptrListener) == listeners.end()) {
            listeners.push_back(ptrListener);
        }
    }

    void Subscribe(ItemListener* ptrItemListener) {
        std::lock_guard<std::mutex> lock(mutex); 
        if (std::find(itemListeners.begin(), itemListeners.end(), ptrItemListener) == itemListeners.end()) {
            itemListeners.push_back(ptrItemListener); 
        }
    }

    void Subscribe(const std::string& tag, Listener* ptrListener) {
        std::lock_guard<std::mutex> lock(mutex);
        auto& tagListeners = tag2Listener[tag];
        if (std::find(tagListeners.begin(), tagListeners.end(), ptrListener) == tagListeners.end()) {
            tagListeners.push_back(ptrListener);
        }
        // Subscribe(ptrListener); // Add to general listeners as well // currently blocked
    }

    void Publish(const std::string& message) { 
        // std::cout << message << " triggered" << std::endl;

        std::lock_guard<std::mutex> lock(mutex);
        for (const auto& ptrListener : listeners) {
            try {
                (*ptrListener)(message);
            }
            catch (const std::exception& e) {
                std::cerr << "Listener exception: " << e.what() << std::endl;
            }
        }
    }

    void Publish(const std::string& tag, const std::string& message) {
        // std::cout << tag << "::" << message << " triggered" << std::endl;

        std::lock_guard<std::mutex> lock(mutex);
        auto it = tag2Listener.find(tag); 

        if (it != tag2Listener.end()) {
            for (const auto& ptrListener : it->second) {
                try {
                    LOG(LOG_DEBUG, "Triggering onEventFunctions")
                    (*ptrListener)(message);
                }
                catch (const std::exception& e) {
                    std::cerr << "Listener exception: " << e.what() << std::endl;
                }
            }
        }
    }

    void Publish(Item* item) {
        std::lock_guard<std::mutex> lock(mutex);

        for (const auto& ptrItemListener : itemListeners) {
            try {
                (*ptrItemListener)(item);
            }
            catch (const std::exception& e) {
                std::cerr << "ItemListener exception: " << e.what() << std::endl;
            }
        }
    }

    void Unsubscribe(Listener* ptrListener) {
        std::lock_guard<std::mutex> lock(mutex);
        listeners.erase(std::remove(listeners.begin(), listeners.end(), ptrListener), listeners.end());
    } 

    void Unsubscribe(const std::string& tag, Listener* ptrListener) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = tag2Listener.find(tag);
        if (it != tag2Listener.end()) {
            it->second.erase(std::remove(it->second.begin(), it->second.end(), ptrListener), it->second.end());
            if (it->second.empty()) {
                tag2Listener.erase(it);
            }
        }
    }

    void Unsubscribe(ItemListener* ptrItemListener) {
        std::lock_guard<std::mutex> lock(mutex);
        itemListeners.erase(std::remove(itemListeners.begin(), itemListeners.end(), ptrItemListener), itemListeners.end());
    }

private:
    // Private constructor to prevent direct instantiation
    EventDispatcher() = default;

    // Delete copy constructor and assignment operator
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;

    std::vector<Listener*> listeners;
    std::map<std::string, std::vector<Listener*>> tag2Listener;


    std::vector<ItemListener*> itemListeners; 
    // std::map<std::string, std::vector<ItemListener*>> tag2ItemListeners; // itemListeners are by iteself specific
    std::mutex mutex;
};



class InputHandler {
public:
    std::string tag = "keyInput"; 

    InputHandler() : quit(false) {
        wPressed = aPressed = sPressed = dPressed = spacePressed = false;
        quit = false;



        //if (SDL_Init(SDL_INIT_VIDEO) != 0) {    

        //    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        //    throw std::runtime_error("Failed to initialize SDL");
        //}

        SDL_SetEventFilter(NULL, NULL); // Remove any custom event filter
    }

    ~InputHandler() {
        SDL_Quit();
    }

    void pollEvents() { // this function will get called every update to check for input
        // Reset key states
        wPressed = aPressed = sPressed = dPressed = spacePressed = false;
        // LOG(LOG_INFO, "Polling events");

        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // LOG(LOG_INFO, "Event triggered");

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

    void Subscribe(Listener* ptrListener) {
        // Get the singleton instance
        EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Subscribe(this->tag, ptrListener);
    }

    void Publish(const std::string& msg) {
        // Get the singleton instance
        EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        LOG(LOG_INFO, msg);
        dispatcher.Publish(this->tag, msg);
    }

private:
    bool quit;
    bool wPressed, aPressed, sPressed, dPressed, spacePressed; 



    void handleKeyDown(SDL_Keycode key) { 
        LOG(LOG_INFO, "Keypressed");

        switch (key) {
        case SDLK_w: wPressed = true; this->Publish("w_down"); break;
        case SDLK_a: aPressed = true; this->Publish("a_down"); break;
        case SDLK_s: sPressed = true; this->Publish("s_down"); break;
        case SDLK_d: dPressed = true; this->Publish("d_down"); break;
        case SDLK_SPACE: spacePressed = true; this->Publish("space_down"); break;
        default: break;
        }
    }

    void handleKeyUp(SDL_Keycode key) {
        // Get the singleton instance
        EventDispatcher& dispatcher = EventDispatcher::GetInstance(); 

        switch (key) {
        case SDLK_w: wPressed = false; this->Publish("w_up"); break;
        case SDLK_a: aPressed = false; this->Publish("a_up"); break;
        case SDLK_s: sPressed = false; this->Publish("s_up"); break;
        case SDLK_d: dPressed = false; this->Publish("d_up"); break;
        case SDLK_SPACE: spacePressed = false; this->Publish("space_up"); break;
        default: break;
        }
    }
};






