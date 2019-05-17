#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "llavesComunicacion.c"
#include "sql_handler.h"
//#include <sqlite3.h>
#include "Colores.c"

// Estructuras
#include "Pregunta.c"
#include "Jugador.c"
#include "Partida.c"

/*	
Instalar paquetes Python Dev:
	sudo apt-get install python-pip python-dev build-essential 
	sudo pip install --upgrade pip 
	sudo pip install --upgrade virtualenv
slqlite3:
	sudo apt install sqlite3
	sudo apt-get install libsqlite3-dev
*/

// Servidor/cliente basado en: 
// -> https://github.com/nikhilroxtomar/Multiple-Client-Server-Program-in-C-using-fork

// Compilar con: gcc servidor.c -o servidor -lpython2.7
// 				 gcc servidor.c -o servidor -lsqlite3
	

#define PUERTO 4444
#define DIRECCION_IP "127.0.0.1"

#define LOGIN_USER "\n[The Test] -> Usuario: "
#define LOGIN_PASS "[The Test] -> Contraseña: "
#define LOGIN_EMAIL "[The Test] -> Correo: "
#define QUESTION_REGISTER_USER "\nDesea continuar?\n[1] - Si\n[2] - No\n>>> "
#define USER_REQUEST "\n\nIngrese el nombre del jugador para jugar\n>>> "
#define MSJ_REGISTER_USER_1 "\nSe ha registrado el usuario!\n"
#define MSJ_REGISTER_USER_2 "\nEl nombre de usuario indicado se encuentra en uso!\n"
#define MSJ_REGISTER_USER_3 "\nSe ha producido un error al registrar el jugador!\n"
#define MSJ_REGISTER_USER_4 "\nSe ha cancelado el registro del nuevo usuario!\n"

char *menuInicio_c = "\n\t\t" Bold_Blue " The Test - Inicio \n" CYAN"[1]"Reset_Color" - Iniciar sesión\n" CYAN"[2]"Reset_Color" - Registrarse\n" CYAN"[3]"Reset_Color" - Salir\n>>> ";
char *menuJugador_c = "\n\t\t" Bold_Blue " The Test - Menú \n" CYAN"[1]"Reset_Color" - Seleccionar jugador\n" CYAN"[2]"Reset_Color" - Estado del servidor\n" CYAN"[3]"Reset_Color" - Cerrar sesión\n>>> ";
int newSocket;
char buffer[1024];

char *menuInicio() {
	char *buffer = malloc(160);
	strcat(buffer, "\n========================" Bold_Blue " The Test - Inicio " Reset_Color "========================\n");
	strcat(buffer, CYAN"[1]"Reset_Color" - Iniciar sesión\n");
	strcat(buffer, CYAN"[2]"Reset_Color" - Registrarse\n");
	strcat(buffer, CYAN"[3]"Reset_Color" - Salir\n>>> ");
	return buffer;
}

char *menuJugador() {
	char *buffer = malloc(160);
	strcat(buffer, "\n========================" Bold_Blue " The Test - Menú " Reset_Color "========================\n");
	strcat(buffer, CYAN"[1]"Reset_Color" - Seleccionar jugador\n");
	strcat(buffer, CYAN"[2]"Reset_Color" - Estado del servidor\n");
	strcat(buffer, CYAN"[3]"Reset_Color" - Cerrar sesión\n>>> ");
	return buffer;
}

struct Jugador* enviar_iniciarSesion(char* username, char* password){
	struct Jugador *jugadorNuevo;
	jugadorNuevo = iniciarSesion(username,password);
	/*if(jugadorNuevo->idJugador == 0){
		return NULL;
	}*/

	return jugadorNuevo;
}

struct Partida* set_Partida(char* jugador1, char* jugador2){
	// Si ya hay una partida entre los 2 jugadores, retorna la misma. Sino crea una nueva
	struct Partida *partida;
	partida = iniciarPartida(jugador1,jugador2);
	printf("Partida creada %d \n",partida->idPartida);
	return partida;
	// Enviar el idPartida para obtener una pregunta y sus opciones para los jugadores
}


