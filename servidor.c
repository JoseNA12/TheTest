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
#include "Colores.c"

// Estructuras
#include "Pregunta.c"
#include "Jugador.c"
#include "Partida.c"

/*	
slqlite3:
	sudo apt install sqlite3
	sudo apt-get install libsqlite3-dev
*/
// Servidor/cliente basado en: 
// -> https://github.com/nikhilroxtomar/Multiple-Client-Server-Program-in-C-using-fork

// Compilar con: gcc servidor.c -o servidor -lsqlite3
// sudo ufw allow 3001
	

#define PUERTO 4444
#define DIRECCION_IP "127.0.0.1"
#define SIZE_BUFF 2000

char LOGIN_USER[] = "\n[The Test] -> Usuario: ";
char LOGIN_PASS[] = "[The Test] -> Contraseña: ";
char LOGIN_EMAIL[] = "[The Test] -> Correo: ";
char QUESTION_REGISTER_USER[] = "\nDesea continuar?\n[1] - Si\n[2] - No\n>>> ";
char USER_REQUEST[] = "\nIngrese el nombre del jugador para jugar\n>>> ";
char MSJ_REGISTER_USER_1[] = Bold_Green "\nSe ha registrado el usuario!" Reset_Color;
char MSJ_REGISTER_USER_2[] = Bold_Red "\nEl nombre de usuario indicado se encuentra en uso!\n" Reset_Color;
char MSJ_REGISTER_USER_3[] = Bold_Red "\nSe ha producido un error al registrar el jugador!\n" Reset_Color;
char MSJ_REGISTER_USER_4[] = Bold_Red "\nSe ha cancelado el registro del nuevo usuario!\n" Reset_Color;
char MSJ_USER_1[] = Bold_Red"\nPor favor espere su turno..." Reset_Color;
char MSJ_ERROR_1[] = Bold_Red "\nEl jugador ingresado es inválido!\n" Reset_Color;
char MSJ_RESPUESTA_PREG[] = "la respuesta de tu amigo es: ";
char MSJ_RESPUESTA_ACIERTO[] = Bold_Green "\nAcertaste la pregunta" Reset_Color;
char MSJ_RESPUESTA_NOACIERTO[] = Bold_Red "\nNo acertaste la pregunta" Reset_Color;
char MSJ_NIVEL_PARTIDA[] = CYAN "\nNivel: " Reset_Color;
char MSJ_PUNTOS_PARTIDA[] = CYAN ", puntos: " Reset_Color;

char menuInicio_c[] = "\n\t\t" Bold_Blue " The Test - Inicio \n" CYAN"[1]"Reset_Color" - Iniciar sesión\n" CYAN"[2]"Reset_Color" - Registrarse\n" CYAN"[3]"Reset_Color" - Salir\n>>> ";
char menuJugador_c[] = "\n\t\t" Bold_Blue " The Test - Menú \n" CYAN"[1]"Reset_Color" - Seleccionar jugador\n" /*CYAN"[2]"Reset_Color" - Estado del servidor\n"*/ CYAN"[2]"Reset_Color" - Cerrar sesión\n>>> ";
char menuMantenimiento[] = "\n\t\t" Bold_Yellow "The Test - Mantenimiento \n" CYAN"[1]"Reset_Color" - Juegos activos\n" CYAN"[2]"Reset_Color" - Jugadores registrados\n" CYAN"[3]"Reset_Color" - Preguntas\n" CYAN"[4]"Reset_Color" - Ranking\n" CYAN"[5]"Reset_Color" - Salir\n>>> ";
char menuEstadisticasPreguntas_c[] = "\n\t\t" Bold_Blue " Preguntas \n" CYAN"[1]"Reset_Color" - Estadisticas generales\n" CYAN"[2]"Reset_Color" - Estadisticas por pregunta\n" CYAN"[3]"Reset_Color" - Ver todas las preguntas\n" CYAN"[4]"Reset_Color" - Gestionar preguntas\n" CYAN"[5]"Reset_Color" - Salir\n>>> ";
char menuEstadisticasPreguntaIndividual_c[] = "\n\t\t" Bold_Blue " Estadísticas por pregunta \n" CYAN"[1]"Reset_Color" - Correctas \n" CYAN"[2]"Reset_Color" - Incorrectas \n" CYAN"[3]"Reset_Color" - Salir\n>>> ";
char menuGestionarPreguntas[] = "\n\t\t" Bold_Blue " Gestionar preguntas \n" CYAN"[1]"Reset_Color" - Crear pregunta \n" CYAN"[2]"Reset_Color" - Modificar pregunta \n" CYAN"[3]"Reset_Color" - Eliminar pregunta \n" CYAN"[4]"Reset_Color" - Salir\n>>> ";

