#include "client.h"

/**
 * Will always be initialized to connect to the server first
 */
Client::Client(const char *&serverIp, const char *&port)
    : _clientSocket(serverIp, port) {


      
    }