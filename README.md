# Turbo-Beep 

Peer to peer file sharing and messaging between different networks including server to act as a mediator.

This repository will create 3 executables.

    1. p2p - Client executable
    2. server - Server executable (to mediate peer connection)
    3. test - basic unit tests

After the peers have successfully connected to each other, the server is no longer needed.

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

It is not intentded to manage several clients, since it is not running any threads for socket management, so it might be showing bad behavior if overused. It is not intended to be run on production.

#### P2P Client
The `peer-to-peer` client is designed to interact with the included `server`. Upon connecting to the server, it will `authenticate`* to it and only after successful authentication, it will be able to proceed and connect to the other peer. It is assumed that the server has the public keys of each of the connecting peers (saved in the certs directory with the hashed public key as name).

*(Authentication is based on a challenge/response, by signing a nonce and sending it back to the connecting party)

Once `peer1` connects to `peer2`, there is a second authentication step. For this step `peer1`, who advertised first, will be acting as the client, whereas `peer2` as the "to connect" party.

An example of how a successful connection would look like is shown below, 

Peer 1 (Mexico)           |  Peer 2 (Switzerland)
:-------------------------:|:-------------------------:
![](https://github.com/edghyhdz/turbo-beep/blob/main/images/peer1_e.png)  |  ![](https://github.com/edghyhdz/turbo-beep/blob/main/images/peer2_e.png)

Where Peer 1 advertised first, and thus authenticated to Peer 2 upon retrieving the information back from the server.


## Usage

Once the installation has been done,

As per default, the port to which the server is connecting to is `54700` (You will need to open the server to a public domain in case the othe peer is in a different network)

**Starting the Server** (In case you are running the server)

```sh
<build_directory>/server/server
```

**Starting the Client**

The 3 ways to use the `p2p` command tool and their corresponding flags are shown below,

<ins>1. Easy mode</ins> 

Flag: `-e`

Description: Connect to other peer by providing an arbitrary username for you and the other peer (unencrypted).

Example,

For peer 1 (michel)
```sh
p2p -e <server host> <server port> michel roxana
```
 For peer 2 (roxana)
```sh
p2p -e <server host> <server port> roxana michel
```

<ins>2. Encrypted messaging</ins>

Flag: `-m`

Description: Connect to other peer by providing the path of your key pair and the path of the other peer's public key

Example,

For peer 1
```sh
p2p -m <server host> <server port> ./certs/peer1/mykeypair.pem ./certs/peer1/peer.pem
```
Where peer1/peer.pem is peer2's public key

 For peer 2
```sh
p2p -m <server host> <server port> ./certs/peer2/mykeypair.pem ./certs/peer2/peer.pem
```
Where peer2/peer.pem is peer1's public key

<ins>3. File sharing (no encryption, for now)</ins>

Flag: `-f` or `-r` (send file and receive file respectively)

Description: Connect to other peer by providing the path of your key pair and the path of the other peer's public key. 

After successfully connecting to other peer, send or receive a file depending on the flag given as argument.

Example,

For peer 1 (sending file)
```sh
p2p -f <server host> <server port> ./certs/peer1/mykeypair.pem ./certs/peer1/peer.pem /path/to/file.ext
```
Where peer1/peer.pem is peer2's public key

 For peer 2 (receiving file)
```sh
p2p -r <server host> <server port> ./certs/peer2/mykeypair.pem ./certs/peer2/peer.pem
```
Where peer2/peer.pem is peer1's public key

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
 cmake .. && make

 # Finally
 source install.sh
 ```
`install.sh` will run the final installation that will create a terimal shortcut named `p2p`. 
 
The final project folder structure is the following (the `certs` folder is included to have a working example after building up the project),

    .
    ├── ...     
    ├── build                  # Build directory
    │   ├── certs              # Example certificatse
    │   │   └─── hashed_key1   # peer1 hashed certificate with public key
    │   │   └─── hashed_key2   # peer1 hashed certificate with public key
    │   │   └─── peer1         # peer1 key pair and peer2 public key
    │   │   └─── peer2         # peer1 key pair and peer2 public key
    │   ├── ./server           # Executable to run the server
    │   ├── ./test             # Exe to run the tests
    │   └── ./p2p              # Exe of peer (client)
    └── ...

## References
The most relevant and helpful references are the following, 

1. Taylor Conor's [`p2pcs`](https://github.com/taylorconor/p2psc) repository. Which is really well devleoped. Used it as inspriation and guidance, as well as for some useful references such as the mediator or peer handshake and other ideas. Although I did not use his libary on this project, it was of great help. 
2. Code snipets from [here](https://www.programmersought.com/article/37955188510/) were taken, for the RSA encryption part. [ProgrammerSought](https://www.programmersought.com/)
3. To deal with protobuffers and sockets, I used [this](https://stackoverflow.com/a/11339251) stackoverflow answer as a reference, and adapted it to fit my project needs.

More references can be found inside the code.

## Disclaimer
Feel free to use, change this code. The use that you might give to this project is at your own risk. This is not intended to be used in production, since its main development purpose was to try p2p networking out.

## Author

Edgar Hernandez 


