# Turbo-Beep 
P2P between different networks.

Peer to peer file sharing, as well as sending messages to a peer without depending on a server to do the job for you. 
This repository builds the `Peer` executable as well as the `Server` one. The latter working only as a mediator, since the connection will be established in the end only by the two peers. 

The process is as follows:

`Peer1` advertises to the server by including his/her information (`port_number`, `ip_address` and `peer2` `hashed_public_key`). Once `peer2` is online and has succesfully authenticated with the server, `peer1` information will be sent to `peer2`, likewise, `peer2` information will be sent to `peer1`. 

Once both peers have each other's information, they can engage into trying to connect to each other via hole punching. 

## Table of Contents

1. [Description](#description)
2. [Usage](#usage)
3. [Dependencies](#dependencies)
4. [Installation](#installation)
5. [References](#references)
6. [Disclaimer](#disclaimer)
7. [Author](#author)

## Description

This is a command line application that depends on a server acting as a mediator, as well as two parties (either in different networks or in the same local one), trying to connect to each other without the need of a server to maintain communication.

#### Server
The server is intended to run on an open port. You can, i.e., open a port by using third party tools such as [ngrok](https://ngrok.com/docs#tcp-examples). 

The server acts as a mediator for both peers to connect. Once both peers are online, the server will be sending relevant information for the peers to connect to each other.

It is not intentded to manage several clients, since it is not running any threads for socket management, so it might be showing bad behavior if overused. It is not intended to be run in production.

#### P2P CLient
The `peer-to-peer` client is designed to interact with the included `server`. Upon connecting to the server, it will `authenticate`* to it and only after successful authentication, it will be able to proceed and connect to the other peer. It is assumed that the server has the public keys of each of the connecting peers.

*(Authentication is based on a challenge/response, by signing a nonce and sending it back to the connecting party)

Once `peer1` connects to `peer2`, there is a second authentication step. For this step `peer1`, who advertised first, will be acting as the client, whereas `peer2` as the "to connect" party.

## Usage

Once the installation has been done,

As per default, the port to which the server is connecting to is `54700` (You will need to open the server to a public domain in case the othe peer is in a different network)

**Starting the Server** (In case you are running the server)

```sh
source server_p2p
```

**Starting the Client**

For peer1 trying to connect to peer2

```sh
source p2p <hostname> <port> <build_directory>/certs/peer1/mykeypairs.pem ./certs/peer1/peer.pem
```
For peer2 trying to connect to peer1

```sh
source p2p <hostname> <port> <build_directory>/certs/peer2/mykeypairs.pem ./certs/peer2/peer.pem
```

`<build_directory>/certs/peer2/peer.pem` is the other peer's public key, so in this example it would be peer1's public key

If everything was done allright, this should have created a connection between two peers that are in two different networks. The messages that they exchange are encrypted with their corresponding private keys.

**Sending a file**

If sending a file is what you want (Although not encrypted for now)


`127.0.0.1` and the port `54000` could be changed for example to your own `TCP` address if you've got one. Of course, the server would have to be running on that address. 


## Dependencies

 1. [Protobuf](https://developers.google.com/protocol-buffers/docs/cpptutorial)``` sudo apt install protobuf-compiler```
 2. [Open SSL](https://www.openssl.org/) ``` sudo apt-get install libssl-dev```
 3. [libcurl](https://curl.se/libcurl/) ``` sudo apt-get install -y libcurl-dev```
 5. [GTest](https://github.com/google/googletest) ```sudo apt-get install -y googletest```
 4. [cmake](https://www.gnu.org/software/make/) ``` sudo apt-get -y install cmake```

## Installation

Clone this repository like so, 
 ```sh
 git clone https://github.com/edghyhdz/turbo-beep.git
 ```
 
 Once inside the root project folder `turbo-beep`,
 ```sh
 # Lets start by creating the build directory
 mkdir build && cd build

 # cmake 
 cmake ..

 # Finally
 source install.sh
 ```
`install.sh` will run the final installation that will create a terimal shortcut named `server_p2p` and `p2p`. 
 
The final project folder structure is the following (the `certs` folder is included to have a working example after building up the project),

    .
    ├── ...     
    ├── build                  # Build directory
    │   ├── certs              # Example certificatse
    │   │   └─── hashed_key1   # peer1 hashed certificate with public key
    │   │   └─── hashed_key2   # peer1 hashed certificate with public key
    │   │   └─── peer1         # peer1 key pair and peer2 public key
    │   │   └─── peer2         # peer1 key pair and peer2 public key
    │   ├── ./server             # Executable to run the server
    │   ├── ./test               # Exe to run the tests
    │   └── ./p2p                # Exe of peer (client)
    └── ...

## References
The most relevant and helpful references are the following, 

1. Taylor Conor's [`p2pcs`](https://github.com/taylorconor/p2psc) repository. Which is really well devleoped. Used it as inspriation and guidance, as well as for some useful references such as the mediator or peer handshake and other ideas. Although I did not use his libary on this project, it was of great help. 
2. Code snipets from [here](https://www.programmersought.com/article/37955188510/) were taken ,for the RSA encryption part.[ProgrammerSought](https://www.programmersought.com/)
3. To deal with protobuffers and sockets, I used [this](https://stackoverflow.com/a/11339251) stackoverflow answer as a basis, and adapted it to fit my project needs.
3. Finally, to generate the key pairs I used [this](https://www.dynamsoft.com/codepool/how-to-use-openssl-generate-rsa-keys-cc.html) code snippet from [Dynamsoft](https://www.dynamsoft.com)

More references can be found inside the code.

## Disclaimer
Feel free to use, change this code. The use that you might give to this project is at your own risk. This is not intended to be used in production, since its main development purpose was to try p2p networking out.

## Author

Edgar Hernandez 


