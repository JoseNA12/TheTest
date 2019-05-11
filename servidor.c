#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "python_handler.c"
#include "preguntaStruct.c"

#include "llavesComunicacion.c"

/*	
Instalar paquetes Python Dev:
	sudo apt-get install python-pip python-dev build-essential 
	sudo pip install --upgrade pip 
	sudo pip install --upgrade virtualenv 
*/

// Servidor/cliente basado en: 
// -> https://github.com/nikhilroxtomar/Multiple-Client-Server-Program-in-C-using-fork

// Compilar con: gcc servidor.c -o servidor -lpython2.7

#define PUERTO 4444
#define DIRECCION_IP "127.0.0.1"


char *menuJugador() {
	char *buffer = (char *) malloc(160);
	strcat(buffer, "\n======================== The Test ========================\n");
	strcat(buffer, "[1] - Seleccionar jugador\n");
	strcat(buffer, "[2] - Estado del servidor\n");
	strcat(buffer, "[3] - Salir\n>>> ");
	return buffer;
}

void crearServidor() {
	int sockfd, ret;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		puts("[-]Error en conexión");
		exit(1);
	}
	puts("[+]Servidor Socket creado!");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PUERTO);
	serverAddr.sin_addr.s_addr = inet_addr(DIRECCION_IP);

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (ret < 0) {
		puts("[-]Error en bind");
		exit(1);
	}
	printf("[+]Puerto enlazado %d\n", PUERTO);

	if (listen(sockfd, 10) == 0) {
		puts("[+]Escuchando peticiones...");
	}
	else {
		puts("[-]Error al enlazar el puerto");
	}

	while (1) {
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if (newSocket < 0) {
			exit(1);
		}
		printf("Conexión aceptada de %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if ((childpid = fork()) == 0) {
			close(sockfd);

			recv(newSocket, buffer, 1024, 0); // apenas se conecta un cliente, recibo el tipo de usuario que es (jugador o de mantenimiento)
			char tipo_usuario[24]; 
			strcpy(tipo_usuario, buffer);
			char usuario_login[50];
			char pass_login[50];
			
			l_login_jugador:
				
				send(newSocket, LOGIN_USER, strlen(LOGIN_USER), 0); // pido que ingrese el usuario

				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 1024, 0); // lo recibo

				strcpy(usuario_login, buffer); // guardo el nombre de usuario

				bzero(buffer, sizeof(buffer));
				send(newSocket, LOGIN_PASS, strlen(LOGIN_PASS), 0); // pido que ingrese la contraseña
				
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 1024, 0); // lo recibo

				strcpy(pass_login, buffer); // guardo la contraseña
				bzero(buffer, sizeof(buffer));

				// llamar a funcion python para verificar credenciales

				if (1) {
					if (strcmp(tipo_usuario, JUGADOR) == 0) {
						while (1) {
							send(newSocket, menuJugador(), 1024, 0);
							bzero(buffer, sizeof(buffer));
							recv(newSocket, buffer, 1024, 0); // recibo alguna opcion del menu

							if (strcmp(buffer, "1") == 0) {
								send(newSocket, "resultado con los jugadores registrados", 1024, 0);
								// llamar funcion python para seleccionar jugadores registrados (quitando los de sesiones activas)
								bzero(buffer, sizeof(buffer));	
								recv(newSocket, buffer, 1024, 0); // recibo alguna opcion del menu
								
							}
							else if (strcmp(buffer, "2") == 0) {
								send(newSocket, "Estado del servidor: jugadores en juego activo y estadísticas tales como : usuarios, número de preguntas, número de usos, aciertos y fallos totales del sistema y por cada pregunta, puntajes entre todos los pares de usuarios en juego como un ranking.", 1024, 0);
								// estado del servidor
								bzero(buffer, sizeof(buffer));	
								recv(newSocket, buffer, 1024, 0); // recibo alguna opcion del menu
								
							}
							else if (strcmp(buffer, "3") == 0) { // salir
								send(newSocket, SALIR, 1024, 0);
								printf("Desconexión de %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
								goto cerrar_conexion;
							}
						}
					}
					else if (strcmp(tipo_usuario, MANTENIMIENTO) == 0) {

					}
				}
				else {
					bzero(usuario_login, sizeof(usuario_login));
					bzero(pass_login, sizeof(pass_login));
					goto l_login_jugador;
				}
		}

	}
	cerrar_conexion:
	close(newSocket);	
}

void cargarPreguntas(){
    PyObject *result = llamarFuncion("selectPreguntas"); 
	const char* retval = (char*)PyString_AsString(result); 
    printf("String obtenido %s \n",retval);
	Py_DECREF(result);;
}

int main() {
    
	/*initPython();
	cargarPreguntas(); // Indicar ID de función
	terminarPython();*/

	crearServidor();
	
	return 0;
}