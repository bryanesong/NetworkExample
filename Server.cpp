#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
using namespace std;

int port, repetitions, serverSd, newSd;

void signalSend (int signal)
{
    char dataBuff[1500];

    // Variables for time stamps and calculations
    struct timeval startTime;
    struct timeval stopTime;
    long dataReceivingTime;

    // Record starting time
    gettimeofday(&startTime, NULL);

    int count = 0;
    for (int i = 0; i < repetitions; i++)
    {
        for (int nRead = 0; ( nRead += read( newSd, dataBuff, 1500 - nRead ) ) < 1500 ; ++count );
    }

    //Stop time recording
    gettimeofday(&stopTime, NULL);

    // Send the number of read( ) calls made as an acknowledgment.
    write(newSd, &count, sizeof(count));

    //print results
    dataReceivingTime = ((stopTime.tv_sec - startTime.tv_sec) * 1000000) + (stopTime.tv_usec - startTime.tv_usec);
    cout << "data-receiving time = " << dataReceivingTime << " usec" << endl;

    // End session and exit
    close(newSd);
    close(serverSd);
    exit(0);
}

int main(int argc, char *argv[])
{
    port = atoi(argv[1]);
    repetitions = atoi(argv[2]);

    // Create socket for client server interaction
    sockaddr_in sockAddress;
    bzero((char *)&sockAddress, sizeof(sockAddress));
    sockAddress.sin_family = AF_INET;
    sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddress.sin_port = htons(port);

    // Opening socket
    serverSd = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));

    //Creating TCP with local address
    int returnCode = bind(serverSd, (sockaddr *)&sockAddress, sizeof(sockAddress));
    if (returnCode < 0)
    {
        cout << "Error with local address" << std::endl;
        close(serverSd);
        return -1;
    }

    // Listening for connections
    listen(serverSd, 5);//maximum connections is 5
    sockaddr_in newSockAddress;
    socklen_t newSockAddressSize = sizeof(newSockAddress);

    // Wait to receive a request from client and get new socket
    newSd = accept(serverSd, (sockaddr *)&newSockAddress, &newSockAddressSize);

    // Wait for signal interruption before closing server
    signal(SIGIO, signalSend);
    fcntl(newSd, F_SETOWN, getpid());
    fcntl(newSd, F_SETFL, FASYNC);

    //close server
    return 0;
}