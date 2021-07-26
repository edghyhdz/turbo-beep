# P2P 
Basic working example between to peers out of local network. 

## Installing
In the project directory

```sh
mkdir build && cd build
cmake ..
make
```

This will create 3 executables. 

    1. p2p - Client executable
    2. server - Server executable (to mediate peer connection)
    3. tests - basic unit tests

## Usage
Start the server with 
```sh
./server/server
```
It will automatically bind to port 54700, you can use tools like `ngrok` to open it to the public. 

After you have the server open, connect to it like so:

```sh
./p2p <hostname> <port> <yourName> <peerName>
```

The other peer should do the same. After connecting to the server and if the given names matched their peer names, the server will send a response with the peer's ip address and port. 

A continous message will be sent to the other peer containing the sending's peer ip address. 


