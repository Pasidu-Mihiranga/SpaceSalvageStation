#include "input.h"
#include "../translation.h"
#include "../rotation.h"
#include "../scaling.h"
#include "../zbuffer.h"
#include "../clipping.h"
#include <cmath>

// Keyboard input handling for game controls
void keyboardDown(unsigned char key, int x, int y) {
    keyStates[key] = true;

    // Process single key press triggers
    if (key == 'c' || key == 'C') {
        scanner.active = !scanner.active;
    }
    else if (key == 'v' || key == 'V') {
        cameraMode = (cameraMode + 1) % 3;
    }
    else if (key == ' ') {
        // Spacebar to grab or release a spacecraft component
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
            // Release component and zoom back out if in inspection mode
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
        // Enter key: try to attach the carried part to the correct slot
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

            // Check if current angles are close to target angles
            float pitchDiff = fmod(fabs(part.rotation.x - part.targetRotation.x), 360.0f);
            float yawDiff = fmod(fabs(part.rotation.y - part.targetRotation.y), 360.0f);
            float rollDiff = fmod(fabs(part.rotation.z - part.targetRotation.z), 360.0f);

            // Handle boundary wrapping at 360 degrees to get shortest rotation path
            if (pitchDiff > 180.0f) pitchDiff = 360.0f - pitchDiff;
            if (yawDiff > 180.0f) yawDiff = 360.0f - yawDiff;
            if (rollDiff > 180.0f) rollDiff = 360.0f - rollDiff;

            bool isAligned = (pitchDiff < 30.0f && yawDiff < 30.0f);

            // Check distance from drone to target slot
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
                    // Attach the part and lock its position/rotation to the dock
                    part.state = ATTACHED;
                    part.position = part.dockSlot;
                    part.rotation = part.targetRotation;
                    drone.carriedPart = -1;
                    gameState.selectedPartIndex = -1;
                    gameState.partsAttached++;
                    gameState.score += 250;
                    gameState.feedbackMessage = "PART SECURED & DOCKED SUCCESSFULLY!";
                    gameState.feedbackTimer = 3.0f;

                    // Advance to the next sector level once parts are attached
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
        // R key triggers inspection or nanite repair
        if (drone.carriedPart != -1) {
            int pId = drone.carriedPart;
            if (!gameState.isInspecting) {
                // Switch state to inspecting and activate zoom animation
                gameState.isInspecting = true;
                parts[pId].state = INSPECTING;
            } else {
                // Perform repair if we are close enough and zoomed in
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
                    
                    // Generate visual repair particles
                    addNaniteParticles(parts[pId].position);

                    // De-escalate and zoom back out automatically
                    gameState.isInspecting = false;
                    gameState.feedbackMessage = "REPAIR COMPLETE! RETRACTING SCANNER...";
                    gameState.feedbackTimer = 2.0f;
                } else {
                    // Exit zoom inspection view
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

    // Translate drone in 3D coordinate space
    // Update drone position based on yaw steering
    float speed = drone.speed;
    float dx = 0.0f, dy = 0.0f, dz = 0.0f;
    bool moved = false;

    // Calculate translation vector relative to yaw direction
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
        dy += speed; // Move drone up on Y axis
        moved = true;
    }
    if (keyStates['e'] || keyStates['E']) {
        dy -= speed; // Move drone down on Y axis
        moved = true;
    }

    // Translate drone position
    if (dx != 0.0f || dy != 0.0f || dz != 0.0f) {
        translateDrone(drone, dx, dy, dz);
    }

    // Yaw steering: rotate drone around Y axis
    if (keyStates['j'] || keyStates['J']) {
        drone.rotation.y += 2.2f;
        moved = true;
    }
    if (keyStates['l'] || keyStates['L']) {
        drone.rotation.y -= 2.2f;
        moved = true;
    }

    // Set engine thrust particle intensity based on movement
    if (moved) {
        drone.thrusterIntensity = fminf(1.0f, drone.thrusterIntensity + 0.12f);
        addThrusterParticles();
        gameState.algorithmActiveTimer[0] = 1.2f; // Active Translation
    } else {
        drone.thrusterIntensity = fmaxf(0.0f, drone.thrusterIntensity - 0.08f);
    }

    // Translate carried part ()
    if (drone.carriedPart != -1) {
        int carriedId = drone.carriedPart;
        SpacePart& part = parts[carriedId];

        // Manipulator arms shape settings when holding
        drone.armRotation = 45.0f; 
        gameState.algorithmActiveTimer[0] = 1.2f; // Active Translation

        if (part.state == PICKED_UP || part.state == INSPECTING) {
            // Call translation helper to move carried part
            translateCarriedPart(drone, part, radYaw, gameState.zoomFactor, gameState.isInspecting);

            // Call scaling function to update zoom factor
            updateInspectZoom(gameState.isInspecting, gameState.zoomLerp, gameState.zoomFactor);
            if (gameState.isInspecting) {
                gameState.algorithmActiveTimer[2] = 1.2f; // Active Scaling
            } else if (gameState.zoomLerp < 0.01f && part.state == INSPECTING) {
                part.state = PICKED_UP;
            }

            // Call rotation function to rotate carried part manually
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

            // Snap part to slot if close and aligned
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
        // Apply gentle idle movement to robot arms
        drone.armRotation = sin(gameState.elapsedTime * 2.0f) * 12.0f;
    }
}


void keyboardExtra(unsigned char key, int x, int y) {
    if (key == 27) { // ESC key
        exit(0);
    }
    keyboardDown(key, x, y);
}


