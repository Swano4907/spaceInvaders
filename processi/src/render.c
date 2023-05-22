#include "render.h"

void collisionManager(int pipePos, int pipeWriteEnemies[ENEMIES][2], int pipePlayer)
{
    //SEZIONE DICHIARATIVA
    Entity nemici[ENEMIES];
    Entity readEntity;
    Entity p;
    WINDOW *w, *status;

    int colliso = -1;
    int i, k;
    int viteP = 3;
    int punteggio = 0;
    bool flagBullet;
    //SEZIONE ESECUTIVA

    //inizializzo i colori
    init_pair(VERDE, COLOR_GREEN, COLOR_BLACK);
    init_pair(GIALLO, COLOR_YELLOW, COLOR_BLACK);
    init_pair(ROSSO, COLOR_RED, COLOR_BLACK);
    init_pair(AZZURRO, COLOR_CYAN, COLOR_BLACK);
    init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(BIANCO, COLOR_WHITE, COLOR_BLACK);

    //inizializzo il vettore di nemici
    for (i = 0; i < ENEMIES; i++)
    {
        nemici[i].id = -1;
        nemici[i].vite = 3;
    }

    //inizializzo finestre di gioco
    //creo la finestra degli status di gioco
    status = newwin(4, MAXC / 3, INITLINE, (MAXC + INITCOL + 1));
    box(status, ACS_VLINE, ACS_HLINE);

    //creo la finesta di gioco
    w = newwin(MAXL, MAXC, INITLINE, INITCOL);
    box(w, ACS_VLINE, ACS_HLINE);
    FILE *f;

    do
    {
        read(pipePos, &readEntity, sizeof(Entity));

        switch (readEntity.type)
        {
        case PLAYER:
            p = readEntity;
            wattron(w, COLOR_PAIR(viteP + 3));
            printSprite(readEntity, w);
            wattroff(w, COLOR_PAIR(viteP + 3));
            break;
            FILE *f;

        case NEMICO:
            if (readEntity.vite > 0)
            {
                //controllo che un nemico non abbia raggiunto il giocatore
                if (readEntity.nave[0][0].y != p.nave[0][0].y || readEntity.nave[0][1].y != p.nave[0][0].y || readEntity.nave[0][2].y != p.nave[0][0].y)
                {
                    //salvo il nemico letto nel vettore dei nemici
                    nemici[readEntity.id] = readEntity;

                    //controllo delle collisioni

                    colliso = checkCollision(readEntity.id, nemici);

                    if (colliso != -1)
                    {
                        nemici[readEntity.id].dir *= -1;
                        nemici[colliso].dir *= -1;

                        write(pipeWriteEnemies[colliso][SCRITTURA], &nemici[colliso], sizeof(Entity));
                        write(pipeWriteEnemies[readEntity.id][SCRITTURA], &nemici[readEntity.id], sizeof(Entity));
                    }
                    //stampo i nemici in base al livello
                    wattron(w, COLOR_PAIR(nemici[readEntity.id].vite));
                    printSprite(nemici[readEntity.id], w);
                    wattroff(w, COLOR_PAIR(nemici[readEntity.id].vite));
                }
                else
                {
                    viteP = 0;
                }
            }
            break;

        case BULLETENEMY:
            mvwaddch(w, readEntity.nave[0][0].oldy, readEntity.nave[0][0].oldx, ' ');
            if (readEntity.nave[0][0].y >= MAXL - 1)
            {
                kill(readEntity.pid, 1);
            }
            else
            {
                if (bulletCollision(readEntity.nave[0][0].x, readEntity.nave[0][0].y, p) == true)
                {
                    kill(readEntity.pid, 1);
                    viteP--;
                    wattron(w, COLOR_PAIR(viteP + 3));
                    printSprite(readEntity, w);
                    wattroff(w, COLOR_PAIR(viteP + 3));
                }
                else
                {
                    mvwaddch(w, readEntity.nave[0][0].y, readEntity.nave[0][0].x, readEntity.nave[0][0].simbol);
                }
            }
            break;
        case BULLET:

            //cancello il proiettile nella vecchia posizione
            mvwaddch(w, readEntity.nave[0][0].oldy, readEntity.nave[0][0].oldx, ' ');

            //controllo che il proiettile non esca fuori dallo schermo
            if (readEntity.nave[0][0].y >= 1 &&
                (readEntity.nave[0][0].x < MAXC - 1 && readEntity.nave[0][0].x >= 1))
            {
                for (i = 0; i < ENEMIES; i++)
                {
                    //se il proiettile ha colliso con un nemico
                    if (bulletCollision(readEntity.nave[0][0].x, readEntity.nave[0][0].y, nemici[i]) == true)
                    {
                        nemici[i].vite--;
                        //aggiorno il processo nemico dell'accaduto nefasto
                        write(pipeWriteEnemies[i][SCRITTURA], &nemici[i], sizeof(Entity));

                        //termino il proiettile
                        kill(readEntity.pid, 1);

                        flagBullet = true;

                        //se il nemico e'
                        if (nemici[i].vite == 0)
                        {
                            //dopo aver ucciso un nemico aumento il punteggio
                            punteggio++;
                            clearSprite(nemici[i], w);
                            wrefresh(w);
                        }
                        //esco dal ciclo
                        i = ENEMIES;

                    } //fine if collisione proiettile
                }

                if (i == ENEMIES) //se collisione avvenuta
                {
                    mvwaddch(w, readEntity.nave[0][0].y, readEntity.nave[0][0].x, readEntity.nave[0][0].simbol);
                    flagBullet = false;
                }
            }
            else
            {
                kill(readEntity.pid, 1);
                flagBullet = true;
            }
            write(pipePlayer, &flagBullet, sizeof(bool));
            break;
        }
        mvwprintw(status, 1, 2, "Vite: %d", viteP);
        mvwprintw(status, 2, 2, "Punti: %d", punteggio);
        wrefresh(status);

        wrefresh(w);
    } while (viteP > 0 && punteggio < (ENEMIES * 2));

    delwin(w);
    delwin(status);

    //finestra finale
    endWindow(viteP, punteggio);
}

