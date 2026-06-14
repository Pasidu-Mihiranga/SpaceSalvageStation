#include "structures.h"
#include "translation.h"
#include "rotation.h"
#include "scaling.h"
#include "zbuffer.h"
#include "clipping.h"
#include "src/input.h"
#include "src/models.h"
#include "src/environment.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <cstdlib>
#include <ctime>

// Definition of global variables declared in structures.h
int windowWidth = 1024;
int windowHeight = 768;

std::vector<SpacePart> parts;
Drone drone;
std::vector<Particle> particles;
Scanner scanner;
GameState gameState;

// Starfield positions

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

// Forward declarations of functions
void initGame();
void update(int value);
void drawScene(bool isScannerView);
void drawHUD();
void drawScannerViewport();
void drawParticles();
void addThrusterParticles();
void addNaniteParticles(Vec3 origin);
void setupCamera(bool isScannerView);
void reshape(int w, int h);
void display();

// Initialize the game state and entities
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

    // Create the player's navigation drone with starting values
    drone.position = Vec3(0.0f, 2.0f, 12.0f);
    drone.rotation = Vec3(0.0f, 0.0f, 0.0f);
    drone.speed = 0.22f;
    drone.carriedPart = -1;
    drone.thrusterIntensity = 0.0f;
    drone.armRotation = 0.0f;

    // Set up the scanner viewport settings and limits
    scanner.active = true;
    scanner.nearClip = 1.0f;
    scanner.farClip = 25.0f; // Limit to show clipping of distant debris ()
    scanner.viewportX = 20;
    scanner.viewportY = windowHeight - 75 - 15 - 240;
    scanner.viewportW = 240;
    scanner.viewportH = 240;

    // Initialize the five spacecraft parts with their locations and target orientations
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
    cockpit.isRepaired = true; // Level 1 cockpit starts repaired
    cockpit.color[0] = 0.2f; cockpit.color[1] = 0.6f; cockpit.color[2] = 0.8f; // Blue canopy color
    cockpit.scale = 1.0f;
    cockpit.damagePercent = 0.0f;
    parts.push_back(cockpit);

    // Engine part configuration
    SpacePart engine;
    engine.id = 1;
    engine.name = "Thruster Engine";
    engine.position = Vec3(7.0f, 0.5f, 4.0f);
    engine.rotation = Vec3(40.0f, 120.0f, 0.0f); // Start with misaligned rotation angles
    engine.targetRotation = Vec3(0.0f, 180.0f, 0.0f); // Target rotation angle
    engine.dockSlot = Vec3(0.0f, 1.6f, -7.0f); // Dock position coordinate
    engine.bobOffset = 2.0f;
    engine.spinSpeed = 0.8f;
    engine.state = FLOATING;
    engine.isRepaired = true; // Level 2 engine starts repaired
    engine.color[0] = 0.8f; engine.color[1] = 0.35f; engine.color[2] = 0.1f; // Orange/Grey color
    engine.scale = 1.0f;
    engine.damagePercent = 0.0f;
    parts.push_back(engine);

    // Solar panel configuration
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
    panel.isRepaired = false; // Solar panel requires repair
    panel.color[0] = 0.9f; panel.color[1] = 0.2f; panel.color[2] = 0.2f; // Damaged red color indicator
    panel.scale = 1.0f;
    panel.damagePercent = 78.0f;
    parts.push_back(panel);

    // Fuel tank configuration
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
    tank.color[0] = 0.9f; tank.color[1] = 0.2f; tank.color[2] = 0.2f; // Damaged red color indicator
    tank.scale = 0.9f;
    tank.damagePercent = 55.0f;
    parts.push_back(tank);

    // Cargo container configuration
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
    cargo.color[0] = 0.9f; cargo.color[1] = 0.2f; cargo.color[2] = 0.2f; // Damaged red color indicator
    cargo.scale = 0.9f;
    cargo.damagePercent = 42.0f;
    parts.push_back(cargo);

    initStars();
    particles.clear();
}

