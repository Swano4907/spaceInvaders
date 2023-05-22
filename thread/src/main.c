/**
 * @file main.c
 * @author Alessio Usai - Riccardo Casula
 * 
 * Progetto Sistemi Operativi
 * 
 * SpaceInvaders versione THREAD
 * 
 */

//Librerie incluse
#include "entity.h"
#include "entDefs.h"
#include "init.h"
#include "render.h"

//Firme funzioni
int startMenu();
void istruzioniWindow();

void arbiter(GameManagerParam *gmp);

void waitForTerminated(pthread_t *th, pthread_mutex_t *mutex, Entity *e);
void joinBullet(Entity *e, pthread_t *th, pthread_mutex_t *mutex);

void handleThread(Entity *e, int i, Entity *player, ThreadParam *p, pthread_t *th, pthread_mutex_t *mutex);
void handleThreads(Entity *e, Entity *player, ThreadParam *p, pthread_t *th, pthread_mutex_t *mutex);
void handleEnemy(Entity *e, int i, ThreadParam *p, pthread_t *th, pthread_mutex_t *mutex);

void modify(Entity *e);

int main()
{
    //SEZIONE DICHIARATIVA

    Entity *buffer[BUFFER_SIZE];
    int bIn = 0;
    int bOut = 0; //Indici inserimenti / estrazione job dal buffer

    Entity eNemici[ENEMIES];
    pthread_t IDNemici[ENEMIES];
    ThreadParam pNemici[ENEMIES];

    Entity player;
    pthread_t IDPlayer;
    ThreadParam pPlayer;

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    sem_t disponibili;
    sem_init(&disponibili, 0, BUFFER_SIZE); // Valore iniziale posti liberi: Dimensione buffer

    sem_t presenti;
    sem_init(&presenti, 0, 0); // Valore iniziale elementi in buffer: nessuno

    int sprite; // sprite da assegnare al nemico
    int i;

    int posInizialeInizio = 1;
    int posInizialeFine = MAXC - 4;
    int rigaIniziale = 1;
    int posinitX = 1;

    int vitePlayer;

    GameManagerParam gmp;
    pthread_t gameManager;
    //SEZIONE ESECUTIVA

    //seed numeri casuali
    srand(time(NULL));

    initscr();
    noecho();
    curs_set(0);
    start_color();
    timeout(0);
    refresh();

    //se l'utente ha deciso di iniziare una nuova partita
    if (startMenu() == 0)
    {
        istruzioniWindow();
        //Inizializza entità giocatore e starta thread
        player = initEntity(MAXC / 2 - 1, MAXL - 5, 4, PLAYER, 0);
        player.vite = 3;
        pPlayer.e = &player;
        pPlayer.mutex = &mutex;
        pPlayer.disponibili = &disponibili;
        pPlayer.presenti = &presenti;
        pPlayer.buffer = buffer;
        pPlayer.bIN = &bIn;
        pPlayer.bOUT = &bOut;

        pthread_create(&IDPlayer, NULL, &playerController, (void *)(&pPlayer)); //Avvio del thread

        for (i = 0; i < ENEMIES; i++)
        {
            //genero la sprite da assegnare al nemico
            sprite = 1 /*rand() % (2 - 0 + 1) + 0*/;

            //Inizializza entità i nemico e starta relativo thread
            // Questo if permette di posizionare i nemici due per riga, uno all'inizio e uno alla fine
            /* if (i % 2 == 1)
            {
                eNemici[i] = initEntity(posInizialeFine, rigaIniziale, sprite, NEMICO, -1);
            }
            else
            {
                eNemici[i] = initEntity(posInizialeInizio, rigaIniziale, sprite, NEMICO, 1);
            }*/

            eNemici[i] = initEntity(posinitX, rigaIniziale, sprite, NEMICO, 1);

            eNemici[i].id = i;
            eNemici[i].vite = 1;

            pNemici[i].e = &eNemici[i];
            pNemici[i].mutex = &mutex;
            pNemici[i].disponibili = &disponibili;
            pNemici[i].presenti = &presenti;
            pNemici[i].buffer = buffer;
            pNemici[i].bIN = &bIn;
            pNemici[i].bOUT = &bOut;

            pthread_create(&IDNemici[i], NULL, &enemyController, (void *)(&pNemici[i])); //Avvio del thread

            /*if (i % 2 == 1)
                rigaIniziale += 6;*/
            if (posinitX > 20)
            {
                posinitX = 1;
                rigaIniziale += 6;
            }
            else
            {
                posinitX += 18;
            }

        } //fine ciclo nemici

        //Chiamata ciclo principale del gioco
        gmp.buffer = buffer;
        gmp.bIn = &bIn;
        gmp.bOut = &bOut;
        gmp.nemici = eNemici;
        gmp.IDNemici = IDNemici;
        gmp.PNemici = pNemici;
        gmp.player = &player;
        gmp.IDPlayer = &IDPlayer;
        gmp.mutex = &mutex;
        gmp.disponibili = &disponibili;
        gmp.presenti = &presenti;

        pthread_create(&gameManager, NULL, &collisionManager, (void *)(&gmp));

        /*
            Da questo momento in poi il main sarà il thread "arbitro".
            Si occuperà di spawnare i thread dei proiettili e di joinarli
            al loro termine.
        */

        arbiter(&gmp);

        pthread_join(gameManager, NULL);
    }

    endwin();
    return 0;
}

