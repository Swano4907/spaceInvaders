#ifndef ENTITY_H
#define ENTITY_H

#include "init.h"

//macro movimento giocatore
#define KEY_W 119
#define KEY_A 97
#define KEY_S 115
#define KEY_D 100

void enemyController(int pipeWrite, int pipeRead, Entity e);
void moveSprite(Sprite n[DIMNAVE][DIMNAVES], int velocita, int larghezza, int dir);
void moveSpriteDown(Sprite n[DIMNAVE][DIMNAVES], int larghezza);
void playerController(int pipeWrite, int pipeRead);

void singleBullet(int pipeWrite, int type, int posX, int posY, int dir);
void doubleBullet();

#endif