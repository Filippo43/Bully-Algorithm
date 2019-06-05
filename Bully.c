/*	Authors: Gabriel Filippo, Vinicius de Oliveira
/	Version: 1.0
/	Description: Implementação do Algoritmo de Bully para eleição entre
/			threads e do Algoritmo de Berkley para sincronização dos 
/			relógios utilizando mensagens em broadcast com protocolo UDP
*/

//IMPORTS
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in */
#include <string.h>     /* for memset() */

//DEFINES
#define THREADS_NUMBER 10
#define BROADCAST_IP "127.0.0.1"
#define BROADCAST_PORT "6000"
#define MAX_SLEEP 3
#define MIN_SLEEP 1
#define MAXRECVSTRING 255

//GLOBAIS
int sock; /* Socket */
struct sockaddr_in* broadcastAddr; /* Socket Structure*/
int broadcastPermission = 1;      /* Socket opt to set permission to broadcast */
int clock_ = 0;

/* External error handling function */
void DieWithError(char *errorMessage)
{
	printf("%s", errorMessage);
	exit(1);
}

//Send a message
void sendMessage(char* message)
{

	unsigned sendStringLen = strlen(message);  /* Find length of sendString */
  	/* Broadcast sendString in datagram to clients*/

     if (sendto(sock, message, sendStringLen, 0, (struct sockaddr *) &(*broadcastAddr), sizeof(*broadcastAddr)) != sendStringLen)
             DieWithError("sendto() sent a different number of bytes than expected");

     sleep(1);
}

//Receive a message
char* receiveMessage()
{
	char recvString[MAXRECVSTRING+1]; /* Buffer for received string */
    int recvStringLen;                /* Length of received string */

    /* Receive a single datagram from the server */
    if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
        DieWithError("recvfrom() failed");

    recvString[recvStringLen] = '\0';
    printf("Received: %s\n", recvString);    /* Print the received string */

    return recvString;
}

//Setup Sockets
void openSocket(char* ip, unsigned short port, struct sockaddr_in* broadcastAddr)
{

	//Header

    if (ip == NULL)
    {
    	printf("Erro de IP/Porta!\n");
    	exit(1);
    }

	 /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Set socket to allow broadcast */
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, 
          sizeof(broadcastPermission)) < 0)
        DieWithError("setsockopt() failed");

    /* Construct local address structure */
    memset(broadcastAddr, 0, sizeof(*broadcastAddr));   /* Zero out structure */
    broadcastAddr->sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr->sin_addr.s_addr = inet_addr(ip);/* Broadcast IP address */
    broadcastAddr->sin_port = htons(port);         /* Broadcast port */

     /* Bind to the broadcast port */
    if (bind(sock, (struct sockaddr *) &(*broadcastAddr), sizeof(*broadcastAddr)) < 0)
   		 DieWithError("bind() failed");
   
}

// The function to be executed by all threads 
void *myThreadFun(void *vargp) 
{ 
	//ID
	unsigned id = *(unsigned*)vargp;

	//Initial sleep
	sleep(1);

	//Clock
	int clock = clock_;
	//Define sleep time
	unsigned sleep_time = rand () % (MAX_SLEEP - MIN_SLEEP) + MIN_SLEEP;

	for (;;clock ++)
	{
		printf("Thread ID %d: time = %d\n", id ,clock);

		sendMessage ("Hello from " + id);
		//Increment

		printf("Received: %s\n", receiveMessage());

		sleep(sleep_time);
	}

}

void* localClock(void *vargp)
{
	for (;;clock_ ++)
	{
		sleep(MIN_SLEEP);
		printf("Relógio Local: %d\n", clock_);
	}
}

//Execution
int main(void) 
{ 
	//Header
	int i; 
	pthread_t tid [THREADS_NUMBER + 1]; 
	unsigned short sock_port = atoi(BROADCAST_PORT);
	broadcastAddr = (struct sockaddr_in*) malloc (sizeof(struct sockaddr_in));
	srand(time(0)); 

	//Abre o socket
	openSocket(BROADCAST_IP, sock_port, broadcastAddr);

	//Tread Relogio
	pthread_create(&tid[0], NULL, localClock, (void *)tid); 

	//Aguarda pelo início do programa
	printf("Relógio Local está ativo. Pressione qualquer tecla para Começar!\n");
	getchar();

	//Criação das threads
	for (i = 1; i < THREADS_NUMBER + 1; i++) 
		pthread_create(&tid[i], NULL, myThreadFun, (void *)&i); 

	pthread_exit(NULL); 


	//free(broadcastAddr);

	return 0; 
} 
