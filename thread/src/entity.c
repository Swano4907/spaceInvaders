#include "entity.h"

/**
 * @brief 
 *  Procedura che permette all'utente di controllare la navicella.
 *  Questa procedura permette di muovere la navicella a sinistra e destra, 
 *  rispettivamente con i tast A e D. Inoltre permette di sparare con la barra spaziatrice
 *  e cambiare la modalita' di sparo con il tasto L.
 * 
 * @param pipeWrite 
 *  Pipe su cui scrive le nuove posizioni. Questa pipe verra' passata anche al processo dello sparo.
 * @param pipeRead 
 */
void *playerController(void *parms)
{
    //SEZIONE DICHIARATIVA
    ThreadParam *par = (ThreadParam*)parms;
    FILE *f;
    bool type;
    bool flag; //vero se esiste un proiettile in campo, falso altrimenti

    //SEZIONE ESECUTIVA
    //inizializzo il player
    pthread_mutex_lock(par->mutex);
    type = par->e->shoot = true;
    type = par->e->shootT = true; // false: sparo normale; true: sparo doppio
    pthread_mutex_unlock(par->mutex);

    int run;

    pthread_mutex_lock(par->mutex);
    run = par->e->vite;
    pthread_mutex_unlock(par->mutex);

    while (run > 0 && run < 4)
    {
        sem_wait(par->disponibili);
        pthread_mutex_lock(par->mutex);

        par->e->dir = getch();

        /*
            Gestione dei controlli del player
            Tasto D per il movimento a destra
            Tasto A per il movimento a sinistra
            Stasto spazio per lo sparo 
            Tasto L per cambiare lo sparo
        */
        switch (par->e->dir)
        {
        case KEY_D:
            if (par->e->nave[0][2].x + 2 < MAXC)
                moveSprite(par->e->nave, 2, DIMNAVE, 1);
            break;
        case KEY_A:
            if (par->e->nave[0][0].x - 2 > 0)
                moveSprite(par->e->nave, 2, DIMNAVE, -1);
            break;
        case ' ':
            if (par->e->shoot == true)
            {
                par->e->shoot = false; // Richiesta di sparo inoltrata all'arbitro: rimarrà falsa fino alla morte dei thread proiettili
            }
            break;
        case 'l': //cambio il tipo di sparo
            if(par->e->shoot == true) // Per facilitare l'arbitro si può cambiare tipo di sparo solo se non ci sono proiettili in vita
            {
                par->e->shootT = !par->e->shootT;
            }
        }

        par->buffer[*(par->bIN)] = par->e;             // Scrittura elemento nel buffer
        *(par->bIN) = (*(par->bIN) + 1) % BUFFER_SIZE; // Aggiornamento indice

        run = par->e->vite;

        sem_post(par->presenti); // Nuovo elemento inserito
        pthread_mutex_unlock(par->mutex);
    }
}

/**
 * @brief 
 *  Procedura che gestisce il processo nemico.
 *  Si occupa di farmi muovere la sprite del nemico andando a modificare
 *  le coordinate della sprite.
 *  Inoltre, quando il nemico di primo livello muore fa comparire una navicella
 *  di secondo livello dove questo e' morto.
 * 
 * @param pipeWrite 
 *  Pipe usata per scrivere le coordinate del nemico. Questa pipe viene passata anche al processo dello sparo
 * @param pipeRead 
 *  Pipe usata per aggiornare la direzione e le vite di un nemico.
 * @param e 
 *  Entity nemico pre inizializzata nel main oppure alla morte della navicella
 *  di primo livello.
 */