// Main game loop timer callback
void update(int value) {
    gameState.elapsedTime += 0.016f;

    processInput();

    // Update positions/rotations of unattached debris floating in space
    // Bobbing and slow auto-spin for unattached debris in space
    for (size_t i = 0; i < parts.size(); ++i) {
        if (parts[i].state == FLOATING) {
            // Call automatic floating debris rotation helper
            rotateDebrisFloating(parts[i]);
            
            // Bobbing on Y
            parts[i].position.y += sin(gameState.elapsedTime * 1.5f + parts[i].bobOffset) * 0.004f;
        }
    }

    // Update UI active algorithm indicators timer
    for (int i = 0; i < 5; ++i) {
        if (gameState.algorithmActiveTimer[i] > 0.0f) {
            gameState.algorithmActiveTimer[i] -= 0.016f;
        }
    }
    // Set indicators active for continuous algorithms
    if (gameState.zBufferEnabled) gameState.algorithmActiveTimer[3] = 1.0f;
    if (scanner.active) gameState.algorithmActiveTimer[4] = 1.0f;

    // Decrement feedback timer
    if (gameState.feedbackTimer > 0.0f) {
        gameState.feedbackTimer -= 0.016f;
    }

    // Update lifespans and positions of exhaust/repair particles
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

    // Animate spaceship launch sequence
    if (gameState.isLaunching) {
        gameState.launchTimer += 0.016f;

        if (gameState.launchTimer < 2.5f) {
            // Spawn initial thrust sparks
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
            // Liftoff physics: move docked spacecraft upwards and scale down
            float liftTime = gameState.launchTimer - 2.5f;
            float yOffset = liftTime * liftTime * 1.8f;
            float scaleFactor = fmaxf(0.0f, 1.0f - liftTime * 0.15f);

            // Shift all attached parts upwards
            for (size_t i = 0; i < parts.size(); ++i) {
                if (parts[i].state == ATTACHED) {
                    parts[i].position.y = parts[i].dockSlot.y + yOffset;
                    parts[i].scale = scaleFactor;
                }
            }

            // Spawn main thrust plume particles
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

// Set up view camera position
void setupCamera(bool isScannerView) {
    if (isScannerView) {
        // View configuration is handled inside drawScannerViewport
        return;
    }

    float radYaw = drone.rotation.y * M_PI / 180.0f;

    // Call camera positioning helper
    setupPrimaryCamera(cameraMode, drone, radYaw, gameState.isLaunching, gameState.launchTimer, cameraPos, cameraTarget);
}

// Scene rendering functions
void drawScene(bool isScannerView) {
    // Configure depth buffer depth test (Z-Buffer)
    configureDepthEngine(gameState.zBufferEnabled);

    // Set up scene ambient and diffuse lighting parameters
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Setup spotlight headlight on drone
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

    // Draw background starfield without lighting influence
    glDisable(GL_LIGHTING);
    drawStarfield();
    glEnable(GL_LIGHTING);

    // Draw docking station base structure
    drawDock();

    // Draw structural hull sections
    drawShipHull();

    // Render spacecraft components
    for (size_t i = 0; i < parts.size(); ++i) {
        if (parts[i].state == ATTACHED && gameState.isLaunching) {
            glPushMatrix();
            // Apply translation matrix using glTranslatef
            applyTranslation(parts[i].position.x, parts[i].position.y, parts[i].position.z);
            // Apply rotation matrix using glRotatef
            applyRotation(parts[i].rotation.x, 1, 0, 0);
            applyRotation(parts[i].rotation.y, 0, 1, 0);
            applyRotation(parts[i].rotation.z, 0, 0, 1);
            // Apply scaling matrix using glScalef
            applyScaling(parts[i].scale, parts[i].scale, parts[i].scale);
            drawPart(parts[i].id, false);
            glPopMatrix();
            continue;
        }

        if (parts[i].state != ATTACHED) {
            glPushMatrix();
            // Apply translation matrix using glTranslatef
            applyTranslation(parts[i].position.x, parts[i].position.y, parts[i].position.z);

            // Apply rotation matrix using glRotatef
            applyRotation(parts[i].rotation.x, 1, 0, 0);
            applyRotation(parts[i].rotation.y, 0, 1, 0);
            applyRotation(parts[i].rotation.z, 0, 0, 1);

            // Apply scaling matrix using glScalef (Zoom inspection mode)
            if (parts[i].state == INSPECTING) {
                applyScaling(gameState.zoomFactor, gameState.zoomFactor, gameState.zoomFactor);
                drawPart(parts[i].id, true);
            } else {
                applyScaling(parts[i].scale, parts[i].scale, parts[i].scale);
                drawPart(parts[i].id, false);
            }
            glPopMatrix();

            // Render text label above target parts
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

    // Render the player drone mesh
    if (cameraMode != 2 && !gameState.isLaunching) {
        glPushMatrix();
        applyTranslation(drone.position.x, drone.position.y, drone.position.z);
        applyRotation(drone.rotation.y, 0.0f, 1.0f, 0.0f);
        drawDrone();
        glPopMatrix();
    }

    // Render thrust smoke particles
    glDisable(GL_LIGHTING);
    drawParticles();
    glEnable(GL_LIGHTING);

    // Draw boundary torus showing scan distance clipping limit
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

void drawParticles() {
    glPointSize(4.2f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < particles.size(); ++i) {
        glColor4f(particles[i].r, particles[i].g, particles[i].b, particles[i].a);
        glVertex3f(particles[i].pos.x, particles[i].pos.y, particles[i].pos.z);
    }
    glEnd();
}

// 3D TEXT RENDERING HELPERS
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

// HUD & ALGORITHM TRACE STATUS PANELS
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

    // 3. FLIGHT CONTROLS HELP PANEL
    renderPanel(windowWidth - 300.0f, 10.0f, 290.0f, 240.0f, 0.05f, 0.08f, 0.15f, 0.75f);
    glColor3f(0.0f, 0.85f, 0.85f);
    glBegin(GL_LINES);
    glVertex2f(windowWidth - 300.0f, 250.0f);
    glVertex2f(windowWidth - 10.0f, 250.0f);
    glEnd();

    renderText(windowWidth - 285.0f, 225.0f, "SPACE FLIGHT INSTRUCTIONS", GLUT_BITMAP_HELVETICA_12, 0.0f, 0.85f, 1.0f);
    renderText(windowWidth - 285.0f, 195.0f, "W/S/A/D/Q/E: Flight Translate", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 175.0f, "J / L KEYS: Rotate/Steer Drone Yaw", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 155.0f, "SPACEBAR: Pickup/Drop target part", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 135.0f, "ARROWS: Pitch/Yaw, U/O: Roll", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 115.0f, "R KEY: Inspect / Nanite Repair", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 95.0f, "ENTER KEY: Attach Part to Dock", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 75.0f, "V KEY: Toggle cockpit camera views", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);
    renderText(windowWidth - 285.0f, 55.0f, "C KEY: Toggle scanner sensor viewport", GLUT_BITMAP_HELVETICA_10, 0.9f, 0.9f, 0.9f);



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

        renderText(25.0f, 485.0f, "DIAGNOSTIC HULL ANALYSIS", GLUT_BITMAP_HELVETICA_10, 1.0f, 0.3f, 0.3f);
        
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

// (Clipping): Draw scanner sub-viewport overlay
void drawScannerViewport() {
    GLint mainViewport[4];
    glGetIntegerv(GL_VIEWPORT, mainViewport);

    // Set viewport and scissor region to target radar box (Viewport)
    activateScannerViewport(scanner);

    // Clear color and depth buffers
    glClearColor(0.01f, 0.06f, 0.04f, 1.0f); // Radar scan dark green
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Force depth test ON for the scanner viewport
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Configure scanner perspective matrix
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    // Set scanner near/far clipping range (Clipping Planes)
    configureScannerProjection(scanner);

    // Set up overhead orthographic look-at camera
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(0.0f, 22.0f, 0.001f,
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);

    // Set wireframe render mode for radar scanner style
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.9f, 0.2f);
    
    // Draw scene wireframe elements
    drawScene(true);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LIGHTING);

    // Pop projection and modelview matrices
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Disable scissor test (Scissor Test)
    deactivateScannerViewport();

    // Restore full screen viewport coordinates
    glViewport(mainViewport[0], mainViewport[1], mainViewport[2], mainViewport[3]);
    glClearColor(0.0f, 0.0f, 0.03f, 1.0f); // Reset color

    // Draw 2D HUD frame overlay for scanner
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Draw frame loop around scanner viewport bounds
    glColor3f(0.0f, 0.9f, 0.3f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(scanner.viewportX, scanner.viewportY);
    glVertex2f(scanner.viewportX + scanner.viewportW, scanner.viewportY);
    glVertex2f(scanner.viewportX + scanner.viewportW, scanner.viewportY + scanner.viewportH);
    glVertex2f(scanner.viewportX, scanner.viewportY + scanner.viewportH);
    glEnd();

    // Draw radar sweep line animation
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

    // Draw scanner title and far clip text labels
    renderText(scanner.viewportX + 12.0f, scanner.viewportY + scanner.viewportH - 22.0f, "SENSOR VIEWPORT SCANNER", GLUT_BITMAP_HELVETICA_10, 0.0f, 1.0f, 0.3f);
    renderText(scanner.viewportX + 12.0f, scanner.viewportY + scanner.viewportH - 38.0f, "CLIP FAR PLANE: " + std::to_string((int)scanner.farClip) + "m", GLUT_BITMAP_HELVETICA_10, 0.0f, 1.0f, 0.3f);

    // Draw spacecraft part data inside scanner HUD
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

// GLUT callback handlers
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set primary window viewport
    glViewport(0, 0, windowWidth, windowHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)windowWidth / windowHeight, 0.5, 400.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    setupCamera(false);

    // Render main 3D scene elements
    drawScene(false);

    // Draw HUD labels and panel screens
    drawHUD();

    // 2. SCANNER viewport ( secondary viewport)
    if (scanner.active && !gameState.missionComplete) {
        drawScannerViewport();
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    scanner.viewportY = windowHeight - 75 - 15 - 240;
    glutPostRedisplay();
}

// Entry point main function
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
