#include "pch.h"
#include <zmq.h>
#include <cstdio>
//#include <cstring>
#include <cassert>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>

using namespace std;

/*int main(void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(responder, "tcp://*:5555");
    assert(rc == 0);

    while (1) {
        char buffer[10];
        zmq_recv(responder, buffer, 5, 0);
        printf("Request received\n");
        //printf("%s\n", buffer);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::string my_string = std::to_string(242424);
        printf("Sending data\n");
        zmq_send(responder, my_string.c_str(), 5, 0);
    }
    return 0;
}*/

int main(void) {
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_SUB);

    
    
    while (true) {
        char buffer[2]; // Just 8 bits

        zmq_connect(requester, "tcp://localhost:5555");

        //cout << "Sending request..." << endl;
        //zmq_send(requester, "data_req", 1, 0);
        zmq_recv(requester, buffer, 1, 0);
        buffer[1] = '\0';

        zmq_disconnect(requester, "tcp://localhost:5555");

        cout << "Received buttons: " << buffer[0] << endl;
            //((int)buffer & 0b00000001) << " " << ((int)buffer & 0b00000010) << endl << endl;
    }

    zmq_close(requester);
    zmq_ctx_destroy(context);
}