/**
 * @brief Funzione che disegna a schermo il menu iniziale del gioco
 *        l'utente qua puo' scegliere se giocare o uscire dal programma
 * 
 * @return int scelta dell'utente
 */
int startMenu()
{
    //SEZIONE DICHIARATIVA
    WINDOW *menu, *istruzioni;
    char key;
    int cont = 0;
    int current = 0;
    //SEZIONE ESECUTIVA
    menu = newwin(MAXL, MAXC, INITLINE, INITCOL);
    box(menu, ACS_VLINE, ACS_HLINE);

    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);

    mvwprintw(menu, 1, MAXC / 4, "SPACE INVADERS [VERSIONE THREAD]");

    //ciclo per la scelta del menu
    do
    {
        key = getch();

        if (key == 65 && current == 1)
        {
            current = 0;
        }
        else if (key == 66 && current == 0)
        {
            current = 1;
        }

        switch (current)
        {
        case 0:
            wattron(menu, COLOR_PAIR(1));
            mvwprintw(menu, MAXL / 2 - 1, MAXC / 3, "> Inizia la partita");
            wattroff(menu, COLOR_PAIR(1));

            wattron(menu, COLOR_PAIR(2));
            mvwprintw(menu, MAXL / 2, MAXC / 3, "  Esci");
            wattroff(menu, COLOR_PAIR(2));
            break;
        case 1:
            wattron(menu, COLOR_PAIR(2));
            mvwprintw(menu, MAXL / 2 - 1, MAXC / 3, "  Inizia la partita");
            wattroff(menu, COLOR_PAIR(2));

            wattron(menu, COLOR_PAIR(1));
            mvwprintw(menu, MAXL / 2, MAXC / 3, "> Esci");
            wattroff(menu, COLOR_PAIR(1));
            break;
        default:
            break;
        }

        wrefresh(menu);

    } while (key != 10);

    delwin(menu);
    refresh();

    return current;
}

/**
 * @brief 
 *  Funzione che crea le finestra con le istruzioni di gioco
 * 
 */
void istruzioniWindow()
{
    //SEZIONE DICHIARATIVA
    WINDOW *w;
    char key;
    //SEZIONE ESECUTIVA

    w = newwin(MAXL, MAXC, INITLINE, INITCOL);
    box(w, ACS_VLINE, ACS_HLINE);

    mvwprintw(w, 2, MAXC / 2 - 5, "ISTRUZIONI");
    mvwprintw(w, MAXL / 3 - 2, MAXC / 3 - 5, "Tasto D -> movimento a destra");
    mvwprintw(w, MAXL / 3, MAXC / 3 - 5, "Tasto A -> movimento a sinistra");
    mvwprintw(w, MAXL / 3 + 2, MAXC / 3, "Tasto SPAZIO -> sparo");
    mvwprintw(w, MAXL / 3 + 4, MAXC / 3 - 7, "Tasto L -> cambio modalita' di sparo");
    mvwprintw(w, MAXL / 2 + 7, MAXC / 3 - 1, "Premi INVIO per iniziare");
    wrefresh(w);

    do
    {
        key = getch();

    } while (key != 10);
}

