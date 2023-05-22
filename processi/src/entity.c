#include "entity.h"

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
void enemyController(int pipeWrite, int pipeRead, Entity e)
{
    //SEZIONE DICHIARATIVA
    Entity readEntity, s;
    pid_t bullet;
    pid_t second;
    int posX;
    int larghezza;
    int supdir = e.dir; //mi salvo la direzione originaria della navicella
    int velocitaSpostamento = 1;
    int timer, timerSparo; //timer di sparo
    //SEZIONE ESECUTIVA

    e.type = NEMICO;

    //scrivo nella pipe la prima volta
    write(pipeWrite, &e, sizeof(Entity));

    //Setto altri dati per la navicella
    if (e.level == 1) //navicella di livello 1
    {
        larghezza = DIMNAVE; //larghezza della sprite
        e.vite = 1;
        timer = rand() % (25 - 10 + 1) + 10 + getpid() % 5; //timer per lo sparo
    }
    else //navicella di secondo livello
    {
        larghezza = DIMNAVES; //La navicella di secondo livello e' piu' larga
        e.vite = 2;
        timer = 35;
    }

    //variabile di supporto dello sparo
    timerSparo = timer;

    //ciclo principale
    while (e.vite > 0)
    {
        //MOVIMENTO
        //leggo dalla pipe per eventuali collisioni
        if (read(pipeRead, &readEntity, sizeof(Entity)) != -1)
        {
            e.vite = readEntity.vite;
            e.dir = readEntity.dir;
        }

        //controllo collisioni con perimentro finestra
        if (e.dir < 0) //nel caso in cui la navicella stia andando verso destra
        {
            if (e.nave[0][0].x - 2 < 1) //collisione con il muro destro
            {
                //if (e.dir != supdir)
                moveSpriteDown(e.nave, larghezza);
                e.dir *= -1; //inverto la direzione della navicella
            }
            else //la navicella non ha incotrato il muro
            {
                moveSprite(e.nave, velocitaSpostamento, larghezza, e.dir);
            }
        }
        else //la navicella sta andando a sinistra
        {
            if (e.nave[0][larghezza - 1].x + 2 >= MAXC - 1) //collisione muro sinistro
            {
                //if (e.dir != supdir)
                moveSpriteDown(e.nave, larghezza);
                e.dir *= -1;
            }
            else
            {
                moveSprite(e.nave, velocitaSpostamento, larghezza, e.dir);
            }
        }

        //SPARO NAVICELLA
        if (timerSparo == 0)
        {
            timerSparo = timer;
            //creo il nuovo processo dello sparo
            bullet = fork();

            switch (bullet)
            {
            case -1:
                perror("Errore creazione del proiettile nemico");
                _exit(1);
                break;
            case 0:
                singleBullet(pipeWrite, BULLETENEMY,
                             e.nave[2][larghezza / 2 + 1].x, e.nave[2][larghezza / 2 + 1].y + 1, 1);
                break;

            default:
                break;
            }
        }

        timerSparo--;
        write(pipeWrite, &e, sizeof(Entity));

        usleep(300000);
    } //fine while

    //CREAZIONE NAVICELLA DI SECONDO LIVELLO
    if (e.level == 1)
    {
        //controllo per impedire che la navicella venga disegnata fuori dallo schermo
        if (e.nave[0][0].x - DIMNAVES < 0)
            posX = 1;
        else
            posX = e.nave[0][0].x - DIMNAVES + 3;
        s = initEntity(posX, e.nave[0][0].y, NEMICOSEC, NEMICO, e.dir);
        /*
            assegno l'id del nemico morto a quello nuovo
            L'id serve alla funzione che controlla le collisioni per 
            capire in quale pipe deve scrivere l'avventa collisione
        */
        s.id = e.id;

        //creazione processo della navicella di secondo livello
        second = fork();

        switch (second)
        {
        case -1:
            perror("Errore creazioe nemico di secondo livello");
            _exit(1);
            break;
        case 0:
            enemyController(pipeWrite, pipeRead, s);
            break;
        default:
            break;
        }
    }

    _exit(0);
}

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
void playerController(int pipeWrite, int pipeRead)
{
    //SEZIONE DICHIARATIVA
    Entity p;
    pid_t b1, b2;
    FILE *f;
    bool type;
    bool flag; //vero se esiste un proiettile in campo, falso altrimenti

    //SEZIONE ESECUTIVA
    //inizializzo il player
    p = initEntity(MAXC / 2 - 1, MAXL - 5, 4, PLAYER, 0);
    p.pid = getpid();
    type = p.shoot = true;

    write(pipeWrite, &p, sizeof(Entity));

    while (1)
    {
        p.dir = getch();

        /*
            Gestione dei controlli del player
            Tasto D per il movimento a destra
            Tasto A per il movimento a sinistra
            Stasto spazio per lo sparo 
            Tasto L per cambiare lo sparo
        */
        switch (p.dir)
        {
        case KEY_D:
            if (p.nave[0][2].x + 2 < MAXC)
                moveSprite(p.nave, 2, DIMNAVE, 1);
            break;
        case KEY_A:
            if (p.nave[0][0].x - 2 > 0)
                moveSprite(p.nave, 2, DIMNAVE, -1);
            break;
        case ' ':
            if (p.shoot == true)
            {
                if (type != false) //sparo doppio
                {
                    b1 = fork();
                    switch (b1)
                    {
                    case -1:
                        perror("Errore creazione del processo proiettile");
                        _exit(1);
                        break;
                    case 0:
                        doubleBullet(pipeWrite, p.nave[0][1].x, p.nave[0][1].y - 1, -1, BULLET);
                        break;

                    default:
                        break;
                    }
                    b2 = fork();
                    switch (b2)
                    {
                    case -1:
                        perror("Errore creazione del processo proiettile");
                        _exit(1);
                    case 0:
                        doubleBullet(pipeWrite, p.nave[0][1].x, p.nave[0][1].y - 1, 1, BULLET);
                        break;
                    default:
                        break;
                    }
                }
                else //sparo singolo
                {
                    b1 = fork();
                    switch (b1)
                    {
                    case -1:
                        perror("Errore creazione processo proiettile");
                        _exit(1);
                        break;
                    case 0:
                        singleBullet(pipeWrite, BULLET, p.nave[0][1].x, p.nave[0][1].y - 1, -1);
                        break;
                    default:
                        break;
                    }
                }
            }
            break;
        case 'l': //cambio il tipo di sparo
            if (type == true)
                type = false;
            else
                type = true;
            break;
        }

        write(pipeWrite, &p, sizeof(Entity));
        //recupero il flag del player per capire se posso sparare
        if (read(pipeRead, &flag, sizeof(bool)) != -1)
            p.shoot = flag;
    }
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
void singleBullet(int pipeWrite, int type, int posX, int posY, int dir)
{
    //SEZIONE DICHIARATIVA
    Entity b;

    //SEZIONE ESECUTIVA
    b = initEntity(posX, posY, type, type, dir);
    b.pid = getpid();
    while (1)
    {
        //salvo la vecchia posizione del proiettile
        b.nave[0][0].oldy = b.nave[0][0].y;

        //sposto il proiettile
        b.nave[0][0].y = b.nave[0][0].y + 1 * b.dir;

        //mando le informazioni al processo di render
        write(pipeWrite, &b, sizeof(Entity));

        usleep(100000);
    } //finew ciclo principale
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
void doubleBullet(int pipeWrite, int posX, int posY, int dir, int type)
{
    //SEZIONE DICHIARATIVA
    Entity b;
    //SEZIONE ESECUTIVA
    b = initEntity(posX, posY, BULLET, BULLET, dir);
    b.pid = getpid();

    while (1)
    {
        //salvo la vecchia posizione
        //salvo la vecchia posizione
        b.nave[0][0].oldy = b.nave[0][0].y;
        b.nave[0][0].oldx = b.nave[0][0].x;

        //sposto in alto il proiettile
        b.nave[0][0].x += 1 * dir;
        b.nave[0][0].y--;

        //mando le informazioni al processo che renderizza
        write(pipeWrite, &b, sizeof(Entity));
        usleep(150000);
    }
}
