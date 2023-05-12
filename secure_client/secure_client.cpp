//////////////////////////////////////////////////////////////////////////////////////////////
// TCP CrossPlatform CLIENT v.1.0 (towards IPV6 ready)
// compiles using GCC 
//
//
// References: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
//             http://long.ccaba.upc.edu/long/045Guidelines/eva/ipv6.html#daytimeServer6
//             Andre Barczak's tcp client codes
//
// Author: Napoleon Reyes, Ph.D.
//         Massey University, Albany  
//
//////////////////////////////////////////////////////////////////////////////////////////////

#if defined __unix__ || defined __APPLE__
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h> //used by getnameinfo()
#include <cstdio>
#include <iostream>
#elif defined _WIN32

#include <winsock2.h>
#include <ws2tcpip.h> //required by getaddrinfo() and special constants
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "../CryptoSystem.h"

#define WSVERS MAKEWORD(2,2) /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
//The high-order byte specifies the minor version number;
//the low-order byte specifies the major version number.

WSADATA wsadata; //Create a WSADATA object called wsadata.
#endif

#define USE_IPV6 true
#define DEFAULT_IPV4_ADDR "127.0.0.1"
#define DEFAULT_IPV6_ADDR "::1"
#define DEFAULT_PORT "1234"
#define BLOCK_SIZE 3
#define SBUFFER_SIZE 4096
#define RBUFFER_SIZE 4096
//remember that the BUFFER SIZE has to be at least big enough to receive the answer from the server
#define SEGMENT_SIZE 256
//segment size, i.e., if fgets gets more than this number of bytes it segments the message

//////////////////////////////////////////////////////////////////////////////////////////////


enum CommandName {
    USER, PASS, SHUTDOWN
};

//using namespace std;
/////////////////////////////////////////////////////////////////////

int recvFromServer(SOCKET ss, char *recv_buffer) {
    int bytes;
    int n = 0;
    while (true) {
        bytes = recv(ss, &recv_buffer[n], 1, 0);

//        std::cout << recv_buffer[n] << std::endl;

        if ((bytes < 0) || (bytes == 0)) break;

        if (recv_buffer[n] == '\n') { /*end on a LF, Note: LF is equal to one character*/
            recv_buffer[n] = '\0';
            break;
        }
        if (recv_buffer[n] != '\r') n++; /*ignore CRs*/
    }
    return bytes;
}

//void printBuffer(const char *header, char *buffer) {
//
//    cout << "------" << header << "------" << endl;
//    for (unsigned int i = 0; i < strlen(buffer); i++) {
//        if (buffer[i] == '\r') {
//            cout << "buffer[" << i << "]=\\r" << endl;
//        } else if (buffer[i] == '\n') {
//            cout << "buffer[" << i << "]=\\n" << endl;
//        } else {
//            cout << "buffer[" << i << "]=" << buffer[i] << endl;
//        }
//    }
//    cout << "---" << endl;
//}

/////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
//*******************************************************************
// Initialization
// What are the important data structures?
//*******************************************************************

    char portNum[12];

#if defined __unix__ || defined __APPLE__
    int s;
#elif defined _WIN32
    SOCKET s;
#endif


    char send_buffer[SBUFFER_SIZE], receive_buffer[RBUFFER_SIZE];
    int n, bytes;

    char serverHost[NI_MAXHOST];
    char serverService[NI_MAXSERV];

    mp::cpp_int eCA = 0;
    mp::cpp_int nCA = 0;
    mp::cpp_int eS = 0;
    mp::cpp_int nS = 0;
    mp::cpp_int nonce = 0;


    //memset(&sin, 0, sizeof(sin));

#if defined __unix__ || defined __APPLE__
    //nothing to do here

#elif defined _WIN32
//********************************************************************
// WSSTARTUP
//********************************************************************