/**
 * @brief 
 *  Procedura che data una Entity stampa la sua sprite 
 *  La cancella nella vecchia posizione e la ri disegna in quella nuova
 * 
 * @param e 
 *  entity contenente la Sprite da stampare
 * @param win 
 *  finestra dove disegnare la sprite
 */
void printSprite(Entity e, WINDOW *win)
{
    //SEZIONE DICHIARATIVA
    int larghezza;
    int i, k;
    //SEZIONE ESECUTIVA

    if (e.level != 3)
        larghezza = DIMNAVE;
    else
        larghezza = DIMNAVES;

    if (win == NULL)
    {
        perror("Finestra di gioco non funzionante");
        _exit(1);
    }
    //cancello la sprite nella vecchia posizione
    for (i = 0; i < DIMNAVE; i++)
    {
        for (k = 0; k < larghezza; k++)
        {
            mvwaddch(win, e.nave[i][k].oldy, e.nave[i][k].oldx, ' ');
        }
    }

    //disegno la sprite nella nuova posizione
    for (i = 0; i < DIMNAVE; i++)
    {
        for (k = 0; k < larghezza; k++)
        {
            mvwaddch(win, e.nave[i][k].y, e.nave[i][k].x, e.nave[i][k].simbol);
        }
    }
    wrefresh(win);
} //fine printSprite

/**
 * @brief 
 *  Funzione che controlla le collisioni 
 *  riceve in come parametro il vettore dove sono salvati tutti i nemici
 *  l'indice del nemico utilizzato sul momento.
 *  Il nemico letto viene confrontato con tutti i nemici in gioco.
 *  La funzione rente in output l'id del nemico con cui si e' colliso
 * 
 * @param idRead 
 *  Nemico letto 
 * @param nemici 
 *  Vettore dei nemici
 * @return int 
 *  Id nemico concui si ha colliso
 */
