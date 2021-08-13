#!/bin/bash

print_options()
{
   echo ""
   echo "Usage: $0 p2p [Options] <server ip address> <server port> <your username or public key> <other peer's username / public key>"
   echo ""
   echo "Options inlcude: "
   echo "   p2p Messaging without encryption. Use your username and other peer's username"
   echo "   p2p Messaging with encrpytion. Provide path to your public key and peer's public key"
   echo "   p2p file sharing. Provide path to your public key and peer's public key and path to file to share (up to 7mb)"
}

# Check before hand whether all args were provided
run_script()
{
    if [ -z "$2" ] || [ -z "$3" ] || [ -z "$4" ] || [ -z "$5" ]
    then
        echo "There are missing arguments. Please refer to the options below";
        print_options
    fi

    if [ $1 = "-f" ]
    then 
        if [ -z $6 ]
        then
            echo "Please include the path of file to share with peer";
            print_options
        fi
    fi

    case $1 in 
        "-e") "${P2P_EXE_PATH}" $2 $3 $4 $5;;
        "-m") "${P2P_EXE_PATH}" $2 $3 "0" $4 $5;;
        "-f") "${P2P_EXE_PATH}" $2 $3 "0" $4 $5 $6;;
    esac
}

while getopts "e:m:f:" opt
do
   case "$opt" in
      e ) run_script "-e" $2 $3 $4 $5;;
      m ) run_script "-m" $2 $3 $4 $5;;
      f ) run_script "-f" $2 $3 $4 $5 $6;;
      ? ) print_options;;
   esac
done
