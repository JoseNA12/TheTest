#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Jugador.c"
#include "llavesComunicacion.c"

#define PUERTO 4444
#define DIRECCION_IP "127.0.0.1" 

int miIDUsuario = 0;

struct Jugador *jugadorActual;

/*
	Compilar con: gcc cliente.c -o cliente -lpython2.7
*/

int crearCliente() {

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];

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

	if (recv(clientSocket, buffer, 1024, 0) < 0) { 
		puts("[-]Error recibiendo información");
	}
	else {/*
		printf("%s", buffer);
		scanf("%s", &buffer[0]);
		send(clientSocket, buffer, strlen(buffer), 0);
		bzero(buffer, sizeof(buffer));
		recv(clientSocket, buffer, 1024, 0);

		printf("%s", buffer);
		scanf("%s", &buffer[0]);
		send(clientSocket, buffer, strlen(buffer), 0);
		bzero(buffer, sizeof(buffer));

		jugadorActual = (struct Jugador*)malloc(sizeof(struct Jugador));
		recv(clientSocket, jugadorActual, sizeof(struct Jugador)+1, 0);

		printf("Jugador obtenido %d \n",jugadorActual->idJugador);
		printf("Jugador obtenido %s \n",jugadorActual->nombreUsuario);
		printf("Jugador obtenido %s \n",jugadorActual->correo);
		*/
		while (1) {
			printf("%s", buffer);
			scanf("%s", &buffer[0]);
			send(clientSocket, buffer, strlen(buffer), 0);
			bzero(buffer, sizeof(buffer)); // Borrar buffer
			recv(clientSocket, buffer, 1024, 0); // Recibir respuesta Serv

			if (strcmp(buffer, SALIR) == 0) {
				close(clientSocket);
				puts("[-]Desconectado del servidor");
				exit(1);
			}
			else if (strcmp(buffer, LOGIN) == 0) {
				jugadorActual = (struct Jugador*)malloc(sizeof(struct Jugador));
				recv(clientSocket, jugadorActual, sizeof(struct Jugador)+1, 0);
				bzero(buffer, sizeof(buffer));
				// recibo el menu
				recv(clientSocket, buffer, sizeof(buffer), 0);
			}
			else if (strcmp(buffer, REGISTER) == 0) { // se ha solicitado registrar un jugador
				recv(clientSocket, buffer, 1024, 0);
				puts(buffer);
				bzero(buffer, sizeof(buffer));
				// recibo el menu
				recv(clientSocket, buffer, 1024, 0);
			}
			/*else if (strcmp(buffer, INICIAR_PARTIDA) == 0) { // se ha solicitado registrar un jugador
				bzero(buffer, sizeof(buffer));

				recv(clientSocket, buffer, sizeof(buffer), 0);
				
				printf("%s", buffer); // ¿Deseo continuar?
				scanf("%s", &buffer[0]); 
				
				send(clientSocket, buffer, strlen(buffer), 0);
				bzero(buffer, sizeof(buffer));
				send(clientSocket, jugadorActual->nombreUsuario, strlen(jugadorActual->nombreUsuario), 0);

				recv(clientSocket, buffer, 1024, 0); // respuesta de insersion
				puts(buffer);

				bzero(buffer, sizeof(buffer));
				// recibo el mensaje de insersion
				recv(clientSocket, buffer, 1024, 0);

			}*/
		}
	}

	/*while (1) {
		printf("Cliente: \t");
		scanf("%s", &buffer[0]);
		send(clientSocket, buffer, strlen(buffer), 0);

		if (strcmp(buffer, ":exit") == 0) {
			close(clientSocket);
			puts("[-]Desconectado del servidor");
			exit(1);
		}

		if (recv(clientSocket, buffer, 1024, 0) < 0) {
			puts("[-]Error recibiendo información");
		}
		else {
			printf("Servidor: \t%s\n", buffer);
		}
	}*/

}

void main() {
	crearCliente();
}