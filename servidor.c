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

// sudo ufw allow 3001
	

#define PUERTO 4444
#define DIRECCION_IP "127.0.0.1"
#define SIZE_BUFF 1024

#define LOGIN_USER "\n[The Test] -> Usuario: "
#define LOGIN_PASS "[The Test] -> Contraseña: "
#define LOGIN_EMAIL "[The Test] -> Correo: "
#define QUESTION_REGISTER_USER "\nDesea continuar?\n[1] - Si\n[2] - No\n>>> "
#define USER_REQUEST "\nIngrese el nombre del jugador para jugar\n>>> "
#define MSJ_REGISTER_USER_1 Bold_Green "\nSe ha registrado el usuario!" Reset_Color
#define MSJ_REGISTER_USER_2 "\nEl nombre de usuario indicado se encuentra en uso!\n"
#define MSJ_REGISTER_USER_3 "\nSe ha producido un error al registrar el jugador!\n"
#define MSJ_REGISTER_USER_4 "\nSe ha cancelado el registro del nuevo usuario!\n"
#define MSJ_USER_1 Bold_Red "\nPor favor espere su turno..." Reset_Color
#define MSJ_ERROR_1 Bold_Red "\nEl jugador ingresado es inválido!\n" Reset_Color
#define MSJ_RESPUESTA_PREG "la respuesta de tu amigo es: "
#define MSJ_RESPUESTA_ACIERTO Bold_Green "\nAcertaste la pregunta" Reset_Color
#define MSJ_RESPUESTA_NOACIERTO Bold_Red "\nNo acertaste la pregunta" Reset_Color

char *menuInicio_c = "\n\t\t" Bold_Blue " The Test - Inicio \n" CYAN"[1]"Reset_Color" - Iniciar sesión\n" CYAN"[2]"Reset_Color" - Registrarse\n" CYAN"[3]"Reset_Color" - Salir\n>>> ";
char *menuJugador_c = "\n\t\t" Bold_Blue " The Test - Menú \n" CYAN"[1]"Reset_Color" - Seleccionar jugador\n" CYAN"[2]"Reset_Color" - Estado del servidor\n" CYAN"[3]"Reset_Color" - Cerrar sesión\n>>> ";
int newSocket;
char buffer[SIZE_BUFF];

int sockfd, ret;
struct sockaddr_in serverAddr;
struct sockaddr_in newAddr;

socklen_t addr_size;
pid_t childpid;


struct Jugador* enviar_iniciarSesion(char* username, char* password){
	struct Jugador *jugadorNuevo;
	jugadorNuevo = iniciarSesion(username,password);
	/*if(jugadorNuevo->idJugador == 0){
		return NULL;
	}*/

	return jugadorNuevo;
}

// dar formato a la pregunta con sus opciones para enviarsela al cliente
void formatoPregunta(char *enunciado_enviar, struct Pregunta *pregunta, int *maximoOpciones) {
	bzero(enunciado_enviar, sizeof(enunciado_enviar));
	strcat(enunciado_enviar, "\n" Bold_Yellow);
	strcat(enunciado_enviar, pregunta->enunciado);
	strcat(enunciado_enviar, Reset_Color);

	int op = 0;
	while (op < 3) {
		if (pregunta->opciones[op].idRespuesta <= 0) {
			break;
		}
		strcat(enunciado_enviar, "\n");
		strcat(enunciado_enviar, Bold_Magenta "[");
		char temp[10]; 
		sprintf(temp, "%d", (op + 1)); 
		strcat(enunciado_enviar, temp);
		strcat(enunciado_enviar, "]" Reset_Color " - ");
		strcat(enunciado_enviar, pregunta->opciones[op].respuesta); 
		op += 1;
		*maximoOpciones += 1;
	}
	strcat(enunciado_enviar, "\n>>> ");
}

void formatoRespuesta(char *enunciado_enviar, struct Opcion *opcion) {
	bzero(enunciado_enviar, sizeof(enunciado_enviar));
	strcat(enunciado_enviar, "\n" Bold_Yellow);
	strcat(enunciado_enviar, "La respuesta de tu amigo es: ");
	strcat(enunciado_enviar, opcion->respuesta); 
	strcat(enunciado_enviar, Reset_Color);
	strcat(enunciado_enviar, "\n>>> ");
}

