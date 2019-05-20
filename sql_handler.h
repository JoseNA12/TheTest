
#ifndef SQL_HANDLER
#define SQL_HANDLER
#include <sqlite3.h>

int initConnection();
int setConnection(char* query);
void endConnection();
struct Pregunta* getPregunta(char* tuplaPreguntasUsadas);
struct Jugador* iniciarSesion(char* username, char* password);
int registrarUsuario(char* username, char* password, char* email);

char* usuariosRegistrados(char pNombreUsuario[]);
int getIdJugador(char* nombreUsuario);
int comprobarMiTurno(int idPartida, int idJugador1);
char* armarTuplaPreguntadasUsadas(int idPartida);
struct Pregunta* getPreguntaTurnoAnterior(int idPartida);
struct Partida verificarExistenciaPartida(int idJugador1, int idJugador2);
struct Partida* crearNuevaPartida(int idJugador1, int idJugador2);
void insert_turno(int idPartida, int idJugador, int idPreguntaRespuesta);
struct Opcion *getRespuestaCorrecta(int idPartida, int idJugadorActual, int idPregunta);
void sumarPuntos(int idPartida, int idPR);
void establecerRespuestaAcertada(int idPR_Marcado, int idPartida);
#endif