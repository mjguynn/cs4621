#pragma once

#include <SDL.h>

// Abstract interface for app modes
class IApp {
public:
    virtual ~IApp() noexcept = 0;
    // Called when the app was just switched to
    // This STRICTLY affects OpenGL rendering and Imgui state
    virtual void activate() = 0;
    // Called right before we switch to a different app
    // This STRICTLY affects OpenGL rendering and Imgui state
    virtual void deactivate() = 0;
    virtual void process_event(const SDL_Event& event) = 0;
    virtual void process_frame(float deltaTime) = 0;
    virtual void render(ivec2 viewportSize) = 0;
    virtual void run_ui() = 0;
};
inline IApp::~IApp() {}

// The reason this is moved outside of main is to enforce proper cleanup order
// Right now we have a bunch of objects with destructors that do OpenGL API calls...
// but if we stick them in main, those destructors will run after we've already
// closed the OpenGL context!
// ... also it's nice to seperate boilerplate from app logic
namespace terrapainter {
    void run(SDL_Window* window);
}