#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<iostream>

#define PORT 1101

using namespace std;

sem_t sem1, sem2;
char client_message[2000];
char buffer[1024];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

class Board
{
public:
    char board[6][7];

    Board()
    {
        for (int i = 0; i < 6; i++)
            for (int j = 0; j < 7; j++)
                board[i][j] = '-';
    }
    
    int insert(int posY, int player)
    {
        char playerSign = 'o';
        if (player == 1) playerSign = 'x';

        if (board[0][posY] != '-') return -1;

        for (int i = 5; i >= 0; i--)
        {
            if (board[i][posY] == '-')
            {
                board[i][posY] = playerSign;
                if (checkWins(i, posY, player))
                {
                    return 1;
                }
                return 0;
            }
        }

        return -1;
    }

    void displayBoard()
    {
        std::cout << "1 2 3 4 5 6 7" << std::endl;
        
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 7; j++)
            {
                std::cout << board[i][j] << " ";
            }

            std::cout << std::endl;
        }
    }

private:

    char checkIdx(int posX, int posY)
    {
        if (posX < 6 && posX >= 0 && posY < 7 && posY >= 0)
        {
            return board[posX][posY];
        }
        else return '!';
    }

    bool checkWins(int posX, int posY, int player)
    {
        char playerSign = 'o';
        if (player == 1) playerSign = 'x';
        int row = 0;
        
        int i = 0;
        if (posX > 2) i = posX - 3;
        int rb = 5;
        if (posX < 3) rb = posX + 3;

        for (; i <= rb; i++)
        {
            if (checkIdx(i, posY) == playerSign) row++;
            else row = 0;

            if (row == 4) return true;
        }

        row = 0;
        i = 0;
        if (posY > 2) i = posY - 3;
        rb = 6;
        if (posY < 4) rb = posY + 3;

        for (; i <= rb; i++)
        {
            if (checkIdx(posX, i) == playerSign) row++;
            else row = 0;

            if (row == 4) return true;
        }

        row = 0;
        i = -3;
        for (; i < 4; i++)
        {
            if (checkIdx(posX + i, posY + i) == playerSign) row++; 
            else row = 0;

            if (row == 4) return true;
        }

        row = 0;
        i = -3;
        for (; i < 4; i++)
        {
            if (checkIdx(posX + i, posY - i) == playerSign) row++; 
            else row = 0;

            if (row == 4) return true;
        }

        return false;
    }
};

class playerPair
{
public:
    pthread_t player1;
    pthread_t player2;
    Board board;
    int sockets[2];
    int gameNumber = 0;
    int playerTurn = 0;
    int pcount = 0;
};

int currPair = 0;
playerPair pairs[1000];

