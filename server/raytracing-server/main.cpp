/*
 *   C++ sockets on Unix and Windows
 *   Copyright (C) 2002
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "PracticalSocket.h"  // For Socket, ServerSocket, and SocketException
#include <iostream>           // For cerr and cout
#include <cstdlib>            // For atoi()
#include "ImageWriter.h"
#include <pthread.h>          // For POSIX threads


#define NUM_CLIENTS 2
#define NUM_BLOCKS_X 2
#define NUM_BLOCKS_Y 1
#define WIDTH 200
#define HEIGHT 200

int currentClients = 0;

Image result = Image(WIDTH, HEIGHT);

const int RCVBUFSIZE = 20;

void HandleTCPClient(TCPSocket *sock);     // TCP client handling function
void *ThreadMain(void *arg);               // Main program of a thread

int main(int argc, char *argv[]) {
    if (argc != 2) {                 // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server Port> " << endl;
        exit(1);
    }
    
    unsigned short echoServPort = atoi(argv[1]);    // First arg:  local port
    
    try {
        TCPServerSocket servSock(echoServPort);   // Socket descriptor for server
        
        for (;;) {      // Run forever
            // Create separate memory for client argument
            TCPSocket *clntSock = servSock.accept();
            
            // Create client thread
            pthread_t threadID;              // Thread ID from pthread_create()
            if (pthread_create(&threadID, NULL, ThreadMain,
                               (void *) clntSock) != 0) {
                cerr << "Unable to create thread" << endl;
                exit(1);
            }
        }
    } catch (SocketException &e) {
        cerr << e.what() << endl;
        exit(1);
    }
    // NOT REACHED
    
    return 0;
}



// TCP client handling function
void HandleTCPClient(TCPSocket *sock) {
    cout << "Handling client ";
    try {
        cout << sock->getForeignAddress() << ":";
    } catch (SocketException &e) {
        cerr << "Unable to get foreign address" << endl;
    }
    try {
        cout << sock->getForeignPort();
    } catch (SocketException &e) {
        cerr << "Unable to get foreign port" << endl;
    }
    cout << " with thread " << pthread_self() << endl;
    
    
    // Welcome the client
    
    currentClients++;
    
    // Block size
    unsigned int numberOfXPixelsInBlock = ceil(WIDTH / NUM_BLOCKS_X);
    unsigned int numberOfYPixelsInBlock = ceil(HEIGHT / NUM_BLOCKS_Y);
    
    
    int currentXBlock = (currentClients-1)/NUM_BLOCKS_X;
    int currentYBlock = (currentClients-1) % NUM_BLOCKS_X;
    
    int startX = currentXBlock * numberOfXPixelsInBlock; // start x
    int endX = (currentXBlock+1) * numberOfXPixelsInBlock; // end x
    int startY = currentYBlock * numberOfYPixelsInBlock; // start y
    int endY = (currentYBlock+1) * numberOfYPixelsInBlock; // end y
    
    // START Y INCORRECT
    
    cout << "Client number: " << currentClients << " << start x: " << startX << " start y: " << startY << endl;
    
    // Send block and image size
    int messageLength = 30;
    char message [messageLength];
        
    sprintf(message, "%i_%i_%i_%i_%i_%i", WIDTH, HEIGHT, startX, startY, endX, endY);
    
    sock->send(message, messageLength);
    
    
    // Send received string and receive again until the end of transmission
    char echoBuffer[RCVBUFSIZE];
    int recvMsgSize;
    while ((recvMsgSize = sock->recv(echoBuffer, RCVBUFSIZE)) > 0 && echoBuffer[0] != '/') { // Zero means
        // end of transmission
        
        
        std::string s = (string) echoBuffer;
        std::string delimiter = "_";
        int i = stoi(s.substr(0, s.find(delimiter)));
        
        float rgbValue = stof(s.substr(s.find(delimiter)+1));
        if (rgbValue > 0 )
            cout << "Received message: " << i << "RGB " << rgbValue << endl;
        
        result.setValue(i, rgbValue);
        
        
       // sock->send(echoBuffer, recvMsgSize);
    }
    
    result.writeImage("result.ppm");
    
    // Destructor closes socket
}



void *ThreadMain(void *clntSock) {
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());
    
    // Extract socket file descriptor from argument
    HandleTCPClient((TCPSocket *) clntSock);
    
    delete (TCPSocket *) clntSock;
    return NULL;
}