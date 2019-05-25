/*******************************************************************************************
 * In this project, we are going to build “Card Matching Fight”. Card matching fight is a
 * simple card game with one dealer, 3 players, and a single deck of cards. The game is
 * composed of three rounds. At the beginning of each round, the dealer shuffles the cards,
 * deals the first three cards and waits for the round to finish, before repeating the same
 * process for the next round. A winner in a round is the first player to get a match (i.e.,
 * two cards of equal rank). In each round, a different player is given the privilege to start.
 * Initially, the dealer shuffles the deck of cards and hands each player a single card in a
 * round robin fashion (say starting from player 1 in round 1). Once the dealer is done
 * handling the cards, the dealer places the remaining deck of cards on the table and the
 * first round begins. Each player (starting from player 1), draws a card from the deck and
 * compares it to the card he/she has. If they match, the player shows the hand, declares
 * him/herself as a winner and the round completes. Otherwise, the player will discard one
 * card (at random) by placing it at the end of the deck of cards on the table and the next
 * player proceeds. Once a round ends, the dealer will shuffle the deck and hands a card to
 * each player. In the second round, the second player starts drawing a card from the deck.
 * In the third round, the third player starts drawing a card from the deck.
 *
 *
 * Copyright © 2019 Alexander Muyshondt. All rights reserved.
 *******************************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

#define NUM_PLAYERS 3
enum Role{PLAYER1, PLAYER2, PLAYER3};

/*******************************************************************************************
 * FUNCTION PROTOTYPES
 *******************************************************************************************/
void *deal(void *);
void *player(void *);
void init();
void shuffleAndDeal();
void newDeck();
void grabFromDeck(long);
bool checkForPair(long);
void discardCard(long);
void displayDeck();
void displayHand(long);

/*******************************************************************************************
 * Card struct contaiining the card value and pointer to the next card
 *******************************************************************************************/
struct card {
  int x;
  struct card * next;
};

struct card * deckHead;
struct card * deckTail;
struct card * hand[3];
long rounds;
long turn;
bool isWinner;
bool playersDone[3];
bool dealing;

/*******************************************************************************************
 * MUTEXES / CONDITION VARIABLES
 *******************************************************************************************/
pthread_mutex_t m_dealing;
pthread_cond_t c_dealing;
pthread_mutex_t m_playing;
pthread_cond_t c_playing;

/*******************************************************************************************
 * DEALER FUNCTION
 * @param seed argument for the random number generation (which will be used in shuffling and
 * in discarding cards)
 * @return
 *******************************************************************************************/
void *deal(void *seed)
{
    do
    {
        // lock mutex and initialize round
        pthread_mutex_lock(&m_dealing);
        newDeck();

        if((rounds+1) < 3)
        {
            printf("Dealer is shuffling . . .\n");
            FILE * fileIO = fopen("Log.txt", "a");
            fprintf(fileIO, "Dealer shuffling.\n");
            fclose(fileIO);
        }

        shuffleAndDeal();
        rounds++;
        turn = rounds;
        isWinner = false;
        dealing = false;

        if(rounds < 3)
        {
            printf("\nRound %ld\n", rounds+1);
            printf("---------------------------\n");

            FILE * fileIO = fopen("Log.txt", "a");
            fprintf(fileIO, "\nRound %ld\n", rounds+1);
            fprintf(fileIO, "---------------------------\n");
            fclose(fileIO);

            displayDeck();
        }

        // players may begin the game
        pthread_cond_signal(&c_playing);
        pthread_mutex_unlock(&m_dealing);

        if(rounds < 3)
        {
            pthread_mutex_lock(&m_dealing);
            pthread_cond_wait(&c_dealing, &m_dealing);
            pthread_mutex_unlock(&m_dealing);
        }
    } while(rounds < 3);
    pthread_exit(NULL);
}