void *player1(void* arg)
{
    int gameId = currPair;
    int n;
    char* message;

    cout << "Game " << gameId << ": Player 1 joins." << endl;

    message = (char*)malloc(sizeof("player o"));
    strcpy(message, "player o");
    send(pairs[gameId].sockets[0], message, sizeof(message), 0);

    for(;;)
    {
        if(pairs[gameId].playerTurn == 1)
        {
            n = recv(pairs[gameId].sockets[0], client_message, 2000, 0);
            if(strstr(client_message, "leave") != NULL || n < 1) 
            {
                break;
            }
            std::cout << "G" << std::to_string(gameId) << "P1 message: " << client_message << std::endl;

            char* message = (char*)malloc(sizeof("Player 1 wins the game!"));
            
            usleep(100);

            string cMsgStr(client_message);
            if(cMsgStr.size() > 1)
            {
                usleep(100);
                message = (char*)malloc(sizeof("wmove"));
                strcpy(message, "wmove");
                send(pairs[gameId].sockets[0], message, sizeof(message), 0);
            }
            else if(stoi(cMsgStr) > 7 || stoi(cMsgStr) < 1)
            {
                usleep(100);
                message = (char*)malloc(sizeof("wmove"));
                strcpy(message, "wmove");
                send(pairs[gameId].sockets[0], message, sizeof(message), 0);
            }
            else 
            {
                int boardResponse = pairs[gameId].board.insert(stoi(cMsgStr) - 1, 1);

                if(boardResponse == -1)
                {
                    usleep(100);
                    message = (char*)malloc(sizeof("wmove"));
                    strcpy(message, "wmove");
                    send(pairs[gameId].sockets[0], message, sizeof(message), 0);
                }
                else if(boardResponse == 0)
                {
                    usleep(100);
                    string sMsg = "board " + cMsgStr;
                    message = (char*)malloc(sizeof(sMsg));
                    strcpy(message, sMsg.c_str());
                    send(pairs[gameId].sockets[0], message, sizeof(message), 0);
                    pairs[gameId].playerTurn = 2;
                    usleep(100);
                    send(pairs[gameId].sockets[1], message, sizeof(message), 0);
                    usleep(100);
                    message = (char*)malloc(sizeof("proceed"));
                    strcpy(message, "proceed");
                    send(pairs[gameId].sockets[1], message, sizeof(message), 0);
                    memset(&client_message, 0, sizeof (client_message));
                }
                else if(boardResponse == 1)
                {
                    usleep(100);
                    string sMsg = "board " + cMsgStr;
                    message = (char*)malloc(sizeof(sMsg));
                    strcpy(message, sMsg.c_str());
                    send(pairs[gameId].sockets[0], message, sizeof(message), 0);
                    send(pairs[gameId].sockets[1], message, sizeof(message), 0);
                    usleep(100);
                    string wMsg = "P1 wins!";
                    message = (char*)malloc(sizeof(wMsg));
                    strcpy(message, wMsg.c_str());
                    send(pairs[gameId].sockets[0], message, sizeof(message), 0);
                    send(pairs[gameId].sockets[1], message, sizeof(message), 0);
                    usleep(100);
                    message = (char*)malloc(sizeof("leave"));
                    strcpy(message, "leave");
                    send(pairs[gameId].sockets[0], message, sizeof(message), 0);
                    send(pairs[gameId].sockets[1], message, sizeof(message), 0);
                    
                    break;
                }
            }
        }
    }

    pairs[gameId].playerTurn = 2;
    std::string msg = "leave";
    send(pairs[gameId].sockets[1], msg.c_str(), sizeof(msg.c_str()), 0);
    close(pairs[gameId].sockets[0]);
    close(pairs[gameId].sockets[1]);
    std::cout << "Player 1 of game " << std::to_string(gameId) << " leaves." << std::endl;
    pthread_exit(NULL);
}