void crearServidor() {
	int sockfd, ret;
	struct sockaddr_in serverAddr;

	/*int newSocket;
	char buffer[1024];*/

	struct sockaddr_in newAddr;

	socklen_t addr_size;

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
		
		printf("Conexión aceptada de " Bold_Yellow "%s:%d\n" Reset_Color, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
		
		if ((childpid = fork()) == 0) {
			close(sockfd);

			recv(newSocket, buffer, 1024, 0); // apenas se conecta un cliente, recibo el tipo de usuario que es (jugador o de mantenimiento)
			char tipo_usuario[24]; 
			strcpy(tipo_usuario, buffer);
			char usuario_login[50];
			char pass_login[50];
			char email_user[50];
			//char *menuInicio_= menuInicio();
			//char *menuJugador_ = menuJugador();

			if (initConnection() != 0) { // abrir la conexion con el DB
				puts("Error al establecer conexión con la base de datos");
				break;
			}

			l_menu_inicio:
				send(newSocket, menuInicio_c, 165, 0); // mostrar menu inicio
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 1024, 0); // lo recibo

				if (strcmp(buffer, "1") == 0) {
					goto l_login_jugador;
				}
				else if (strcmp(buffer, "2") == 0) { // registrar usuario
					goto l_registrar_jugador;
				}
				else if (strcmp(buffer, "3") == 0) {
					send(newSocket, SALIR, 1024, 0);
					endConnection(); // cerrar comunicacion con la BD
					printf("Desconexión de " Bold_Red "%s:%d\n" Reset_Color, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					goto l_cerrar_conexion;
				}
				else {
					goto l_menu_inicio;
				}

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
				
				struct Jugador *jugadorNuevo;
				jugadorNuevo = enviar_iniciarSesion(usuario_login,pass_login);
				//send(newSocket, jugadorNuevo, sizeof(struct Jugador)+1, 0); 
				
				if (jugadorNuevo->idJugador > 0) {

					if (strcmp(tipo_usuario, JUGADOR) == 0) {
						
						send(newSocket, LOGIN, 1024, 0); // le envio su ID al cliente
						//send(newSocket, &resp, sizeof(resp), 0);
						send(newSocket, jugadorNuevo, sizeof(struct Jugador)+1, 0); 
						
						while (1) {
							send(newSocket, menuJugador_c, 1024, 0);
							bzero(buffer, sizeof(buffer));
							recv(newSocket, buffer, 1024, 0); // recibo alguna opcion del menu

							if (strcmp(buffer, "1") == 0) {
								struct Partida *partida;
								char *consulta = usuariosRegistrados(usuario_login);
								strcat(consulta, USER_REQUEST);
								send(newSocket, consulta, 1024, 0);
								bzero(buffer, sizeof(buffer));	
								recv(newSocket, buffer, 1024, 0); // recibo alguna opcion del menu*/
								partida = set_Partida(usuario_login,buffer);
								send(newSocket, "La partida ha sido creada", 1024, 0);								
							}
							else if (strcmp(buffer, "2") == 0) {
								send(newSocket, "Estado del servidor: jugadores en juego activo y estadísticas tales como : usuarios, número de preguntas, número de usos, aciertos y fallos totales del sistema y por cada pregunta, puntajes entre todos los pares de usuarios en juego como un ranking.", 1024, 0);
								// estado del servidor
								bzero(buffer, sizeof(buffer));	
								recv(newSocket, buffer, 1024, 0); // recibo alguna opcion del menu
								
							}
							else if (strcmp(buffer, "3") == 0) { // cerrar sesion
								// endConnection();
								goto l_menu_inicio;
							}
						}
					}
					else if (strcmp(tipo_usuario, MANTENIMIENTO) == 0) {

					}
				}
				else {
					bzero(usuario_login, sizeof(usuario_login));
					bzero(pass_login, sizeof(pass_login));
					goto l_menu_inicio;
				}

			l_registrar_jugador:
				send(newSocket, LOGIN_USER, strlen(LOGIN_USER), 0); // pido que ingrese el usuario
				
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 1024, 0); // lo recibo
				strcpy(usuario_login, buffer); // guardo el nombre de usuario

				send(newSocket, LOGIN_PASS, strlen(LOGIN_PASS), 0); // pido que ingrese el usuario
				
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 1024, 0); // lo recibo
				strcpy(pass_login, buffer); // guardo la contraseña

				send(newSocket, LOGIN_EMAIL, strlen(LOGIN_EMAIL), 0); // pido que ingrese el usuario
				
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 1024, 0); // lo recibo
				strcpy(email_user, buffer); // guardo el correo

				//send(newSocket, REGISTER, strlen(REGISTER), 0); // envio llave de registrar 
				send(newSocket, QUESTION_REGISTER_USER, strlen(QUESTION_REGISTER_USER), 0); // envio mensaje de confirmación
				bzero(buffer, sizeof(buffer));				
				recv(newSocket, buffer, 1024, 0); // lo recibo
				
				if (strcmp(buffer, "1") == 0) {
					
					send(newSocket, REGISTER, strlen(REGISTER), 0); // envio llave de registrar 
					int resp = registrarUsuario(usuario_login, pass_login, email_user);
					
					if (resp == 1) { // registro completado
						send(newSocket, Bold_Yellow MSJ_REGISTER_USER_1 Reset_Color, strlen(MSJ_REGISTER_USER_1) + 14, 0); // +14 de los colores
						printf("Se ha registrado un nuevo usuario: " Bold_Yellow "%s\n" Reset_Color, usuario_login);
					}
					else if (resp == 0) { // usuario ingresado en uso
						send(newSocket, Bold_Red MSJ_REGISTER_USER_2 Reset_Color, strlen(MSJ_REGISTER_USER_2) + 13, 0);
					}
					else { // error
						send(newSocket, Bold_Red MSJ_REGISTER_USER_3 Reset_Color, strlen(MSJ_REGISTER_USER_3) + 13, 0);
					}
				}
				else {
					//send(newSocket, Bold_Red MSJ_REGISTER_USER_4 Reset_Color, strlen(MSJ_REGISTER_USER_3) + 13, 0);					
				}
				bzero(buffer, sizeof(buffer));
				goto l_menu_inicio;


		/*	t_iniciar_partida:
				send(newSocket, INICIAR_PARTIDA, strlen(INICIAR_PARTIDA), 0); // pido que ingrese el usuario
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 1024, 0); // lo recibo
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 1024, 0); // lo recibo

				strcpy(usuario_login, buffer); // guardo el nombre de usuario

				send(newSocket, strcat(usuariosRegistrados(usuario_login), USER_REQUEST), 1024, 0);
				bzero(buffer, sizeof(buffer));	
				recv(newSocket, buffer, 1024, 0); // recibo alguna opcion del menu
				iniciarPartida();*/
			

			/*t_iniciar_partida:
				char *consulta = usuariosRegistrados(usuario_login);
				strcat(consulta, USER_REQUEST);
				send(newSocket, consulta, 1024, 0);
				bzero(buffer, sizeof(buffer));	
				recv(newSocket, buffer, 1024, 0); // recibo alguna opcion del menu*/
				

		}
	}
	l_cerrar_conexion:
	close(newSocket);	
}


int main() {

	/*struct Pregunta *preguntaNueva;
	preguntaNueva = getPregunta();
	printf("Struct preguntaNueva  %s - %d \n",preguntaNueva->enunciado, preguntaNueva->puntaje);*/
	/*struct Jugador *jugador;
	jugador = enviar_iniciarSesion("xd","123");*/

	/*struct Partida *partida;
	partida = iniciarPartida(2,3);
	printf("Servidor %d \n",partida->idPartida);
    printf("Servidor %d \n",partida->idJugador1);
    printf("Servidor %d \n",partida->idJugador2);
    printf("Servidor %d \n",partida->nivel);
    printf("Servidor %d \n",partida->puntaje);*/

	crearServidor();

	return 0;
}