/**
 * @file main.c
 * @author Alessio Usai - Riccardo Casula
 * 
 * Progetto Sistemi Operativi
 * 
 * SpaceInvaders versione PIPES
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

int main()
{
    //SEZIONE DICHIARATIVA
    //dichiarazione id per i processi
    pid_t nemici[ENEMIES];
    pid_t playerID;

    Entity nemico;

    //dichiarazione delle pipes
    int pipeNemici[ENEMIES][2];
    int pipePos[2];
    int pipePlayer[2];

    int sprite; // sprite da assegnare al nemico
    int i;

    int posInizialeInizio = 1;
    int posInizialeFine = MAXC - 4;

    int posinitX = 1;
    int dir = 1;
    int rigaIniziale = 1;

    int vitePlayer;
    //SEZIONE ESECUTIVA

    //creazione pipe coordinate per il render
    if (pipe(pipePos) == -1)
    {
        perror("Errore creazione pipePos");
        _exit(1);
    }

    //creo la pipe per dare il permesso di sparare al player
    if (pipe(pipePlayer) == -1)
    {
        perror("Errore nella creazione della pipePlayer");
        _exit(1);
    }

    //rendo la pipe del player non bloccante
    if (fcntl(pipePlayer[LETTURA], F_SETFL, O_NONBLOCK) < 0)
    {
        perror("Errore creazione della pipe dei nemici");
        _exit(1);
    }

    //creazione pipe nemici
    for (i = 0; i < ENEMIES; i++)
    {
        //creo la pipe
        if (pipe(pipeNemici[i]) == -1)
        {
            perror("Errore creazione pipe nemici");
            _exit(1);
        }

        //rendo la pipe non bloccante in lettura
        if (fcntl(pipeNemici[i][LETTURA], F_SETFL, O_NONBLOCK) < 0)
        {
            perror("Errore creazione della pipe dei nemici");
            _exit(1);
        }
    }

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
        playerID = fork();
        switch (playerID)
        {
        case -1:
            perror("Errore creazione giocatore");
            _exit(1);
            break;
        case 0:
            close(pipePos[LETTURA]);
            close(pipePlayer[SCRITTURA]);
            playerController(pipePos[SCRITTURA], pipePlayer[LETTURA]);

        default:
            //creazione dei processi nemici

            for (i = 0; i < ENEMIES; i++)
            {
                //genero la sprite da assegnare al nemico
                sprite = 2; //rand() % (2 - 0 + 1) + 0;

                nemici[i] = fork();

                switch (nemici[i])
                {
                case -1:
                    perror("Errore nella creazione del nemico");
                    _exit(1);
                    break;

                case 0:
                    close(pipePos[LETTURA]);
                    close(pipeNemici[i][SCRITTURA]);

                    /*
                        Questo if permette di posizionare i nemici due per riga, uno all'inizio e uno alla fine
                    */
                    /* if (i % 2 == 1)
                    {
                        nemico = initEntity(posInizialeFine, rigaIniziale, sprite, NEMICO, -1);
                    }
                    else
                    {
                        nemico = initEntity(posInizialeInizio, rigaIniziale, sprite, NEMICO, 1);
                    }
                    nemico.id = i;*/

                    nemico = initEntity(posinitX, rigaIniziale, sprite, NEMICO, dir);
                    nemico.id = i;

                    enemyController(pipePos[SCRITTURA], pipeNemici[i][LETTURA], nemico);
                default:
                    break;
                } //fine switch nemici
                if (posinitX > 20)
                {
                    posinitX = 1;
                    rigaIniziale += 6;
                }
                else
                {
                    posinitX += 18;
                }
                /* if (i % 2 == 1)
                    rigaIniziale += 6;*/

            } //fine for nemici

            close(pipePos[SCRITTURA]);
            close(pipePlayer[LETTURA]);
            for (i = 0; i < ENEMIES; i++)
                close(pipeNemici[i][LETTURA]);

            collisionManager(pipePos[LETTURA], pipeNemici, pipePlayer[SCRITTURA]);
        } //fine switch player
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

    mvwprintw(menu, 1, MAXC / 4, "SPACE INVADERS [VERSIONE PIPES]");

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