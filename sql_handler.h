
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
struct Partida *getPuntosYNivelPartida(int idPartida);
void ranking(char* enunciado_enviar);
void jugadoresRegistrados(char* enunciado_enviar);
void jugadoresEnJuegoActivo(char* enunciado_enviar);
int numeroPreguntasPorEstado(int estadoPregunta);
int numeroPreguntas();
void estadisticasPorPregunta(char* enunciado_enviar, int estadoPregunta);
int crearPregunta(char* enunciado, char* opcion1, char* opcion2, char* opcion3, int puntaje);
void insertOpcion(int idPregunta, char* opcion);
int updatePregunta(int idPregunta, char* enunciado, int puntaje);
int deletePregunta(int idPregunta);
void selectOpciones(int idPregunta, char* enunciado_enviar);
void selectAllPregunta(char* enunciado_enviar);
#endif