#include "pch.h"
#include <zmq.h>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>

int main(void)
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
}