/*******************************************************************************************
 * PLAYER FUNCTION
 * @param playerId value between 0-2 to differentiate players
 * @return
 *******************************************************************************************/
void *player(void *playerId)
{
    long id;
    id = (long)playerId;

    while(rounds < 3)
    {
        while(isWinner == false)
        {
            // lock mutex and wait for signal to begin play
            pthread_mutex_lock(&m_dealing);
            while(turn != id)
            {
                pthread_cond_wait(&c_playing, &m_dealing);

                if(turn != id)
                    pthread_cond_signal(&c_playing);
            }
            if(isWinner == false)
            {
                displayHand(id);
                printf("Player %ld is drawing . . .\n", id+1);
                grabFromDeck(id);

                // player checks for a match
                if(checkForPair(id) == true)
                {
                    displayHand(id);

                    FILE * fileIO = fopen("Log.txt", "a");
                    fprintf(fileIO ,"Player %ld has won.\n", id+1);
                    printf("Player %ld has won.\n", id+1);
                    fclose(fileIO);

                    playersDone[id] = true;
                    isWinner = true;
                    dealing = true;
                }
                else
                {
                    discardCard(id);
                    displayHand(id);
                }

                // next player
                long temp = turn + 1;
                if(temp == 3)
                    turn = PLAYER1;
                else
                    turn += 1;

                displayDeck();
                pthread_cond_signal(&c_playing);
                pthread_mutex_unlock(&m_dealing);
            }
        }

        if(playersDone[id] != true)
        {
            switch(id)
            {
                case PLAYER1:
                    if(playersDone[PLAYER2] == true && playersDone[PLAYER3] == true )
                    {
                        playersDone[PLAYER1] = true;
                        pthread_cond_signal(&c_dealing);
                        pthread_mutex_unlock(&m_dealing);
                    }
                    else
                    {
                        playersDone[PLAYER1] = true;
                        turn = PLAYER2;
                        pthread_cond_broadcast(&c_playing);
                        pthread_mutex_unlock(&m_dealing);
                    }
                    break;

                case PLAYER2:
                    if(playersDone[PLAYER1] == true && playersDone[PLAYER3] == true )
                    {
                        playersDone[PLAYER2] = true;
                        pthread_cond_signal(&c_dealing);
                        pthread_mutex_unlock(&m_dealing);
                    }
                    else
                    {
                        playersDone[PLAYER2] = true;
                        turn = PLAYER3;
                        pthread_cond_broadcast(&c_playing);
                        pthread_mutex_unlock(&m_dealing);
                    }
                    break;
                    
                    case PLAYER3:
                        if(playersDone[PLAYER2] == true && playersDone[PLAYER1] == true )
                        {
                            playersDone[PLAYER3] = true;
                            pthread_cond_signal(&c_dealing);
                            pthread_mutex_unlock(&m_dealing);
                        }
                        else
                        {
                            playersDone[PLAYER3] = true;
                            turn = PLAYER1;
                            pthread_cond_broadcast(&c_playing);
                            pthread_mutex_unlock(&m_dealing);
                        }
                        break;
                    default:
                        printf("Error.\n");
                        break;
                }
        }
        // loop until the dealer completes
        while(dealing == true)
        {
        }
    }
    pthread_exit(NULL);
}

/*******************************************************************************************
 * MAIN FUNCTION
 * @param argc
 * @param argv
 * @return
 *******************************************************************************************/
