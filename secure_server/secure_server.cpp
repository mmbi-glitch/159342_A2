//////////////////////////////////////////////////////////////
// TCP SECURE SERVER GCC (IPV6 ready)
//
//
// References: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
//             http://long.ccaba.upc.edu/long/045Guidelines/eva/ipv6.html#daytimeServer6
//
//////////////////////////////////////////////////////////////


#define USE_IPV6 true  //if set to false, IPv4 addressing scheme will be used; you need to set this to true to
//enable IPv6 later on.  The assignment will be marked using IPv6!

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
#include <iostream>
#include <boost/multiprecision/cpp_int.hpp>

#elif defined __WIN32__

#include <winsock2.h>
#include <ws2tcpip.h> //required by getaddrinfo() and special constants
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <boost/multiprecision/cpp_int.hpp>

#define WSVERS MAKEWORD(2,2) /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
//The high-order byte specifies the minor version number;
//the low-order byte specifies the major version number.
WSADATA wsadata; //Create a WSADATA object called wsadata.
#endif


#define DEFAULT_PORT "1234"

#define BUFFER_SIZE 500
#define RBUFFER_SIZE 256

using namespace std;
/////////////////////////////////////////////////////////////////////

int recvFromClient(SOCKET cs, char *recv_buffer) {
    int bytes;
    int n = 0;
    while (true) {
        bytes = recv(cs, &recv_buffer[n], 1, 0);

        if ((bytes < 0) || (bytes == 0)) break;

        if (recv_buffer[n] == '\n') { /*end on a LF, Note: LF is equal to one character*/
            recv_buffer[n] = '\0';
            break;
        }
        if (recv_buffer[n] != '\r') n++; /*ignore CRs*/
    }
    return bytes;
}

void printBuffer(const char *header, char *buffer) {
    cout << "------" << header << "------" << endl;
    for (unsigned int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == '\r') {
            cout << "buffer[" << i << "]=\\r" << endl;
        } else if (buffer[i] == '\n') {
            cout << "buffer[" << i << "]=\\n" << endl;
        } else {
            cout << "buffer[" << i << "]=" << buffer[i] << endl;
        }
    }
    cout << "---" << endl;
}

/////////////////////////////////////////////////////////////////////

//*******************************************************************
//MAIN
//*******************************************************************
int main(int argc, char *argv[]) {

//********************************************************************
// INITIALIZATION of the SOCKET library
//********************************************************************

    struct sockaddr_storage clientAddress; //IPV6

    char clientHost[NI_MAXHOST];
    char clientService[NI_MAXSERV];


    char send_buffer[BUFFER_SIZE], receive_buffer[RBUFFER_SIZE];
    int n, bytes, addrlen;
    char portNum[NI_MAXSERV];
    // char username[80];
    // char passwd[80];

    //memset(&localaddr,0,sizeof(localaddr));


#if defined __unix__ || defined __APPLE__
    int s,ns;

#elif defined _WIN32

    SOCKET s, ns;

//********************************************************************
// WSSTARTUP
/*	All processes (applications or DLLs) that call Winsock functions must 
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


//********************************************************************
/* Confirm that the WinSock DLL supports 2.2.        */
/* Note that if the DLL supports versions greater    */
/* than 2.2 in addition to 2.2, it will still return */
/* 2.2 in wVersion since that is the version we      */
/* requested.                                        */
//********************************************************************

    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        exit(1);
    } else {
        printf("\n\n<<<TCP SERVER>>>\n");
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


//********************************************************************
// STEP#0 - Specify server address information and socket properties
//********************************************************************


//ZeroMemory(&hints, sizeof (hints)); //alternatively, for Windows only
    memset(&hints, 0, sizeof(struct addrinfo));

    if (USE_IPV6) {
        hints.ai_family = AF_INET6;
    } else { //IPV4
        hints.ai_family = AF_INET;
    }

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE; // For wildcard IP address
    //setting the AI_PASSIVE flag indicates the caller intends to use
    //the returned socket address structure in a call to the bind function.

// Resolve the local address and port to be used by the server
    if (argc == 2) {
        iResult = getaddrinfo(nullptr, argv[1], &hints,
                              &result); //converts human-readable text strings representing hostnames or IP addresses
        //into a dynamically allocated linked list of struct addrinfo structures
        //IPV4 & IPV6-compliant
        sprintf(portNum, "%s", argv[1]);
        printf("\nargv[1] = %s\n", argv[1]);
    } else {
        iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints,
                              &result); //converts human-readable text strings representing hostnames or IP addresses
        //into a dynamically allocated linked list of struct addrinfo structures
        //IPV4 & IPV6-compliant
        sprintf(portNum, "%s", DEFAULT_PORT);
        printf("\nUsing DEFAULT_PORT = %s\n", portNum);
    }

