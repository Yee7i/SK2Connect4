#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

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
                    std::cout << " ! ! ! ! ! " << std::endl;
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
            if (checkIdx(i, posY) == playerSign)
            {
                row++;
            }
            else
                row = 0;

            if (row == 4) return true;
        }

        row = 0;
        i = 0;
        if (posY > 2) i = posY - 3;
        rb = 6;
        if (posY < 4) rb = posY + 3;

        for (; i <= rb; i++)
        {
            if (checkIdx(posX, i) == playerSign)
            {
                row++;
            }
            else
                row = 0;

            if (row == 4) return true;
        }

        row = 0;
        i = -3;
        for (; i < 4; i++)
        {
            if (checkIdx(posX + i, posY + i) == playerSign)
            {
                row++;
            }
            else
                row = 0;

            if (row == 4) return true;
        }

        row = 0;
        i = -3;
        for (; i < 4; i++)
        {
            if (checkIdx(posX + i, posY - i) == playerSign)
            {
                row++;
            }
            else
                row = 0;

            if (row == 4) return true;
        }

        return false;
    }
};

int main(int argc, char** argv)
{
    if(argc <= 1) 
    {
        std::cout << "You must provide an IP address (1) and a port (2).";
        return -1;
    }
    
    char message[1000];
    char buffer[1024];
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;
    string msgStr;
    bool notify = false;

    Board board;

    int player = 1;
    std::string response = "";

    int boardResponse = 0;

    char playerSign = 'o';

    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    addr_size = sizeof(serverAddr);
    connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

    for(;;)
    {
        if(recv(clientSocket, buffer, 1024, 0) < 0)
        {
            if(!notify) cout << "No server connection or read error. Waiting for server..." << endl;
            notify = true;
            connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
        }
        else
        {
            notify = false;
            if(strstr(buffer, "player") != NULL)
            {
                playerSign = buffer[7];
                if(playerSign == 'x') player = 2;
                else player = 1;

                board.displayBoard();
                cout << "Connection established and a new game is starting! You're player " << player << " and your sign is o. Wait for your turn." << endl;
            }
            else
            if(strstr(buffer, "proceed") != NULL || strstr(buffer, "wmove") != NULL)
            {
                if(strstr(buffer, "wmove") != NULL) 
                    cout << "Provide a correct value (1-7 from the not filled columns): ";
                else
                    cout << "It's your turn: ";
                
                cin.clear();
                cin >> message;

                if(strstr(message, "exit") != NULL)
                {
                    printf("Exiting\n");
                    break;
                }

                if(send(clientSocket, message, strlen(message), 0) < 0)
                {
                    printf("Send failed\n");
                }

                memset(&message, 0, sizeof (message));
            }
            else if(strstr(buffer, "leave") != NULL)
            {
                printf("The game has ended.\n");
                break;
            }
            else if(strstr(buffer, "board") != NULL)
            {
                char move = buffer[6];
                char sign;
                player == 1 ? player = 2 : player = 1;
                board.insert((move - '0') - 1, player);
                board.displayBoard();
            }
            else
            {
                cout << "Server message: \"" << buffer << "\"" << endl;
            }
        }
    }

    close(clientSocket);
    return 0;
}
