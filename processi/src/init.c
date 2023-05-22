#include "init.h"

char sprt[9][10] = {
    {'\\', ' ', '/', ' ', '#', ' ', ' ', 'V', ' '},
    {' ', 'V', ' ', '@', '@', '@', '\\', '_', '/'},
    {'/', 'U', '\\', 'I', 'W', 'I', '\\', 'O', '/'},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', '+', ' ', '(', 'H', ')', '/', '~', '\\'},
    {'*', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'+', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', 'T', ' ', 'T', ' ', 'T', ' ', 'T', ' '},
    {' ', 'V', ' ', 'V', ' ', 'V', ' ', 'V', ' '}};

/**
 * @brief
 *  Funzione che inizializza una entita di gioco con i parametri principali.
 * 
 * @param posX 
 * @param posY 
 * @param spriteType 
 * da questo posso dedurre la larghezza della sprite
 * 
 * @param type 
 * @return Entity 
 */
Entity initEntity(int posX, int posY, int spriteType, int type, int dir)
{
    //SEZIONE DICHIARATIVA
    Entity e;

    //SEZIONE ESECUTIVA
    //
    e.type = type;
    e.dir = dir;

    if (spriteType == NEMICOSEC)
    {
        e.level = NEMICOSEC;
        initSpriteSecond(e.nave, posX, posY);
    }
    else
    {
        e.level = 1;
        if (type == PLAYER)
            e.level = 2;
        initSpriteFirst(e.nave, spriteType, posX, posY);
    }

    return e;
}

/**
 * @brief 
 *  Procedura che inizializza una sprite di una navicella
 *  in base al tipo di sprite posso dedurre la larghezza della navicella 
 *  e capire se e' un nemico di primo livello o di secondo. Per il player non 
 *  fa differenza
 * 
 * @param n 
 * Matrice contenente la sprite di un'Entity 
 * @param sprite 
 * Tipo di sprite da assegnare alla matrice
 * @param posX 
 * Posizione iniziale in x
 * @param posY 
 * Posizione iniziale in y
 */
void initSpriteFirst(Sprite n[DIMNAVE][DIMNAVES], int sprite, int posX, int posY)
{
    //SEZIONE DICHIARATIVA
    int larghezza;
    int supX = posX;
    int i, k, h = 0;

    //SEZIONE ESECUTIVA
    //trovo la larghezza della navicella;

    //inizializzo COORDINATE sprite
    for (i = 0; i < DIMNAVE; i++)
    {
        for (k = 0; k < DIMNAVES; k++)
        {
            n[i][k].oldx = n[i][k].x = -1;
            n[i][k].oldy = n[i][k].y = -1;
        }
    }

    for (i = 0; i < DIMNAVE; i++)
    {
        for (k = 0; k < DIMNAVE; k++)
        {
            n[i][k].simbol = sprt[sprite][h];
            n[i][k].oldx = n[i][k].x = posX;
            n[i][k].oldy = n[i][k].y = posY;

            posX++;
            h++;
        } //fine for righe
        posX = supX;
        posY++;
    }
}

/**
 * @brief 
 *  Procedura che inizializza la sprite del nemico di secondo livello
 *  
 * 
 * @param n 
 *  Sprite da inizializzare
 * @param posX 
 *  Coordinata iniziale
 * @param posY 
 *  Coordinata iniziale
 */
void initSpriteSecond(Sprite n[DIMNAVE][DIMNAVES], int posX, int posY)
{
    //SEZIONE DICHIARATIVA
    int i, k;
    int supX = posX;
    //SEZIONE ESECUTIVA
    for (i = 0; i < DIMNAVES; i++) //inizializzo la prima riga della sprite
    {
        n[0][i].oldx = n[0][i].x = posX;
        n[0][i].oldy = n[0][i].y = posY;
        n[0][i].simbol = ' ';
        posX++;
    }
    posY++;
    posX = supX;
    for (i = 0; i < DIMNAVES; i++) //inizializzo la seconda riga della sprite
    {
        n[1][i].oldx = n[1][i].x = posX;
        n[1][i].oldy = n[1][i].y = posY;
        n[1][i].simbol = sprt[7][i];
        posX++;
    }
    posY++;
    posX = supX;
    for (i = 0; i < DIMNAVES; i++) //inizializzo la terza riga della sprite
    {
        n[2][i].oldx = n[2][i].x = posX;
        n[2][i].oldy = n[2][i].y = posY;
        n[2][i].simbol = sprt[8][i];
        posX++;
    }
}