#if defined __unix__ || defined __APPLE__

    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);

        return 1;
    }
#elif defined _WIN32

    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);

        WSACleanup();
        return 1;
    }
#endif

//********************************************************************
// STEP#1 - Create welcome SOCKET
//********************************************************************

#if defined __unix__ || defined __APPLE__
    s = -1;
#elif defined _WIN32
    s = INVALID_SOCKET; //socket for listening
#endif
// Create a SOCKET for the server to listen for client connections

    s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

#if defined __unix__ || defined __APPLE__

    if (s < 0) {
        printf("Error at socket()");
        freeaddrinfo(result);
        exit(1);//return 1;
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
//********************************************************************


//********************************************************************
//STEP#2 - BIND the welcome socket
//********************************************************************

// bind the TCP welcome socket to the local address of the machine and port number
    iResult = bind(s, result->ai_addr, (int) result->ai_addrlen);

#if defined __unix__ || defined __APPLE__ 
    if (iResult != 0) {
        printf("bind failed with error");
        freeaddrinfo(result);

        close(s);
        
        return 1;
    }

#elif defined _WIN32 

    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);

        closesocket(s);
        WSACleanup();
        return 1;
    }
#endif

    freeaddrinfo(result); //free the memory allocated by the getaddrinfo
    //function for the server's address, as it is
    //no longer needed
//********************************************************************

/*
   if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) == SOCKET_ERROR) {
      printf("Bind failed!\n");
   }
*/

//********************************************************************
//STEP#3 - LISTEN on welcome socket for any incoming connection
//********************************************************************
#if defined __unix__ || defined __APPLE__ 
    if (listen( s, SOMAXCONN ) < 0 ) {
     printf( "Listen failed with error\n");
     close(s);
     
     exit(1);
   }

#elif defined _WIN32 	 
    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        exit(1);
    }
#endif