//********************************************************************
// WSSTARTUP
/*  All processes (applications or DLLs) that call Winsock functions must 
  initialize the use of the Windows Sockets DLL before making other Winsock 
  functions calls. 
  This also makes certain that Winsock is supported on the system.
*/
//********************************************************************
    int err;

    err = WSAStartup(WSVERS, &wsadata);
    if (err != 0) {
        WSACleanup();
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        exit(1);
    }


    if (USE_IPV6) {

        printf("\n=== IPv6 ===\n");
    } else { //IPV4

        printf("\n=== IPv4 ===\n");
    }

//********************************************************************
/* Confirm that the WinSock DLL supports 2.2.        */
/* Note that if the DLL supports versions greater    */
/* than 2.2 in addition to 2.2, it will still return */
/* 2.2 in wVersion since that is the version we      */
/* requested.                                        */
//********************************************************************
    printf("\n\n<<<TCP (CROSS-PLATFORM, IPv6-ready) CLIENT, by nhreyes>>>\n");

    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        exit(1);
    } else {

        printf("\nThe Winsock 2.2 dll was initialised.\n");
    }

#endif


//********************************************************************
// set the socket address structure.
//
//********************************************************************
    struct addrinfo *result = nullptr;
    struct addrinfo hints;
    int iResult;


//ZeroMemory(&hints, sizeof (hints)); //alternatively, for Windows only
    memset(&hints, 0, sizeof(struct addrinfo));


    if (USE_IPV6) {
        hints.ai_family = AF_INET6;
        printf("\n=== IPv6 ===\n");
    } else { //IPV4
        hints.ai_family = AF_INET;
        printf("\n=== IPv4 ===\n");
    }

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
//hints.ai_flags = AI_PASSIVE;// PASSIVE is only for a SERVER	

//*******************************************************************
//	Dealing with user's arguments
//*******************************************************************



    //if there are 3 parameters passed to the argv[] array.
    if (argc == 3) {
        //sin.sin_port = htons((u_short)atoi(argv[2])); //get Remote Port number
        snprintf(portNum, NI_MAXHOST, "%s", argv[2]);
        printf("\nUsing port: %s \n", portNum);
        iResult = getaddrinfo(argv[1], portNum, &hints, &result);
        //iResult = getaddrinfo("0:0:0:0:0:0:0:1", portNum, &hints, &result); //works! test only!
    } else {
        //sin.sin_port = htons(1234); //use default port number
        printf("USAGE: Client IP-address [port]\n"); //missing IP address
        snprintf(portNum, NI_MAXSERV, "%s", DEFAULT_PORT);
        printf("Default portNum = %s\n", portNum);
        if (USE_IPV6) {
            printf("Using default settings, IP:%s, Port:%s\n", DEFAULT_IPV6_ADDR, DEFAULT_PORT);
            iResult = getaddrinfo(DEFAULT_IPV6_ADDR, portNum, &hints, &result);
        } else {
            printf("Using default settings, IP:%s, Port:%s\n", DEFAULT_IPV4_ADDR, DEFAULT_PORT);
            iResult = getaddrinfo(DEFAULT_IPV4_ADDR, portNum, &hints, &result);
        }
    }

    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
#if defined _WIN32
        WSACleanup();
#endif
        return 1;
    }

//*******************************************************************
//CREATE CLIENT'S SOCKET 
//*******************************************************************

#if defined __unix__ || defined __APPLE__
    s = -1;
#elif defined _WIN32
    s = INVALID_SOCKET;
#endif

    //s = socket(PF_INET, SOCK_STREAM, 0);
    s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

#if defined __unix__ || defined __APPLE__
    if (s < 0) {
    printf("socket failed\n");
    freeaddrinfo(result);
    }
#elif defined _WIN32
    //check for errors in socket allocation
    if (s == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        exit(1);//return 1;
    }
#endif

    //sin.sin_family = AF_INET;
//~ //*******************************************************************
//~ //GETHOSTBYNAME
//~ //*******************************************************************
    //~ if ((h=gethostbyname(argv[1])) != NULL) {
    //~ memcpy(&sin.sin_addr,h->h_addr,h->h_length); //get remote IP address
    //~ } else if ((sin.sin_addr.s_addr = inet_addr(argv[1])) == INADDR_NONE) {
    //~ printf("An error occured when trying to translate to IP address\n");
    //~ WSACleanup();
    //~ exit(1);
    //~ }