/*
    arbiter [WARNING: DESCRIZIONE PROLISSA MA NECESSARIA]

    Questa procedura si occupa di gestire l'avvio e l'arresto
    dei thread dall'avvio del thread gamemanager fino al termine
    del programma.

    Ogni thread segnala il proprio stato da comunicare all'arbiter
    tramite la scrittura di una variabile nella propria entità

    Questo thread non è un consumatore ne produttore, di
    conseguenza necessita l'utilizzo del solo mutex per accedere
    alle entità dei thread in esecuzione

    L'arbiter è colui che setta i flag delle entità nemiche e
    player per far loro sparare
    Si preoccupa inoltre di ri-spawnare i thread nemici upgradati
    al secondo livello una volta che vengono sconfitti al primo,
    ovviamente aggiornando le entità e i loro parametri


    argomenti: -Entità dei nemici e del player con relativi thread
                e parametri;
               -mutex;
*/
void arbiter(GameManagerParam *gmp)
{
    // ---               Proiettili dei nemici               ---

    Entity eb[ENEMIES];        // eb   = Enemy Bullets
    pthread_t ebts[ENEMIES];   // ebts = Enemy Bullet ThreadS
    ThreadParam ebps[ENEMIES]; // ebps = Enemy Bullet ParameterS

    // ---               Proiettili del player               ---

    Entity pb[2];        // pb   = Player Bullets
    pthread_t pbts[2];   // pbts = Player Bullet ThreadS
    ThreadParam pbps[2]; // pbps = Player Bullet ParameterS

    bool shoot, shootT;
    int lives;
    int pv;
    
    pthread_mutex_lock(gmp->mutex);
    gmp->player->shoot = true;
    pthread_mutex_unlock(gmp->mutex);

    // Inizializzazione valori dei thread
    int i = 0;

    while (i < ENEMIES)
    {
        ebps[i].mutex = gmp->mutex;
        ebps[i].disponibili = gmp->disponibili;
        ebps[i].presenti = gmp->presenti;

        ebps[i].e = &eb[i];

        ebps[i].bIN = gmp->bIn;
        ebps[i].bOUT = gmp->bOut;
        ebps[i].buffer = gmp->buffer;

        eb[i].vite = -1;
        i++;
    }

    i = 0;
    while (i < 2)
    {
        pbps[i].mutex = gmp->mutex;
        pbps[i].disponibili = gmp->disponibili;
        pbps[i].presenti = gmp->presenti;

        pbps[i].e = &pb[i];

        pbps[i].bIN = gmp->bIn;
        pbps[i].bOUT = gmp->bOut;
        pbps[i].buffer = gmp->buffer;

        pb[i].vite = -1;

        i++;
    }

    do
    {
        /*
            CONTROLLO SPARI DEL PLAYER

            Se il player ha sparato (shoot = false)
            allora è necessario "amministrare" i thread
        */

        pthread_mutex_lock(gmp->mutex);
        shoot = gmp->player->shoot;
        shootT = gmp->player->shootT;
        pthread_mutex_unlock(gmp->mutex);

        if (shoot == false) // Il Player ha sparato / Ha proiettili in vita
        {
            if (shootT == false)
                handleThread(pb, 0, gmp->player, pbps, pbts, gmp->mutex);
            else
                handleThreads(pb, gmp->player, pbps, pbts, gmp->mutex);
        }

        //Controllo spari dei nemici
        i = 0;
        while (i < ENEMIES)
        {
            pthread_mutex_lock(gmp->mutex);
            shoot = gmp->nemici[i].shoot;
            pthread_mutex_unlock(gmp->mutex);

            if (shoot == false)
                handleThread(eb, i, &gmp->nemici[i], ebps, ebts, gmp->mutex);

            i++;
        }

        //Controllo dei nemici
        i = 0;
        while (i < ENEMIES)
        {
            handleEnemy(gmp->nemici, i, gmp->PNemici, gmp->IDNemici, gmp->mutex);
            i++;
        }

        pthread_mutex_lock(gmp->mutex);
        pv = gmp->player->vite;
        pthread_mutex_unlock(gmp->mutex);

    } while (pv > 0 && pv < 4);
}

