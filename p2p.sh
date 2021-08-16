#!/bin/bash

print_options()
{
   echo ""
   echo "Usage: $0 [Options] <server ip address> <server port> <your username or public key> <other peer's username / public key>"
   echo ""
   echo "Options inlcude: "
   echo "   -e: p2p Messaging without encryption. Use your username and other peer's username"
   echo "   -m: p2p Messaging with encrpytion. Provide path to your public key and peer's public key"
   echo "   -f: p2p file sharing (send). Provide path to your public key and peer's public key and path to file to share (up to 7mb)"
   echo "   -r: p2p file sharing (receive). Provide path to your public key and peer's public key"
   echo ""
   exit 0
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
        "-r") "${P2P_EXE_PATH}" $2 $3 "0" $4 $5 "-r";;
    esac
    exit 0
}

while getopts "e:m:f:r:" opt
do
   case "$opt" in
      e ) run_script "-e" $2 $3 $4 $5;;
      m ) run_script "-m" $2 $3 $4 $5;;
      f ) run_script "-f" $2 $3 $4 $5 $6;;
      r ) run_script "-r" $2 $3 $4 $5;;
      h ) print_options;;
      ? ) print_options;;
   esac
done

# In case no option was given
print_options; 