//*******************************************************************
//CONNECT
//*******************************************************************
    //~ if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
    //~ printf("connect failed\n");
    //~ WSACleanup();
    //~ exit(1);
    //~ }

    if (connect(s, result->ai_addr, result->ai_addrlen) != 0) {
        printf("\nconnect failed\n");
        freeaddrinfo(result);
#if defined _WIN32
        WSACleanup();
#endif
        exit(1);
    } else {
        //~ printf("connected to server.\n");
        //~ struct sockaddr_in sa;
        //~ char ipstr[INET_ADDRSTRLEN];

        // store this IP address in sa:
        //inet_pton(AF_INET, result->ai_addr, &(sa.sin_addr));

        //-----------------------------------
        //~ void *addr;
        char ipver[80];

        // Get the pointer to the address itself, different fields in IPv4 and IPv6
        if (result->ai_family == AF_INET) {
            // IPv4
            //~ struct sockaddr_in *ipv4 = (struct sockaddr_in *)result->ai_addr;
            //~ addr = &(ipv4->sin_addr);
            strcpy(ipver, "IPv4");
        } else if (result->ai_family == AF_INET6) {
            // IPv6
            //~ struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)result->ai_addr;
            //~ addr = &(ipv6->sin6_addr);
            strcpy(ipver, "IPv6");
        }

        // printf("\nConnected to <<<SERVER>>> with IP address: %s, %s at port: %s\n", argv[1], ipver,portNum);

        //--------------------------------------------------------------------------------
        //getnameinfo() can be used to extract the IP address of the SERVER, in case a hostname was
        //              supplied by the user instead.

#if defined __unix__ || defined __APPLE__     
        int returnValue;
#elif defined _WIN32      
        DWORD returnValue;
#endif

        memset(serverHost, 0, sizeof(serverHost));
        memset(serverService, 0, sizeof(serverService));

        //int addrlen = sizeof (struct sockaddr); 
        // int addrlen = sizeof (*(result->ai_addr)); 

        returnValue = getnameinfo((struct sockaddr *) result->ai_addr, /*addrlen*/ result->ai_addrlen,
                                  serverHost, sizeof(serverHost),
                                  serverService, sizeof(serverService), NI_NUMERICHOST);

        // returnValue=getnameinfo((struct sockaddr *)result->ai_addr, /* sizeof(*(result->ai_addr)) */&addrlen,
        //              serverHost, sizeof(serverHost),
        //              serverService, sizeof(serverService),
        //            NI_NUMERICHOST);


        //~ getnameinfo(result->ai_addr, sizeof(*(result->ai_addr)),
        //~ serverHost, sizeof(serverHost),
        //~ serverService, sizeof(serverService),
        //~ NI_NAMEREQD); //works only if the DNS can resolve the hostname; otherwise, will result in an error

        if (returnValue != 0) {

#if defined __unix__ || defined __APPLE__     
            printf("\nError detected: getnameinfo() failed with error\n");
#elif defined _WIN32      
            printf("\nError detected: getnameinfo() failed with error#%d\n", WSAGetLastError());
#endif
            exit(1);

        } else {
            printf("\nConnected to <<<SERVER>>> extracted IP address: %s, %s at port: %s\n", serverHost,
                   ipver,/* serverService */ portNum);  //serverService is nfa
            //printf("\nConnected to <<<SERVER>>> extracted IP address: %s, at port: %s\n", serverHost, serverService);
        }
        //--------------------------------------------------------------------------------

    }