int main(int argc, char *argv[])
{
    // create thread variables and seed random values
    pthread_t players[NUM_PLAYERS];
    pthread_t dealer;
    srand(atoi(argv[1]));
    init();

    FILE * fileIO = fopen("Log.txt", "w");
    fprintf(fileIO, "Game is starting.\n");
    fprintf(fileIO, "Seed is: %s\n", argv[1]);
    fclose(fileIO);

    int ret;
    ret = pthread_create(&dealer, NULL, deal, NULL);
    if (ret) {
        printf("ERROR: return code from pthread_create() is %d\n", ret);
        exit(-1);
    }

    long t;
    for(t=0; t<NUM_PLAYERS; t++)
    {
        printf("In main: creating thread %ld\n", (t+1));
        ret = pthread_create(&players[t], NULL, player, (void *)t);
        if (ret) {
            printf("ERROR: return code from pthread_create() is %d\n", ret);
            exit(-1);
        }
    }

    ret = pthread_join(dealer, NULL);
    if (ret) {
         printf("ERROR: return code from pthread_join() is %d\n", ret);
         exit(-1);
    }
    printf("\nDealer Thread Joined.\n");
    for(t=0; t<NUM_PLAYERS; t++) {
        ret = pthread_join(players[t], NULL);
        if (ret) {
            printf("ERROR: return code from pthread_join() is %d\n", ret);
            exit(-1);
        }
        printf("Player %ld thread joined.\n", t+1);
    }
    fileIO = fopen("Log.txt", "a");
    fprintf(fileIO, "\nProgram complete.\n");
    fclose(fileIO);

    return 0;
}

/*******************************************************************************************
 * INIT FUNCTION
 * Initialize environment for game to begin
 *******************************************************************************************/
void init()
{
    pthread_mutex_init(&m_playing, NULL);
	pthread_cond_init(&c_playing, NULL);
    pthread_mutex_init (&m_dealing, NULL);
    pthread_cond_init (&c_dealing, NULL);
    deckHead = NULL;
    deckTail = NULL;
    hand[PLAYER1] = NULL;
    hand[PLAYER2] = NULL;
    hand[PLAYER3] = NULL;
    dealing = true;
    isWinner = false;
    playersDone[PLAYER1] = false;
    playersDone[PLAYER2] = false;
    playersDone[PLAYER3] = false;
    rounds = -1;
    turn = -1;
}

/*******************************************************************************************
 * SHUFFLE AND DEAL FUNCTION
 * Shuffles deck and then deals to players
 *******************************************************************************************/
void shuffleAndDeal()
{
    // unshuffled deck of cards
    // 11 - Jack,
    // 12 - Queen,
    // 13 - King,
    // 14 - Ace
    int deck[52] = {2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,8,
                    9,9,9,9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,
                    14,14,14,14};

    int cardsLeft = 52;

    while(cardsLeft != 0)
    {
        int randPos = rand() % 52;

        if(deck[randPos] != -1)
        {
            struct card * temp;
            temp = (struct card *)malloc(sizeof(struct card));
            temp->x = deck[randPos];
            deck[randPos] = -1;

            if(deckHead == NULL)
            {
                deckHead = temp;
                deckTail = deckHead;
                deckHead->next = NULL;
            }
            else
            {
                deckTail->next = temp;
                deckTail = temp;
                deckTail->next = NULL;
            }
            cardsLeft--;
        }
    }

    hand[PLAYER1] = deckHead;
    deckHead = deckHead->next;
    hand[PLAYER1]->next = NULL;

    hand[PLAYER2] = deckHead;
    deckHead = deckHead->next;
    hand[PLAYER2]->next = NULL;

    hand[PLAYER3] = deckHead;
    deckHead = deckHead->next;
    hand[PLAYER3]->next = NULL;
}

/*******************************************************************************************
 * GRAB FROM DECK FUNCTION
 * @param playerid value between 0-2 to differentiate players
 *******************************************************************************************/
void grabFromDeck(long playerid)
{
    struct card * temp = deckHead;
    deckHead = deckHead->next;
    hand[playerid]->next = temp;
    temp->next = NULL;

    FILE * fileIO = fopen("Log.txt", "a");
    fprintf(fileIO, "Player %ld draws: ", playerid+1);
    fprintf(fileIO, "%d\n", temp->x);
    fclose(fileIO);
    temp = NULL;
}

