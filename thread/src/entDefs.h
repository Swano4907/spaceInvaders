#ifndef ENTDEFS_H
#define ENTDEFS_H

//LIBRERIE
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h> //per processi
#include <sys/types.h>
#include <sys/wait.h> //processi
#include <pthread.h> //thread
#include <semaphore.h> //thread
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>   //Per id univoci

//MACRO

//macro finestre di gioco
#define MAXL 30
#define MAXC 61
#define INITLINE 0
#define INITCOL 0

//macro nemici
#define ENEMIES 6

//macro buffer
#define BUFFER_SIZE 256

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
    intptr_t pid;
    bool shoot;
    bool shootT;
    int vite;
    int type;
    int dir;
} Entity;

typedef struct
{
    pthread_mutex_t *mutex;
    sem_t *disponibili; // Numero posizioni libere nel buffer
    sem_t *presenti;    // Numero elementi inseriti nel buffer

    int *bIN, *bOUT;

    Entity *e;
    Entity **buffer;
}ThreadParam;


/*
    GameManagerParam

    Struttura dedicata del thread del "Collision Manager",
    ovvero il thread che si preoccupa della gestione delle
    collisioni e del rendering a schermo.
*/
typedef struct
{
    Entity **buffer;
    int *bIn;
    int *bOut;
    Entity *nemici;
    pthread_t *IDNemici;
    ThreadParam *PNemici;
    Entity *player;
    pthread_t *IDPlayer;
    pthread_mutex_t *mutex;
    sem_t *disponibili;
    sem_t *presenti;
}GameManagerParam;
#endif