//*******************************************************************
// Key Exchange Session
//*******************************************************************

    // local variables for key exchange session
    char msg_type[40];
    char str_e[1024];
    char str_n[1024];

    // set to default values
    memset(&msg_type, 0, sizeof(msg_type));
    memset(&str_e, 0, sizeof(str_e));
    memset(&str_n, 0, sizeof(str_n));

    printf("\n--------------------------------------------\n");
    printf("the <<<CLIENT>>> is preparing to authenticate.\n");

    CryptoSystem cryptoSystem = CryptoSystem();

    // *** 1. Generate certificate authority keys - public only *** //

    cryptoSystem.generate_fixed_rsa_key(
            mp::cpp_int("194807849380634026909804860311046155297"),
            mp::cpp_int("280080400733839583120536697216595482831")
    );

    eCA = cryptoSystem.get_e();
    nCA = cryptoSystem.get_n();

    // *** 2. Get encrypted public server key from server *** //

    recvFromServer(s, &receive_buffer[0]);
    int scannedItems = std::sscanf(receive_buffer, "%s %s %s", msg_type, str_e, str_n);
    if (scannedItems < 3) {
        printf("Error: public key was not received from server\n");
    }
    eS = mp::cpp_int(str_e);
    nS = mp::cpp_int(str_n);
    std::cout << std::endl << "Received " << msg_type << " --> " << eS << " " << nS << std::endl;

    // send acknowledgement of public server key over to server
    snprintf(send_buffer, SBUFFER_SIZE, "ACK 226 Public server key received\r\n");
    send(s, send_buffer, (int) strlen(send_buffer), 0);
    std::cout << std::endl << "---> Sending reply to server: ACK 226 Public server key received" << std::endl;

    // decrypt_rsa public server key
    eS = cryptoSystem.decrypt_rsa(eS, eCA, nCA);
    nS = cryptoSystem.decrypt_rsa(nS, eCA, nCA);

    std::cout << std::endl << "---> Decrypted server public key: [" << eS << ", " << nS << "]" << std::endl;

    // *** 3. Send encrypted nonce to server *** //

    nonce = cryptoSystem.get_rand_num();
    mp::cpp_int eNonce = cryptoSystem.encrypt_rsa(nonce, eS, nS);

    snprintf(send_buffer, SBUFFER_SIZE, "NONCE %s\r\n", eNonce.str().c_str());
    send(s, send_buffer, (int) strlen(send_buffer), 0);

    std::cout << std::endl << "---> Sending encrypted nonce to server: NONCE " << eNonce << std::endl;

    recvFromServer(s, &receive_buffer[0]);
    std::cout << std::endl << "Received packet <---: " << receive_buffer << std::endl;
    scannedItems = std::sscanf(receive_buffer, "%s %*s", msg_type);
    if ((scannedItems < 1) || strcmp(msg_type, "ACK") != 0) {
        printf("Error: nonce was not ACKed by server\n");
    }
    std::cout << std::endl << "nonce ACKed by server" << std::endl;


