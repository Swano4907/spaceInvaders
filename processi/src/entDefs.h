#ifndef ENTDEFS_H
#define ENTDEFS_H

//LIBRERIE
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h> //per processi
#include <sys/types.h>
#include <sys/wait.h> //processi
#include <fcntl.h>
#include <errno.h>
#include <time.h>

//MACRO

//macro finestre di gioco
#define MAXL 30
#define MAXC 61
#define INITLINE 0
#define INITCOL 0

//macro nemici
#define ENEMIES 9

//macro usate nello switch del render per capire cosa si viene letto
#define NEMICO 1
#define NEMICOSEC 3 
#define PLAYER 2
#define BULLET 5
#define BULLETENEMY 6

#define LETTURA 0
#define SCRITTURA 1


#define DIMNAVE 3  //DIMENSIONE NAVE SINGOLA
#define DIMNAVES 8  //DIMENSIONE NAVE SINGOLA



#define VERDE 3
#define GIALLO 2
#define ROSSO 1
#define AZZURRO 5
#define BLUE 6
#define BIANCO 4

//SPRITE NAVICELLE

//STRUTTURE
typedef struct
{

    char simbol;
    int x, y, oldx, oldy;
} Sprite;

typedef struct
{
    Sprite nave[DIMNAVE][DIMNAVES];
    int level;
    int id;
    int pid;
    bool shoot;
    int vite;
    int type;
    int dir;
} Entity;

#endif
