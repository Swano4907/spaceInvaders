#ifndef RENDER_H
#define RENDER_H

#include "entDefs.h"

void printSprite(Entity e, WINDOW *win);
void printSpriteSecond(Entity e, WINDOW *w);
void clearSprite(Entity e, WINDOW *w);


void *collisionManager(void* parms);

int checkCollision(int idRead, Entity nemici[]);
bool bulletCollision(int posX, int posY, Entity e);

void endWindow(int vite, int punteggio);

bool shouldSkip(intptr_t uid, intptr_t **buffer, int size, int index);
void addSkip(intptr_t uid, intptr_t **buffer, int *size, int *index);

#endif