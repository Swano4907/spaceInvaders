#ifndef INIT_H
#define INIT_H

#include "entDefs.h"

Entity initEntity(int posX, int posY, int spriteType, int type, int dir);
void initSpriteSecond(Sprite n[DIMNAVE][DIMNAVES], int posX, int posY);
void initSpriteFirst(Sprite n[DIMNAVE][DIMNAVES], int sprite, int posX, int posY);

#endif