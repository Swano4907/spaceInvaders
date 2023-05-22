#include "render.h"

void *collisionManager(void *parms)
{
    //SEZIONE DICHIARATIVA

    GameManagerParam *par = (GameManagerParam *)parms;

    Entity **buffer = par->buffer;
    int *bIn = par->bIn;
    int *bOut = par->bOut;
    Entity *nemici = par->nemici;
    pthread_t *IDNemici = par->IDNemici;
    Entity *player = par->player;
    pthread_t *IDPlayer = par->IDPlayer;
    pthread_mutex_t *mutex = par->mutex;
    sem_t *disponibili = par->disponibili;
    sem_t *presenti = par->presenti;

    Entity *readEntity; //Job (da copiare dal buffer)
    WINDOW *w, *status;

    int colliso = -1;
    int i, k;
    int viteP = 3;
    int punteggio = 0;
    bool flagBullet;

    Entity p = *player;

    //SEZIONE ESECUTIVA

    //inizializzo i colori
    init_pair(VERDE, COLOR_GREEN, COLOR_BLACK);
    init_pair(GIALLO, COLOR_YELLOW, COLOR_BLACK);
    init_pair(ROSSO, COLOR_RED, COLOR_BLACK);
    init_pair(AZZURRO, COLOR_CYAN, COLOR_BLACK);
    init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(BIANCO, COLOR_WHITE, COLOR_BLACK);

    //inizializzo finestre di gioco
    //creo la finestra degli status di gioco
    status = newwin(4, MAXC / 3, INITLINE, (MAXC + INITCOL + 1));
    box(status, ACS_VLINE, ACS_HLINE);

    //creo la finesta di gioco
    w = newwin(MAXL, MAXC, INITLINE, INITCOL);
    box(w, ACS_VLINE, ACS_HLINE);
    FILE *f;

    //Roba per fixare le collisioni proiettile - player

    bool pCol; //Flag collisionie
    pCol = false;

    intptr_t *skipBuffer = NULL; // Buffer che tiene conto degli ultimi proiettili morti per skippare i loro job - è una CODA CIRCOLARE
    int skip_I = 0;              // Indice del prossimo spazio del vettore da occupare
    int skipSize = 0;            // Dimensione corrente del buffer (a inizio programma non ci sono bullet recentemente morti)

    intptr_t tempID; // ID Univoco del proiettile letto nel job in esame

    do
    {
        //[-!-]Leggi dal buffer --> Job = Entity

        sem_wait(presenti);        // Attendi elementi nel buffer
        pthread_mutex_lock(mutex); // Inizio sezione critica

        readEntity = buffer[*bOut];        // Lettura elemento dal buffer
        *bOut = (*bOut + 1) % BUFFER_SIZE; // Aggiornamento indice

        //[-!-]Controllo su entità letta - readEntity sarà puntatore

        switch (readEntity->type)
        {
        case PLAYER:
            wattron(w, COLOR_PAIR(viteP + 3));
            printSprite(*readEntity, w);
            wattroff(w, COLOR_PAIR(viteP + 3));
            break;
            FILE *f;

        case NEMICO:
            if (readEntity->vite > 0)
            {
                //controllo che un nemico non abbia raggiunto il giocatore
                if (readEntity->nave[0][0].y != player->nave[0][0].y || readEntity->nave[0][1].y != player->nave[0][0].y || readEntity->nave[0][2].y != player->nave[0][0].y)
                {
                    //controllo delle collisioni

                    colliso = checkCollision(readEntity->id, nemici);

                    if (colliso != -1)
                    {
                        //[-!-]Invertire direzione ad entrambi
                        nemici[readEntity->id].dir *= -1;
                        nemici[colliso].dir *= -1;
                    }

                    //stampo i nemici in base al livello
                    wattron(w, COLOR_PAIR(nemici[readEntity->id].vite));
                    printSprite(nemici[readEntity->id], w);
                    wattroff(w, COLOR_PAIR(nemici[readEntity->id].vite));
                }
                else
                {
                    viteP = 0;
                    player->vite = 0;
                }
            }
            break;

        case BULLETENEMY:
            mvwaddch(w, readEntity->nave[0][0].oldy, readEntity->nave[0][0].oldx, ' ');

            //Salvo l'UID del proiettile (salvato nel campo pid)
            tempID = readEntity->pid;

            //Controllo se l'UID del thread non sia presente nel buffer dei proiettili morti
            if (/*shouldSkip(tempID, &skipBuffer, skipSize, skip_I) == true*/ readEntity->vite < 1)
            {
                //mvwprintw(status, 5, 2, "[SKIP JOB SIGNAL DETECTED] %li", tempID);
                break;
            }

            if (readEntity->nave[0][0].y >= MAXL - 1)
            {
                readEntity->vite = 0;
            }
            else
            {
                /* if(readEntity->vite > 0 && readEntity->nave[0][0].y == player->nave[0][0].y)
                {
                    if(readEntity->nave[0][0].x >= player->nave[0][0].x &&
                       readEntity->nave[0][0].x <= player->nave[2][2].x)
                       {
                           pCol = true;
                           readEntity->vite = 0;
                       }
                       else
                           pCol = false;


                }*/

                pCol = false;
                if (readEntity->vite > 0)
                {
                    for (i = 0; i < DIMNAVE; i++)
                    {
                        for (k = 0; k < DIMNAVE; k++)
                        {
                            if (readEntity->nave[0][0].y == player->nave[i][k].y && readEntity->nave[0][0].x == player->nave[i][k].x)
                                pCol = true;
                        }
                    }
                }

               // mvwprintw(status, 4, 2, "[DEBUG]:");
               // mvwprintw(status, 5, 2, "pCol: %d", pCol);

                //if (bulletCollision(readEntity->nave[0][0].x, readEntity->nave[0][0].y, (*player)) == true)
                if (pCol == true)
                {
                    addSkip(tempID, &skipBuffer, &skipSize, &skip_I); // I prossimi job di questo proiettile vanno ignorati
                    readEntity->vite = 0;
                    viteP--;
                    player->vite--;
                    wattron(w, COLOR_PAIR(viteP + 3));
                    printSprite(*readEntity, w);
                    wattroff(w, COLOR_PAIR(viteP + 3));
                    pCol = false;
                }
                else
                {
                    mvwaddch(w, readEntity->nave[0][0].y, readEntity->nave[0][0].x, readEntity->nave[0][0].simbol);
                }

               /* if (readEntity->nave[0][0].y == player->nave[0][0].y)
                {
                   /* if (readEntity->nave[0][0].x >= player->nave[0][0].x &&
                        readEntity->nave[0][0].x <= player->nave[2][2].x)
                    {
                        mvwprintw(status, 7, 2, "     ");
                    }
                    else
                        mvwprintw(status, 7, 2, "ERROR"); //Si è verificata un'incongruenza nel rilevamento collisioni
                }*/
            }
            break;
        case BULLET:

            if (readEntity->vite == 0)
                break;

            //cancello il proiettile nella vecchia posizione
            mvwaddch(w, readEntity->nave[0][0].oldy, readEntity->nave[0][0].oldx, ' ');

            //controllo che il proiettile non esca fuori dallo schermo
            if (readEntity->nave[0][0].y >= 1 &&
                (readEntity->nave[0][0].x < MAXC - 1 && readEntity->nave[0][0].x >= 1))
            {
                for (i = 0; i < ENEMIES; i++)
                {
                    //se il proiettile ha colliso con un nemico
                    if (bulletCollision(readEntity->nave[0][0].x, readEntity->nave[0][0].y, nemici[i]) == true)
                    {
                        nemici[i].vite--;

                        //termino il proiettile
                        readEntity->vite = 0;

                        //se il nemico e'
                        if (nemici[i].vite == 0)
                        {
                            //dopo aver ucciso un nemico aumento il punteggio
                            punteggio++;
                            clearSprite(nemici[i], w);
                        }
                        //esco dal ciclo
                        i = ENEMIES;

                    } //fine if collisione proiettile
                }

                if (i == ENEMIES) //se collisione avvenuta
                {
                    mvwaddch(w, readEntity->nave[0][0].y, readEntity->nave[0][0].x, readEntity->nave[0][0].simbol);
                }
            }
            else
            {
                readEntity->vite = 0;
            }
            break;
        }

        mvwprintw(status, 1, 2, "Vite: %d", viteP);
        mvwprintw(status, 2, 2, "Punti: %d", punteggio);
       //mvwprintw(status, 3, 2, "Player: %d,%d", (*player).nave[0][0].x, (*player).nave[0][0].y);
        //mvwprintw(status, 6, 2, "SHUTt: %d   SHUT: %d", player->shootT, player->shoot);
        wrefresh(status);

        wrefresh(w);

        sem_post(disponibili);       // Nuova posizione libera nel buffer
        pthread_mutex_unlock(mutex); // Fine sezione critica
    } while (viteP > 0 && punteggio < (ENEMIES * 2));

    if (punteggio >= (ENEMIES * 2))
        player->vite = 4;

    delwin(w);
    delwin(status);

    //finestra finale
    endWindow(viteP, punteggio);
}

