#include "PracticalSocket.h"  // For Socket and SocketException
#include <iostream>           // For cerr and cout
#include <cstdlib>            // For atoi()
#include <string>
#include <vector>
#include <sstream>
#include <utility>


using namespace std;



TCPSocket connection("127.0.0.1", 2002);

const int RCVBUFSIZE = 30;    // Size of receive buffer

class Communication {
    
public:
    
    static std::vector<int> receiveInitMessage() {
        
        try {
            char echoBuffer[RCVBUFSIZE + 1];    // Buffer for echo string + \0
            int recvMsgSize;
            
            // Receive the same string back from the server
            cout << "Received: ";               // Setup to print the echoed string
            if ((recvMsgSize = connection.recv(echoBuffer, RCVBUFSIZE)) > 0) {
                
                std::string s = (string) echoBuffer;
                
                std::vector<int> result = explode(s, '_');
                
                cout << result[0] << endl;
                
                return explode(s, '_');
            }
            cout << endl;
            
            // Destructor closes the socket
            
        } catch(SocketException &e) {
            cerr << e.what() << endl;
        }
        
        return std::vector<int>();
    }
    
    
    static void sendImage(Image image) {
        
        int messageLength = 20;
        
        float rgbValue;
        char message [messageLength];
        
        for (unsigned int i = 0; i < image._image.size(); ++i) {
            
            rgbValue =  (image._image[i] * 255.0f);
            
            sprintf(message, "%d_%f", i, rgbValue);
             
            connection.send(message, 20);
            
            cout << i/(float)image._image.size() << endl;
            
        }
        
        connection.send((const char*) &"/", 1);
        
    }

private:
    static std::vector<int> explode(std::string const & s, char delim)
    {
        std::vector<int> result;
        std::istringstream iss(s);
        
        for (std::string token; std::getline(iss, token, delim); )
        {
            result.push_back(std::move(stoi(token)));
        }
        
        return result;
    }

    
};