//*******************************************************************
// Get input while user don't type "."
//*******************************************************************


    printf("\n--------------------------------------------\n");
    printf("you may now start sending commands to the <<<SERVER>>>\n");
    printf("\nType here:");
    memset(&send_buffer, 0, SBUFFER_SIZE);
    if (fgets(send_buffer, SEGMENT_SIZE, stdin) == nullptr) {
        printf("error using fgets()\n");
        exit(1);
    }

    while ((strncmp(send_buffer, ".", 1) != 0)) { // <--- Start of send/receive loop


        //*******************************************************************
        // ENCRYPT message before sending (RSA_CBC)
        //*******************************************************************

        // *** 1. Calculate how many blocks can be made and how many bytes remaining *** //

        int total_blocks = (int) strlen(send_buffer) / BLOCK_SIZE;

        int remaining = (int) strlen(send_buffer) % BLOCK_SIZE;

//        printf("DEBUG: Total length of string: %d\n", (int) strlen(send_buffer));
//        printf("DEBUG (Before padding): Total blocks: %d\n", total_blocks);
//        printf("DEBUG: Remaining: %d\n", remaining);

        // *** 2. Pad remaining bits with carriage return character *** //
        if ((remaining > 0) && (strlen(send_buffer) + (BLOCK_SIZE - remaining) < SBUFFER_SIZE)) {
            for (int i = 0; i < (BLOCK_SIZE - remaining); i++) {
                strcat(send_buffer, "\r");
            }
            total_blocks++;
        }


//        printf("DEBUG (After padding): Total blocks: %d\n", total_blocks);

        // *** 3. Split string into blocks of the right size *** //

        char blocks[total_blocks][BLOCK_SIZE + 1];

        for (int i = 0; i < total_blocks; i++) {
            memset(&blocks[i], 0, sizeof(blocks[i]));
        }

        int k = 0;
        for (int i = 0; i < total_blocks; i++) {
            memcpy(blocks[i], send_buffer + k, BLOCK_SIZE);
            k += BLOCK_SIZE;
        }

//        for (int i = 0; i < total_blocks; i++) {
//            printf("DEBUG: blocks[%d] = %s\n", i, blocks[i]);
//        }

        // *** 4. RSA CBC encryption

        char numToChar[64];

        memset(&numToChar, 0, 64);

        mp::cpp_int ciphertext[total_blocks];

        memset(&ciphertext, 0, sizeof(ciphertext));

        mp::cpp_int randNum = nonce;

        for (int i = 0; i < total_blocks; i++) {
            sprintf(numToChar, "%d%d%d", (int) blocks[i][0], (int) blocks[i][1], (int) blocks[i][2]);
            ciphertext[i] = cryptoSystem.encrypt_rsa_cbc(mp::cpp_int(numToChar), eS, nS, randNum);
            randNum = ciphertext[i];
        }

        // *** 4. load send buffer with ciphertext *** //

        // make a new send_buffer
        // need to make a new one because the original one used
        // for retrieving remaining message when message >= segment size
        char send_buffer_2[SBUFFER_SIZE];
        memset(send_buffer_2, 0, SBUFFER_SIZE);

        for (int i = 0; i < total_blocks; i++) {
            strcat(send_buffer_2, ciphertext[i].str().c_str());
            strcat(send_buffer_2, " ");
        }

        send_buffer_2[strlen(send_buffer_2) - 1] = '\0'; // replace last char with null terminating char

        strcat(send_buffer_2, "\r\n"); // concatenates "\r\n" with send_buffer and stores result in send_buffer

        //*******************************************************************
        // SEND
        //*******************************************************************

        bytes = send(s, send_buffer_2, (int) strlen(send_buffer_2), 0);
        printf("\nMSG SENT <--: %s\n", send_buffer_2);//line sent
        printf("Message length: %d \n", (int) strlen(send_buffer_2));


#if defined __unix__ || defined __APPLE__     
        if (bytes == -1) {
           printf("send failed\n");
           exit(1);
        }
#elif defined _WIN32      
        if (bytes == SOCKET_ERROR) {
            printf("send failed\n");
            WSACleanup();
            exit(1);
        }
#endif

        n = 0;

        //*******************************************************************
        // RECEIVE
        //*******************************************************************

        while (true) {
            bytes = recv(s, &receive_buffer[n], 1, 0);

#if defined __unix__ || defined __APPLE__  
            if ((bytes == -1) || (bytes == 0)) {
               printf("recv failed\n");
                exit(1);
            }

#elif defined _WIN32      
            if ((bytes == SOCKET_ERROR) || (bytes == 0)) {
                printf("recv failed\n");
                exit(1);
            }
#endif


            if (receive_buffer[n] == '\n') {  /*end on a LF*/
                receive_buffer[n] = '\0';
                break;
            }
            if (receive_buffer[n] != '\r') n++;   /*ignore CR's*/
        }

        printf("MSG RECEIVED --> %s\n", receive_buffer);

        // get another user input

        memset(&send_buffer, 0, SBUFFER_SIZE);
        printf("\nType here:");
        if (fgets(send_buffer, SEGMENT_SIZE, stdin) == nullptr) {
            printf("error using fgets()\n");
            exit(1);
        }


    } // <--- End of send/receive loop

    printf("\n--------------------------------------------\n");
    printf("<<<CLIENT>>> is shutting down...\n");

//*******************************************************************
//CLOSESOCKET   
//*******************************************************************
#if defined __unix__ || defined __APPLE__
    close(s);//close listening socket
#elif defined _WIN32
    closesocket(s);//close listening socket
    WSACleanup(); /* call WSACleanup when done using the Winsock dll */
#endif


    return 0;
}

