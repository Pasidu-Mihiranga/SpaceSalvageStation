#include "structures.h"
#include "translation.h"
#include "rotation.h"
#include "scaling.h"
#include "zbuffer.h"
#include "clipping.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <cstdlib>
#include <ctime>

// -------------------------------------------------------------
// DEFINITION OF GLOBALS (Declared in structures.h)
// -------------------------------------------------------------
int windowWidth = 1024;
int windowHeight = 768;

std::vector<SpacePart> parts;
Drone drone;
std::vector<Particle> particles;
Scanner scanner;
GameState gameState;

// Starfield positions
std::vector<Vec3> stars;
const int NUM_STARS = 400;

// Camera configurations
int cameraMode = 0; // 0 = Third Person, 1 = Top-Down, 2 = Cockpit View
Vec3 cameraPos;
Vec3 cameraTarget;

// Keyboard state tracking for smooth movement
bool keyStates[256] = {};
bool specialKeyStates[256] = {};

// Light configurations
GLfloat lightPos[] = { 10.0f, 25.0f, 10.0f, 1.0f };
GLfloat lightAmb[] = { 0.15f, 0.15f, 0.2f, 1.0f };
GLfloat lightDiff[] = { 0.7f, 0.7f, 0.8f, 1.0f };
GLfloat lightSpec[] = { 0.9f, 0.9f, 0.9f, 1.0f };

