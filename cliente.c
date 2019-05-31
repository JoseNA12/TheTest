#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Jugador.c"
#include "Pregunta.c"
#include "llavesComunicacion.c"

#define PUERTO 4444
#define DIRECCION_IP "127.0.0.1" 


#define SIZE_BUFF 2000

struct Jugador *jugadorActual;

/*
	Compilar con: gcc cliente.c -o cliente
*/

int crearCliente() {
	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[SIZE_BUFF];
	struct Pregunta *preguntaActual;

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0) {
		puts("[-]Error en conexión");
		exit(1);
	}
	puts("[+]Cliente Socket creado!");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PUERTO);
	serverAddr.sin_addr.s_addr = inet_addr(DIRECCION_IP);

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (ret < 0) {
		puts("[-]Error en connect");
		exit(1);
	}
	puts("[+]Conectado al servidor");

	// le digo al servidor que soy JUGADOR
	send(clientSocket, JUGADOR, strlen(JUGADOR), 0);
	bzero(buffer, sizeof(buffer));

	if (recv(clientSocket, buffer, SIZE_BUFF, 0) < 0) { 
		puts("[-]Error recibiendo información");
	}
	else {
		while (1) {
			printf("%s", buffer);
			scanf("%s", &buffer[0]);
			send(clientSocket, buffer, strlen(buffer), 0);
			bzero(buffer, sizeof(buffer)); // Borrar buffer
			l_brinco_magico:
			recv(clientSocket, buffer, SIZE_BUFF, 0); // Recibir respuesta Serv

			if (strcmp(buffer, SALIR) == 0) {
				close(clientSocket);
				puts("[-]Desconectado del servidor");
				exit(1);
			}
			else if (strcmp(buffer, LOGIN) == 0) {
				send(clientSocket, "ok", strlen("ok"), 0);
				jugadorActual = (struct Jugador*)malloc(sizeof(struct Jugador));
				recv(clientSocket, jugadorActual, sizeof(struct Jugador)+1, 0);
				send(clientSocket, "ok", strlen("ok"), 0);
				bzero(buffer, sizeof(buffer));
				// recibo el menu
				recv(clientSocket, buffer, sizeof(buffer), 0);
			}
			else if (strncmp(buffer, REGISTER, strlen(REGISTER)) == 0) { // se ha solicitado registrar un jugador
				send(clientSocket, "ok", strlen("ok"), 0);
				recv(clientSocket, buffer, SIZE_BUFF, 0);
				puts(buffer);
				bzero(buffer, sizeof(buffer));
				// recibo el menu
				recv(clientSocket, buffer, SIZE_BUFF, 0);
			}
			else if (strncmp(buffer, SEND_PREGUNTA, strlen(SEND_PREGUNTA)) == 0) { // se ha solicitado registrar un jugador
				send(clientSocket, "ok", strlen("ok"), 0);
				bzero(buffer, sizeof(buffer));
				recv(clientSocket, buffer, sizeof(buffer), 0);
			}
			else if (strncmp(buffer, RECEIVE_OPCIONES, strlen(RECEIVE_OPCIONES)) == 0) { // se ha solicitado registrar un jugador
				send(clientSocket, "ok", strlen("ok"), 0);
				bzero(buffer, sizeof(buffer));
				recv(clientSocket, buffer, sizeof(buffer), 0);
				puts(buffer);
				bzero(buffer, sizeof(buffer));
				goto l_brinco_magico;
			}
		}
	}
}

void main() {
	crearCliente();
}