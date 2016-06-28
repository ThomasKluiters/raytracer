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
#include <vector>


unsigned int NUM_CLIENTS;
unsigned int NUM_BLOCKS_X;
unsigned int NUM_BLOCKS_Y;
unsigned int WIDTH;
unsigned int HEIGHT;


std::vector<float> origin00(3);
std::vector<float> origin01(3);
std::vector<float> origin10(3);
std::vector<float> origin11(3);


std::vector<float> dest00(3);
std::vector<float> dest01(3);
std::vector<float> dest10(3);
std::vector<float> dest11(3);


int currentClients = 0;

Image *result;

const int RCVBUFSIZE = 20;

void HandleTCPClient(TCPSocket *sock);     // TCP client handling function
void *ThreadMain(void *arg);               // Main program of a thread

int main(int argc, char *argv[]) {
    
    origin00[0] = -0.00466308;
    origin00[1] = 0.00466308;
    origin00[2] = 3.99;
    origin01[0] = -0.00466308;
    origin01[1] = -0.00461645;
    origin01[2] = 3.99;
    origin10[0] = 0.00461645;
    origin10[1] = 0.00466308;
    origin10[2] = 3.99;
    origin11[0] = 0.00461645;
    origin11[1] = -0.00461645;
    origin11[2] = 3.99;


    dest00[0] = -4.66308;
    dest00[1] = 4.66308;
    dest00[2] = -6.00001;
    dest01[0] = -4.66308;
    dest01[1] = -4.61645;
    dest01[2] = -6.00001;
    dest10[0] = 4.61645;
    dest10[1] = 4.66308;
    dest10[2] = -6.00001;
    dest11[0] = 4.61645;
    dest11[1] = -4.61645;
    dest11[2] = -6.00001;
    
    
    if (argc != 7) {                 // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server Port> <Number of Clients> <Number of block in X direction> <Number of blocks in Y direction> <Width> <Height>" << endl;
        exit(1);
    }
    
    unsigned short echoServPort = atoi(argv[1]);    // First arg:   local port
    NUM_CLIENTS = atoi(argv[2]);                    // Second arg:  total number of clients
    NUM_BLOCKS_X = atoi(argv[3]);                   // Third arg:   blocks in x direction
    NUM_BLOCKS_Y = atoi(argv[4]);                   // Fourth arg:  blocks in y direction
    WIDTH = atoi(argv[5]);                          // Fifth arg:   width of image
    HEIGHT = atoi(argv[6]);                         // Sixth arg:   height of image
    
    result = new Image(WIDTH, HEIGHT);
    
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
    
    if(currentClients > NUM_CLIENTS) {
        cout << "Max clients reached" << endl;
        return;
    }
    
    
    int messageLength = 40;
    char message [messageLength];
    
    
    // Send origin and destination
    sprintf(message, "-000_%f_%f_%f", origin00[0], origin00[1], origin00[2]);
    sock->send(message, messageLength);
    sprintf(message, "-001_%f_%f_%f", origin01[0], origin01[1], origin01[2]);
    sock->send(message, messageLength);
    sprintf(message, "-010_%f_%f_%f", origin10[0], origin10[1], origin10[2]);
    sock->send(message, messageLength);
    sprintf(message, "-011_%f_%f_%f", origin11[0], origin11[1], origin11[2]);
    sock->send(message, messageLength);
    
    sprintf(message, "-100_%f_%f_%f", dest00[0], dest00[1], dest00[2]);
    sock->send(message, messageLength);
    sprintf(message, "-101_%f_%f_%f", dest01[0], dest01[1], dest01[2]);
    sock->send(message, messageLength);
    sprintf(message, "-110_%f_%f_%f", dest10[0], dest10[1], dest10[2]);
    sock->send(message, messageLength);
    sprintf(message, "-111_%f_%f_%f", dest11[0], dest11[1], dest11[2]);
    sock->send(message, messageLength);
    
    
    
    // Block size
    unsigned int numberOfXPixelsInBlock = ceil(WIDTH / NUM_BLOCKS_X);
    unsigned int numberOfYPixelsInBlock = ceil(HEIGHT / NUM_BLOCKS_Y);
    
    
    int currentXBlock = (currentClients-1) % NUM_BLOCKS_X;
    int currentYBlock = (currentClients-1) / NUM_BLOCKS_X;
    
    int startX = currentXBlock * numberOfXPixelsInBlock; // start x
    int endX = (currentXBlock+1) * numberOfXPixelsInBlock; // end x
    int startY = currentYBlock * numberOfYPixelsInBlock; // start y
    int endY = (currentYBlock+1) * numberOfYPixelsInBlock; // end y
    
    cout << "Client number: " << currentClients << " << start x: " << startX << " start y: " << startY << endl;
    
    // Send block and image size
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
        
        result->setValue(i, rgbValue);
    }
    
    result->writeImage("result.ppm");
    
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