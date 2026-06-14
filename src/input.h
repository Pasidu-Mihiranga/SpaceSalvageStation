#ifndef INPUT_H
#define INPUT_H

#include "../structures.h"

void keyboardDown(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void specialDown(int key, int x, int y);
void specialUp(int key, int x, int y);
void processInput();
void keyboardExtra(unsigned char key, int x, int y);

#endif