// validar que la opcion que ingresó el usuario segun la pregunta enviada, 
// sea valida, porque hay preguntas con 2 o 3 opciones para responder 
int validarOpcionRespuestaUsuario(char opcion[], int maximoOpciones) {
	char op_1[] = "1";
	char op_2[] = "2";
	char op_3[] = "3";

	if (maximoOpciones == 2) {
		if ((strcmp(opcion, op_1) == 0) || (strcmp(opcion, op_2) == 0)) {
			return 1;
		} 
	}
	else {
		if ((strcmp(opcion, op_1) == 0) || (strcmp(opcion, op_2) == 0) || (strcmp(opcion, op_3) == 0) ) {
			return 1;
		} 
	}
	return 0;
}

void responder_nuevasPreguntas(int idJugadorActual, int idPartida) {
	char *preguntaUtilizadas;
	struct Pregunta *pregunta;
	int maximoOpciones = 0;

	for(int i=0; 2>i; i+=1){
		preguntaUtilizadas = armarTuplaPreguntadasUsadas(idPartida);
		pregunta = getPregunta(preguntaUtilizadas);
		
		send(newSocket, SEND_PREGUNTA, SIZE_BUFF, 0); // pido que ingrese el usuario

		char enunciado_enviar[SIZE_BUFF];
		formatoPregunta(enunciado_enviar, pregunta, &maximoOpciones);
		
		l_opcion_pregunta:
			send(newSocket, enunciado_enviar, SIZE_BUFF, 0);
			bzero(buffer, sizeof(buffer));
			// guardar respuesta
			recv(newSocket, buffer, sizeof(buffer), 0);

			if (validarOpcionRespuestaUsuario(buffer, maximoOpciones)) {
				printf("Insertando NUEVA pregunta %d y respuesta %d \n. Partida %d \n. Jugador %d \n",pregunta->idPregunta, pregunta->opciones[atoi(buffer) - 1].idRespuesta, idPartida, idJugadorActual);
				insert_turno(idPartida, idJugadorActual, pregunta->opciones[atoi(buffer) - 1].idRespuesta);
			}
			else { goto l_opcion_pregunta; }

		free(pregunta); // liberar el malloc hecho por la funcion
		free(preguntaUtilizadas);
		maximoOpciones = 0;
	}
}

void responder_preguntas_oponente(int idJugadorActual, int idPartida) {		
	struct Pregunta *preguntas;
	struct Opcion *opcionCorrecta = (struct Opcion*) malloc(sizeof(struct Opcion) * 2);
	int maximoOpciones = 0; char opcionesRespuestas[10] = " 2, "; char respuesta_oponente[500];
	preguntas = getPreguntaTurnoAnterior(idPartida); // Lista de preguntas
	char enunciado_enviar[SIZE_BUFF];

	for(int i=0; 2>i; i+=1){
		send(newSocket, SEND_PREGUNTA, SIZE_BUFF, 0); // pido que ingrese el usuario

		formatoPregunta(enunciado_enviar, &preguntas[i], &maximoOpciones);

		l_opcion_pregunta:
			send(newSocket, enunciado_enviar, SIZE_BUFF, 0);
			bzero(buffer, sizeof(buffer));
			recv(newSocket, buffer, sizeof(buffer), 0);

			if (validarOpcionRespuestaUsuario(buffer, maximoOpciones)) {
				//printf("Insertando pregunta %d y respuesta %d \n. Partida %d \n. Jugador %d \n",preguntas[i].idPregunta, preguntas[i].opciones[atoi(buffer) - 1].idRespuesta, idPartida, idJugadorActual);
				insert_turno(idPartida, idJugadorActual, preguntas[i].opciones[atoi(buffer) - 1].idRespuesta);
				opcionCorrecta = getRespuestaCorrecta(idPartida, idJugadorActual, preguntas[i].idPregunta);
				
				if(preguntas[i].opciones[atoi(buffer) - 1].idRespuesta == opcionCorrecta->idRespuesta) {
					establecerRespuestaAcertada(preguntas[i].opciones[atoi(buffer) - 1].idRespuesta, idPartida);
					sumarPuntos(idPartida, preguntas[i].puntaje);
					
					strcat(respuesta_oponente, MSJ_RESPUESTA_ACIERTO);
					strcat(respuesta_oponente, opcionesRespuestas);
				}
				else{ // no acertaste
					strcat(respuesta_oponente, MSJ_RESPUESTA_NOACIERTO);	
					strcat(respuesta_oponente, opcionesRespuestas);				
				}
				strcpy(opcionesRespuestas, " 1, ");

				bzero(enunciado_enviar, sizeof(enunciado_enviar));

				strcat(respuesta_oponente, MSJ_RESPUESTA_PREG);
				strcat(respuesta_oponente, opcionCorrecta->respuesta);
			}
			else { goto l_opcion_pregunta; }

		free(opcionCorrecta);
		maximoOpciones = 0;
	}
	
	free(preguntas);

	send(newSocket, RECEIVE_OPCIONES, strlen(RECEIVE_OPCIONES), 0);
	send(newSocket, respuesta_oponente, SIZE_BUFF, 0);
	bzero(buffer, sizeof(buffer));
}