void printSprite(Entity e, WINDOW *win)
{
    //SEZIONE DICHIARATIVA
    int larghezza;
    int i, k;
    //SEZIONE ESECUTIVA

    if (e.level != NEMICOSEC)
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

int checkCollision(int idRead, Entity nemici[])
{
    //SEZIONE DICHIARATIVA
    int collisione = -1;
    int i;
    int larghezzaR, larghezzaN;
    FILE *f;

    //SEZIONE ESECUTIVA
    if (nemici[idRead].level != 3)
        larghezzaR = DIMNAVE;
    else
        larghezzaR = DIMNAVES;

    for (i = 0; i < ENEMIES; i++)
    {
        //recupero la larghezza del nemico nel vettore
        if (nemici[i].level != NEMICOSEC)
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
                    f = fopen("collisioni.txt", "a");
                    fprintf(f, "%d-%d\n", idRead, i);
                    fclose(f);
                    i = ENEMIES;
                }
                else if ((nemici[idRead].dir == -1 && nemici[i].dir == 1) && nemici[idRead].nave[0][0].x - 1 == nemici[i].nave[0][larghezzaN - 1].x + 1)
                {
                    collisione = i;
                    f = fopen("collisioni.txt", "a");
                    fprintf(f, "%d-%d\n", idRead, i);
                    fclose(f);
                    i = ENEMIES;
                }
            }
        }
    }
    return collisione;
}

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

    for (i = 0; i < DIMNAVE; i++)
    {
        for (k = 0; k < larghezza; k++)
        {
            if (e.vite > 0)
            {
                if (e.level == 2) //se e' il giocatore
                {
                    if (e.nave[i][k].x == posX && e.nave[i][k].y == posY)
                    {
                        r = true;
                        k = i = larghezza;
                    }
                }
                else
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

/*
    shouldSkip

    Questa funzione prende in input un id di un proiettile e lo confronta
    con gli altri id nel buffer. Se questo non è presente, lo inserisce in
    coda.

    uid:    L'ID in esame da cercare
    buffer: Il buffer di ID, se è NULL il buffer viene inizializzato (per questo è ptr di ptr)
    size:   La dimensione corrente del buffer (dimensione massima = BUFFER_SIZE *)
    index:  La posizione in cui l'ID verrà messo se non presente (o se il BUFFER_SIZE non è ancora raggiunto)

    return:
            -true:  L'ID in esame è già presente nel buffer, quindi in job in questione va skippato poichè l'entità è morta
            -false: L'ID in esame NON è presente nel buffer, quindi questo job è l'ultimo job che l'entità ha mandato 
                    prima che il main mandasse la richiesta di cancellazione al suo thread
*/
bool shouldSkip(intptr_t uid, intptr_t **buffer, int size, int index)
{
    // Buffer vuoto, di sicuro non c'è
    if (*buffer == NULL)
        return false;

    //Indici di ricerca nel buffer
    bool present = false;
    int i = 0;

    //Ricerca nel buffer
    while (present == false && i < size)
    {
        if (uid == *buffer[i])
            present = true;

        i++;
    }

    return present; // False = non trovato, true = trovato
}

/*
    addSkip

    Questa funzione prende in input un id di un proiettile e lo inserisce in
    coda al buffer degli ID.

    uid:    L'ID in da aggiungere
    buffer: Il buffer di ID, se è NULL il buffer viene inizializzato (per questo è ptr di ptr)
    size:   La dimensione corrente del buffer (dimensione massima = BUFFER_SIZE *)
    index:  La posizione in cui l'ID verrà messo se non presente (o se il BUFFER_SIZE non è ancora raggiunto)

    * Inizialmente il buffer non ha dimensione. Man mano che nuovi ID vengono aggiunti la dimensione cresce
      fino ad eguagliare quella del buffer dei job (BUFFER_SIZE). Quando questo avviene il buffer id diventa
      una coda circolare, e si comporta come tale fino al termine del programma.

      NB: A fine funzione il valore di index (se il buffer non è ancora circolare) sarà [size - 1]
*/
void addSkip(intptr_t uid, intptr_t **buffer, int *size, int *index)
{
    //Il buffer è vuoto, va tutto inizializzato
    if (*buffer == NULL)
    {
        *buffer = (intptr_t *)malloc(sizeof(intptr_t));
        if (*buffer == NULL)
        {
            printf("ERRORE: IMPOSSIBILE INIZIALIZZARE BUFFER ID PROIETTILI");
            _exit(-1);
        }

        *size = 1;
        *index = 0;

        *buffer[0] = uid; // Primo inserimento

        return;
    }

    // Inserimento nel buffer

    if (*size < BUFFER_SIZE) // Il buffer NON è ancora una coda circolare
    {
        //Incremento dimensione del buffer
        *buffer = (intptr_t *)realloc(*buffer, ((*size) + 1) * sizeof(intptr_t));

        *size = (*size) + 1;  // Aggiorno parametro dimensione
        *index *(*index) + 1; // Aggiorno indice inserimento
    }
    else // Il buffer è una coda circolare
    {
        //Passiamo all'indice successivo in modo circolare
        *index = ((*index) + 1) % BUFFER_SIZE;
    }

    //Inserimento dell'UID nel buffer
    *buffer[*index] = uid;
}