void *player2(void* arg)
{
    int gameId = currPair;
    cout << "Game " << gameId << ": Player 2 joins." << endl;
    int n;
    char* message = (char*)malloc(sizeof("proceed"));
    sleep(1);
    pairs[gameId].playerTurn = 1;

    message = (char*)malloc(sizeof("player x"));
    strcpy(message, "player x");
    send(pairs[gameId].sockets[1], message, sizeof(message), 0);

    std::string msg = "proceed";
    send(pairs[gameId].sockets[0], msg.c_str(), sizeof(msg.c_str()), 0);

    for(;;)
    {
        if(pairs[gameId].playerTurn == 2)
        {
            n = recv(pairs[gameId].sockets[1], client_message, 2000, 0);
            if(strstr(client_message, "leave") != NULL || n < 1) 
            {
                break;
            }
            std::cout << "G" << std::to_string(gameId) << "P2 message: " << client_message << std::endl;
            
            usleep(100);

            string cMsgStr(client_message);
            if(cMsgStr.size() > 1)
            {
                usleep(100);
                message = (char*)malloc(sizeof("wmove"));
                strcpy(message, "wmove");
                send(pairs[gameId].sockets[1], message, sizeof(message), 0);
            }
            else if(stoi(cMsgStr) > 7 || stoi(cMsgStr) < 1)
            {
                usleep(100);
                message = (char*)malloc(sizeof("wmove"));
                strcpy(message, "wmove");
                send(pairs[gameId].sockets[1], message, sizeof(message), 0);
            }
            else
            {
                int boardResponse = pairs[gameId].board.insert(stoi(cMsgStr) - 1, 2);

                if(boardResponse == -1)
                {
                    usleep(100);
                    message = (char*)malloc(sizeof("wmove"));
                    strcpy(message, "wmove");
                    send(pairs[gameId].sockets[1], message, sizeof(message), 0);
                }
                else if(boardResponse == 0)
                {
                    usleep(100);
                    string sMsg = "board " + cMsgStr;
                    message = (char*)malloc(sizeof(sMsg));
                    strcpy(message, sMsg.c_str());

                    send(pairs[gameId].sockets[1], message, sizeof(message), 0);
                    pairs[gameId].playerTurn = 1;
                    usleep(100);
                    send(pairs[gameId].sockets[0], message, sizeof(message), 0);
                    usleep(100);
                    message = (char*)malloc(sizeof("proceed"));
                    strcpy(message, "proceed");
                    send(pairs[gameId].sockets[0], message, sizeof(message), 0);
                    memset(&client_message, 0, sizeof (client_message));
                }
                else if(boardResponse == 1)
                {
                    usleep(100);
                    string sMsg = "board " + cMsgStr;
                    message = (char*)malloc(sizeof(sMsg));
                    strcpy(message, sMsg.c_str());
                    send(pairs[gameId].sockets[0], message, sizeof(message), 0);
                    send(pairs[gameId].sockets[1], message, sizeof(message), 0);
                    usleep(100);
                    string wMsg = "P2 wins!";
                    message = (char*)malloc(sizeof(wMsg));
                    strcpy(message, wMsg.c_str());
                    send(pairs[gameId].sockets[0], message, sizeof(message), 0);
                    send(pairs[gameId].sockets[1], message, sizeof(message), 0);
                    usleep(100);
                    message = (char*)malloc(sizeof("leave"));
                    strcpy(message, "leave");
                    send(pairs[gameId].sockets[0], message, sizeof(message), 0);
                    send(pairs[gameId].sockets[1], message, sizeof(message), 0);
                    
                    break;
                }
            }
        }
    }

    pairs[gameId].playerTurn = 1;
    msg = "leave";
    send(pairs[gameId].sockets[0], msg.c_str(), sizeof(msg.c_str()), 0);
    close(pairs[gameId].sockets[0]);
    close(pairs[gameId].sockets[1]);
    std::cout << "Player 2 of game " << std::to_string(gameId) << " leaves." << std::endl;
    pthread_exit(NULL);
}

int main()
{
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    int acceptedCycle = 0;

    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;

    serverAddr.sin_port = htons(PORT);

    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    if(listen(serverSocket,50)==0)
        printf("Server started.\n");
    else
        printf("Error\n");

    pthread_t thread_id;

    while(1)
    {
        addr_size = sizeof(serverStorage);
        usleep(200);

        if(acceptedCycle == 2)
        {
            currPair++;
            acceptedCycle = 0;
        }

        pairs[currPair].sockets[pairs[currPair].pcount] = accept(serverSocket, (struct sockaddr*)&serverStorage, &addr_size);
        
        acceptedCycle++;

        if(pairs[currPair].pcount == 0)
        {
            pairs[currPair].gameNumber = currPair;

            if(pthread_create(&pairs[currPair].player1, NULL, player1, &pairs[currPair].sockets[pairs[currPair].pcount]) != 0)
                printf("Failed to create player 1 thread\n");
            else
            {
                pthread_detach(pairs[currPair].player1);
                pairs[currPair].pcount = 1;
            }
        }
        else
        if(pairs[currPair].pcount == 1)
        {
            pairs[currPair].gameNumber = currPair;

            if(pthread_create(&pairs[currPair].player2, NULL, player2, &pairs[currPair].sockets[pairs[currPair].pcount]) != 0)
                printf("Failed to create player 2 thread\n");
            else
            {
                pthread_detach(pairs[currPair].player2);
                pairs[currPair].pcount = 2;
            }
        }
    }

    close(serverSocket);
    return 0;
}