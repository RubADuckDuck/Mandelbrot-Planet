#pragma once

#include <functional>
#include <string> 
#include <vector>
#include <SDL.h> 
#include <iostream>
#include <unordered_map>
#include <mutex>
#include "LOG.h"
#include "InteractionInfo.h"
#include <map>

class Item;
class GameObject;

// below Code is defining a proxy named (left) for the (right), 
// sort of like assigning variables but with classnames I guess  
using Listener = std::function<void(const std::vector<uint8_t>)>;
using ItemListener = std::function<void(InteractionInfo*)>; // args are (who, what, where1, where2)


enum class Tag {
    UDP,
    TCP,
    USER_INPUT,
};

class EventDispatcher {
public:
    // Public method to get the singleton instance
    static EventDispatcher& GetInstance();

    void Subscribe(Listener* ptrListener);

    void Subscribe(ItemListener* ptrItemListener);

    void Subscribe(const Tag tag, Listener* ptrListener);

    void Publish(const std::vector<uint8_t> message);

    void Publish(const Tag tag, const std::vector<uint8_t> message);

    void Publish(InteractionInfo* interactionInfo);

    void Unsubscribe(Listener* ptrListener);

    void Unsubscribe(const Tag tag, Listener* ptrListener);

    void Unsubscribe(ItemListener* ptrItemListener);

private:
    // Private constructor to prevent direct instantiation
    EventDispatcher() = default;

    // Delete copy constructor and assignment operator
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;

    std::vector<Listener*> listeners;
    std::map<Tag, std::vector<Listener*>> tag2Listener;


    std::vector<ItemListener*> itemListeners;
    // std::map<std::string, std::vector<ItemListener*>> tag2ItemListeners; // itemListeners are by iteself specific
    std::mutex mutex;
};


class GameEngine; 

class InputHandler {
public:
    Tag tag = Tag::USER_INPUT; 

    GameEngine* gameEngine;

    InputHandler();

    ~InputHandler();

    void pollEvents();

    bool isQuit() const;
    bool isWPressed() const;
    bool isAPressed() const;
    bool isSPressed() const;
    bool isDPressed() const;
    bool isSpacePressed() const;

    void Subscribe(Listener* ptrListener);

    void Publish(Direction direction);  

    void Publish(const std::vector<uint8_t> msg);


private:
    bool quit;
    bool wPressed, aPressed, sPressed, dPressed, spacePressed;



    void handleKeyDown(SDL_Keycode key);

    void handleKeyUp(SDL_Keycode key);
};






