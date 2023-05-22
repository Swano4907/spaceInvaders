#ifndef RENDER_H
#define RENDER_H

#include "entDefs.h"

void printSprite(Entity e, WINDOW *win);
void printSpriteSecond(Entity e, WINDOW *w);
void clearSprite(Entity e, WINDOW *w);


void collisionManager(int pipePos, int pipeWriteEnemies[ENEMIES][2], int pipePlayer);

int checkCollision(int idRead, Entity nemici[]);
bool bulletCollision(int posX, int posY, Entity e);

void endWindow(int vite, int punteggio);

#endif