void jugarPartida(char usuario_login[]) {
	struct Partida *partida; 
	partida = /*(struct Partida*)*/malloc(sizeof(struct Partida));
	char *consulta; consulta = usuariosRegistrados(usuario_login);
	strcat(consulta, "\n" USER_REQUEST); // mensaje de ingresar el nombre del jugador
	send(newSocket, consulta, SIZE_BUFF, 0); // le envio los jugadores registrados
	free(consulta);

	bzero(buffer, sizeof(buffer));	
	recv(newSocket, buffer, SIZE_BUFF, 0); // recibo el jugador con el que quiere jugar

	if (getIdJugador(buffer) != -1 && strcmp(usuario_login, buffer) != 0) { // valido que el jugador digitado sea valido, y no se haya insertado el mismo
		int idJugador2_Int = getIdJugador(buffer);
		int idJugador1_Int = getIdJugador(usuario_login);

		*partida = verificarExistenciaPartida(idJugador1_Int, idJugador2_Int);

		if (partida->idPartida == 0) { // la partida no existe, hay que crear una. Respondo nuevas preguntas para mi
			partida = crearNuevaPartida(idJugador1_Int, idJugador2_Int);
			printf("Se ha creado una nueva partida: " Bold_Cyan "%s" Bold_Yellow " vs " Bold_Cyan "%s\n" Reset_Color, usuario_login, buffer);

			responder_nuevasPreguntas(idJugador1_Int, partida->idPartida);
		}
		else { // si existe, se continua la partida
			if (comprobarMiTurno(partida->idPartida, idJugador1_Int)) {  
				responder_preguntas_oponente(idJugador1_Int, partida->idPartida);	// 1) Responder preguntas oponente
				responder_nuevasPreguntas(idJugador1_Int, partida->idPartida);		// 2) Responder nuevas preguntas para mi
			}
			else { // Turno del rival. Osea que el mae actual se pone a esperar. 
				send(newSocket, REGISTER, strlen(REGISTER), 0);
				send(newSocket, MSJ_USER_1, SIZE_BUFF, 0); // msj diciendo que tiene que esperar
			}
		}
		bzero(buffer, sizeof(buffer));
		free(partida);
	}
	else {
		send(newSocket, REGISTER, strlen(REGISTER), 0);
		send(newSocket, MSJ_ERROR_1, SIZE_BUFF, 0);
	}
}