/*
    waitForTerminated

    Procedura specifica per i thread proiettile

    Effettua un test sulla variabile vite dell'entità
    ed effettua la join sul thread corrispondente in caso
    le vite siano a 0 (segnale di terminazione)


    th:    Il thread da terminare
    mutex: Mutex necessario per fare il test (la variabile è condivisa)
    e:     L'entità da prendere in esame
*/
void waitForTerminated(pthread_t *th, pthread_mutex_t *mutex, Entity *e)
{
    bool terminated;

    pthread_mutex_lock(mutex);
    terminated = (e->vite == 0);
    pthread_mutex_unlock(mutex);

    if (terminated == true) // Thread terminato: Si fa la join
        pthread_join(*th, NULL);
}

/*
    waitForTerminated

    Procedura specifica per i thread proiettile

    Joina il thread specificato con waitForTerminated
    e da il segnale all'arbiter dell'avvenuto arresto del
    thread mettendo le vite dell'entità corrispondente a -1

    th:    Il thread da terminare
    mutex: Mutex necessario per fare il test (la variabile è condivisa)
    e:     L'entità da prendere in esame
*/
void joinBullet(Entity *e, pthread_t *th, pthread_mutex_t *mutex)
{
    waitForTerminated(th, mutex, e); // Joina il thread

    pthread_mutex_lock(mutex);
    e->vite = -1;
    pthread_mutex_unlock(mutex);
}

/*
    handleThread

    Procedura specifica per i thread proiettile

    Si occupa di gestire il ciclo vitale di un proiettile singolo
    (Proiettile nemico / Proiettile di debug player)

    e:      L'entità del proiettile da gestire / Vettore delle entità (Proiettili nemici)
    i:      Indice identificativo all'interno del vettore di proiettili nemici
    player: L'entità cui appartiene il proiettile (Player / vettore di nemici)
    th:     Thread del proiettile / Vettore dei thread proiettili nemici
    p:      Parametri thread / Vettore parametri thread nemici
    mutex:  Per le sezioni critiche (Accesso ad entità)
*/
void handleThread(Entity *e, int i, Entity *player, ThreadParam *p, pthread_t *th, pthread_mutex_t *mutex)
{
    int lives;
    int larghezza;

    pthread_mutex_lock(mutex);
    lives = e[i].vite;
    pthread_mutex_unlock(mutex);

    // Il proiettile è morto?
    if (lives < 1) // Il proiettile può essere: Joinabile o da spawnare
    {
        switch (lives)
        {
        case 0: // Morto (da joinare)

            joinBullet(&e[i], &th[i], mutex);

            pthread_mutex_lock(mutex);
            player->shoot = true;
            pthread_mutex_unlock(mutex);

            break;

        case -1: // Pronto da spawnare

            // Preparare parametri
            pthread_mutex_lock(mutex);

            if (player->type == PLAYER)
                e[i] = initEntity(player->nave[0][1].x, player->nave[0][1].y, BULLET, BULLET, -1);
            else if (player->type == NEMICO)
            {
                if (player->level == 1)
                    larghezza = DIMNAVE;
                else
                    larghezza = DIMNAVES;

                e[i] = initEntity(player->nave[2][larghezza / 2 + 1].x, player->nave[2][larghezza / 2 + 1].y - 1, BULLET, BULLETENEMY, 1);
            }

            e[i].vite = 1;
            pthread_mutex_unlock(mutex);

            // Spawnare il thread
            pthread_create(&th[i], NULL, &singleBullet, (void *)(&p[i]));
            break;

        default:
            break;
        }
    }
}