void *enemyController(void *parms)
{
    //SEZIONE DICHIARATIVA
    ThreadParam *par = (ThreadParam*)parms;

    bool shoot;
    
    int posX;
    int larghezza;
    int supdir = par->e->dir; //mi salvo la direzione originaria della navicella
    int velocitaSpostamento = 1;
    int timer, timerSparo; //timer di sparo

    int vite;
    //SEZIONE ESECUTIVA

    pthread_mutex_lock(par->mutex);

    //Setto altri dati per la navicella
    if (par->e->level == 1) //navicella di livello 1
    {
        larghezza = DIMNAVE; //larghezza della sprite
        par->e->vite = 1;
        timer = rand() % (25 - 10 + 1) + 10; //timer per lo sparo
    }
    else //navicella di secondo livello
    {
        larghezza = DIMNAVES; //La navicella di secondo livello e' piu' larga
        par->e->vite = 2;
        timer = 35;
    }

    vite = par->e->vite;

    pthread_mutex_unlock(par->mutex);

    //variabile di supporto dello sparo
    timerSparo = timer;
    par->e->shoot = true;

    //ciclo principale
    while (vite > 0)
    {    
        //MOVIMENTO

        sem_wait(par->disponibili);
        pthread_mutex_lock(par->mutex);

        //controllo collisioni con perimentro finestra
        if (par->e->dir < 0) //nel caso in cui la navicella stia andando verso destra
        {
            if (par->e->nave[0][0].x - 2 < 1) //collisione con il muro destro
            {
                //if (par->e->dir != supdir)
                    moveSpriteDown(par->e->nave, larghezza);
                par->e->dir *= -1; //inverto la direzione della navicella
            }
            else //la navicella non ha incontrato il muro
            {
                moveSprite(par->e->nave, velocitaSpostamento, larghezza, par->e->dir);
            }
        }
        else //la navicella sta andando a sinistra
        {
            if (par->e->nave[0][larghezza - 1].x + 2 >= MAXC - 1) //collisione muro sinistro
            {
                //if (par->e->dir != supdir)
                    moveSpriteDown(par->e->nave, larghezza);
                par->e->dir *= -1;
            }
            else
            {
                moveSprite(par->e->nave, velocitaSpostamento, larghezza, par->e->dir);
            }
        }

        //SPARO NAVICELLA

        if(par->e->shoot == true)
        {
            if (timerSparo <= 0)
            {
                par->e->shoot = false;
                timerSparo = timer;
            }
            else
                timerSparo --;
        }

        par->buffer[*(par->bIN)] = par->e;            // Scrittura elemento nel buffer
        *(par->bIN) = (*(par->bIN) + 1) % BUFFER_SIZE; // Aggiornamento indice

        sem_post(par->presenti); // Nuovo elemento inserito
        pthread_mutex_unlock(par->mutex);

        vite = par->e->vite;

        usleep(300000);
    } //fine while
}

/**
 * @brief 
 *  Procedura che si occupa di muovere una sprite passata come parametro 
 *  di un determinato offset in una direzione passata
 * 
 * @param n 
 *  Sprite da muovere
 * @param velocita 
 *  Offset di cui spostare le coordinate della sprite
 * @param larghezza 
 *  Larghezza della sprite
 * @param dir 
 *  Direzione in cui spostare la sprite
 */
void moveSprite(Sprite n[DIMNAVE][DIMNAVES], int velocita, int larghezza, int dir)
{
    //SEZIONE DICHIARATIVA
    int i, k;
    //SEZIONE ESECUTIVA
    for (i = 0; i < DIMNAVE; i++)
    {
        for (k = 0; k < larghezza; k++)
        {
            n[i][k].oldx = n[i][k].x;
            n[i][k].oldy = n[i][k].y;
            n[i][k].x += (dir * velocita);
        }
    }
}

/**
 * @brief 
 *  Una volta raggiunto uno dei muri la sprite va spostata nella riga sottostante.
 * 
 * @param n 
 *  Sprite da spostare in basso 
 * @param larghezza 
 *  Larghezza della sprite
 */
