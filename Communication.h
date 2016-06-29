#pragma once
#include "PracticalSocket.h"  // For Socket and SocketException
#include "FilmPlane.h"
#include <iostream>           // For cerr and cout
#include <cstdlib>            // For atoi()
#include <string>
#include <vector>
#include <sstream>
#include <utility>


using namespace std;


Vec3Df origin00, dest00;
Vec3Df origin01, dest01;
Vec3Df origin10, dest10;
Vec3Df origin11, dest11;

TCPSocket *connection;

//TCPSocket connection("127.0.0.1", 2003);

const int RCVBUFSIZE = 40;    // Size of receive buffer

class Communication {
    
public:
    
    static std::vector<int> receiveInitMessage() {
        
        try {
            char echoBuffer[RCVBUFSIZE + 1];    // Buffer for echo string + \0
            int recvMsgSize;

            // Receive the same string back from the server
            cout << "Received: ";               // Setup to print the echoed string
            while ((recvMsgSize = connection->recv(echoBuffer, RCVBUFSIZE)) > 0) {
                
                std::string s = (string) echoBuffer;
                
                std::vector<float> result = explodeFloat(s, '_');
                
                if (result[0] == -000) {
                    origin00 = Vec3Df(result[1], result[2], result[3]);
                }
                else if (result[0] == -001) {
                    origin01 = Vec3Df(result[1], result[2], result[3]);
                }
                else if (result[0] == -10) {
                    origin10 = Vec3Df(result[1], result[2], result[3]);
                }
                else if (result[0] == -11) {
                    origin11 = Vec3Df(result[1], result[2], result[3]);
                }
                
                else if (result[0] == -100) {
                    dest00 = Vec3Df(result[1], result[2], result[3]);
                }
                else if (result[0] == -101) {
                    dest01 = Vec3Df(result[1], result[2], result[3]);
                }
                else if (result[0] == -110) {
                    dest10 = Vec3Df(result[1], result[2], result[3]);
                }
                else if (result[0] == -111) {
                    dest11 = Vec3Df(result[1], result[2], result[3]);
                }
                
                else {
                    return explode(s, '_');
                }
                
            }
            cout << endl;
            
            // Destructor closes the socket
            
        } catch(SocketException &e) {
            cerr << e.what() << endl;
        }
        
        return std::vector<int>();
    }

	static void sendImage(FilmPlane sensor)
	{
		const int messageLength = 20;

		float rgbValue;
		char message [messageLength];

		cout << "Image Data Sending: Yo, some image data incoming, right about.... now." << endl;

		for (unsigned int i = 0; i < sensor.imageData.size() * 3; ++i)
		{
			rgbValue = (sensor.imageData[(i / 3) % sensor.imageData.size()][(i / 3) / sensor.imageData.size()][i % 3]);

			// Only send value when necessary
			if (rgbValue > 0.0f)
			{
				snprintf(message, messageLength, "%d_%f", i, rgbValue);

				connection->send(message, 20);
			}

			//            cout << i/(float)image._image.size() << endl;
		}

		connection->send((const char*) &"/", 1);

		cout << "Image Data Sent: That's all image data (for now)." << endl;
	        
	    }
    
//    static void sendImage(Image image) {
//        
//        const int messageLength = 20;
//        
//        float rgbValue;
//        char message [messageLength];
//        
//		cout << "Image Data Sending: Yo, some image data incoming, right about.... now." << endl;
//
//        for (unsigned int i = 0; i < image._image.size(); ++i) {
//            
//            rgbValue =  (image._image[i] * 255.0f);
//            
//            // Only send value when necessary
//            if (rgbValue > 0.0f) {
//            
//                sprintf(message, "%d_%f", i, rgbValue);
//                 
//                connection->send(message, 20);
//                
//            }
//            
////            cout << i/(float)image._image.size() << endl;
//            
//        }
//        
//        connection->send((const char*) &"/", 1);
//		
//		cout << "Image Data Sent: That's all image data (for now)." << endl;
//        
//    }

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
                    
    static std::vector<float> explodeFloat(std::string const & s, char delim)
    {
        std::vector<float> result;
        std::istringstream iss(s);
        
        for (std::string token; std::getline(iss, token, delim); )
        {
            result.push_back(std::move(stof(token)));
        }
        
        return result;
    }

    
};