// -------------------------------------------------------------
// FUNCTION PROFILES
// -------------------------------------------------------------
void initGame();
void initStars();
void update(int value);
void drawScene(bool isScannerView);
void drawHUD();
void drawScannerViewport();
void drawDrone();
void drawDock();
void drawShipHull();
void drawPart(int id, bool isInspecting);
void drawStarfield();
void drawParticles();
void addThrusterParticles();
void addNaniteParticles(Vec3 origin);
void setupCamera(bool isScannerView);
void renderText(float x, float y, const std::string& text, void* font, float r = 1.0f, float g = 1.0f, float b = 1.0f);
void renderText3D(float x, float y, float z, const std::string& text, void* font);
void renderPanel(float x, float y, float w, float h, float r, float g, float b, float a);
void keyboardDown(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void specialDown(int key, int x, int y);
void specialUp(int key, int x, int y);
void reshape(int w, int h);
void display();

// -------------------------------------------------------------
// INITIALIZATION
// -------------------------------------------------------------
void initGame() {
    gameState.currentLevel = 1;
    gameState.partsAttached = 0;
    gameState.missionComplete = false;
    gameState.elapsedTime = 0.0f;
    gameState.zoomFactor = 1.0f;
    gameState.zoomLerp = 0.0f;
    gameState.zBufferEnabled = true;
    gameState.isLaunching = false;
    gameState.launchTimer = 0.0f;
    gameState.selectedPartIndex = -1;
    gameState.score = 0;
    gameState.isInspecting = false;

    gameState.feedbackMessage = "";
    gameState.feedbackTimer = 0.0f;

    for (int i = 0; i < 5; ++i) {
        gameState.algorithmActiveTimer[i] = 0.0f;
    }

    // Initialize Drone
    drone.position = Vec3(0.0f, 2.0f, 12.0f);
    drone.rotation = Vec3(0.0f, 0.0f, 0.0f);
    drone.speed = 0.22f;
    drone.carriedPart = -1;
    drone.thrusterIntensity = 0.0f;
    drone.armRotation = 0.0f;

    // Initialize scanner
    scanner.active = true;
    scanner.nearClip = 1.0f;
    scanner.farClip = 25.0f; // Limit to show clipping of distant debris (M5)
    scanner.viewportX = 20;
    scanner.viewportY = 115;
    scanner.viewportW = 240;
    scanner.viewportH = 240;

    // Initialize 5 Spacecraft Parts (Damaged status)
    parts.clear();

    // 0: Cockpit - Level 1 Target
    SpacePart cockpit;
    cockpit.id = 0;
    cockpit.name = "Command Cockpit";
    cockpit.position = Vec3(-7.0f, 1.5f, 5.0f);
    cockpit.rotation = Vec3(0.0f, 45.0f, 0.0f);
    cockpit.targetRotation = Vec3(0.0f, 0.0f, 0.0f);
    cockpit.dockSlot = Vec3(0.0f, 1.6f, -1.0f); // Center front on dock
    cockpit.bobOffset = 0.0f;
    cockpit.spinSpeed = 0.4f;
    cockpit.state = FLOATING;
    cockpit.isRepaired = true; // Cockpit is pre-repaired for Level 1 training
    cockpit.color[0] = 0.2f; cockpit.color[1] = 0.6f; cockpit.color[2] = 0.8f; // Blue Canopy
    cockpit.scale = 1.0f;
    cockpit.damagePercent = 0.0f;
    parts.push_back(cockpit);

    // 1: Engine - Level 2 Target
    SpacePart engine;
    engine.id = 1;
    engine.name = "Thruster Engine";
    engine.position = Vec3(7.0f, 0.5f, 4.0f);
    engine.rotation = Vec3(40.0f, 120.0f, 0.0f); // Misaligned rotation
    engine.targetRotation = Vec3(0.0f, 180.0f, 0.0f); // Must rotate to face backward
    engine.dockSlot = Vec3(0.0f, 1.6f, -7.0f); // Center back on dock
    engine.bobOffset = 2.0f;
    engine.spinSpeed = 0.8f;
    engine.state = FLOATING;
    engine.isRepaired = true; // Engine starts repaired, rotation is the main challenge
    engine.color[0] = 0.8f; engine.color[1] = 0.35f; engine.color[2] = 0.1f; // Orange/Grey
    engine.scale = 1.0f;
    engine.damagePercent = 0.0f;
    parts.push_back(engine);

    // 2: Solar Panel - Level 3 Target
    SpacePart panel;
    panel.id = 2;
    panel.name = "Solar Array Wing";
    panel.position = Vec3(-9.0f, 3.0f, -4.0f);
    panel.rotation = Vec3(-10.0f, 10.0f, 50.0f);
    panel.targetRotation = Vec3(0.0f, 90.0f, 0.0f); // Alignment angle
    panel.dockSlot = Vec3(-4.0f, 2.0f, -4.0f); // Left slot
    panel.bobOffset = 4.0f;
    panel.spinSpeed = 0.5f;
    panel.state = FLOATING;
    panel.isRepaired = false; // Damaged! Needs Zoom scaling & Repair
    panel.color[0] = 0.9f; panel.color[1] = 0.2f; panel.color[2] = 0.2f; // Damaged color: Red
    panel.scale = 1.0f;
    panel.damagePercent = 78.0f;
    parts.push_back(panel);

    // 3: Fuel Tank - Level 4 Target
    SpacePart tank;
    tank.id = 3;
    tank.name = "Fuel Storage Core";
    tank.position = Vec3(6.0f, -2.0f, -6.0f);
    tank.rotation = Vec3(80.0f, 250.0f, 10.0f);
    tank.targetRotation = Vec3(90.0f, 0.0f, 0.0f);
    tank.dockSlot = Vec3(0.0f, 1.5f, -4.0f); // Center core
    tank.bobOffset = 1.5f;
    tank.spinSpeed = 0.6f;
    tank.state = FLOATING;
    tank.isRepaired = false; // Damaged!
    tank.color[0] = 0.9f; tank.color[1] = 0.2f; tank.color[2] = 0.2f; // Damaged color: Red
    tank.scale = 0.9f;
    tank.damagePercent = 55.0f;
    parts.push_back(tank);

    // 4: Cargo Container - Level 4 Target
    SpacePart cargo;
    cargo.id = 4;
    cargo.name = "Shielded Cargo Pod";
    cargo.position = Vec3(9.0f, 2.0f, -1.0f);
    cargo.rotation = Vec3(15.0f, 45.0f, 95.0f);
    cargo.targetRotation = Vec3(0.0f, 0.0f, 0.0f);
    cargo.dockSlot = Vec3(4.0f, 2.0f, -4.0f); // Right slot
    cargo.bobOffset = 3.2f;
    cargo.spinSpeed = 0.3f;
    cargo.state = FLOATING;
    cargo.isRepaired = false; // Damaged!
    cargo.color[0] = 0.9f; cargo.color[1] = 0.2f; cargo.color[2] = 0.2f; // Damaged color: Red
    cargo.scale = 0.9f;
    cargo.damagePercent = 42.0f;
    parts.push_back(cargo);

    initStars();
    particles.clear();
}

void initStars() {
    stars.clear();
    for (int i = 0; i < NUM_STARS; ++i) {
        float theta = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
        float phi = acos(2.0f * ((float)rand() / RAND_MAX) - 1.0f);
        float radius = 120.0f + ((float)rand() / RAND_MAX) * 80.0f; // Outer shell range

        float x = radius * sin(phi) * cos(theta);
        float y = radius * sin(phi) * sin(theta);
        float z = radius * cos(phi);
        stars.push_back(Vec3(x, y, z));
    }
}

// -------------------------------------------------------------
// KEYBOARD & MOVEMENT PROCESSING
// -------------------------------------------------------------
void keyboardDown(unsigned char key, int x, int y) {
    keyStates[key] = true;

    // Single-trigger actions
    if (key == 'c' || key == 'C') {
        scanner.active = !scanner.active;
    }
    else if (key == 'v' || key == 'V') {
        cameraMode = (cameraMode + 1) % 3;
    }
    else if (key == ' ') {
        // Spacebar: Pick up / Drop logic
        if (drone.carriedPart == -1) {
            // Try picking up the active level part
            for (size_t i = 0; i < parts.size(); ++i) {
                if (parts[i].state == FLOATING) {
                    // Level restrictions
                    if (gameState.currentLevel == 1 && i != 0) continue;
                    if (gameState.currentLevel == 2 && i != 1) continue;
                    if (gameState.currentLevel == 3 && i != 2) continue;
                    if (gameState.currentLevel == 4 && i != 3 && i != 4) continue;

                    // Distance check
                    float dx = parts[i].position.x - drone.position.x;
                    float dy = parts[i].position.y - drone.position.y;
                    float dz = parts[i].position.z - drone.position.z;
                    float dist = sqrt(dx*dx + dy*dy + dz*dz);

                    if (dist < 3.8f) {
                        // Pick it up!
                        drone.carriedPart = i;
                        parts[i].state = PICKED_UP;
                        gameState.selectedPartIndex = i;
                        gameState.zoomFactor = 1.0f; // Reset zoom
                        gameState.zoomLerp = 0.0f;
                        gameState.isInspecting = false;
                        break;
                    }
                }
            }
        } else {
            // Drop the part & exit inspection if active
            int pId = drone.carriedPart;
            parts[pId].state = FLOATING;
            drone.carriedPart = -1;
            gameState.selectedPartIndex = -1;
            gameState.zoomFactor = 1.0f;
            gameState.zoomLerp = 0.0f;
            gameState.isInspecting = false;
        }
    }
    else if (key == 13) {
        // Enter: Dock / Attach logic
        if (drone.carriedPart == -1) {
            gameState.feedbackMessage = "NO SPACECRAFT PART CARRIED!";
            gameState.feedbackTimer = 3.0f;
        }
        else if (gameState.isInspecting) {
            gameState.feedbackMessage = "CANNOT DOCK WHILE IN INSPECTION MODE!";
            gameState.feedbackTimer = 3.0f;
        }
        else {
            int pId = drone.carriedPart;
            SpacePart& part = parts[pId];

            // Verify alignment (Member 2 Rotation Check)
            float pitchDiff = fmod(fabs(part.rotation.x - part.targetRotation.x), 360.0f);
            float yawDiff = fmod(fabs(part.rotation.y - part.targetRotation.y), 360.0f);
            float rollDiff = fmod(fabs(part.rotation.z - part.targetRotation.z), 360.0f);

            // Wrap angles to shortest path
            if (pitchDiff > 180.0f) pitchDiff = 360.0f - pitchDiff;
            if (yawDiff > 180.0f) yawDiff = 360.0f - yawDiff;
            if (rollDiff > 180.0f) rollDiff = 360.0f - rollDiff;

            bool isAligned = (pitchDiff < 30.0f && yawDiff < 30.0f);

            // Distance to docking slot check (use DRONE position, not carried part)
            float dx = drone.position.x - part.dockSlot.x;
            float dy = drone.position.y - part.dockSlot.y;
            float dz = drone.position.z - part.dockSlot.z;
            float dist = sqrt(dx*dx + dy*dy + dz*dz);

            if (dist < 8.0f) {
                if (!part.isRepaired) {
                    gameState.feedbackMessage = "PART IS DAMAGED! REPAIR FIRST ('R').";
                    gameState.feedbackTimer = 3.0f;
                } else if (!isAligned) {
                    gameState.feedbackMessage = "MISALIGNED! ROTATE WITH ARROW KEYS.";
                    gameState.feedbackTimer = 3.0f;
                } else {
                    // Snap docking animation
                    part.state = ATTACHED;
                    part.position = part.dockSlot;
                    part.rotation = part.targetRotation;
                    drone.carriedPart = -1;
                    gameState.selectedPartIndex = -1;
                    gameState.partsAttached++;
                    gameState.score += 250;
                    gameState.feedbackMessage = "PART SECURED & DOCKED SUCCESSFULLY!";
                    gameState.feedbackTimer = 3.0f;

                    // Progress level
                    if (gameState.currentLevel == 1 && gameState.partsAttached == 1) {
                        gameState.currentLevel = 2;
                    } else if (gameState.currentLevel == 2 && gameState.partsAttached == 2) {
                        gameState.currentLevel = 3;
                    } else if (gameState.currentLevel == 3 && gameState.partsAttached == 3) {
                        gameState.currentLevel = 4;
                    } else if (gameState.currentLevel == 4 && gameState.partsAttached == 5) {
                        gameState.isLaunching = true;
                        gameState.launchTimer = 0.0f;
                    }
                }
            } else {
                gameState.feedbackMessage = "TOO FAR FROM DOCK SLOT! GET CLOSER.";
                gameState.feedbackTimer = 3.0f;
            }
        }
    }
    else if (key == 'r' || key == 'R') {
        // Repair/Inspect button (Member 3 Scaling Repair loop)
        if (drone.carriedPart != -1) {
            int pId = drone.carriedPart;
            if (!gameState.isInspecting) {
                // Engage Inspection Mode -> Zoom In Scaling
                gameState.isInspecting = true;
                parts[pId].state = INSPECTING;
            } else {
                // If already inspecting, check if we can perform repair
                if (!parts[pId].isRepaired && gameState.zoomFactor >= 2.0f) {
                    parts[pId].isRepaired = true;
                    parts[pId].damagePercent = 0.0f;
                    // Color updates
                    if (pId == 2) { // Solar panel: Space Blue
                        parts[pId].color[0] = 0.1f; parts[pId].color[1] = 0.45f; parts[pId].color[2] = 0.85f;
                    } else if (pId == 3) { // Fuel Tank: Glowing Green
                        parts[pId].color[0] = 0.2f; parts[pId].color[1] = 0.85f; parts[pId].color[2] = 0.35f;
                    } else if (pId == 4) { // Cargo: Solid Orange/Yellow
                        parts[pId].color[0] = 0.85f; parts[pId].color[1] = 0.65f; parts[pId].color[2] = 0.1f;
                    }
                    gameState.score += 150;
                    
                    // Nanite spark burst
                    addNaniteParticles(parts[pId].position);

                    // De-escalate and zoom back out automatically
                    gameState.isInspecting = false;
                    gameState.feedbackMessage = "REPAIR COMPLETE! RETRACTING SCANNER...";
                    gameState.feedbackTimer = 2.0f;
                } else {
                    // Manual cancel / exit inspection view (repaired parts or zoom not ready)
                    gameState.isInspecting = false;
                    gameState.feedbackMessage = "RETRACTING SCANNER INSPECT VIEW...";
                    gameState.feedbackTimer = 2.0f;
                }
            }
        }
    }
    // Lecturer Evaluation Secondary back-ups
    else if (key == 'z' || key == 'Z') {
        gameState.zBufferEnabled = !gameState.zBufferEnabled;
    }
    else if (key == '[') {
        scanner.farClip = fmaxf(5.0f, scanner.farClip - 1.0f);
    }
    else if (key == ']') {
        scanner.farClip = fminf(50.0f, scanner.farClip + 1.0f);
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

void specialDown(int key, int x, int y) {
    specialKeyStates[key] = true;

    // F1 - F3 evaluation keybindings
    if (key == GLUT_KEY_F1) {
        gameState.zBufferEnabled = !gameState.zBufferEnabled;
    }
    else if (key == GLUT_KEY_F2) {
        scanner.farClip = fmaxf(5.0f, scanner.farClip - 1.0f);
    }
    else if (key == GLUT_KEY_F3) {
        scanner.farClip = fminf(50.0f, scanner.farClip + 1.0f);
    }
}

void specialUp(int key, int x, int y) {
    specialKeyStates[key] = false;
}

void processInput() {
    if (gameState.isLaunching) return;

    // Member 1 Algorithm: 3D Translation controls
    // Moves the drone through 3D space
    float speed = drone.speed;
    float dx = 0.0f, dy = 0.0f, dz = 0.0f;
    bool moved = false;

    // Movement relative to drone orientation (yaw)
    float radYaw = drone.rotation.y * M_PI / 180.0f;

    if (keyStates['w'] || keyStates['W']) {
        dx -= sin(radYaw) * speed;
        dz -= cos(radYaw) * speed;
        moved = true;
    }
    if (keyStates['s'] || keyStates['S']) {
        dx += sin(radYaw) * speed;
        dz += cos(radYaw) * speed;
        moved = true;
    }
    if (keyStates['a'] || keyStates['A']) {
        dx -= cos(radYaw) * speed;
        dz += sin(radYaw) * speed;
        moved = true;
    }
    if (keyStates['d'] || keyStates['D']) {
        dx += cos(radYaw) * speed;
        dz -= sin(radYaw) * speed;
        moved = true;
    }
    if (keyStates['q'] || keyStates['Q']) {
        dy += speed; // Fly upward
        moved = true;
    }
    if (keyStates['e'] || keyStates['E']) {
        dy -= speed; // Fly downward
        moved = true;
    }

    // Member 1 Delegation: translateDrone
    if (dx != 0.0f || dy != 0.0f || dz != 0.0f) {
        translateDrone(drone, dx, dy, dz);
    }

    // Simple drone steering yaw control (J/L rotates yaw)
    if (keyStates['j'] || keyStates['J']) {
        drone.rotation.y += 2.2f;
        moved = true;
    }
    if (keyStates['l'] || keyStates['L']) {
        drone.rotation.y -= 2.2f;
        moved = true;
    }

    // Set thruster intensity for engine plumes
    if (moved) {
        drone.thrusterIntensity = fminf(1.0f, drone.thrusterIntensity + 0.12f);
        addThrusterParticles();
        gameState.algorithmActiveTimer[0] = 1.2f; // Active Translation
    } else {
        drone.thrusterIntensity = fmaxf(0.0f, drone.thrusterIntensity - 0.08f);
    }

    // Carry part translation (M1 Translation attachment logic)
    if (drone.carriedPart != -1) {
        int carriedId = drone.carriedPart;
        SpacePart& part = parts[carriedId];

        // Robotic arms movement when holding
        drone.armRotation = 45.0f; 
        gameState.algorithmActiveTimer[0] = 1.2f; // Active Translation

        if (part.state == PICKED_UP || part.state == INSPECTING) {
            // Member 1 Delegation: translateCarriedPart
            translateCarriedPart(drone, part, radYaw, gameState.zoomFactor, gameState.isInspecting);

            // Member 3 Delegation: updateInspectZoom
            updateInspectZoom(gameState.isInspecting, gameState.zoomLerp, gameState.zoomFactor);
            if (gameState.isInspecting) {
                gameState.algorithmActiveTimer[2] = 1.2f; // Active Scaling
            } else if (gameState.zoomLerp < 0.01f && part.state == INSPECTING) {
                part.state = PICKED_UP;
            }

            // Member 2 Delegation: rotatePartManual
            bool rotateLeft = specialKeyStates[GLUT_KEY_LEFT];
            bool rotateRight = specialKeyStates[GLUT_KEY_RIGHT];
            bool rotateUp = specialKeyStates[GLUT_KEY_UP];
            bool rotateDown = specialKeyStates[GLUT_KEY_DOWN];
            bool rotateRollLeft = keyStates['u'] || keyStates['U'];
            bool rotateRollRight = keyStates['o'] || keyStates['O'];
            
            if (rotateLeft || rotateRight || rotateUp || rotateDown || rotateRollLeft || rotateRollRight) {
                rotatePartManual(part, rotateLeft, rotateRight, rotateUp, rotateDown, 2.0f);
                gameState.algorithmActiveTimer[1] = 1.2f; // Active Rotation
            }

            // Dynamic Magnetic Snapping: If close to dock slot and alignment is within 30 degrees, snap it!
            float dx = drone.position.x - part.dockSlot.x;
            float dy = drone.position.y - part.dockSlot.y;
            float dz = drone.position.z - part.dockSlot.z;
            float dist = sqrt(dx*dx + dy*dy + dz*dz);

            float pitchDiff = fmod(fabs(part.rotation.x - part.targetRotation.x), 360.0f);
            if (pitchDiff > 180.0f) pitchDiff = 360.0f - pitchDiff;
            float yawDiff = fmod(fabs(part.rotation.y - part.targetRotation.y), 360.0f);
            if (yawDiff > 180.0f) yawDiff = 360.0f - yawDiff;

            if (dist < 8.0f && pitchDiff < 45.0f && yawDiff < 45.0f && part.isRepaired && !gameState.isInspecting) {
                // Smooth interpolation (lerp) toward target rotation
                part.rotation.x += (part.targetRotation.x - part.rotation.x) * 0.2f;
                part.rotation.y += (part.targetRotation.y - part.rotation.y) * 0.2f;
                part.rotation.z += (part.targetRotation.z - part.rotation.z) * 0.2f;

                gameState.feedbackMessage = "ALIGNMENT LOCKED! PRESS ENTER TO DOCK";
                if (gameState.feedbackTimer < 0.2f) {
                    gameState.feedbackTimer = 0.2f;
                }
            }
        }
    } else {
        // Manipulator arms breathing movement when empty
        drone.armRotation = sin(gameState.elapsedTime * 2.0f) * 12.0f;
    }
}

// -------------------------------------------------------------
// TIMER UPDATE & SIMULATION
// -------------------------------------------------------------
void update(int value) {
    gameState.elapsedTime += 0.016f;

    processInput();

    // Floating animation (Shared Project-Wide Animation Feature)
    // Bobbing and slow auto-spin for unattached debris in space
    for (size_t i = 0; i < parts.size(); ++i) {
        if (parts[i].state == FLOATING) {
            // Member 2 Delegation: rotateDebrisFloating
            rotateDebrisFloating(parts[i]);
            
            // Bobbing on Y
            parts[i].position.y += sin(gameState.elapsedTime * 1.5f + parts[i].bobOffset) * 0.004f;
        }
    }

    // Decrement algorithm active monitors
    for (int i = 0; i < 5; ++i) {
        if (gameState.algorithmActiveTimer[i] > 0.0f) {
            gameState.algorithmActiveTimer[i] -= 0.016f;
        }
    }
    // Z-Buffer (M4) and Scanner (M5) are active continuously if active
    if (gameState.zBufferEnabled) gameState.algorithmActiveTimer[3] = 1.0f;
    if (scanner.active) gameState.algorithmActiveTimer[4] = 1.0f;

    // Decrement feedback timer
    if (gameState.feedbackTimer > 0.0f) {
        gameState.feedbackTimer -= 0.016f;
    }

    // Update particles
    for (int i = (int)particles.size() - 1; i >= 0; --i) {
        particles[i].pos.x += particles[i].vel.x;
        particles[i].pos.y += particles[i].vel.y;
        particles[i].pos.z += particles[i].vel.z;
        particles[i].life--;
        particles[i].a = (float)particles[i].life / particles[i].maxLife;

        if (particles[i].life <= 0) {
            particles.erase(particles.begin() + i);
        }
    }

    // Launch sequence animation logic
    if (gameState.isLaunching) {
        gameState.launchTimer += 0.016f;

        if (gameState.launchTimer < 2.5f) {
            // Thrust ignition sparks
            for (int k = 0; k < 6; ++k) {
                Particle p;
                p.pos = Vec3(0.0f, 1.0f, -7.0f);
                p.vel = Vec3(((float)rand() / RAND_MAX - 0.5f) * 0.35f,
                             -0.2f - ((float)rand() / RAND_MAX) * 0.4f,
                             ((float)rand() / RAND_MAX - 0.5f) * 0.35f);
                p.r = 1.0f; p.g = 0.3f + 0.6f * ((float)rand() / RAND_MAX); p.b = 0.1f; p.a = 1.0f;
                p.life = p.maxLife = 40 + rand() % 30;
                p.size = 0.35f + 0.3f * ((float)rand() / RAND_MAX);
                particles.push_back(p);
            }
        } else {
            // Liftoff!
            float liftTime = gameState.launchTimer - 2.5f;
            float yOffset = liftTime * liftTime * 1.8f;
            float scaleFactor = fmaxf(0.0f, 1.0f - liftTime * 0.15f);

            // Re-map dock slots upwards
            for (size_t i = 0; i < parts.size(); ++i) {
                if (parts[i].state == ATTACHED) {
                    parts[i].position.y = parts[i].dockSlot.y + yOffset;
                    parts[i].scale = scaleFactor;
                }
            }

            // Engine fire exhaust plume
            for (int k = 0; k < 10; ++k) {
                Particle p;
                p.pos = Vec3(0.0f, 1.0f + yOffset, -7.0f);
                p.vel = Vec3(((float)rand() / RAND_MAX - 0.5f) * 0.15f,
                             -0.5f,
                             ((float)rand() / RAND_MAX - 0.5f) * 0.15f);
                p.r = 0.2f; p.g = 0.6f; p.b = 1.0f; p.a = 1.0f; // Plasma Blue
                p.life = p.maxLife = 45;
                p.size = 0.45f;
                particles.push_back(p);
            }

            if (scaleFactor <= 0.05f) {
                gameState.missionComplete = true;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void addThrusterParticles() {
    float radYaw = drone.rotation.y * M_PI / 180.0f;
    float backX = drone.position.x + sin(radYaw) * 1.2f;
    float backY = drone.position.y - 0.4f;
    float backZ = drone.position.z + cos(radYaw) * 1.2f;

    for (int i = 0; i < 2; ++i) {
        Particle p;
        p.pos = Vec3(backX, backY, backZ);
        p.vel = Vec3(sin(radYaw) * 0.08f + ((float)rand() / RAND_MAX - 0.5f) * 0.06f,
                     ((float)rand() / RAND_MAX - 0.5f) * 0.06f,
                     cos(radYaw) * 0.08f + ((float)rand() / RAND_MAX - 0.5f) * 0.06f);
        p.r = 1.0f;
        p.g = 0.4f + 0.4f * ((float)rand() / RAND_MAX);
        p.b = 0.1f;
        p.a = 1.0f;
        p.life = p.maxLife = 20 + rand() % 15;
        p.size = 0.15f + 0.1f * ((float)rand() / RAND_MAX);
        particles.push_back(p);
    }
}

void addNaniteParticles(Vec3 origin) {
    for (int i = 0; i < 60; ++i) {
        Particle p;
        p.pos = origin;
        float theta = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
        float phi = acos(2.0f * ((float)rand() / RAND_MAX) - 1.0f);
        float radius = 0.5f + ((float)rand() / RAND_MAX) * 0.5f;

        p.vel = Vec3(radius * sin(phi) * cos(theta) * 0.15f,
                     radius * sin(phi) * sin(theta) * 0.15f,
                     radius * cos(phi) * 0.15f);
        // Nanite spark colors: Neon Cyan/Green
        p.r = 0.0f;
        p.g = 0.8f + 0.2f * ((float)rand() / RAND_MAX);
        p.b = 0.8f + 0.2f * ((float)rand() / RAND_MAX);
        p.a = 1.0f;
        p.life = p.maxLife = 30 + rand() % 20;
        p.size = 0.15f + 0.1f * ((float)rand() / RAND_MAX);
        particles.push_back(p);
    }
}

// -------------------------------------------------------------
// CAMERA SETUP
// -------------------------------------------------------------
void setupCamera(bool isScannerView) {
    if (isScannerView) {
        // Scanner overhead orthographic-style view (M5 delegation inside drawScannerViewport)
        return;
    }

    float radYaw = drone.rotation.y * M_PI / 180.0f;

    // Member 4 Delegation: setupPrimaryCamera
    setupPrimaryCamera(cameraMode, drone, radYaw, gameState.isLaunching, gameState.launchTimer, cameraPos, cameraTarget);
}

// -------------------------------------------------------------
// RENDERING FUNCTIONS
// -------------------------------------------------------------
void drawScene(bool isScannerView) {
    // Member 4 Delegation: configureDepthEngine Z-buffer toggle
    configureDepthEngine(gameState.zBufferEnabled);

    // Set Lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Setup Drone Headlight (Spotlight - M4 & Lighting)
    glEnable(GL_LIGHT1);
    float radYaw = drone.rotation.y * M_PI / 180.0f;
    GLfloat spotPos[] = { (float)(drone.position.x - sin(radYaw)*0.8f), (float)drone.position.y, (float)(drone.position.z - cos(radYaw)*0.8f), 1.0f };
    GLfloat spotDir[] = { (float)-sin(radYaw), -0.2f, (float)-cos(radYaw) };
    glLightfv(GL_LIGHT1, GL_POSITION, spotPos);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0f);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 12.0f);
    GLfloat spotAmb[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat spotDiff[] = { 1.0f, 1.0f, 0.85f, 1.0f };
    GLfloat spotSpec[] = { 1.0f, 1.0f, 0.85f, 1.0f };
    glLightfv(GL_LIGHT1, GL_AMBIENT, spotAmb);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, spotDiff);
    glLightfv(GL_LIGHT1, GL_SPECULAR, spotSpec);

    // Render stars background (no light)
    glDisable(GL_LIGHTING);
    drawStarfield();
    glEnable(GL_LIGHTING);

    // Render Dock (control tower, runway lights)
    drawDock();

    // Render assembled ship hull connecting structure
    drawShipHull();

    // Render Debris Spacecraft Parts
    for (size_t i = 0; i < parts.size(); ++i) {
        if (parts[i].state == ATTACHED && gameState.isLaunching) {
            glPushMatrix();
            // Member 1 Delegation: applyTranslation
            applyTranslation(parts[i].position.x, parts[i].position.y, parts[i].position.z);
            // Member 2 Delegation: applyRotation
            applyRotation(parts[i].rotation.x, 1, 0, 0);
            applyRotation(parts[i].rotation.y, 0, 1, 0);
            applyRotation(parts[i].rotation.z, 0, 0, 1);
            // Member 3 Delegation: applyScaling
            applyScaling(parts[i].scale, parts[i].scale, parts[i].scale);
            drawPart(parts[i].id, false);
            glPopMatrix();
            continue;
        }

        if (parts[i].state != ATTACHED) {
            glPushMatrix();
            // Member 1 Delegation: applyTranslation
            applyTranslation(parts[i].position.x, parts[i].position.y, parts[i].position.z);

            // Member 2 Delegation: applyRotation
            applyRotation(parts[i].rotation.x, 1, 0, 0);
            applyRotation(parts[i].rotation.y, 0, 1, 0);
            applyRotation(parts[i].rotation.z, 0, 0, 1);

            // Member 3 Delegation: applyScaling (Zoom inspection mode)
            if (parts[i].state == INSPECTING) {
                applyScaling(gameState.zoomFactor, gameState.zoomFactor, gameState.zoomFactor);
                drawPart(parts[i].id, true);
            } else {
                applyScaling(parts[i].scale, parts[i].scale, parts[i].scale);
                drawPart(parts[i].id, false);
            }
            glPopMatrix();

            // Label above floating target parts
            if (parts[i].state == FLOATING && !isScannerView) {
                bool isTarget = (gameState.currentLevel == 1 && i == 0) ||
                                (gameState.currentLevel == 2 && i == 1) ||
                                (gameState.currentLevel == 3 && i == 2) ||
                                (gameState.currentLevel == 4 && (i == 3 || i == 4));
                if (isTarget) {
                    glDisable(GL_LIGHTING);
                    glColor3f(1.0f, 0.3f, 0.3f);
                    renderText3D(parts[i].position.x - 0.7f, parts[i].position.y + 1.6f, parts[i].position.z, "[TARGET]", GLUT_BITMAP_HELVETICA_10);
                    glEnable(GL_LIGHTING);
                }
            }
        } else {
            // Lock slot attached models
            glPushMatrix();
            applyTranslation(parts[i].dockSlot.x, parts[i].dockSlot.y, parts[i].dockSlot.z);
            applyRotation(parts[i].targetRotation.x, 1, 0, 0);
            applyRotation(parts[i].targetRotation.y, 0, 1, 0);
            applyRotation(parts[i].targetRotation.z, 0, 0, 1);
            applyScaling(parts[i].scale, parts[i].scale, parts[i].scale);
            drawPart(parts[i].id, false);
            glPopMatrix();
        }
    }

    // Render Drone (unless in First Person or Launching)
    if (cameraMode != 2 && !gameState.isLaunching) {
        glPushMatrix();
        applyTranslation(drone.position.x, drone.position.y, drone.position.z);
        applyRotation(drone.rotation.y, 0.0f, 1.0f, 0.0f);
        drawDrone();
        glPopMatrix();
    }

    // Render thruster plume particles
    glDisable(GL_LIGHTING);
    drawParticles();
    glEnable(GL_LIGHTING);

    // Draw scanner volume bounds Torus (M5 Clipping limits indicator)
    if (scanner.active && !isScannerView) {
        glDisable(GL_LIGHTING);
        glColor4f(0.0f, 0.8f, 0.3f, 0.4f);
        glPushMatrix();
        applyRotation(90.0f, 1.0f, 0.0f, 0.0f);
        glutWireTorus(0.05f, scanner.farClip, 4, 32);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }
}

void drawStarfield() {
    glColor3f(0.8f, 0.8f, 0.9f);
    glPointSize(1.2f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < stars.size(); ++i) {
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
    }
    glEnd();
}

// -------------------------------------------------------------
// DOCKING STATION DESIGN (Control Tower, Antennas, Beacons)
// -------------------------------------------------------------
void drawDock() {
    // 1. Base Platform
    glPushMatrix();
    glColor3f(0.22f, 0.25f, 0.28f);
    applyTranslation(0.0f, 0.5f, -4.0f);
    applyScaling(12.0f, 0.6f, 16.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 2. Yellow Runway Warning Stripes
    glPushMatrix();
    glColor3f(0.85f, 0.75f, 0.1f);
    applyTranslation(-5.9f, 0.9f, -4.0f);
    applyScaling(0.3f, 0.3f, 16.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.85f, 0.75f, 0.1f);
    applyTranslation(5.9f, 0.9f, -4.0f);
    applyScaling(0.3f, 0.3f, 16.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 3. Control Tower
    glPushMatrix();
    glColor3f(0.18f, 0.2f, 0.25f);
    applyTranslation(0.0f, 4.5f, -11.5f); // Back center
    applyScaling(1.2f, 8.0f, 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Tower Cupola / Command Cabin
    glPushMatrix();
    glColor3f(0.2f, 0.4f, 0.5f);
    applyTranslation(0.0f, 9.0f, -11.5f);
    applyScaling(2.0f, 1.5f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Tower Antenna
    glDisable(GL_LIGHTING);
    glColor3f(0.7f, 0.7f, 0.7f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 9.75f, -11.5f);
    glVertex3f(0.0f, 12.0f, -11.5f);
    glEnd();

    // Rotating Radar Dish
    float radarRot = gameState.elapsedTime * 60.0f;
    glPushMatrix();
    applyTranslation(0.0f, 10.0f, -11.5f);
    applyRotation(radarRot, 0.0f, 1.0f, 0.0f);
    applyRotation(-30.0f, 1.0f, 0.0f, 0.0f); // Tilted dish
    glColor3f(0.4f, 0.45f, 0.5f);
    glutSolidCone(0.8f, 0.5f, 8, 2);
    glPopMatrix();

    // Blinking red beacon light
    float blink = sin(gameState.elapsedTime * 5.0f) * 0.5f + 0.5f;
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    applyTranslation(0.0f, 12.1f, -11.5f);
    glutSolidSphere(0.18f + (blink * 0.05f), 8, 8);
    glPopMatrix();
    glEnable(GL_LIGHTING);

    // 4. Runway guidance light strips (Sequential pulse)
    glDisable(GL_LIGHTING);
    for (int j = 0; j < 8; ++j) {
        float zPos = -11.0f + (j * 2.8f);
        float pulseSeed = sin(gameState.elapsedTime * 3.0f - (j * 0.5f)) * 0.5f + 0.5f;

        glColor3f(0.0f, 0.8f * pulseSeed, 0.2f * pulseSeed);
        glPushMatrix();
        applyTranslation(-5.7f, 0.82f, zPos);
        glutSolidSphere(0.12f, 6, 6);
        glPopMatrix();

        glPushMatrix();
        applyTranslation(5.7f, 0.82f, zPos);
        glutSolidSphere(0.12f, 6, 6);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);

    // 5. DOCK SLOT HOLOGRAPHIC MARKERS — shows WHERE to place parts
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (size_t i = 0; i < parts.size(); ++i) {
        if (parts[i].state != ATTACHED) {
            // Only highlight current level's target slots
            bool isTarget = (gameState.currentLevel == 1 && (int)i == 0) ||
                            (gameState.currentLevel == 2 && (int)i == 1) ||
                            (gameState.currentLevel == 3 && (int)i == 2) ||
                            (gameState.currentLevel == 4 && ((int)i == 3 || (int)i == 4));
            if (!isTarget) continue;

            float pulse = sin(gameState.elapsedTime * 3.0f + i * 1.5f) * 0.3f + 0.7f;

            glPushMatrix();
            applyTranslation(parts[i].dockSlot.x, parts[i].dockSlot.y, parts[i].dockSlot.z);

            // Draw holographic ghost template showing correct docking orientation
            glPushMatrix();
            applyRotation(parts[i].targetRotation.x, 1, 0, 0);
            applyRotation(parts[i].targetRotation.y, 0, 1, 0);
            applyRotation(parts[i].targetRotation.z, 0, 0, 1);
            applyScaling(parts[i].scale, parts[i].scale, parts[i].scale);

            // Temporarily set wireframe mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glColor4f(0.0f, 1.0f, 0.4f, 0.25f * pulse);
            drawPart(parts[i].id, false);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glPopMatrix();

            // Inner pulsing sphere core indicator
            glColor4f(0.0f, 0.9f, 0.4f, 0.12f * pulse);
            glutWireSphere(0.7f, 8, 8);

            // Rotating targeting ring around the slot
            glPushMatrix();
            applyRotation(gameState.elapsedTime * 45.0f + i * 90.0f, 0.0f, 1.0f, 0.0f);
            applyRotation(90.0f, 1.0f, 0.0f, 0.0f);
            glColor4f(0.0f, 1.0f, 0.3f, 0.35f * pulse);
            glutWireTorus(0.04f, 1.2f, 4, 16);
            glPopMatrix();

            // Vertical guide beam above slot
            glColor4f(0.0f, 0.9f, 0.4f, 0.12f);
            glLineWidth(1.5f);
            glBegin(GL_LINES);
            glVertex3f(0.0f, -0.5f, 0.0f);
            glVertex3f(0.0f, 6.0f, 0.0f);
            glEnd();

            // 3D label above the slot
            glColor3f(0.0f, 1.0f, 0.4f);
            renderText3D(-0.6f, 2.2f, 0.0f, "[DOCK SLOT]", GLUT_BITMAP_HELVETICA_10);
            renderText3D(-0.8f, 1.8f, 0.0f, parts[i].name.c_str(), GLUT_BITMAP_HELVETICA_10);

            glPopMatrix();
        }
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// -------------------------------------------------------------
// ASSEMBLED SHIP HULL & STRUCTURAL CHASSIS
// Draws connecting fuselage, wings, tail fins, and details
// between docked parts so they form one cohesive spacecraft.
// -------------------------------------------------------------
void drawShipHull() {
    // Only draw hull when at least one part is docked
    int attachedCount = 0;
    for (int i = 0; i < 5; i++) {
        if (parts[i].state == ATTACHED) attachedCount++;
    }
    if (attachedCount < 1) return;

    // During launch, match vertical offset & scale of attached parts
    float yOff = 0.0f;
    float sc = 1.0f;
    if (gameState.isLaunching && gameState.launchTimer > 2.5f) {
        float lt = gameState.launchTimer - 2.5f;
        yOff = lt * lt * 1.8f;
        sc = fmaxf(0.0f, 1.0f - lt * 0.15f);
    }

    glPushMatrix();
    applyTranslation(0.0f, yOff, 0.0f);
    applyScaling(sc, sc, sc);

    // ========== 1. CENTRAL FUSELAGE KEEL ==========
    // Upper fuselage plate
    glColor3f(0.18f, 0.20f, 0.24f);
    glPushMatrix();
    applyTranslation(0.0f, 1.92f, -4.0f);
    applyScaling(0.88f, 0.07f, 7.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Lower fuselage plate (belly)
    glPushMatrix();
    applyTranslation(0.0f, 1.08f, -4.0f);
    applyScaling(0.88f, 0.07f, 7.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left fuselage wall
    glColor3f(0.20f, 0.22f, 0.26f);
    glPushMatrix();
    applyTranslation(-0.44f, 1.50f, -4.0f);
    applyScaling(0.05f, 0.78f, 7.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right fuselage wall
    glPushMatrix();
    applyTranslation(0.44f, 1.50f, -4.0f);
    applyScaling(0.05f, 0.78f, 7.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ========== 2. NOSE FAIRING ==========
    glColor3f(0.22f, 0.25f, 0.30f);
    glPushMatrix();
    applyTranslation(0.0f, 1.50f, -0.4f);
    glutSolidCone(0.42f, 1.4f, 10, 4);
    glPopMatrix();

    // Nose cone accent ring
    glColor3f(0.28f, 0.30f, 0.35f);
    glPushMatrix();
    applyTranslation(0.0f, 1.50f, -0.35f);
    glutSolidTorus(0.05f, 0.40f, 6, 12);
    glPopMatrix();

    // ========== 3. TAIL SECTION ==========
    // Vertical stabilizer (dorsal fin)
    glColor3f(0.16f, 0.18f, 0.22f);
    glPushMatrix();
    applyTranslation(0.0f, 2.85f, -7.25f);
    applyScaling(0.06f, 2.2f, 1.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Fin tip cap
    glColor3f(0.14f, 0.16f, 0.20f);
    glPushMatrix();
    applyTranslation(0.0f, 3.98f, -7.25f);
    applyScaling(0.06f, 0.12f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left horizontal stabilizer
    glColor3f(0.18f, 0.20f, 0.24f);
    glPushMatrix();
    applyTranslation(-1.1f, 1.60f, -7.35f);
    applyScaling(2.2f, 0.05f, 0.75f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right horizontal stabilizer
    glPushMatrix();
    applyTranslation(1.1f, 1.60f, -7.35f);
    applyScaling(2.2f, 0.05f, 0.75f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Ventral keel fin (small bottom fin)
    glColor3f(0.15f, 0.17f, 0.21f);
    glPushMatrix();
    applyTranslation(0.0f, 0.85f, -7.0f);
    applyScaling(0.05f, 0.55f, 0.7f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ========== 4. WING PYLONS ==========
    // Left wing pylon (to solar panel at x=-4)
    glColor3f(0.22f, 0.24f, 0.28f);
    glPushMatrix();
    applyTranslation(-2.1f, 1.78f, -4.0f);
    applyScaling(4.4f, 0.14f, 0.38f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left pylon lower strut (angled brace)
    glColor3f(0.19f, 0.21f, 0.25f);
    glPushMatrix();
    applyTranslation(-1.5f, 1.42f, -3.55f);
    applyRotation(-10.0f, 0.0f, 0.0f, 1.0f);
    applyScaling(2.4f, 0.07f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left pylon upper strut
    glPushMatrix();
    applyTranslation(-1.5f, 1.92f, -4.45f);
    applyRotation(10.0f, 0.0f, 0.0f, 1.0f);
    applyScaling(2.4f, 0.07f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right wing pylon (to cargo at x=+4)
    glColor3f(0.22f, 0.24f, 0.28f);
    glPushMatrix();
    applyTranslation(2.1f, 1.78f, -4.0f);
    applyScaling(4.4f, 0.14f, 0.38f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right pylon lower strut
    glColor3f(0.19f, 0.21f, 0.25f);
    glPushMatrix();
    applyTranslation(1.5f, 1.42f, -3.55f);
    applyRotation(10.0f, 0.0f, 0.0f, 1.0f);
    applyScaling(2.4f, 0.07f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right pylon upper strut
    glPushMatrix();
    applyTranslation(1.5f, 1.92f, -4.45f);
    applyRotation(-10.0f, 0.0f, 0.0f, 1.0f);
    applyScaling(2.4f, 0.07f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ========== 5. DORSAL SPINE RIDGE ==========
    glColor3f(0.14f, 0.16f, 0.20f);
    glPushMatrix();
    applyTranslation(0.0f, 2.08f, -3.5f);
    applyScaling(0.20f, 0.18f, 5.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ========== 6. FUSELAGE CROSS-FRAME RIBS ==========
    glColor3f(0.24f, 0.26f, 0.30f);
    for (int rib = 0; rib < 8; rib++) {
        float zz = -0.5f - rib * 0.95f;
        glPushMatrix();
        applyTranslation(0.0f, 1.50f, zz);
        applyScaling(0.93f, 0.80f, 0.04f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // ========== 7. ARMOR BULKHEADS ==========
    glColor3f(0.26f, 0.28f, 0.32f);
    // Forward bulkhead
    glPushMatrix();
    applyTranslation(0.0f, 1.50f, -0.45f);
    applyScaling(0.95f, 0.78f, 0.06f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Mid bulkhead (between cockpit and fuel core)
    glPushMatrix();
    applyTranslation(0.0f, 1.50f, -2.5f);
    applyScaling(0.95f, 0.78f, 0.06f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Rear bulkhead (between fuel core and engine)
    glPushMatrix();
    applyTranslation(0.0f, 1.50f, -5.5f);
    applyScaling(0.95f, 0.78f, 0.06f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ========== 8. ENGINE MOUNT COLLAR ==========
    glColor3f(0.15f, 0.17f, 0.20f);
    glPushMatrix();
    applyTranslation(0.0f, 1.55f, -6.5f);
    glutSolidTorus(0.10f, 0.52f, 8, 16);
    glPopMatrix();

    // Second intake ring further back
    glPushMatrix();
    applyTranslation(0.0f, 1.55f, -7.0f);
    glutSolidTorus(0.08f, 0.60f, 8, 16);
    glPopMatrix();

    // ========== 9. HULL PANEL LINES (Wireframe Detail) ==========
    glDisable(GL_LIGHTING);
    glColor4f(0.0f, 0.65f, 0.85f, 0.22f);
    glLineWidth(1.0f);

    // Longitudinal top panel lines
    glBegin(GL_LINES);
    glVertex3f(0.0f, 1.97f, -0.4f);   glVertex3f(0.0f, 1.97f, -7.6f);
    glVertex3f(-0.36f, 1.97f, -0.4f);  glVertex3f(-0.36f, 1.97f, -7.6f);
    glVertex3f(0.36f, 1.97f, -0.4f);   glVertex3f(0.36f, 1.97f, -7.6f);
    glEnd();

    // Side panel lines
    glBegin(GL_LINES);
    glVertex3f(-0.48f, 1.72f, -0.4f);  glVertex3f(-0.48f, 1.72f, -7.6f);
    glVertex3f(0.48f, 1.72f, -0.4f);   glVertex3f(0.48f, 1.72f, -7.6f);
    glVertex3f(-0.48f, 1.28f, -0.4f);  glVertex3f(-0.48f, 1.28f, -7.6f);
    glVertex3f(0.48f, 1.28f, -0.4f);   glVertex3f(0.48f, 1.28f, -7.6f);
    glEnd();

    // Wing pylon edge highlights
    glColor4f(0.0f, 0.8f, 0.3f, 0.18f);
    glBegin(GL_LINES);
    glVertex3f(-0.48f, 1.85f, -3.80f); glVertex3f(-4.3f, 1.85f, -3.80f);
    glVertex3f(-0.48f, 1.85f, -4.20f); glVertex3f(-4.3f, 1.85f, -4.20f);
    glVertex3f(0.48f, 1.85f, -3.80f);  glVertex3f(4.3f, 1.85f, -3.80f);
    glVertex3f(0.48f, 1.85f, -4.20f);  glVertex3f(4.3f, 1.85f, -4.20f);
    glEnd();

    // ========== 10. RUNNING LIGHTS & STROBES ==========
    float navPulse = sin(gameState.elapsedTime * 3.5f) * 0.4f + 0.6f;

    // Port navigation light (GREEN - left wingtip)
    glColor3f(0.0f, 0.9f * navPulse, 0.0f);
    glPushMatrix();
    applyTranslation(-4.3f, 1.88f, -4.0f);
    glutSolidSphere(0.09f, 6, 6);
    glPopMatrix();

    // Starboard navigation light (RED - right wingtip)
    glColor3f(0.9f * navPulse, 0.0f, 0.0f);
    glPushMatrix();
    applyTranslation(4.3f, 1.88f, -4.0f);
    glutSolidSphere(0.09f, 6, 6);
    glPopMatrix();

    // Dorsal strobe (WHITE blink)
    float strobeBlink = sin(gameState.elapsedTime * 8.0f) > 0.7f ? 1.0f : 0.12f;
    glColor3f(strobeBlink, strobeBlink, strobeBlink);
    glPushMatrix();
    applyTranslation(0.0f, 2.20f, -3.0f);
    glutSolidSphere(0.07f, 6, 6);
    glPopMatrix();

    // Tail anti-collision beacon (AMBER)
    float tailBlink = sin(gameState.elapsedTime * 5.0f) > 0.0f ? 1.0f : 0.25f;
    glColor3f(1.0f * tailBlink, 0.5f * tailBlink, 0.0f);
    glPushMatrix();
    applyTranslation(0.0f, 4.06f, -7.25f);
    glutSolidSphere(0.10f, 6, 6);
    glPopMatrix();

    // Forward approach lights (dual CYAN)
    float fwdPulse = sin(gameState.elapsedTime * 2.0f) * 0.3f + 0.7f;
    glColor3f(0.0f, 0.7f * fwdPulse, 0.85f * fwdPulse);
    glPushMatrix();
    applyTranslation(-0.32f, 1.50f, -0.35f);
    glutSolidSphere(0.06f, 6, 6);
    glPopMatrix();
    glPushMatrix();
    applyTranslation(0.32f, 1.50f, -0.35f);
    glutSolidSphere(0.06f, 6, 6);
    glPopMatrix();

    glEnable(GL_LIGHTING);

    // ========== 11. WING ROOT WARNING STRIPES ==========
    glColor3f(0.85f, 0.75f, 0.1f);
    glPushMatrix();
    applyTranslation(-0.50f, 1.78f, -3.88f);
    applyScaling(0.12f, 0.12f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    applyTranslation(0.50f, 1.78f, -3.88f);
    applyScaling(0.12f, 0.12f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ========== 12. HULL DESIGNATION MARKINGS ==========
    // Ship registry text on fuselage side
    glDisable(GL_LIGHTING);
    glColor3f(0.6f, 0.65f, 0.7f);
    renderText3D(-0.43f, 1.65f, -2.8f, "SSV-07", GLUT_BITMAP_HELVETICA_10);
    renderText3D(0.47f, 1.65f, -2.8f, "SSV-07", GLUT_BITMAP_HELVETICA_10);
    glEnable(GL_LIGHTING);

    glPopMatrix(); // End launch offset transform
}

// -------------------------------------------------------------
// DRONE VISUAL DESIGN
// -------------------------------------------------------------
void drawDrone() {
    // 1. Drone Cockpit chassis sphere
    glColor3f(0.35f, 0.45f, 0.52f);
    glutSolidSphere(0.8f, 16, 16);

    // Front Visor / Sensor Core
    glColor3f(0.0f, 0.85f, 0.9f);
    glPushMatrix();
    applyTranslation(0.0f, 0.15f, -0.62f);
    glutSolidSphere(0.35f, 12, 12);
    glPopMatrix();

    // Headlight visual spotlight beam cone (semi-transparent)
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 0.85f, 0.08f + 0.12f * drone.thrusterIntensity);
    glPushMatrix();
    applyTranslation(0.0f, 0.15f, -0.65f);
    applyRotation(180.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(0.5f, 3.8f, 16, 4); // Volumetric beam
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    // 2. Manipulator Robotic Arms
    glColor3f(0.2f, 0.25f, 0.3f);
    if (drone.carriedPart != -1) {
        // Extended grasping arms
        // Left arm clamp
        glPushMatrix();
        applyTranslation(0.75f, -0.2f, -0.4f);
        applyRotation(-25.0f, 0.0f, 1.0f, 0.0f);
        applyScaling(0.15f, 0.15f, 0.8f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Right arm clamp
        glPushMatrix();
        applyTranslation(-0.75f, -0.2f, -0.4f);
        applyRotation(25.0f, 0.0f, 1.0f, 0.0f);
        applyScaling(0.15f, 0.15f, 0.8f);
        glutSolidCube(1.0f);
        glPopMatrix();
    } else {
        // Folded searching arms (slow breathing swing)
        // Left Folded arm
        glPushMatrix();
        applyTranslation(0.75f, -0.2f, -0.1f);
        applyRotation(40.0f + drone.armRotation, 0.0f, 1.0f, 0.0f);
        applyScaling(0.15f, 0.15f, 0.6f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Right Folded arm
        glPushMatrix();
        applyTranslation(-0.75f, -0.2f, -0.1f);
        applyRotation(-40.0f - drone.armRotation, 0.0f, 1.0f, 0.0f);
        applyScaling(0.15f, 0.15f, 0.6f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // 3. Side Engine Mount Blocks
    glColor3f(0.18f, 0.22f, 0.26f);
    glPushMatrix();
    applyTranslation(0.85f, 0.0f, 0.1f);
    applyScaling(0.4f, 0.3f, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    applyTranslation(-0.85f, 0.0f, 0.1f);
    applyScaling(0.4f, 0.3f, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 4. Rear Engine cone nozzle
    glColor3f(0.12f, 0.12f, 0.15f);
    glPushMatrix();
    applyTranslation(0.0f, -0.3f, 0.7f);
    applyRotation(180.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(0.28f, 0.55f, 10, 10);
    glPopMatrix();

    // Glowing core flame cone inside engine
    if (drone.thrusterIntensity > 0.05f) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.45f * drone.thrusterIntensity, 0.05f);
        glPushMatrix();
        applyTranslation(0.0f, -0.3f, 0.8f);
        applyScaling(1.0f, 1.0f, drone.thrusterIntensity * 1.5f);
        glutSolidCone(0.16f, 0.6f, 8, 4);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }
}

// -------------------------------------------------------------
// SPACECRAFT GEOMETRY DESIGN (Damaged vs Repaired states)
// -------------------------------------------------------------
void drawPart(int id, bool isInspecting) {
    // Inspection mode (M3 Zoom): Render Damage hotspot indicators
    if (isInspecting && !parts[id].isRepaired) {
        glDisable(GL_LIGHTING);
        float pulse = sin(gameState.elapsedTime * 6.5f) * 0.5f + 0.5f;
        glColor3f(1.0f, 0.0f, 0.0f);
        
        glPushMatrix();
        applyTranslation(0.0f, 0.4f, 0.1f);
        glutWireSphere(0.38f + (pulse * 0.06f), 8, 8);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }

    glColor3f(parts[id].color[0], parts[id].color[1], parts[id].color[2]);

    switch (id) {
        case 0: // Command Cockpit: Glass dome
            {
                if (isInspecting) {
                    // Member 5 Delegation: enableCustomClipPlane cockpit slice
                    enableCustomClipPlane();
                }
                
                // Main spherical body (Grey steel color)
                glColor3f(parts[0].color[0], parts[0].color[1], parts[0].color[2]);
                glutSolidSphere(0.85f, 16, 16);

                // Side thruster pipes (left and right)
                glColor3f(0.25f, 0.28f, 0.32f);
                glPushMatrix();
                applyTranslation(0.88f, 0.0f, 0.0f);
                applyRotation(90.0f, 0.0f, 1.0f, 0.0f);
                GLUquadric* sideQuad1 = gluNewQuadric();
                gluCylinder(sideQuad1, 0.15f, 0.1f, 0.3f, 8, 2);
                gluDeleteQuadric(sideQuad1);
                glPopMatrix();

                glPushMatrix();
                applyTranslation(-0.88f, 0.0f, 0.0f);
                applyRotation(-90.0f, 0.0f, 1.0f, 0.0f);
                GLUquadric* sideQuad2 = gluNewQuadric();
                gluCylinder(sideQuad2, 0.15f, 0.1f, 0.3f, 8, 2);
                gluDeleteQuadric(sideQuad2);
                glPopMatrix();

                // Front Cockpit visor screen (facing front - negative Z)
                glPushMatrix();
                glColor3f(0.0f, 0.85f, 0.95f); // Neon Cyan Visor
                applyTranslation(0.0f, 0.18f, -0.58f);
                applyScaling(0.6f, 0.35f, 0.4f);
                glutSolidSphere(0.65f, 12, 12);
                glPopMatrix();

                // Nose Cone Tip (facing forward - negative Z)
                glPushMatrix();
                glColor3f(0.9f, 0.2f, 0.2f); // Red Nose Cone
                applyTranslation(0.0f, -0.05f, -0.85f);
                applyRotation(180.0f, 0.0f, 1.0f, 0.0f);
                glutSolidCone(0.3f, 0.4f, 12, 2);
                glPopMatrix();

                // Back stabilizer connector structure (positive Z)
                glPushMatrix();
                glColor3f(0.3f, 0.33f, 0.38f);
                applyTranslation(0.0f, 0.0f, 0.75f);
                applyScaling(0.38f, 0.38f, 0.4f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Tail stabilizer fin
                glPushMatrix();
                glColor3f(parts[0].color[0] * 0.7f, parts[0].color[1] * 0.7f, parts[0].color[2] * 0.7f);
                applyTranslation(0.0f, 0.65f, 0.5f);
                applyScaling(0.1f, 0.6f, 0.4f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Panel lines overlay (Lecturer will love the detail)
                glDisable(GL_LIGHTING);
                glColor4f(0.0f, 0.85f, 0.95f, 0.3f);
                glLineWidth(1.0f);
                glPushMatrix();
                applyRotation(90.0f, 1.0f, 0.0f, 0.0f);
                glutWireTorus(0.02f, 0.86f, 4, 12);
                glPopMatrix();
                glEnable(GL_LIGHTING);

                // Restore primary color
                glColor3f(parts[0].color[0], parts[0].color[1], parts[0].color[2]);
                
                if (isInspecting) {
                    // Member 5 Delegation: disableCustomClipPlane
                    disableCustomClipPlane();
                    
                    // Internal Pilot seat detail
                    glColor3f(0.2f, 0.22f, 0.25f);
                    glPushMatrix();
                    applyTranslation(0.0f, -0.38f, -0.1f);
                    applyScaling(0.45f, 0.4f, 0.45f);
                    glutSolidCube(1.0f);
                    glPopMatrix();
                }

                // If DAMAGED: Draw jagged cracked structures on shell (red/black stripes)
                if (!parts[0].isRepaired) {
                    glDisable(GL_LIGHTING);
                    glColor3f(1.0f, 0.1f, 0.1f);
                    glLineWidth(2.0f);
                    glBegin(GL_LINES);
                    glVertex3f(0.0f, 1.0f, 0.0f);   glVertex3f(0.2f, 0.7f, 0.3f);
                    glVertex3f(0.2f, 0.7f, 0.3f);   glVertex3f(0.08f, 0.4f, 0.7f);
                    glVertex3f(0.2f, 0.7f, 0.3f);   glVertex3f(0.45f, 0.6f, -0.1f);
                    glVertex3f(-0.3f, 0.85f, 0.2f);  glVertex3f(-0.6f, 0.5f, 0.4f);
                    glEnd();
                    glEnable(GL_LIGHTING);
                }
            }
            break;

        case 1: // Thruster Engine
            {
                // Main combustion chamber cylinder
                glColor3f(parts[1].color[0], parts[1].color[1], parts[1].color[2]);
                glPushMatrix();
                GLUquadric* quad = gluNewQuadric();
                gluCylinder(quad, 0.48f, 0.48f, 0.85f, 16, 4);
                gluDeleteQuadric(quad);
                glPopMatrix();

                // Detailed cooling ribs around cylinder
                glColor3f(0.3f, 0.35f, 0.4f);
                for (int rib = 0; rib < 3; ++rib) {
                    glPushMatrix();
                    applyTranslation(0.0f, 0.0f, 0.2f + rib * 0.22f);
                    glutSolidTorus(0.06f, 0.49f, 6, 16);
                    glPopMatrix();
                }

                // Fuel pipes running along engine body
                glColor3f(0.7f, 0.5f, 0.2f); // Copper piping
                for (int pipe = 0; pipe < 4; ++pipe) {
                    glPushMatrix();
                    applyRotation(pipe * 90.0f, 0.0f, 0.0f, 1.0f);
                    applyTranslation(0.44f, 0.0f, 0.0f);
                    applyScaling(0.06f, 0.06f, 0.8f);
                    glutSolidCube(1.0f);
                    glPopMatrix();
                }

                // Exhaust cone nozzle
                glPushMatrix();
                applyTranslation(0.0f, 0.0f, 0.85f);
                
                if (!parts[1].isRepaired) {
                    glDisable(GL_LIGHTING);
                    glColor3f(0.85f, 0.1f, 0.1f);
                    glLineWidth(2.0f);
                    glBegin(GL_LINES);
                    for (int n = 0; n < 8; ++n) {
                        float angle = n * 2.0f * M_PI / 8.0f;
                        if (n % 3 != 0) {
                            glVertex3f(cos(angle)*0.45f, sin(angle)*0.45f, 0.0f);
                            glVertex3f(cos(angle)*0.75f, sin(angle)*0.75f, 0.8f);
                        }
                    }
                    glEnd();
                    glEnable(GL_LIGHTING);
                } else {
                    // Solid cone exhaust (Metallic steel outside)
                    glColor3f(0.22f, 0.24f, 0.27f);
                    glutSolidCone(0.75f, 0.95f, 16, 8);

                    // Inner glowing plasma burner cone
                    glDisable(GL_LIGHTING);
                    glColor3f(1.0f, 0.4f, 0.0f);
                    glPushMatrix();
                    applyTranslation(0.0f, 0.0f, 0.1f);
                    glutSolidCone(0.55f, 0.65f, 12, 4);
                    glPopMatrix();
                    glEnable(GL_LIGHTING);
                }
                glPopMatrix();
            }
            break;

        case 2: // Solar Panel
            {
                // Core pivot axle
                glColor3f(0.35f, 0.38f, 0.42f);
                glPushMatrix();
                applyScaling(0.18f, 0.18f, 2.6f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Structural Support Trusses (V-shape)
                glColor3f(0.25f, 0.28f, 0.32f);
                glPushMatrix();
                applyScaling(0.8f, 0.12f, 0.12f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Left Solar Panel Array
                glPushMatrix();
                applyTranslation(-1.4f, 0.0f, 0.0f);
                applyScaling(2.4f, 0.06f, 1.8f);
                glColor3f(0.15f, 0.22f, 0.35f); // Deep Space Blue Panel
                glutSolidCube(1.0f);
                glPopMatrix();

                // Right Solar Panel Array
                glPushMatrix();
                applyTranslation(1.4f, 0.0f, 0.0f);
                if (!parts[2].isRepaired) {
                    // Damaged state: Right solar array is severely cracked & broken off
                    applyScaling(1.1f, 0.06f, 1.0f);
                    applyTranslation(-0.4f, 0.0f, 0.0f);
                    glColor3f(0.6f, 0.2f, 0.2f); // Cracked array red-tint
                    glutSolidCube(1.0f);
                } else {
                    // Repaired state: Fully calibrated arrays
                    applyScaling(2.4f, 0.06f, 1.8f);
                    glColor3f(0.15f, 0.22f, 0.35f);
                    glutSolidCube(1.0f);
                }
                glPopMatrix();

                // Photovoltaic grid details (wires)
                glDisable(GL_LIGHTING);
                glColor3f(0.0f, 0.85f, 1.0f);
                glLineWidth(1.0f);
                
                // Left Grid lines
                glPushMatrix();
                applyTranslation(-1.4f, 0.04f, 0.0f);
                applyScaling(2.42f, 0.06f, 1.82f);
                glutWireCube(1.0f);
                glPopMatrix();

                if (parts[2].isRepaired) {
                    // Right Grid lines (repaired)
                    glPushMatrix();
                    applyTranslation(1.4f, 0.04f, 0.0f);
                    applyScaling(2.42f, 0.06f, 1.82f);
                    glutWireCube(1.0f);
                    glPopMatrix();
                } else {
                    // Red sparks / debris cracks for damaged pane
                    glColor3f(1.0f, 0.1f, 0.1f);
                    glLineWidth(2.0f);
                    glBegin(GL_LINES);
                    glVertex3f(0.3f, 0.05f, -0.6f);   glVertex3f(1.2f, 0.1f, -0.3f);
                    glVertex3f(0.3f, 0.05f, 0.1f);    glVertex3f(1.0f, -0.05f, 0.5f);
                    glVertex3f(0.5f, 0.05f, 0.4f);    glVertex3f(1.4f, 0.08f, 0.9f);
                    glEnd();
                }
                glEnable(GL_LIGHTING);
            }
            break;

        case 3: // Plutonium Fuel Storage Core
            {
                // Main storage core cylinder
                glColor3f(parts[3].color[0], parts[3].color[1], parts[3].color[2]);
                glPushMatrix();
                GLUquadric* quad = gluNewQuadric();
                gluCylinder(quad, 0.58f, 0.58f, 1.75f, 16, 4);
                gluDeleteQuadric(quad);

                // Cylinder sphere caps
                applyTranslation(0.0f, 0.0f, 0.0f);
                glutSolidSphere(0.57f, 12, 12);
                applyTranslation(0.0f, 0.0f, 1.75f);
                glutSolidSphere(0.57f, 12, 12);
                glPopMatrix();

                // 3 Glowing Containment Reactor Rings
                if (parts[3].isRepaired) {
                    glColor3f(0.0f, 0.95f, 0.35f); // Glowing Neon Green
                } else {
                    glColor3f(0.95f, 0.2f, 0.1f);  // Warning Red
                }
                for (int ring = 0; ring < 3; ++ring) {
                    glPushMatrix();
                    applyTranslation(0.0f, 0.0f, 0.35f + ring * 0.52f);
                    glutSolidTorus(0.06f, 0.61f, 8, 16);
                    glPopMatrix();
                }

                // External structural protective bars (Roll cage cages)
                glColor3f(0.32f, 0.35f, 0.38f);
                for (int bar = 0; bar < 4; ++bar) {
                    glPushMatrix();
                    applyRotation(bar * 90.0f, 0.0f, 0.0f, 1.0f);
                    applyTranslation(0.68f, 0.0f, 0.88f);
                    applyScaling(0.06f, 0.06f, 1.85f);
                    glutSolidCube(1.0f);
                    glPopMatrix();
                }

                // Damaged Reactor Leaks
                if (!parts[3].isRepaired) {
                    glDisable(GL_LIGHTING);
                    glColor3f(1.0f, 0.3f, 0.2f);
                    glPushMatrix();
                    applyTranslation(0.4f, 0.4f, 0.8f);
                    glutSolidSphere(0.18f, 6, 6);
                    glPopMatrix();
                    glEnable(GL_LIGHTING);
                }
            }
            break;

        case 4: // Cargo Pod
            {
                // Draw a beautiful octagonal cargo container container
                glColor3f(parts[4].color[0], parts[4].color[1], parts[4].color[2]);
                
                // Base Box
                glPushMatrix();
                applyScaling(1.3f, 0.9f, 1.3f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Reinforced Corner brackets (Dark Grey)
                glColor3f(0.24f, 0.26f, 0.28f);
                for (int cx = -1; cx <= 1; cx += 2) {
                    for (int cy = -1; cy <= 1; cy += 2) {
                        for (int cz = -1; cz <= 1; cz += 2) {
                            glPushMatrix();
                            applyTranslation(cx * 0.65f, cy * 0.45f, cz * 0.65f);
                            glutSolidSphere(0.18f, 8, 8);
                            glPopMatrix();
                        }
                    }
                }

                // Handle bars on the sides
                glColor3f(0.6f, 0.65f, 0.7f);
                glPushMatrix();
                applyTranslation(0.66f, 0.0f, 0.0f);
                applyScaling(0.05f, 0.3f, 0.1f);
                glutSolidCube(1.0f);
                glPopMatrix();

                glPushMatrix();
                applyTranslation(-0.66f, 0.0f, 0.0f);
                applyScaling(0.05f, 0.3f, 0.1f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Warning hazard plates decals (Yellow / Black stripe frames)
                if (parts[4].isRepaired) {
                    glDisable(GL_LIGHTING);
                    glColor3f(0.85f, 0.75f, 0.1f); // Yellow
                    glPushMatrix();
                    applyTranslation(0.0f, 0.46f, 0.0f);
                    applyScaling(0.9f, 0.02f, 0.9f);
                    glutWireCube(1.0f);
                    glPopMatrix();
                    glEnable(GL_LIGHTING);
                }
            }
            break;
    }
}

void drawParticles() {
    glPointSize(4.2f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < particles.size(); ++i) {
        glColor4f(particles[i].r, particles[i].g, particles[i].b, particles[i].a);
        glVertex3f(particles[i].pos.x, particles[i].pos.y, particles[i].pos.z);
    }
    glEnd();
}

// -------------------------------------------------------------
// 3D TEXT RENDERING HELPERS
// -------------------------------------------------------------
void renderText3D(float x, float y, float z, const std::string& text, void* font) {
    glRasterPos3f(x, y, z);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void renderText(float x, float y, const std::string& text, void* font, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void renderPanel(float x, float y, float w, float h, float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// -------------------------------------------------------------
// HUD & ALGORITHM TRACE STATUS PANELS
// -------------------------------------------------------------
void drawHUD() {
    // 2D Orthographic mode
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. TOP STATUS HUB
    renderPanel(10.0f, windowHeight - 75.0f, windowWidth - 20.0f, 65.0f, 0.05f, 0.08f, 0.15f, 0.85f);
    glColor3f(0.0f, 0.85f, 0.85f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(10.0f, windowHeight - 75.0f);
    glVertex2f(windowWidth - 10.0f, windowHeight - 75.0f);
    glEnd();

    renderText(25.0f, windowHeight - 38.0f, "SPACE RECOVERY TERMINAL v3.2 (MODULAR)", GLUT_BITMAP_HELVETICA_18, 0.0f, 0.85f, 1.0f);
    renderText(25.0f, windowHeight - 60.0f, "MISSION TARGET: DOCK AND REBUILD RECONSTRUCTION DOCK SCHEMATICS", GLUT_BITMAP_HELVETICA_10, 0.7f, 0.7f, 0.8f);

    std::string scoreStr = "SCORE: " + std::to_string(gameState.score);
    renderText(windowWidth - 200.0f, windowHeight - 36.0f, scoreStr, GLUT_BITMAP_HELVETICA_18, 1.0f, 0.85f, 0.0f);

    std::string levelStr = "SECTOR LEVEL: " + std::to_string(gameState.currentLevel) + "/4";
    renderText(windowWidth - 200.0f, windowHeight - 58.0f, levelStr, GLUT_BITMAP_HELVETICA_12, 0.0f, 1.0f, 0.5f);

    // 2. LIVE ALGORITHM ACTIVITY STATUS PANEL
    renderPanel(windowWidth - 300.0f, windowHeight - 240.0f, 290.0f, 155.0f, 0.05f, 0.08f, 0.15f, 0.8f);
    glColor3f(0.0f, 0.85f, 0.85f);
    glBegin(GL_LINES);
    glVertex2f(windowWidth - 300.0f, windowHeight - 240.0f);
    glVertex2f(windowWidth - 10.0f, windowHeight - 240.0f);
    glEnd();

    renderText(windowWidth - 285.0f, windowHeight - 105.0f, "LECTURER ALGORITHM VERIFICATION", GLUT_BITMAP_HELVETICA_10, 0.0f, 0.85f, 1.0f);

    std::string algNames[] = {
        "M1: Drone Navigation (Translation)",
        "M2: Part Alignment System (Rotation)",
        "M3: Zoom Repair Scanner (Scaling)",
        "M4: Hidden-Surface Engine (Z-Buffer)",
        "M5: Sensor Viewport (Clipping)"
    };

    for (int i = 0; i < 5; ++i) {
        float status = gameState.algorithmActiveTimer[i];
        bool isActive = (status > 0.0f);
        float yPos = windowHeight - 128.0f - (i * 20.0f);

        if (isActive) {
            renderText(windowWidth - 285.0f, yPos, algNames[i], GLUT_BITMAP_HELVETICA_10, 0.0f, 1.0f, 0.4f);
            renderText(windowWidth - 65.0f, yPos, "[RUNNING]", GLUT_BITMAP_HELVETICA_10, 0.0f, 1.0f, 0.4f);
        } else {
            renderText(windowWidth - 285.0f, yPos, algNames[i], GLUT_BITMAP_HELVETICA_10, 0.6f, 0.6f, 0.6f);
            renderText(windowWidth - 65.0f, yPos, "[STANDBY]", GLUT_BITMAP_HELVETICA_10, 0.5f, 0.5f, 0.5f);
        }
    }

    // 3. FLIGHT CONTROLS HELP PANEL
    renderPanel(windowWidth - 300.0f, 10.0f, 290.0f, 240.0f, 0.05f, 0.08f, 0.15f, 0.75f);
    glColor3f(0.0f, 0.85f, 0.85f);
    glBegin(GL_LINES);
    glVertex2f(windowWidth - 300.0f, 250.0f);
    glVertex2f(windowWidth - 10.0f, 250.0f);
    glEnd();

    renderText(windowWidth - 285.0f, 225.0f, "SPACE FLIGHT INSTRUCTIONS", GLUT_BITMAP_HELVETICA_12, 0.0f, 0.85f, 1.0f);
    renderText(windowWidth - 285.0f, 195.0f, "W/S/A/D/Q/E: Flight Translate (M1)", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 175.0f, "J / L KEYS: Rotate/Steer Drone Yaw", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 155.0f, "SPACEBAR: Pickup/Drop target part", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 135.0f, "ARROWS: Pitch/Yaw, U/O: Roll (M2)", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 115.0f, "R KEY: Inspect / Nanite Repair (M3)", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 95.0f, "ENTER KEY: Attach Part to Dock", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 75.0f, "V KEY: Toggle cockpit camera views", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 55.0f, "C KEY: Toggle scanner sensor viewport", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);

    // 4. LECTURER EVALUATION DEBUG SHORTCUTS
    renderPanel(windowWidth - 300.0f, 260.0f, 290.0f, 75.0f, 0.08f, 0.05f, 0.05f, 0.8f);
    glColor3f(1.0f, 0.2f, 0.2f);
    glBegin(GL_LINES);
    glVertex2f(windowWidth - 300.0f, 335.0f);
    glVertex2f(windowWidth - 10.0f, 335.0f);
    glEnd();
    renderText(windowWidth - 285.0f, 315.0f, "LECTURER DEBUG EVALUATION CODES", GLUT_BITMAP_HELVETICA_10, 1.0f, 0.3f, 0.3f);
    renderText(windowWidth - 285.0f, 295.0f, "F1: Toggle Z-Buffer visibility bypass", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 275.0f, "F2 / F3: Adjust scanner clip limit", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);

    // 5. DIRECTIVE PROGRESS BAR
    renderPanel(10.0f, 10.0f, 330.0f, 150.0f, 0.05f, 0.08f, 0.15f, 0.8f);
    glColor3f(0.0f, 0.85f, 0.85f);
    glBegin(GL_LINES);
    glVertex2f(10.0f, 160.0f);
    glVertex2f(340.0f, 160.0f);
    glEnd();

    renderText(25.0f, 140.0f, "SYSTEM STATE DIRECTIVES", GLUT_BITMAP_HELVETICA_12, 0.0f, 0.85f, 1.0f);

    std::string levelDir = "";
    std::string stateDir = "";
    if (gameState.currentLevel == 1) {
        levelDir = "LEVEL 1: Recover Cockpit Canopy";
        stateDir = "Grab Cockpit. Bring to center front slot.";
    } else if (gameState.currentLevel == 2) {
        levelDir = "LEVEL 2: Align Spacecraft Thrusters";
        stateDir = "Needs pitch/yaw alignment to fit backend slot.";
    } else if (gameState.currentLevel == 3) {
        levelDir = "LEVEL 3: Calibrate Solar Arrays";
        stateDir = "Needs hull inspection 'R' and align before dock.";
    } else if (gameState.currentLevel == 4) {
        levelDir = "LEVEL 4: Secure Fuel Core & Supply Cargo";
        stateDir = "Retrieve, Repair, Rotate remaining units.";
    }

    if (gameState.isLaunching) {
        levelDir = "IGNITION SEQUENCE STARTING";
        stateDir = "Launching spacecraft. Stay clear of dock.";
    }

    renderText(25.0f, 117.0f, levelDir, GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f);
    renderText(25.0f, 99.0f, stateDir, GLUT_BITMAP_HELVETICA_10, 0.95f, 0.9f, 0.4f);

    if (drone.carriedPart != -1) {
        SpacePart& cp = parts[drone.carriedPart];
        
        float pitchDiff = fmod(fabs(cp.rotation.x - cp.targetRotation.x), 360.0f);
        if (pitchDiff > 180.0f) pitchDiff = 360.0f - pitchDiff;
        float yawDiff = fmod(fabs(cp.rotation.y - cp.targetRotation.y), 360.0f);
        if (yawDiff > 180.0f) yawDiff = 360.0f - yawDiff;
        float rollDiff = fmod(fabs(cp.rotation.z - cp.targetRotation.z), 360.0f);
        if (rollDiff > 180.0f) rollDiff = 360.0f - rollDiff;

        std::string inspectStatus = "UNIT: " + cp.name + " (" + (cp.isRepaired ? "HEALTHY" : "DAMAGED") + ")";
        renderText(25.0f, 75.0f, inspectStatus, GLUT_BITMAP_HELVETICA_10, cp.isRepaired ? 0.0f : 1.0f, cp.isRepaired ? 0.9f : 0.2f, cp.isRepaired ? 0.2f : 0.2f);
        
        // Detailed step directions (turns green when within snap range!)
        std::string pitchStr = "PITCH OFFSET: " + std::to_string((int)pitchDiff) + " DEG (ARROW UP/DOWN)";
        std::string yawStr   = "YAW OFFSET:   " + std::to_string((int)yawDiff) + " DEG (ARROW LEFT/RIGHT)";
        std::string rollStr  = "ROLL OFFSET:  " + std::to_string((int)rollDiff) + " DEG (KEYS U / O)";
        
        renderText(25.0f, 53.0f, pitchStr, GLUT_BITMAP_HELVETICA_10, pitchDiff < 45.0f ? 0.0f : 1.0f, pitchDiff < 45.0f ? 1.0f : 0.6f, pitchDiff < 45.0f ? 0.4f : 0.6f);
        renderText(25.0f, 35.0f, yawStr,   GLUT_BITMAP_HELVETICA_10, yawDiff < 45.0f ? 0.0f : 1.0f, yawDiff < 45.0f ? 1.0f : 0.6f, yawDiff < 45.0f ? 0.4f : 0.6f);
        renderText(25.0f, 17.0f, rollStr,  GLUT_BITMAP_HELVETICA_10, 0.7f, 0.7f, 0.8f);
    } else {
        renderText(25.0f, 75.0f, "UNIT HELD: None (Scan debris coordinates)", GLUT_BITMAP_HELVETICA_10, 0.7f, 0.7f, 0.7f);
    }

    // 6. LIVE DEBUG WARNING PANEL
    if (!gameState.zBufferEnabled) {
        renderPanel(windowWidth / 2 - 220.0f, windowHeight - 125.0f, 440.0f, 35.0f, 0.8f, 0.1f, 0.1f, 0.75f);
        renderText(windowWidth / 2 - 195.0f, windowHeight - 113.0f, "Z-BUFFER BYPASSED! OVERLAPPING PIXELS DRAW OUT OF ORDER", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f);
    }

    // 8. SCREEN FEEDBACK WARNING MESSAGES
    if (gameState.feedbackTimer > 0.0f) {
        float alpha = fminf(1.0f, gameState.feedbackTimer);
        renderPanel(windowWidth / 2.0f - 220.0f, windowHeight / 2.0f - 20.0f, 440.0f, 40.0f, 0.05f, 0.08f, 0.15f, 0.9f * alpha);
        glColor4f(0.0f, 0.85f, 1.0f, alpha);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(windowWidth / 2.0f - 220.0f, windowHeight / 2.0f - 20.0f);
        glVertex2f(windowWidth / 2.0f + 220.0f, windowHeight / 2.0f - 20.0f);
        glVertex2f(windowWidth / 2.0f + 220.0f, windowHeight / 2.0f + 20.0f);
        glVertex2f(windowWidth / 2.0f - 220.0f, windowHeight / 2.0f + 20.0f);
        glEnd();
        glLineWidth(1.0f);

        // Center text on screen
        float offset = gameState.feedbackMessage.length() * 3.5f;
        renderText(windowWidth / 2.0f - offset, windowHeight / 2.0f - 4.0f, gameState.feedbackMessage, GLUT_BITMAP_HELVETICA_10, 1.0f, 0.9f, 0.2f);
    }

    // 7. ZOOM HULL INSPECTION VIEWPORT
    if (gameState.selectedPartIndex != -1 && parts[gameState.selectedPartIndex].state == INSPECTING) {
        renderPanel(10.0f, 370.0f, 320.0f, 140.0f, 0.02f, 0.05f, 0.15f, 0.85f);
        glColor3f(1.0f, 0.2f, 0.2f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(10.0f, 370.0f);
        glVertex2f(330.0f, 370.0f);
        glVertex2f(330.0f, 510.0f);
        glVertex2f(10.0f, 510.0f);
        glEnd();

        renderText(25.0f, 485.0f, "DIAGNOSTIC HULL ANALYSIS (M3)", GLUT_BITMAP_HELVETICA_10, 1.0f, 0.3f, 0.3f);
        
        SpacePart& cp = parts[gameState.selectedPartIndex];
        std::string nameStr = "PART: " + cp.name;
        std::string zoomStr = "ZOOM RATIO: " + std::to_string(gameState.zoomFactor).substr(0, 4) + "x / 2.2x";
        
        renderText(25.0f, 460.0f, nameStr, GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f);
        renderText(25.0f, 440.0f, zoomStr, GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);

        if (gameState.zoomFactor < 2.0f) {
            renderText(25.0f, 400.0f, "ANALYZING... ENLARGE ZOOM STAGE", GLUT_BITMAP_HELVETICA_10, 0.7f, 0.7f, 0.7f);
        } else {
            if (!cp.isRepaired) {
                float pulse = sin(gameState.elapsedTime * 8.0f) * 0.5f + 0.5f;
                renderText(25.0f, 400.0f, "STRUCTURAL CRITICAL CORE FAULT DETECTED", GLUT_BITMAP_HELVETICA_10, 1.0f, 0.1f * pulse, 0.1f * pulse);
                renderText(25.0f, 385.0f, ">>> PRESS 'R' TO INITIATE NANITE REPAIR <<<", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 0.0f);
            } else {
                renderText(25.0f, 400.0f, "REPAIR COMPLETED. STRUCTURAL HULL SEALED.", GLUT_BITMAP_HELVETICA_10, 0.0f, 1.0f, 0.5f);
                renderText(25.0f, 385.0f, "ZOOM RETRACTING AUTOMATICALLY...", GLUT_BITMAP_HELVETICA_10, 0.7f, 0.9f, 1.0f);
            }
        }

        glColor4f(0.0f, 0.85f, 0.9f, 0.4f);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex2f(windowWidth / 2.0f - 80.0f, windowHeight / 2.0f);
        glVertex2f(windowWidth / 2.0f + 80.0f, windowHeight / 2.0f);
        glVertex2f(windowWidth / 2.0f, windowHeight / 2.0f - 80.0f);
        glVertex2f(windowWidth / 2.0f, windowHeight / 2.0f + 80.0f);
        glEnd();
    }

    if (gameState.missionComplete) {
        renderPanel(0.0f, 0.0f, windowWidth, windowHeight, 0.02f, 0.05f, 0.12f, 0.9f);
        renderText(windowWidth / 2.0f - 180.0f, windowHeight / 2.0f + 50.0f, "MISSION COMPLETE", GLUT_BITMAP_HELVETICA_18, 0.0f, 1.0f, 0.5f);
        renderText(windowWidth / 2.0f - 145.0f, windowHeight / 2.0f + 10.0f, "SPACECRAFT RECONSTRUCTED & DEPLOYED", GLUT_BITMAP_HELVETICA_10, 0.8f, 0.85f, 0.95f);
        renderText(windowWidth / 2.0f - 110.0f, windowHeight / 2.0f - 40.0f, "Press Esc to exit space terminal.", GLUT_BITMAP_HELVETICA_12, 1.0f, 1.0f, 1.0f);
    }

    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// -------------------------------------------------------------
// MEMBER 5: SCAN OVERLAY SCANNER VIEWPORT
// -------------------------------------------------------------
void drawScannerViewport() {
    GLint mainViewport[4];
    glGetIntegerv(GL_VIEWPORT, mainViewport);

    // Member 5 Delegation: activateScannerViewport
    activateScannerViewport(scanner);

    // Clear color and depth (M4 & M5)
    glClearColor(0.01f, 0.06f, 0.04f, 1.0f); // Radar scan dark green
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Force Z-buffer ON for scanner regardless of toggle (Bug fix)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Save and configure scanner projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    // Member 5 Delegation: configureScannerProjection (tight clipping planes)
    configureScannerProjection(scanner);

    // Overhead sensor view Setup
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(0.0f, 22.0f, 0.001f,
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);

    // Enable Scanner Radar Green Matrix Rendering (Polygon wireframes - M5 View style)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.9f, 0.2f);
    
    // Draw scene wireframes
    drawScene(true);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LIGHTING);

    // Restore Modelview and Projection matrices
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Member 5 Delegation: deactivateScannerViewport
    deactivateScannerViewport();

    // Reset Viewport to full screen for scanner viewport borders drawing
    glViewport(mainViewport[0], mainViewport[1], mainViewport[2], mainViewport[3]);
    glClearColor(0.0f, 0.0f, 0.03f, 1.0f); // Reset color

    // Draw scanner borders overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Radar HUD border frame (Member 5 active verification indicator)
    glColor3f(0.0f, 0.9f, 0.3f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(scanner.viewportX, scanner.viewportY);
    glVertex2f(scanner.viewportX + scanner.viewportW, scanner.viewportY);
    glVertex2f(scanner.viewportX + scanner.viewportW, scanner.viewportY + scanner.viewportH);
    glVertex2f(scanner.viewportX, scanner.viewportY + scanner.viewportH);
    glEnd();

    // Scan sweep indicator line
    float sweepY = (sin(gameState.elapsedTime * 2.8f) * 0.5f + 0.5f) * scanner.viewportH + scanner.viewportY;
    glColor4f(0.0f, 0.9f, 0.2f, 0.22f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glVertex2f(scanner.viewportX, sweepY);
    glVertex2f(scanner.viewportX + scanner.viewportW, sweepY);
    glVertex2f(scanner.viewportX + scanner.viewportW, sweepY - 12.0f);
    glVertex2f(scanner.viewportX, sweepY - 12.0f);
    glEnd();
    glDisable(GL_BLEND);

    // Write diagnostic values in viewport scanner HUD
    renderText(scanner.viewportX + 12.0f, scanner.viewportY + scanner.viewportH - 22.0f, "SENSOR VIEWPORT SCANNER (M5)", GLUT_BITMAP_HELVETICA_10, 0.0f, 1.0f, 0.3f);
    renderText(scanner.viewportX + 12.0f, scanner.viewportY + scanner.viewportH - 38.0f, "CLIP FAR PLANE: " + std::to_string((int)scanner.farClip) + "m", GLUT_BITMAP_HELVETICA_10, 0.0f, 1.0f, 0.3f);

    // Debris diagnostic scan readout
    if (drone.carriedPart != -1) {
        SpacePart& cp = parts[drone.carriedPart];
        
        float pitchDiff = fmod(fabs(cp.rotation.x - cp.targetRotation.x), 360.0f);
        if (pitchDiff > 180.0f) pitchDiff = 360.0f - pitchDiff;

        float yawDiff = fmod(fabs(cp.rotation.y - cp.targetRotation.y), 360.0f);
        if (yawDiff > 180.0f) yawDiff = 360.0f - yawDiff;

        std::string partNameStr = "TARGET: " + cp.name;
        std::string damageStr = "HULL: " + std::to_string((int)cp.damagePercent) + "% DAMAGED";
        std::string rotErrStr = "PITCH:" + std::to_string((int)pitchDiff) + " YAW:" + std::to_string((int)yawDiff) + " ERR";

        renderText(scanner.viewportX + 12.0f, scanner.viewportY + 45.0f, partNameStr, GLUT_BITMAP_HELVETICA_10, 0.0f, 0.9f, 0.2f);
        renderText(scanner.viewportX + 12.0f, scanner.viewportY + 29.0f, damageStr, GLUT_BITMAP_HELVETICA_10, cp.isRepaired ? 0.0f : 1.0f, cp.isRepaired ? 0.9f : 0.2f, 0.2f);
        renderText(scanner.viewportX + 12.0f, scanner.viewportY + 13.0f, rotErrStr, GLUT_BITMAP_HELVETICA_10, 0.0f, 0.9f, 0.2f);
    } else {
        renderText(scanner.viewportX + 12.0f, scanner.viewportY + 15.0f, "COORDINATE DISPERSION: IDLE", GLUT_BITMAP_HELVETICA_10, 0.6f, 0.6f, 0.6f);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// -------------------------------------------------------------
// GLUT WINDOW CALLBACKS
// -------------------------------------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. PRIMARY VIEWPORT
    glViewport(0, 0, windowWidth, windowHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)windowWidth / windowHeight, 0.5, 400.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    setupCamera(false);

    // Render full 3D graphics scene
    drawScene(false);

    // Render 2D overlays HUD
    drawHUD();

    // 2. SCANNER viewport (M5 secondary viewport)
    if (scanner.active && !gameState.missionComplete) {
        drawScannerViewport();
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glutPostRedisplay();
}

void keyboardExtra(unsigned char key, int x, int y) {
    if (key == 27) { // ESC key
        exit(0);
    }
    keyboardDown(key, x, y);
}

// -------------------------------------------------------------
// MAIN ENTRY POINT
// -------------------------------------------------------------
int main(int argc, char** argv) {
    srand(time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Space Salvage Station: Spacecraft Recovery Operation");

    initGame();

    glClearColor(0.0f, 0.0f, 0.03f, 1.0f);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, update, 0);

    glutKeyboardFunc(keyboardExtra);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);

    glutMainLoop();
    return 0;
}