int checkCollision(int idRead, Entity nemici[])
{
    //SEZIONE DICHIARATIVA
    int collisione = -1;
    int i;
    int larghezzaR, larghezzaN;

    //SEZIONE ESECUTIVA
    if (nemici[idRead].level != 3)
        larghezzaR = DIMNAVE;
    else
        larghezzaR = DIMNAVES;

    for (i = 0; i < ENEMIES; i++)
    {
        //recupero la larghezza del nemico nel vettore
        if (nemici[i].level != 3)
            larghezzaN = DIMNAVE;
        else
            larghezzaN = DIMNAVES;

        //controllo la collisione
        if (idRead != i)
        {
            if (nemici[idRead].nave[0][0].y == nemici[i].nave[0][0].y && nemici[i].vite > 0)
            {
                if ((nemici[idRead].dir == 1 && nemici[i].dir == -1) && nemici[idRead].nave[0][larghezzaR - 1].x + 1 == nemici[i].nave[0][0].x - 1)
                {
                    collisione = i;
                    i = ENEMIES;
                }
                else if ((nemici[idRead].dir == -1 && nemici[i].dir == 1) && nemici[idRead].nave[0][0].x - 1 == nemici[i].nave[0][larghezzaN - 1].x + 1)
                {
                    collisione = i;
                    i = ENEMIES;
                }
            }
        }
    }
    return collisione;
}

/**
 * @brief 
 *  
 * 
 * @param posX 
 * @param posY 
 * @param e 
 * @return true 
 * @return false 
 */
bool bulletCollision(int posX, int posY, Entity e)
{
    //SEZIONE DICHIARATIVA
    bool r = false;
    int larghezza;
    int i, k;
    //SEZIONE ESECUTIVA
    if (e.level != 3)
        larghezza = DIMNAVE;
    else
        larghezza = DIMNAVES;
    if (e.vite > 0)
    {
        for (i = 0; i < DIMNAVE; i++)
        {
            for (k = 0; k < larghezza; k++)
            {
                if (e.level == 2) //se e' il giocatore
                {
                    if (e.nave[i][k].x == posX && e.nave[i][k].y == posY)
                    {
                        r = true;
                        k = i = larghezza;
                    }
                }
                else //nemico
                {
                    if ((e.nave[i][k].x == posX + 1 || e.nave[i][k].x == posX - 1) && e.nave[i][k].y == posY)
                    {
                        r = true;
                        k = i = larghezza;
                    }
                }
            }
        }
    }

    return r;
}

void clearSprite(Entity e, WINDOW *w)
{
    //SEZIONE DICHIARATIVA
    int i, k;
    int larghezza;
    //SEZIONE ESECUTIVA
    if (e.level != 3)
        larghezza = DIMNAVE;
    else
        larghezza = DIMNAVES;

    for (i = 0; i < DIMNAVE; i++)
    {
        for (k = 0; k < larghezza; k++)
            mvwaddch(w, e.nave[i][k].y, e.nave[i][k].x, ' ');
    }
    wrefresh(w);
}

void endWindow(int vite, int punteggio)
{
    //SEZIONE DICHIARATIVA
    WINDOW *win;
    char enter;
    //SEZIONE ESECUTIVA

    win = newwin(MAXL, MAXC, INITLINE, INITCOL);
    box(win, ACS_VLINE, ACS_HLINE);

    do
    {
        enter = getch();
        if (vite > 0)
        {
            mvwprintw(win, MAXL / 2 - 1, (MAXC / 2) - 5, "HAI VINTO!");
            mvwprintw(win, MAXL / 2 + 1, 21, "Vite rimaste: %d", vite);
        }
        else
            mvwprintw(win, MAXL / 2 - 1, (MAXC / 2) - 5, "GAME OVER!");

        mvwprintw(win, MAXL / 2, 23, "Punteggio: %d", punteggio);
        mvwprintw(win, MAXL / 2 + 2, 21, "Premi INVIO per uscire");
        wrefresh(win);
    } while (enter != 10);
    delwin(win);
}
