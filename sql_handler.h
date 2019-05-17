
#ifndef SQL_HANDLER
#define SQL_HANDLER 

int initConnection();
int setConnection(char* query);
void endConnection();
struct Pregunta* getPregunta();
struct Jugador* iniciarSesion(char* username, char* password);
int registrarUsuario(char* username, char* password, char* email);

char *usuariosRegistrados(char* username);

struct Partida* iniciarPartida(char* idJugador1, char* idJugador2);

#endif