void crearServidor() {
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

	if (initConnection() != 0) { // abrir la conexion con el DB
		puts("Error al establecer conexión con la base de datos");
	}

	while (1) {
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if (newSocket < 0) {
			exit(1);
		}
		
		printf("Conexión aceptada de " Bold_Yellow "%s:%d\n" Reset_Color, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
		
		if ((childpid = fork()) == 0) {
			close(sockfd);

			recv(newSocket, buffer, SIZE_BUFF, 0); // apenas se conecta un cliente, recibo el tipo de usuario que es (jugador o de mantenimiento)
			char tipo_usuario[24]; 
			strcpy(tipo_usuario, buffer);
			char usuario_login[50];
			char pass_login[50];
			char email_user[50];

			l_menu_inicio:
				send(newSocket, menuInicio_c, 165, 0); // mostrar menu inicio
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, SIZE_BUFF, 0); // lo recibo

				if (strcmp(buffer, "1") == 0) {
					goto l_login_jugador;
				}
				else if (strcmp(buffer, "2") == 0) { // registrar usuario
					goto l_registrar_jugador;
				}
				else if (strcmp(buffer, "3") == 0) {
					send(newSocket, SALIR, SIZE_BUFF, 0);
					//endConnection(); // cerrar comunicacion con la BD
					printf("Desconexión de " Bold_Red "%s:%d\n" Reset_Color, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					goto l_cerrar_conexion;
				}
				else {
					goto l_menu_inicio;
				}

			l_login_jugador:				
				send(newSocket, LOGIN_USER, strlen(LOGIN_USER), 0); // pido que ingrese el usuario

				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, SIZE_BUFF, 0); // lo recibo

				strcpy(usuario_login, buffer); // guardo el nombre de usuario

				bzero(buffer, sizeof(buffer));
				send(newSocket, LOGIN_PASS, strlen(LOGIN_PASS), 0); // pido que ingrese la contraseña
				
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, SIZE_BUFF, 0); // lo recibo

				strcpy(pass_login, buffer); // guardo la contraseña
				bzero(buffer, sizeof(buffer));
				
				struct Jugador *jugadorNuevo;
				jugadorNuevo = enviar_iniciarSesion(usuario_login,pass_login);
				//send(newSocket, jugadorNuevo, sizeof(struct Jugador)+1, 0); 
				
				if (jugadorNuevo->idJugador > 0) {

					if (strcmp(tipo_usuario, JUGADOR) == 0) {
						
						send(newSocket, LOGIN, SIZE_BUFF, 0); // le envio su ID al cliente
						//send(newSocket, &resp, sizeof(resp), 0);
						send(newSocket, jugadorNuevo, sizeof(struct Jugador)+1, 0); 
						
						while (1) {
							send(newSocket, menuJugador_c, SIZE_BUFF, 0);
							bzero(buffer, sizeof(buffer));
							recv(newSocket, buffer, SIZE_BUFF, 0); // recibo alguna opcion del menu

							if (strcmp(buffer, "1") == 0) {
								jugarPartida(usuario_login);
								bzero(buffer, sizeof(buffer));
								//send(newSocket, "La partida ha sido creada", SIZE_BUFF, 0);								
							}
							else if (strcmp(buffer, "2") == 0) {
								send(newSocket, "Estado del servidor: jugadores en juego activo y estadísticas tales como : usuarios, número de preguntas, número de usos, aciertos y fallos totales del sistema y por cada pregunta, puntajes entre todos los pares de usuarios en juego como un ranking.", SIZE_BUFF, 0);
								// estado del servidor
								bzero(buffer, sizeof(buffer));	
								recv(newSocket, buffer, SIZE_BUFF, 0); // recibo alguna opcion del menu
								
							}
							else if (strcmp(buffer, "3") == 0) { // cerrar sesion
								free(jugadorNuevo);
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
				recv(newSocket, buffer, SIZE_BUFF, 0); // lo recibo
				strcpy(usuario_login, buffer); // guardo el nombre de usuario

				send(newSocket, LOGIN_PASS, strlen(LOGIN_PASS), 0); // pido que ingrese el usuario
				
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, SIZE_BUFF, 0); // lo recibo
				strcpy(pass_login, buffer); // guardo la contraseña

				send(newSocket, LOGIN_EMAIL, strlen(LOGIN_EMAIL), 0); // pido que ingrese el usuario
				
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, SIZE_BUFF, 0); // lo recibo
				strcpy(email_user, buffer); // guardo el correo

				//send(newSocket, REGISTER, strlen(REGISTER), 0); // envio llave de registrar 
				send(newSocket, QUESTION_REGISTER_USER, strlen(QUESTION_REGISTER_USER), 0); // envio mensaje de confirmación
				bzero(buffer, sizeof(buffer));				
				recv(newSocket, buffer, SIZE_BUFF, 0); // lo recibo
				
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
					send(newSocket, REGISTER, strlen(REGISTER), 0); // envio llave de registrar 
					send(newSocket, Bold_Red MSJ_REGISTER_USER_4 Reset_Color, strlen(MSJ_REGISTER_USER_3) + 13, 0);					
				}
				bzero(buffer, sizeof(buffer));
				goto l_menu_inicio;

		}
	}
	l_cerrar_conexion:
	close(newSocket);	
}


int main() {

	/*initConnection();
	struct Pregunta *preguntas;
	preguntas = getPreguntaTurnoAnterior(2); // Lista de preguntas
	struct Pregunta *pregunta;
	struct Opcion opcionPrint;
	for(int i=0; 2>i; i+=1){
		pregunta = &preguntas[i];
		printf("Enunciado pregunta %d: %s \n",i,pregunta->enunciado);
		printf("Puntaje pregunta %d: %d \n",i,pregunta->puntaje);

		opcionPrint = pregunta->opciones[0];
		printf("Opcion 1 %s \n",opcionPrint.respuesta);

		opcionPrint = pregunta->opciones[1];
		printf("Opcion 2 %s \n",opcionPrint.respuesta);

		opcionPrint = pregunta->opciones[2];
		printf("Opcion 3 %s \n",opcionPrint.respuesta);
	}*/

	crearServidor();

	return 0;
}