void moveSpriteDown(Sprite n[DIMNAVE][DIMNAVES], int larghezza)
{
    //SEZIONE DICHIARATIVA
    int i, k;
    //SEZIONE ESECUTIVA
    for (i = 0; i < DIMNAVE; i++)
    {
        for (k = 0; k < larghezza; k++)
        {
            n[i][k].oldy = n[i][k].y;
            n[i][k].oldx = n[i][k].x;
            n[i][k].y += 3;
        }
    }
}

/**
 * @brief 
 *  Procedura che comanda il proiettile che si muove in linea retta. Si occupa di aggiornare le 
 *  coordinate del proiettile. Le collisioni di questo vengono controllate nella
 *  funzione collisionManager.
 * 
 * @param pipeWrite 
 *  Pipe dove scrivere le coordinate
 * @param type 
 *  Tipo dell'Entity puo' essere BULLETENEMY o BULLET
 * @param posX 
 *  Posizione inizale in X
 * @param posY 
 *  Posizione iniziale in Y
 * @param dir 
 *  La direzione del proiettile
 */
void *singleBullet(void *parms)
{
    //SEZIONE DICHIARATIVA
    ThreadParam *par = (ThreadParam*)parms;

    pthread_mutex_lock(par->mutex);
    par->e->id = 255;
    par->e->vite = 1;

    par->e->pid = (intptr_t)(par->e) + rand();
    pthread_mutex_unlock(par->mutex);

    //SEZIONE ESECUTIVA
    while (par->e->vite != 0)
    {
        sem_wait(par->disponibili);
        pthread_mutex_lock(par->mutex);

        //salvo la vecchia posizione del proiettile
        par->e->nave[0][0].oldy = par->e->nave[0][0].y;

        //sposto il proiettile
        par->e->nave[0][0].y = par->e->nave[0][0].y + 1 * par->e->dir;

        //mando le informazioni al processo di render
        par->buffer[*(par->bIN)] = par->e;            // Scrittura elemento nel buffer
        *(par->bIN) = (*(par->bIN) + 1) % BUFFER_SIZE; // Aggiornamento indice

        sem_post(par->presenti); // Nuovo elemento inserito
        pthread_mutex_unlock(par->mutex);

        usleep(150000);
    } //fine ciclo principale

} //fine procedura singleBullet

/**
 * @brief 
 *  Procedura che comanda il proiettile che si muove in obliquo. Si occupa di aggiornare le 
 *  coordinate del proiettile. Le collisioni di questo vengono controllate nella
 *  funzione collisionManager.
 * 
 * @param pipeWrite 
 *  Pipe dove scrivere le coordinate
 * @param type 
 *  Tipo dell'Entity puo' essere BULLETENEMY o BULLET
 * @param posX 
 *  Posizione inizale in X
 * @param posY 
 *  Posizione iniziale in Y
 * @param dir 
 *  La direzione del proiettile
 */
void *doubleBullet(void *parms)
{
    //SEZIONE DICHIARATIVA
    ThreadParam *par = (ThreadParam*)parms;

    par->e->id = 255;

    while (par->e->vite != 0)
    {
        sem_wait(par->disponibili);
        pthread_mutex_lock(par->mutex);

        //salvo la vecchia posizione
        //salvo la vecchia posizione
        par->e->nave[0][0].oldy = par->e->nave[0][0].y;
        par->e->nave[0][0].oldx = par->e->nave[0][0].x;

        //sposto in alto il proiettile
        par->e->nave[0][0].x += 1 * par->e->dir;
        par->e->nave[0][0].y--;

        //mando le informazioni al processo che renderizza
        par->buffer[*(par->bIN)] = par->e;            // Scrittura elemento nel buffer
        *(par->bIN) = (*(par->bIN) + 1) % BUFFER_SIZE; // Aggiornamento indice

        sem_post(par->presenti); // Nuovo elemento inserito
        pthread_mutex_unlock(par->mutex);
        usleep(150000);
    }
}
