#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
using namespace std;

const int BUFF_SIZE = 1500;

int main(int argc, char *argv[])
{
    int port = atoi(argv[1]);
    int repetitions = atoi(argv[2]);
    int nbuf = atoi(argv[3]);
    int bufsize = atoi(argv[4]);
    int type = atoi(argv[6]);

    const char *server = argv[5];
    char databuf[nbuf][bufsize];

    struct timeval startTime;
    struct timeval trackTime;
    struct timeval stopTime;

    long dataSendTime, roundTripTime;

    // Get hostent structure corresponding to server IP/name passed in
    struct hostent *hostName = gethostbyname(server);
    if (hostName == NULL)
    {
        cout << "Error with hostname" << std::endl;
        return EXIT_FAILURE;
    }

    // Create socket for client server interaction
    sockaddr_in socketAddress;
    bzero((char *)&socketAddress, sizeof(socketAddress));
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = inet_addr(inet_ntoa( *(struct in_addr*) (*hostName->h_addr_list)));
    socketAddress.sin_port = htons(port);

    // Opening socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        std::cout << "Error with client socket" << std::endl;
        close(clientSocket);
        return EXIT_FAILURE;
    }

    // connect server to client
    int returnNum = connect(clientSocket, (sockaddr *)&socketAddress, sizeof(socketAddress));
    if (returnNum < 0)
    {
        std::cout << "Error! CONNECT failed." << std::endl;
        //close(clientSocker);
        return -1;
    }


    // Start recording time
    gettimeofday(&startTime, NULL);

    // Write to server depending on write type
    for (int count = 0; count < repetitions; count++) {
        if (type == 1) //Multiple writes
        {
            for (int position = 0; position < nbuf; position++) {
                write(clientSocket, databuf[position], bufsize);
            }
        } else if (type == 2) //Writev
        {
            struct iovec vector[nbuf];
            for (int position = 0; position < nbuf; position++) {
                vector[position].iov_base = databuf[position];
                vector[position].iov_len = bufsize;
            }
            writev(clientSocket, vector, nbuf);
        } else //Single Write
        {
            write(clientSocket, databuf, (nbuf * bufsize));
        }
    }
    gettimeofday(&trackTime, NULL);

    //Timed results
    dataSendTime = ((trackTime.tv_sec - startTime.tv_sec) * 1000000) + (trackTime.tv_usec - startTime.tv_usec);

    //print out results
    cout << "data-sending time = " << dataSendTime << " usec, ";

    close(clientSocket); //closing socket
    return 0;
}