/*
    handleThreads

    Procedura specifica per i thread proiettile PLAYER

    Si occupa di gestire il ciclo vitale dei proiettili del player
    ( Diagonali ).

    e:      Vettore delle entità dei proiettili
    player: Riferimento all'entità del player ( Per inizializzazione )
    th:     Vettore dei thread proiettili
    p:      Vettore parametri thread
    mutex:  Per le sezioni critiche (Accesso ad entità)
*/
void handleThreads(Entity *e, Entity *player, ThreadParam *p, pthread_t *th, pthread_mutex_t *mutex)
{
    int lives1, lives2;

    pthread_mutex_lock(mutex);
    lives1 = e[0].vite;
    lives2 = e[1].vite;
    pthread_mutex_unlock(mutex);

    int d = 0;

    if (lives1 < 1)
    {
        switch (lives1)
        {
        case 0: // Morto (da joinare)

            joinBullet(&e[0], &th[0], mutex);
            d++;
            break;

        case -1: // Pronto da spawnare

            // Preparare parametri
            if (lives2 == -1)
            {
                pthread_mutex_lock(mutex);

                e[0] = initEntity(player->nave[0][1].x, player->nave[0][1].y, BULLET, BULLET, -1);
                e[0].vite = 1;

                pthread_mutex_unlock(mutex);

                // Spawnare il thread
                pthread_create(&th[0], NULL, &doubleBullet, (void *)(&p[0]));
            }
            break;

        default:
            break;
        }
    }

    if (lives2 < 1)
    {
        switch (lives2)
        {
        case 0: // Morto (da joinare)

            joinBullet(&e[1], &th[1], mutex);
            d++;
            break;

        case -1: // Pronto da spawnare

            // Preparare parametri
            if (lives1 == -1)
            {
                pthread_mutex_lock(mutex);

                e[1] = initEntity(player->nave[0][1].x, player->nave[0][1].y, BULLET, BULLET, 1);
                e[1].vite = 1;
                pthread_mutex_unlock(mutex);

                // Spawnare il thread
                pthread_create(&th[1], NULL, &doubleBullet, (void *)(&p[1]));
            }
            break;

        default:
            break;
        }
    }

    // Eventuale ripristino del permesso per sparare nel caso entrambi
    // i thread siano morti e pronti allo spawn
    pthread_mutex_lock(mutex);
    if (e[0].vite == -1 && e[1].vite == -1)
    {
        if (player->shoot == false)
            player->shoot = true;
    }
    pthread_mutex_unlock(mutex);
}

/*
    handleEnemy

    Procedura specifica per i thread nemici

    Si occupa di gestire il ciclo vitale dei nemici

    e:      Vettore delle entità dei nemici
    i:      Indice del nemico corrente da gestire nel vettore
    th:     Vettore dei thread nemici
    p:      Vettore parametri nemici
    mutex:  Per le sezioni critiche (Accesso ad entità)
*/
void handleEnemy(Entity *e, int i, ThreadParam *p, pthread_t *th, pthread_mutex_t *mutex)
{
    int lives;
    int livello;

    pthread_mutex_lock(mutex);
    lives = e[i].vite;
    livello = e[i].level;
    pthread_mutex_unlock(mutex);

    // Il nemico è morto?
    if (lives < 1)
    {
        if (lives == 0)
        {
            waitForTerminated(&th[i], mutex, &e[i]); // Joina
            pthread_mutex_lock(mutex);
            e[i].vite = -1;
            pthread_mutex_unlock(mutex);
        }

        if (livello == 1) // Spawna il nemico di 2o livello
        {
            pthread_mutex_lock(mutex);

            modify(&e[i]);

            e[i].level = 3;
            e[i].vite = 1;

            pthread_mutex_unlock(mutex);

            pthread_create(&th[i], NULL, &enemyController, (void *)(&p[i]));
        }
    }
}

/*
    modify

    Procedura per convertire un'entità nemico
    in secondo livello

    e: Entità da convertire
*/
void modify(Entity *e)
{
    int posX;
    int posY;

    if (e->nave[0][0].x - DIMNAVES < 0)
        posX = 1;
    else
        posX = e->nave[0][0].x - DIMNAVES + 3;

    posY = e->nave[0][0].y;

    initSpriteSecond(e->nave, posX, posY);

    e->level = NEMICOSEC;
}