//*******************************************************************
//INFINITE LOOP
//********************************************************************
    while (true) {  //main loop
        printf("\n<<<SERVER>>> is listening at PORT: %s\n", portNum);
        addrlen = sizeof(clientAddress); //IPv4 & IPv6-compliant

//********************************************************************
//NEW SOCKET newsocket = accept
//********************************************************************
#if defined __unix__ || defined __APPLE__ 
        ns = -1;
#elif defined _WIN32       
        ns = INVALID_SOCKET;
#endif

        //Accept a client socket
        //ns = accept(s, NULL, NULL);

//********************************************************************	
// STEP#4 - Accept a client connection.  
//	accept() blocks the iteration, and causes the program to wait.  
//	Once an incoming client is detected, it returns a new socket ns
// exclusively for the client.  
// It also extracts the client's IP address and Port number and stores
// it in a structure.
//********************************************************************

#if defined __unix__ || defined __APPLE__ 
        ns = accept(s,(struct sockaddr *)(&clientAddress),(socklen_t*)&addrlen); //IPV4 & IPV6-compliant

        if (ns < 0) {
             printf("accept failed\n");
             close(s);

             return 1;
        }
#elif defined _WIN32 
        ns = accept(s, (struct sockaddr *) (&clientAddress), &addrlen); //IPV4 & IPV6-compliant
        if (ns == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(s);
            WSACleanup();
            return 1;
        }
#endif


        printf("\nA <<<CLIENT>>> has been accepted.\n");


        memset(clientHost, 0, sizeof(clientHost));
        memset(clientService, 0, sizeof(clientService));
        getnameinfo((struct sockaddr *) &clientAddress, addrlen,
                    clientHost, sizeof(clientHost),
                    clientService, sizeof(clientService),
                    NI_NUMERICHOST);

        printf("\nConnected to <<<Client>>> with IP address:%s, at Port:%s\n", clientHost, clientService);


//********************************************************************		
//Communicate with the Client
//********************************************************************

//*******************************************************************
// Key Exchange Session
//*******************************************************************

        printf("\n--------------------------------------------\n");
        printf("the <<<SERVER>>> is preparing to authenticate.\n\n");

        int eCA = 10000;
        int nCA = 76902;
        int dCA = 120202;
        int eServer = 1341;
        int nServer = 1342;

        // 1. Send public certificate authority key.

        sprintf(send_buffer, "PUBLIC_CA_KEY %d %d\r\n", eCA, nCA);
        bytes = send(ns, send_buffer, (int) strlen(send_buffer), 0);
        printf("Sending packet: --> %s\n", send_buffer);
        if (bytes < 0) break;

        bytes = recvFromClient(ns, &receive_buffer[0]);
        if (bytes < 0) break;
        printf("MSG RECEIVED <--: %s\n", receive_buffer);

        // 2. Send public server key.

        sprintf(send_buffer, "PUBLIC_SERVER_KEY %d %d\r\n", eServer, nServer);
        bytes = send(ns, send_buffer, (int) strlen(send_buffer), 0);
        printf("Sending packet: --> %s\n", send_buffer);
        if (bytes < 0) break;

        bytes = recvFromClient(ns, &receive_buffer[0]);
        if (bytes < 0) break;
        printf("MSG RECEIVED <--: %s\n", receive_buffer);

        bytes = recvFromClient(ns, &receive_buffer[0]);
        if (bytes < 0) break;
        printf("MSG RECEIVED <--: %s\n", receive_buffer);


        printf("\n--------------------------------------------\n");
        printf("the <<<SERVER>>> is waiting to receive messages.\n");

        while (true) {
            n = 0;
//********************************************************************
//RECEIVE one command (delimited by \r\n)
//********************************************************************
            while (true) {
                bytes = recv(ns, &receive_buffer[n], 1, 0);

                if ((bytes < 0) || (bytes == 0)) break;

                if (receive_buffer[n] == '\n') { /*end on a LF, Note: LF is equal to one character*/
                    receive_buffer[n] = '\0';
                    break;
                }
                if (receive_buffer[n] != '\r') n++; /*ignore CRs*/
            }

            if ((bytes < 0) || (bytes == 0)) break;
            sprintf(send_buffer, "Message:'%s' - There are %d bytes of information\r\n", receive_buffer, n);

//********************************************************************
//PROCESS REQUEST
//********************************************************************			
            printf("MSG RECEIVED <--: %s\n", receive_buffer);
            //printBuffer("RECEIVE_BUFFER", receive_buffer);

//********************************************************************
//SEND
//********************************************************************         
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
            printf("MSG SENT --> %s\n", send_buffer);
            //printBuffer("SEND_BUFFER", send_buffer);

#if defined __unix__ || defined __APPLE__ 
            if (bytes < 0) break;
#elif defined _WIN32 		 
            if (bytes == SOCKET_ERROR) break;
#endif

        }
//********************************************************************
//CLOSE SOCKET
//********************************************************************


#if defined __unix__ || defined __APPLE__ 
        int iResult = shutdown(ns, SHUT_WR);
        if (iResult < 0) {
           printf("shutdown failed with error\n");
           close(ns);

           exit(1);
        }
        close(ns);

#elif defined _WIN32 
        iResult = shutdown(ns, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ns);
            WSACleanup();
            exit(1);
        }

        closesocket(ns);
#endif
//***********************************************************************


        printf("\ndisconnected from << Client >> with IP address:%s, Port:%s\n", clientHost, clientService);
        printf("=============================================");

    } //main loop
//***********************************************************************
#if defined __unix__ || defined __APPLE__ 
    close(s);
#elif defined _WIN32 
    closesocket(s);
    WSACleanup(); /* call WSACleanup when done using the Winsock dll */
#endif

    return 0;
}