/*******************************************************************************************
 * CHECK FOR PAIR FUNCTION
 * Player checks to see if they have a pair in their hand
 * @param playerid value between 0-2 to differentiate players
 * @return
 *******************************************************************************************/
bool checkForPair(long playerid)
{
    if(hand[playerid]->x == hand[playerid]->next->x)
        return true;
    else
        return false;
}

/*******************************************************************************************
 * DISCARD CARD FUNCTION
 * Discards a card from player's hand
 *******************************************************************************************/
void discardCard(long playerid)
{
    int disCard = rand() % 2 + 1;
    if(disCard == 1)
    {
        deckTail->next = hand[playerid];
        hand[playerid] = hand[playerid]->next;
        deckTail = deckTail->next;
        deckTail->next = NULL;
    }
    else
    {
        struct card * temp = hand[playerid]->next;
        hand[playerid]->next = NULL;
        deckTail->next = temp;
        deckTail = temp;
        deckTail->next = NULL;
        temp = NULL;
    }
    FILE * fileIO = fopen("Log.txt", "a");
    printf("Player %ld discards: ", playerid+1);
    printf("%d\n", deckTail->x);
    fprintf(fileIO, "Player %ld's discards: ", playerid+1);
    fprintf(fileIO, "%d\n", deckTail->x);
    fclose(fileIO);
}

/*******************************************************************************************
 * NEW DECK FUNCTION
 * Frees up memory from past decks
 *******************************************************************************************/
void newDeck()
{
    struct card * temp = deckHead;
    int x;
    
    for(x = 0; x < NUM_PLAYERS; x++)
    {
        if(hand[x] != NULL)
        {
            if(hand[x]->next != NULL)
            {
                deckTail->next = hand[x];
                deckTail = hand[x]->next;
            }
            else
            {
                deckTail->next = hand[x];
                deckTail = deckTail->next;
            }
            hand[x] = NULL;
        }
    }
    while(deckHead != deckTail)
    {
      deckHead = deckHead->next;
      free(temp);
      temp = deckHead;
    }
    free(deckHead);
    deckHead = NULL;
    deckTail = NULL;
    temp = NULL;
    playersDone[PLAYER1] = false;
    playersDone[PLAYER2] = false;
    playersDone[PLAYER3] = false;
}

/*******************************************************************************************
 * DISPLAY DECK FUNCTION
 * Outputs content of deck to console and writes contents to file
 *******************************************************************************************/
void displayDeck()
{
    struct card * temp = deckHead;
    FILE * fileIO = fopen("Log.txt", "a");
    fprintf(fileIO, "\nDeck: ");
    printf("\nDeck: ");
    while(temp != deckTail)
    {
        fprintf(fileIO, " %d,", temp->x);
        printf(" %d,", temp->x);
        temp = temp->next;
    }
    fprintf(fileIO, " %d\n\n", temp->x);
    printf(" %d\n\n", temp->x);
    temp = NULL;
    fclose(fileIO);
}

/*******************************************************************************************
 * DISPLAY HAND FUNCTION
 * Outputs content of player's hand to console and writes contents to file
 * @param playerId value between 0-2 to differentiate players
 *******************************************************************************************/
void displayHand(long playerId)
{
    FILE * fileIO = fopen("Log.txt", "a");
    printf("Player %ld's Hand: ", playerId+1);
    fprintf(fileIO, "Player %ld's Hand: ", playerId+1);

    if(hand[playerId]->next == NULL)
    {
        printf("%d\n", hand[playerId]->x);
        fprintf(fileIO, "%d\n", hand[playerId]->x);
    }
    else
    {
        printf("%d", hand[playerId]->x);
        fprintf(fileIO, "%d", hand[playerId]->x);
        printf(", %d\n", hand[playerId]->next->x);
        fprintf(fileIO, ", %d\n", hand[playerId]->next->x);
    }
    fclose(fileIO);
}
