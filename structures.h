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

// -------------------------------------------------------------
// STRUCTS DEFINITION
// -------------------------------------------------------------

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
};

enum RepairState {
    FLOATING,       // Debris floating in space
    PICKED_UP,      // Carried by the drone
    INSPECTING,     // Zoomed in for inspection & repair (M3 Scaling)
    REPAIRED,       // Repaired, ready for docking
    ATTACHED        // Locked into the space station dock
};

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

struct Drone {
    Vec3 position;
    Vec3 rotation;             // Drone orientation (yaw, etc.)
    float speed;
    int carriedPart;           // -1 if none, 0-4 index of carried part
    float thrusterIntensity;   // For thruster glow & particle generation
    float armRotation;         // Angle of robot manipulator arms
};

struct Particle {
    Vec3 pos;
    Vec3 vel;
    float r, g, b, a;
    int life;
    int maxLife;
    float size;
};

struct Scanner {
    bool active;
    float nearClip;
    float farClip;             // Adjustable far clip plane (Member 5 Demo)
    int viewportX, viewportY;
    int viewportW, viewportH;
};

struct GameState {
    int currentLevel;          // 1 = Cockpit, 2 = Engine (requires rotation), 3 = Solar Panel (rotation + scale/repair), 4 = Fuel Tank & Cargo
    int partsAttached;
    bool missionComplete;
    float elapsedTime;
    float zoomFactor;          // Zoom/Scale factor during inspection (Member 3 Demo)
    float zoomLerp;            // Smooth inspection zoom interpolation
    bool zBufferEnabled;       // Toggle depth test (Member 4 Demo)
    bool isLaunching;          // Launch animation trigger
    float launchTimer;
    int selectedPartIndex;     // Index of debris currently selected or carried
    int score;
    bool isInspecting;         // True if in scaling zoom-repair UI
    float algorithmActiveTimer[5]; // Active display indicators for M1-M5
    std::string feedbackMessage;   // Gameplay feedback warnings
    float feedbackTimer;           // Display duration for feedback warning
};

// -------------------------------------------------------------
// EXTERNAL GLOBALS DECLARATION (To be defined in main.cpp)
// -------------------------------------------------------------
extern int windowWidth;
extern int windowHeight;
extern std::vector<SpacePart> parts;
extern Drone drone;
extern std::vector<Particle> particles;
extern Scanner scanner;
extern GameState gameState;
extern bool keyStates[256];
extern bool specialKeyStates[256];

#endif
