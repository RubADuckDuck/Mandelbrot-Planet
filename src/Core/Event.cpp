#include "Network/NetworkMessage.h"
#include "Core/PlayerDirection.h" 
#include "Utils/LOG.h"

// Public method to get the singleton instance

EventDispatcher& EventDispatcher::GetInstance() {
    static EventDispatcher instance; // Guaranteed to be initialized only once
    return instance;
}

void EventDispatcher::Subscribe(Listener* ptrListener) {
    std::lock_guard<std::mutex> lock(mutex);
    if (std::find(listeners.begin(), listeners.end(), ptrListener) == listeners.end()) {
        listeners.push_back(ptrListener);
    }
}

void EventDispatcher::Subscribe(ItemListener* ptrItemListener) {
    std::lock_guard<std::mutex> lock(mutex);
    if (std::find(itemListeners.begin(), itemListeners.end(), ptrItemListener) == itemListeners.end()) {
        itemListeners.push_back(ptrItemListener);
    }
}

void EventDispatcher::Subscribe(const Tag tag, Listener* ptrListener) {
    std::lock_guard<std::mutex> lock(mutex);
    auto& tagListeners = tag2Listener[tag];
    if (std::find(tagListeners.begin(), tagListeners.end(), ptrListener) == tagListeners.end()) {
        tagListeners.push_back(ptrListener);
    }
    // Subscribe(ptrListener); // Add to general listeners as well // currently blocked
}

void EventDispatcher::Publish(const std::vector<uint8_t> message) {
    // std::cout << message << " triggered" << std::endl;

    // std::lock_guard<std::mutex> lock(mutex);
    for (const auto& ptrListener : listeners) {
        try {
            (*ptrListener)(message);
        }
        catch (const std::exception& e) {
            std::cerr << "Listener exception: " << e.what() << std::endl;
        }
    }
}

void EventDispatcher::Publish(const Tag tag, const std::vector<uint8_t> message) {
    // std::cout << tag << "::" << message << " triggered" << std::endl;

    // std::lock_guard<std::mutex> lock(mutex);
    auto it = tag2Listener.find(tag);

    if (it != tag2Listener.end()) {
        for (const auto& ptrListener : it->second) {
            try {
                LOG(LOG_DEBUG, "Triggering onEventFunctions");
                (*ptrListener)(message);
            }
            catch (const std::exception& e) {
                std::cerr << "Listener exception: " << e.what() << std::endl;
            }
        }
    }
}

void EventDispatcher::Publish(InteractionInfo* interactionInfo) {
    // std::lock_guard<std::mutex> lock(mutex);

    for (const ItemListener* ptrItemListener : itemListeners) {
        try {
            (*ptrItemListener)(interactionInfo);
        }
        catch (const std::exception& e) {
            std::cerr << "ItemListener exception: " << e.what() << std::endl;
        }
    }
}

void EventDispatcher::Unsubscribe(Listener* ptrListener) {
    std::lock_guard<std::mutex> lock(mutex);
    listeners.erase(std::remove(listeners.begin(), listeners.end(), ptrListener), listeners.end());
}

void EventDispatcher::Unsubscribe(const Tag tag, Listener* ptrListener) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = tag2Listener.find(tag);
    if (it != tag2Listener.end()) {
        it->second.erase(std::remove(it->second.begin(), it->second.end(), ptrListener), it->second.end());
        if (it->second.empty()) {
            tag2Listener.erase(it);
        }
    }
}

void EventDispatcher::Unsubscribe(ItemListener* ptrItemListener) {
    std::lock_guard<std::mutex> lock(mutex);
    itemListeners.erase(std::remove(itemListeners.begin(), itemListeners.end(), ptrItemListener), itemListeners.end());
}

InputHandler::InputHandler() : quit(false) {
    wPressed = aPressed = sPressed = dPressed = spacePressed = false;
    quit = false;



    //if (SDL_Init(SDL_INIT_VIDEO) != 0) {    

    //    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
    //    throw std::runtime_error("Failed to initialize SDL");
    //}

    SDL_SetEventFilter(NULL, NULL); // Remove any custom event filter
}

InputHandler::~InputHandler() {
    SDL_Quit();
}

void InputHandler::pollEvents() { // this function will get called every update to check for input
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

bool InputHandler::isQuit() const { return quit; }

bool InputHandler::isWPressed() const { return wPressed; }

bool InputHandler::isAPressed() const { return aPressed; }

bool InputHandler::isSPressed() const { return sPressed; }

bool InputHandler::isDPressed() const { return dPressed; }

bool InputHandler::isSpacePressed() const { return spacePressed; }



void InputHandler::Subscribe(Listener* ptrListener) {
    // Get the singleton instance
    EventDispatcher& dispatcher = EventDispatcher::GetInstance();
    dispatcher.Subscribe(this->tag, ptrListener);
}

void InputHandler::Publish(Direction direction)
{
    // create player input message 
    PlayerInputMessage playerInputMsg = PlayerInputMessage(direction, 0);  

    LOG(LOG_INFO, "Publishing Event Message through event dispatcher"); 

    this->Publish(NetworkCodec::Encode(&playerInputMsg)); 
}

void InputHandler::Publish(const std::vector<uint8_t> msg) {
    // Get the singleton instance
    EventDispatcher& dispatcher = EventDispatcher::GetInstance();

    dispatcher.Publish(this->tag, msg);
}

void InputHandler::handleKeyDown(SDL_Keycode key) {
    LOG(LOG_INFO, "Keypressed");

    //switch (key) {
    //case SDLK_w: wPressed = true; this->Publish("w_down"); break;
    //case SDLK_a: aPressed = true; this->Publish("a_down"); break;
    //case SDLK_s: sPressed = true; this->Publish("s_down"); break;
    //case SDLK_d: dPressed = true; this->Publish("d_down"); break;
    //case SDLK_SPACE: spacePressed = true; this->Publish("space_down"); break;
    //default: break;
    //}
}

void InputHandler::handleKeyUp(SDL_Keycode key) {
    // Get the singleton instance
    EventDispatcher& dispatcher = EventDispatcher::GetInstance();


    switch (key) {
    case SDLK_w: wPressed = false; this->Publish(Direction::UP); break;
    case SDLK_a: aPressed = false; this->Publish(Direction::LEFT); break;
    case SDLK_s: sPressed = false; this->Publish(Direction::DOWN); break;
    case SDLK_d: dPressed = false; this->Publish(Direction::RIGHT); break;
    case SDLK_SPACE: spacePressed = false; this->Publish(Direction::IDLE); break;
    default: break;
    }
}