int newSocket;

int sockfd, ret;
struct sockaddr_in serverAddr;
struct sockaddr_in newAddr;

socklen_t addr_size;
pid_t childpid;

// dar formato a la pregunta con sus opciones para enviarsela al cliente
void formatoPregunta(char *enunciado_enviar, struct Pregunta *pregunta, int *maximoOpciones) {
	memset(enunciado_enviar, 0, SIZE_BUFF + 1);
	//bzero(enunciado_enviar, sizeof(enunciado_enviar));
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
	memset(enunciado_enviar, 0, SIZE_BUFF + 1);
	//bzero(enunciado_enviar, sizeof(enunciado_enviar));
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
	char buffer_rnp[SIZE_BUFF]; memset(buffer_rnp, 0, SIZE_BUFF + 1);

	for(int i=0; 2>i; i+=1){
		preguntaUtilizadas = armarTuplaPreguntadasUsadas(idPartida);
		printf("*********************Pregunta nueva*********************\n");
		printf("La tupla armada es: %s\n",preguntaUtilizadas);
		pregunta = getPregunta(preguntaUtilizadas);
		printf("La pregunta conseguida es: %d \n",pregunta->idPregunta);

		send(newSocket, SEND_PREGUNTA, SIZE_BUFF, 0); // pido que ingrese el usuario
		recv(newSocket, buffer_rnp, sizeof(buffer_rnp), 0);
		char enunciado_enviar[SIZE_BUFF];
		formatoPregunta(enunciado_enviar, pregunta, &maximoOpciones);
		
		l_opcion_pregunta:
			send(newSocket, enunciado_enviar, SIZE_BUFF, 0);
			memset(buffer_rnp, 0, SIZE_BUFF + 1);
			//bzero(buffer_rnp, sizeof(buffer_rnp));
			// guardar respuesta
			recv(newSocket, buffer_rnp, sizeof(buffer_rnp), 0);

			if (validarOpcionRespuestaUsuario(buffer_rnp, maximoOpciones)) {
				//printf("Insertando NUEVA pregunta %d y respuesta %d \n. Partida %d \n. Jugador %d \n",pregunta->idPregunta, pregunta->opciones[atoi(buffer_rnp) - 1].idRespuesta, idPartida, idJugadorActual);
				insert_turno(idPartida, idJugadorActual, pregunta->opciones[atoi(buffer_rnp) - 1].idRespuesta);
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
	int maximoOpciones = 0; char opcionesRespuestas[10] = " 2, "; 
	char respuesta_oponente[500]; memset(respuesta_oponente, 0, 501); 
	preguntas = getPreguntaTurnoAnterior(idPartida); // Lista de preguntas
	char enunciado_enviar[SIZE_BUFF];
	char buffer_rpo[SIZE_BUFF]; memset(buffer_rpo, 0, SIZE_BUFF + 1);

	for(int i=0; 2>i; i+=1){
		send(newSocket, SEND_PREGUNTA, SIZE_BUFF, 0); // pido que ingrese el usuario
		recv(newSocket, buffer_rpo, sizeof(buffer_rpo), 0);
		formatoPregunta(enunciado_enviar, &preguntas[i], &maximoOpciones);

		l_opcion_pregunta:
			send(newSocket, enunciado_enviar, SIZE_BUFF, 0);
			memset(buffer_rpo, 0, SIZE_BUFF + 1);
			//bzero(buffer_rpo, sizeof(buffer_rpo));
			recv(newSocket, buffer_rpo, sizeof(buffer_rpo), 0);

			if (validarOpcionRespuestaUsuario(buffer_rpo, maximoOpciones)) {
				insert_turno(idPartida, idJugadorActual, preguntas[i].opciones[atoi(buffer_rpo) - 1].idRespuesta);
				opcionCorrecta = getRespuestaCorrecta(idPartida, idJugadorActual, preguntas[i].idPregunta);

				if(preguntas[i].opciones[atoi(buffer_rpo) - 1].idRespuesta == opcionCorrecta->idRespuesta) {
					establecerRespuestaAcertada(preguntas[i].opciones[atoi(buffer_rpo) - 1].idRespuesta, idPartida);
					sumarPuntos(idPartida, preguntas[i].puntaje);
					
					strcat(respuesta_oponente, MSJ_RESPUESTA_ACIERTO);
					strcat(respuesta_oponente, opcionesRespuestas);
				}
				else{ // no acertaste
					strcat(respuesta_oponente, MSJ_RESPUESTA_NOACIERTO);	
					strcat(respuesta_oponente, opcionesRespuestas);				
				}
				strcpy(opcionesRespuestas, " 1, ");

				memset(enunciado_enviar, 0, SIZE_BUFF + 1);
				//bzero(enunciado_enviar, sizeof(enunciado_enviar));

				strcat(respuesta_oponente, MSJ_RESPUESTA_PREG);
				strcat(respuesta_oponente, opcionCorrecta->respuesta);
			}
			else { goto l_opcion_pregunta; }

		free(opcionCorrecta);
		maximoOpciones = 0;
	}
	
	free(preguntas);

	//--- Puntos y nivel
 	struct Partida *partida = (struct Partida*) malloc(sizeof(struct Partida));
 	partida = getPuntosYNivelPartida(idPartida);
 	char str_n[16]; sprintf(str_n, "%d", partida->nivel);
 	char str_pts[16]; sprintf(str_pts, "%d", partida->puntaje);
 	strcat(respuesta_oponente, MSJ_NIVEL_PARTIDA);
 	strcat(respuesta_oponente, str_n);
 	strcat(respuesta_oponente, MSJ_PUNTOS_PARTIDA);
 	strcat(respuesta_oponente, str_pts);
 	free(partida);
 	//---

	puts(respuesta_oponente);

	send(newSocket, RECEIVE_OPCIONES, strlen(RECEIVE_OPCIONES), 0);
	recv(newSocket, buffer_rpo, sizeof(buffer_rpo), 0);
	send(newSocket, respuesta_oponente, SIZE_BUFF, 0);
	bzero(buffer_rpo, sizeof(buffer_rpo));
}

/*
Dar inicio a una partida de The Test.
Si no existe una partida entre 2 usuarios, se crea dicha partida
Sino, se continua la partida retomando el proceso anteriormente jugado
*/
void jugarPartida(char usuario_login[]) {
	struct Partida *partida; 
	partida = malloc(sizeof(struct Partida));
	char *consulta; consulta = usuariosRegistrados(usuario_login);
	strcat(consulta, USER_REQUEST); // mensaje de ingresar el nombre del jugador
	send(newSocket, consulta, SIZE_BUFF, 0); // le envio los jugadores registrados
	free(consulta);
	char buffer_jp[SIZE_BUFF]; memset(buffer_jp, 0, SIZE_BUFF + 1);

	//bzero(buffer_jp, sizeof(buffer_jp));	
	recv(newSocket, buffer_jp, SIZE_BUFF, 0); // recibo el jugador con el que quiere jugar

	if (getIdJugador(buffer_jp) != -1 && strcmp(usuario_login, buffer_jp) != 0) { // valido que el jugador digitado sea valido, y no se haya insertado el mismo
		int idJugador2_Int = getIdJugador(buffer_jp);
		int idJugador1_Int = getIdJugador(usuario_login);

		*partida = verificarExistenciaPartida(idJugador1_Int, idJugador2_Int);

		if (partida->idPartida == 0) { // la partida no existe, hay que crear una. Respondo nuevas preguntas para mi
			partida = crearNuevaPartida(idJugador1_Int, idJugador2_Int);
			printf("Se ha creado una nueva partida: " Bold_Cyan "%s" Bold_Yellow " vs " Bold_Cyan "%s\n" Reset_Color, usuario_login, buffer_jp);

			responder_nuevasPreguntas(idJugador1_Int, partida->idPartida);
		}
		else { // si existe, se continua la partida
			if (comprobarMiTurno(partida->idPartida, idJugador1_Int)) {  
				responder_preguntas_oponente(idJugador1_Int, partida->idPartida);	// 1) Responder preguntas oponente
				responder_nuevasPreguntas(idJugador1_Int, partida->idPartida);		// 2) Responder nuevas preguntas para mi
			}
			else { // Turno del rival. Osea que el mae actual se pone a esperar. 
				send(newSocket, REGISTER, strlen(REGISTER), 0);
				recv(newSocket, buffer_jp, SIZE_BUFF, 0);
				send(newSocket, MSJ_USER_1, SIZE_BUFF, 0); // msj diciendo que tiene que esperar
			}
		}
		memset(buffer_jp, 0, SIZE_BUFF + 1);
		//bzero(buffer_jp, sizeof(buffer_jp));
		free(partida);
	}
	else {
		send(newSocket, REGISTER, strlen(REGISTER), 0);
		recv(newSocket, buffer_jp, SIZE_BUFF, 0);
		send(newSocket, MSJ_ERROR_1, SIZE_BUFF, 0);
	}
}


void estadisticasPreguntasGenerales(char* enunciado_enviar){
	int cantPreguntas, cantCorrectas, cantIncorrectas;
	char temp[10];
	memset(enunciado_enviar, 0, SIZE_BUFF);

	strcat(enunciado_enviar, "\n" Bold_Yellow);
	strcat(enunciado_enviar, "\nEstadísticas Generales: ");
	strcat(enunciado_enviar, Reset_Color);

	cantPreguntas = numeroPreguntas();
	cantCorrectas = numeroPreguntasPorEstado(1);
	cantIncorrectas = numeroPreguntasPorEstado(0);

	strcat(enunciado_enviar, "\nCantidad de preguntas almacenadas: ");
	sprintf(temp, "%d", cantPreguntas); 
	strcat(enunciado_enviar, temp);
	memset(temp, 0, 10);
	strcat(enunciado_enviar, "\nCantidad de preguntas respondidas correctamente: ");
	sprintf(temp, "%d", cantCorrectas); 
	strcat(enunciado_enviar, temp);
	memset(temp, 0, 10);
	strcat(enunciado_enviar, "\nCantidad de preguntas respondidas incorrectamente: ");
	sprintf(temp, "%d", cantIncorrectas); 
	strcat(enunciado_enviar, temp);
	strcat(enunciado_enviar, "\n\nPresione cualquier tecla para salir \n>>> ");
}


void wizardCrearPregunta(){
	int size_buffer = 2000;
	int indice = 0;
	char buffer_mp[size_buffer];
	char solicitarEnunciado[] = "Indique el enunciado de la pregunta \n>>> ";
	char solicitarPuntaje[] = "Indique el puntaje de la pregunta \n>>> ";
	char solicitarOpcion[] = "Indique una opción para la pregunta \n>>> ";
	char solicitarNuevaOpcion[] = "Desea agregar una opción más? y/n \n>>> ";
	char mensajeExito[] = "La pregunta ha sido creada correctamente \n\nIngrese cualquier tecla para continuar \n>>> ";
	char enunciado[size_buffer];
	int puntaje;
	struct Opcion *opciones = malloc(3*sizeof(struct Opcion));

	send(newSocket, solicitarEnunciado, strlen(solicitarEnunciado), 0);
	recv(newSocket, enunciado, size_buffer, 0);

	send(newSocket, solicitarPuntaje, strlen(solicitarPuntaje), 0);
	recv(newSocket, buffer_mp, size_buffer, 0);
	puntaje = atoi(buffer_mp);

	while(indice < 3){
		if(indice == 2){
			send(newSocket, solicitarNuevaOpcion, strlen(solicitarNuevaOpcion), 0);
			memset(buffer_mp, 0, size_buffer + 1);
			recv(newSocket, buffer_mp, size_buffer, 0);
			if(strcmp(buffer_mp,"n")==0)
				break;
		}
		send(newSocket, solicitarOpcion, strlen(solicitarOpcion), 0);
		memset(buffer_mp, 0, size_buffer + 1);
		recv(newSocket, buffer_mp, size_buffer, 0);
		strcat(opciones[indice].respuesta, buffer_mp); 
		indice+=1;
	}
	crearPregunta(enunciado,opciones,puntaje);
	send(newSocket, mensajeExito, strlen(mensajeExito), 0);
	memset(buffer_mp, 0, size_buffer + 1);
	recv(newSocket, buffer_mp, size_buffer, 0);
}

void wizardModOpciones(int idPregunta){
	char solicitarOpcion[] = "\nIndique la nueva opción \n>>>";
	char buffer_mp[2000];
	int idOpcion;
	getOpciones( idPregunta, buffer_mp);
	send(newSocket, buffer_mp, strlen(buffer_mp), 0);
	bzero(buffer_mp, strlen(buffer_mp));
	recv(newSocket, buffer_mp, SIZE_BUFF, 0); 
	idOpcion = atoi(buffer_mp);

	send(newSocket, solicitarOpcion, strlen(solicitarOpcion), 0);
	bzero(buffer_mp, strlen(buffer_mp));
	recv(newSocket, buffer_mp, SIZE_BUFF, 0);
	updateOpcion(idOpcion, buffer_mp);
}

void wizardModEnunciadoPregunta(int idPregunta){
	char solicitarEnunciado[] = "Indique el nuevo enunciado \n>>>";
	char buffer_mp[2000];
	send(newSocket, solicitarEnunciado, strlen(solicitarEnunciado), 0);
	bzero(buffer_mp, strlen(buffer_mp));
	recv(newSocket, buffer_mp, sizeof(buffer_mp), 0); 
	printf("recibido %s",buffer_mp);
	updatePregunta(idPregunta, buffer_mp, -1);
}

void wizardModPuntajePregunta(int idPregunta){
	char solicitarPuntaje[] = "Indique el nuevo puntaje \n>>>";
	char buffer_mp[10];
	send(newSocket, solicitarPuntaje, strlen(solicitarPuntaje), 0);
	bzero(buffer_mp, strlen(buffer_mp));
	recv(newSocket, buffer_mp, sizeof(buffer_mp), 0); 
	updatePregunta(idPregunta, "", atoi(buffer_mp));
}

void wizardModPregunta(){
	int idPregunta;
	char buffer_mp[2];
	char solicitarIDPregunta[] = "\nIndique el identificador de pregunta \n>>> ";
	char menuModPreguntas[] = "\n\t\t" Bold_Blue " Modificar pregunta \n" CYAN"[1]"Reset_Color" - Enunciado \n" CYAN"[2]"Reset_Color" - Opciones \n" CYAN"[3]"Reset_Color" - Puntaje \n" CYAN"[4]"Reset_Color" - Salir \n>>> ";
	send(newSocket, solicitarIDPregunta, strlen(solicitarIDPregunta), 0);
	memset(buffer_mp, 0, 2);
	recv(newSocket, buffer_mp, 2, 0);
	idPregunta = atoi(buffer_mp);
	send(newSocket, menuModPreguntas, strlen(menuModPreguntas), 0);
	memset(buffer_mp, 0, 2);
	recv(newSocket, buffer_mp, 2, 0);
	if (strcmp(buffer_mp, "1") == 0) { // Modificar enunciado
		wizardModEnunciadoPregunta(idPregunta);
	}
	else if(strcmp(buffer_mp, "2") == 0) { // Modificar opciones
		wizardModOpciones(idPregunta);
	}
	else if(strcmp(buffer_mp, "3") == 0) { // Modificar puntaje
		wizardModPuntajePregunta(idPregunta);
	}
}

void wizardDeletePregunta(){
	int idPregunta;
	char buffer_mp[10];
	char solicitarIDPregunta[] = "\nIndique el identificador de pregunta \n>>> ";
	char mensajeExito[] = "\nLa pregunta ha sido eliminada correctamente \n\nIngrese cualquier tecla para continuar \n>>> ";
	send(newSocket, solicitarIDPregunta, strlen(solicitarIDPregunta), 0);
	memset(buffer_mp, 0, 10);
	recv(newSocket, buffer_mp, 10, 0);
	idPregunta = atoi(buffer_mp);
	deletePregunta(idPregunta);
	send(newSocket, mensajeExito, strlen(mensajeExito), 0);
	memset(buffer_mp, 0, 10 + 1);
	recv(newSocket, buffer_mp, 10, 0);
}

/*
Parte del cliente de mantenimiento.
Muestra datos de las preguntas registadas en la base de datos
*/
void menuPreguntas(){
	char enunciado_enviar[SIZE_BUFF];
	int size_buffer = 1;
	char buffer_mp[size_buffer]; memset(buffer_mp, 0, size_buffer + 1);
	send(newSocket, menuEstadisticasPreguntas_c, strlen(menuEstadisticasPreguntas_c), 0); 
	recv(newSocket, buffer_mp, size_buffer, 0); 

	if (strcmp(buffer_mp, "1") == 0) { // Estadisticas generales
		estadisticasPreguntasGenerales(enunciado_enviar);
		send(newSocket, enunciado_enviar, strlen(enunciado_enviar), 0);
		memset(buffer_mp, 0, size_buffer + 1);
		recv(newSocket, buffer_mp, SIZE_BUFF, 0);
	}
	else if (strcmp(buffer_mp, "2") == 0) { // Estadisticas por pregunta
		send(newSocket, menuEstadisticasPreguntaIndividual_c, strlen(menuEstadisticasPreguntaIndividual_c), 0);
		memset(buffer_mp, 0, size_buffer + 1);
		recv(newSocket, buffer_mp, size_buffer, 0); 
		if(strcmp(buffer_mp, "1") == 0) {
			estadisticasPorPregunta(enunciado_enviar,1);
		}else if(strcmp(buffer_mp, "2") == 0){
			estadisticasPorPregunta(enunciado_enviar,0);
		}
		strcat(enunciado_enviar, "\n\nPresione cualquier tecla para salir \n>>> ");
		send(newSocket, enunciado_enviar, sizeof(enunciado_enviar), 0);
		memset(buffer_mp, 0, size_buffer + 1);
		recv(newSocket, buffer_mp, SIZE_BUFF, 0);
	}
	else if (strcmp(buffer_mp, "3") == 0) { // mostrar preguntar
		char preguntasActuales[20000];
        char miBuffer[SIZE_BUFF];
        memset(miBuffer, 0, SIZE_BUFF);
        selectAllPregunta(preguntasActuales);
        char *token = strtok(preguntasActuales, "$"); 

        while (token != NULL) {
            strcat(miBuffer, token);
            strcat(miBuffer, Bold_Cyan "\nSiguiente pregunta ? (S/N): " Reset_Color);
            send(newSocket, miBuffer, strlen(miBuffer), 0);
            recv(newSocket, buffer_mp, size_buffer, 0);
            if ((strcmp(buffer_mp, "n") == 0) || (strcmp(miBuffer, "N") == 0) ) 
                break;
            
            memset(miBuffer, 0, SIZE_BUFF); memset(buffer_mp, 0, 2);
            token = strtok(NULL, "$");
        }
        strcat(enunciado_enviar, "\n\nPresione cualquier tecla para salir \n>>> ");
        send(newSocket, enunciado_enviar, sizeof(enunciado_enviar), 0);
        memset(buffer_mp, 0, size_buffer + 1);
        recv(newSocket, buffer_mp, SIZE_BUFF, 0);
	}
	else if(strcmp(buffer_mp, "4") == 0) {
		send(newSocket, menuGestionarPreguntas, strlen(menuGestionarPreguntas), 0);
		memset(buffer_mp, 0, size_buffer + 1);
		recv(newSocket, buffer_mp, size_buffer, 0);
		if (strcmp(buffer_mp, "1") == 0) { // crear pregunta
			wizardCrearPregunta();
		}
		else if (strcmp(buffer_mp, "2") == 0) { // modificar pregunta
			wizardModPregunta();
		}
		else if (strcmp(buffer_mp, "3") == 0) { 
			wizardDeletePregunta();
		}
	}
}

/*
Función encargada de manejar toda petición por parte de 
un cliente jugador
*/
void gestionClienteJugador() {
	char buffer[SIZE_BUFF]; memset(buffer, 0, SIZE_BUFF);
	char usuario_login[50];
	char pass_login[50];
	char email_user[50];

	l_menu_inicio:
		send(newSocket, menuInicio_c, 165, 0); // mostrar menu inicio
		memset(buffer, 0, SIZE_BUFF + 1);
		//bzero(buffer, sizeof(buffer));
		recv(newSocket, buffer, SIZE_BUFF, 0); // lo recibo

		if (strcmp(buffer, "1") == 0) {
			goto l_login_jugador;
		}
		else if (strcmp(buffer, "2") == 0) { // registrar usuario
			goto l_registrar_jugador;
		}
		else if (strcmp(buffer, "3") == 0) {
			send(newSocket, SALIR, SIZE_BUFF, 0);
			printf("Desconexión de " Bold_Red "%s:%d\n" Reset_Color, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
			close(newSocket);
		}
		else {
			goto l_menu_inicio;
		}

	l_login_jugador:			
		send(newSocket, LOGIN_USER, strlen(LOGIN_USER), 0); // pido que ingrese el usuario

		memset(buffer, 0, SIZE_BUFF + 1);
		//bzero(buffer, sizeof(buffer));
		recv(newSocket, buffer, SIZE_BUFF, 0); // lo recibo

		strcpy(usuario_login, buffer); // guardo el nombre de usuario

		memset(buffer, 0, SIZE_BUFF + 1);
		//bzero(buffer, sizeof(buffer));
		send(newSocket, LOGIN_PASS, strlen(LOGIN_PASS), 0); // pido que ingrese la contraseña
		
		memset(buffer, 0, SIZE_BUFF + 1);
		//bzero(buffer, sizeof(buffer));
		recv(newSocket, buffer, SIZE_BUFF, 0); // lo recibo

		strcpy(pass_login, buffer); // guardo la contraseña
		//bzero(buffer, sizeof(buffer));
		memset(buffer, 0, SIZE_BUFF + 1);
		
		struct Jugador *jugadorNuevo;
		jugadorNuevo = iniciarSesion(usuario_login,pass_login);
		
		if (jugadorNuevo->idJugador > 0) {
			send(newSocket, LOGIN, SIZE_BUFF, 0); // le envio su ID al cliente
			recv(newSocket, buffer, SIZE_BUFF, 0);
			memset(buffer, 0, SIZE_BUFF + 1);
			send(newSocket, jugadorNuevo, sizeof(struct Jugador)+1, 0); 
			recv(newSocket, buffer, SIZE_BUFF, 0);

			while (1) {
				send(newSocket, menuJugador_c, SIZE_BUFF, 0);
				//bzero(buffer, sizeof(buffer));
				memset(buffer, 0, SIZE_BUFF + 1);
				recv(newSocket, buffer, SIZE_BUFF, 0); // recibo alguna opcion del menu

				if (strcmp(buffer, "1") == 0) {
					jugarPartida(usuario_login);
					memset(buffer, 0, SIZE_BUFF + 1);
					//bzero(buffer, sizeof(buffer));							
				}
				/*else if(strcmp(buffer, "2") == 0) { // Estado del servidor
					send(newSocket, menuEstadisticas_c, 200, 0); 
					memset(buffer, 0, SIZE_BUFF + 1);
					//bzero(buffer, sizeof(buffer));
					menuEstadisticas();
					
				}*/
				else if(strcmp(buffer, "2") == 0) { // cerrar sesion
					printf(CYAN"%s"Reset_Color": ha cerrado sesión\n", jugadorNuevo->nombreUsuario);
					free(jugadorNuevo);
					goto l_menu_inicio;
				}
			}
		}
		else {
			memset(usuario_login, 0, strlen(usuario_login) + 1);
			memset(pass_login, 0, strlen(pass_login) + 1);
			//bzero(usuario_login, sizeof(usuario_login));
			//bzero(pass_login, sizeof(pass_login));
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
			recv(newSocket, buffer, SIZE_BUFF, 0);
			int resp = registrarUsuario(usuario_login, pass_login, email_user);
			
			if (resp == 1) { // registro completado
				send(newSocket, MSJ_REGISTER_USER_1, strlen(MSJ_REGISTER_USER_1) + 14, 0); // +14 de los colores
				printf("Se ha registrado un nuevo usuario: " Bold_Yellow "%s\n" Reset_Color, usuario_login);
			}
			else if (resp == 0) { // usuario ingresado en uso
				send(newSocket, MSJ_REGISTER_USER_2, strlen(MSJ_REGISTER_USER_2) + 13, 0);
			}
			else { // error
				send(newSocket, MSJ_REGISTER_USER_3, strlen(MSJ_REGISTER_USER_3) + 13, 0);
			}
		}
		else {
			send(newSocket, REGISTER, strlen(REGISTER), 0); // envio llave de registrar
			recv(newSocket, buffer, SIZE_BUFF, 0);
			send(newSocket, MSJ_REGISTER_USER_4, strlen(MSJ_REGISTER_USER_3) + 13, 0);					
		}
		bzero(buffer, sizeof(buffer));
		goto l_menu_inicio;
}

/*
Función encargada de manejar toda petición por parte de
un cliente de mantenimiento
*/
void gestionClienteMantenimiento() {
	char buffer[SIZE_BUFF]; memset(buffer, 0, SIZE_BUFF);
	char enunciado_enviar[SIZE_BUFF];
	int size_buffer = 1;
	char buffer_me[size_buffer]; memset(buffer_me, 0, size_buffer + 1);
	while (1) {
		send(newSocket, menuMantenimiento, strlen(menuMantenimiento), 0); // envio el menu
		memset(buffer_me, 0, size_buffer + 1);
		recv(newSocket, buffer_me, size_buffer, 0);
		if (strcmp(buffer_me, "1") == 0) { // jugadores en estado activo
			jugadoresEnJuegoActivo(enunciado_enviar);
			send(newSocket, enunciado_enviar, sizeof(enunciado_enviar), 0);
			bzero(buffer_me, sizeof(buffer_me));
			recv(newSocket, buffer_me, SIZE_BUFF, 0); // lo recibo
		}
		else if(strcmp(buffer_me, "2") == 0) { // jugadores registrados
			jugadoresRegistrados(enunciado_enviar);
			send(newSocket, enunciado_enviar, sizeof(enunciado_enviar), 0);
			bzero(buffer_me, sizeof(buffer_me));
			recv(newSocket, buffer_me, SIZE_BUFF, 0); // lo recibo
		}
		else if(strcmp(buffer_me, "3") == 0) { // menu de preguntas
			menuPreguntas();
		}
		else if(strcmp(buffer_me, "4") == 0) { // ranking de partidas activas
			ranking(enunciado_enviar);
			send(newSocket, enunciado_enviar, sizeof(enunciado_enviar), 0);
			bzero(buffer_me, sizeof(buffer_me));
			recv(newSocket, buffer_me, SIZE_BUFF, 0); // lo recibo
		}
		else if (strcmp(buffer_me, "5") == 0) { // salir
			send(newSocket, SALIR, SIZE_BUFF, 0);
			printf("Desconexión de " Bold_Red "%s:%d " Bold_Magenta "[Mantenimiento]\n" Reset_Color, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
			close(newSocket);
			break;
		}
	}
}

void crearServidor() {
	char buffer[SIZE_BUFF];
	addr_size = sizeof(struct sockaddr_in);
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
			memset(buffer, 0, SIZE_BUFF);
			recv(newSocket, buffer, SIZE_BUFF, 0); // apenas se conecta un cliente, recibo el tipo de usuario que es (jugador o de mantenimiento)

			if (strcmp(buffer, JUGADOR) == 0) {
				gestionClienteJugador();
			}
			else if (strcmp(buffer, MANTENIMIENTO) == 0) {
				gestionClienteMantenimiento();
			}
		}
	}	
}

int main() {
	crearServidor();
	return 0;
}