#ifndef STRUCTURES_H
#define STRUCTURES_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <GL/glu.h>

// Structs Definition

// 3D vector helper struct
struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
};

// State machine for space parts repair and assembly
enum RepairState {
    FLOATING,       // Debris floating in space
    PICKED_UP,      // Carried by the drone
    INSPECTING,     // Zoomed in for inspection & repair ( Scaling)
    REPAIRED,       // Repaired, ready for docking
    ATTACHED        // Locked into the space station dock
};

// Structure for space debris parts
struct SpacePart {
    int id;                    // 0 = Cockpit, 1 = Fuel Tank, 2 = Engine, 3 = Solar Panel, 4 = Cargo Container
    std::string name;
    Vec3 position;             // Current world position
    Vec3 rotation;             // Current rotation angles (pitch, yaw, roll)
    Vec3 targetRotation;       // Target rotation required for attachment
    Vec3 dockSlot;             // Position on dock when attached
    float bobOffset;           // Wave phase offset for floating bob animation
    float spinSpeed;           // Speed of slow auto-spin when floating
    RepairState state;
    bool isRepaired;           // True if repaired during inspection
    float color[3];            // RGB color values
    float scale;               // Base draw scale
    float damagePercent;       // Damage metric for display
};

// Structure for the player's navigation drone
struct Drone {
    Vec3 position;
    Vec3 rotation;             // Drone orientation (yaw, etc.)
    float speed;
    int carriedPart;           // -1 if none, 0-4 index of carried part
    float thrusterIntensity;   // For thruster glow & particle generation
    float armRotation;         // Angle of robot manipulator arms
};

// Structure for engine exhaust particle system
struct Particle {
    Vec3 pos;
    Vec3 vel;
    float r, g, b, a;
    int life;
    int maxLife;
    float size;
};

// Structure for radar scanner viewport
struct Scanner {
    bool active;
    float nearClip;
    float farClip;             // Adjustable far clip plane (Clipping Demo)
    int viewportX, viewportY;
    int viewportW, viewportH;
};

// Structure to track game progress, level, and algorithm variables
struct GameState {
    int currentLevel;          // 1 = Cockpit, 2 = Engine (requires rotation), 3 = Solar Panel (rotation + scale/repair), 4 = Fuel Tank & Cargo
    int partsAttached;
    bool missionComplete;
    float elapsedTime;
    float zoomFactor;          // Zoom/Scale factor during inspection (Scaling Demo)
    float zoomLerp;            // Smooth inspection zoom interpolation
    bool zBufferEnabled;       // Toggle depth test (Depth Test Demo)
    bool isLaunching;          // Launch animation trigger
    float launchTimer;
    int selectedPartIndex;     // Index of debris currently selected or carried
    int score;
    bool isInspecting;         // True if in scaling zoom-repair UI
    float algorithmActiveTimer[5]; // Active display indicators for algorithms
    std::string feedbackMessage;   // Gameplay feedback warnings
    float feedbackTimer;           // Display duration for feedback warning
};

// External Globals Declaration (To be defined in main.cpp)

extern int windowWidth;
extern int windowHeight;
extern std::vector<SpacePart> parts;
extern Drone drone;
extern std::vector<Particle> particles;
extern Scanner scanner;
extern GameState gameState;
extern bool keyStates[256];
extern bool specialKeyStates[256];
extern int cameraMode;
extern Vec3 cameraPos;
extern Vec3 cameraTarget;

// Rendering helper utilities
void renderText(float x, float y, const std::string& text, void* font, float r = 1.0f, float g = 1.0f, float b = 1.0f);
void renderText3D(float x, float y, float z, const std::string& text, void* font);
void renderPanel(float x, float y, float w, float h, float r, float g, float b, float a);

// Particle helper utilities
void addNaniteParticles(Vec3 origin);
void addThrusterParticles();

#endif
