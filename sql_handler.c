
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include "sql_handler.h"
#include "Colores.c"

//........Estructuras.......
#include "Pregunta.c"
#include "Partida.c"
#include "Jugador.c"
//..........................


#define DB_PATH "./database/storage.db"


//===============Global Variables SQL Connection===============
sqlite3 *db;  // DB
sqlite3_stmt *res; // Statement
// ============================================================


// SQLite Commands: https://www.tutorialspoint.com/sqlite/sqlite_commands.htm

int initConnection() {
    int rc = sqlite3_open(DB_PATH, &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return -1;
    }
    return 0;
}

int setConnection(char* query) {
    int rc = sqlite3_prepare_v2(db, query, -1, &res, NULL);    
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    return rc;       
}

void endConnection(){
    sqlite3_finalize(res);
    sqlite3_close(db);
}


struct Pregunta* getPregunta(char* tuplaPreguntasUsadas){
    // select idUsuario from usuarios where idUsuario not in (1,2,5);
    int rc = setConnection("SELECT p.enunciado, p.puntaje FROM pregunta p where p.idPregunta NOT IN ?;");

    // bind an integer to the parameter placeholder. 
    rc = sqlite3_bind_text(res, 1, tuplaPreguntasUsadas, -1, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 

    struct Pregunta *preguntaJuego;
	preguntaJuego = (struct Pregunta*)malloc(sizeof(struct Pregunta));

	while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
		strcpy(preguntaJuego->enunciado,(char*)sqlite3_column_text(res, 0));
		preguntaJuego->puntaje = sqlite3_column_int(res, 1); 
	}

	printf("Pregunta %s - %d \n",preguntaJuego->enunciado, preguntaJuego->puntaje);

    return preguntaJuego;
}

struct Jugador* iniciarSesion(char* username, char* password){
    struct Jugador *usuarioJugador;
	usuarioJugador = (struct Jugador*)malloc(sizeof(struct Jugador));

    int rc = setConnection("SELECT u.idUSuario, u.nombreUsuario, u.correo FROM usuarios u where u.nombreUsuario = ? and u.contrasenia = ?;");
    
    rc = sqlite3_bind_text(res, 1, username, -1, 0);
    rc = sqlite3_bind_text(res, 2, password, -1, 0);

    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    }

    usuarioJugador->idJugador = 0; // en caso de no entrar el while, 0 indica que no encontró nada

    // Si usuario no existe no entra al while.
	while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
		usuarioJugador->idJugador = sqlite3_column_int(res, 0);
        printf(CYAN"%s"Reset_Color": ha solicitado ingresar sesión\n", sqlite3_column_text(res, 1));
        usuarioJugador->idJugador = sqlite3_column_int(res, 0); 
        strcpy(usuarioJugador->nombreUsuario, sqlite3_column_text(res, 1)); 
        strcpy(usuarioJugador->correo, (char*)sqlite3_column_text(res, 2));
    }
	
    //endConnection();

    return usuarioJugador;
}

int registrarUsuario(char* username, char* password, char* email) {

    int rc = setConnection("SELECT u.nombreUsuario FROM usuarios u where u.nombreUsuario == ?;");

    // bind an integer to the parameter placeholder. 
    rc = sqlite3_bind_text(res, 1, username, strlen(username), 0);
    
    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 

	if ((rc = sqlite3_step(res)) == SQLITE_ROW) {
		return 0; // el usuario ingresado esta opcupado
	}

    char *zErrMsg = 0;
    sqlite3_stmt *stmt;
    const char *pzTest;
    char *szSQL;

    // Insert data item into myTable
    szSQL = "INSERT INTO usuarios (nombreUsuario, contrasenia, correo) values (?,?,?)";

    rc = sqlite3_prepare(db, szSQL, strlen(szSQL), &stmt, &pzTest);

    if (rc == SQLITE_OK) {
        // bind the value 
        sqlite3_bind_text(stmt, 1, username, strlen(username), 0);
        sqlite3_bind_text(stmt, 2, password, strlen(password), 0);
        sqlite3_bind_text(stmt, 3, email, strlen(email), 0);

        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return 1; // se registró el usuario
    }
    return -1; // error
}

char *usuariosRegistrados(char* username) {
    int rc = setConnection("SELECT u.nombreUsuario FROM usuarios u where u.nombreUsuario != ?");

    // bind an integer to the parameter placeholder. 
    rc = sqlite3_bind_text(res, 1, username, -1, 0);

    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 

    char *usuarios; usuarios = malloc (sizeof (char) * 1024);
    strcat(usuarios, Bold_Blue);
    
    int cont = 0;

	while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
		strcat(usuarios, sqlite3_column_text(res, 0)); 
        
        if (cont < 4) {
            strcat(usuarios, "\t");
        }
        else {
            strcat(usuarios, "\n");  
        }
        cont += 1;
    }
    strcat(usuarios, Reset_Color);
	
    //endConnection();

    return usuarios;
}

int getIdJugador(char* nombreUsuario){
    int rc = setConnection("select idUsuario from usuarios where nombreUsuario = ?;");
    rc = sqlite3_bind_text(res, 1, nombreUsuario, -1, 0);

    if ((rc = sqlite3_step(res)) == SQLITE_ROW) {
		return sqlite3_column_int(res, 0);
	}
    return -1;

}
struct Partida verificarExistenciaPartida(int idJugador1, int idJugador2){
    struct Partida partida = {0,0,0,0,0};
    int rc = setConnection("select idPartida, idJugador1, idJugador2, nivel, puntos from partida where idJugador1 = ? AND idJugador2 = ?;");
    rc = sqlite3_bind_int(res, 1, idJugador1);
    rc = sqlite3_bind_int(res, 2, idJugador2);

    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);

    }
    // Si partida no existe no entra al while
    while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        partida.idPartida = sqlite3_column_int(res, 0); 
        partida.idJugador1 = sqlite3_column_int(res, 1); 
        partida.idJugador2 = sqlite3_column_int(res, 2);
        partida.nivel = sqlite3_column_int(res, 3);
        partida.puntaje = sqlite3_column_int(res, 4);
    }
    
    printf("Verificacion %d \n",partida.idPartida);
    printf("Verificacion %d \n",partida.idJugador1);
    printf("Verificacion %d \n",partida.idJugador2);
    printf("Verificacion %d \n",partida.nivel);
    printf("Verificacion %d \n",partida.puntaje);
    return partida;
}

struct Partida* iniciarPartida(char* idJugador1, char* idJugador2){
    int rc;
    struct Partida *partida;
	partida = (struct Partida*)malloc(sizeof(struct Partida));

    int idJugador2_Int = getIdJugador(idJugador2);
    int idJugador1_Int = getIdJugador(idJugador1);

    printf("ID jugador 1: %d \n",idJugador1_Int);
    printf("ID jugador 2: %d \n",idJugador2_Int);

    *partida = verificarExistenciaPartida(idJugador1_Int,idJugador2_Int);

    if(partida->idPartida == 0){ // Si no existe la partida crear una nueva
        rc = setConnection("INSERT INTO partida(idJugador1, idJugador2, nivel, puntos) values(?,?,?,?); SELECT idPartida, idJugador1, idJugador2, nivel, puntos FROM partida ORDER BY idPartida DESC LIMIT 1;");
        rc = sqlite3_bind_int(res, 1, idJugador1_Int);
        rc = sqlite3_bind_int(res, 2, idJugador2_Int);
        rc = sqlite3_bind_int(res, 3, 0);
        rc = sqlite3_bind_int(res, 4, 0);

        while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
            partida->idPartida = sqlite3_column_int(res, 0); 
            partida->idJugador1 = sqlite3_column_int(res, 1); 
            partida->idJugador2 = sqlite3_column_int(res, 2);
            partida->nivel = sqlite3_column_int(res, 3);
            partida->puntaje = sqlite3_column_int(res, 4);
        }
    }
	printf("Struct %d \n",partida->idPartida);
    printf("Struct %d \n",partida->idJugador1);
    printf("Struct %d \n",partida->idJugador2);
    printf("Struct %d \n",partida->nivel);
    printf("Struct %d \n",partida->puntaje);

    return partida;
}

char* armarTuplaPreguntadasUsadas(int idPartida){
char* preguntasUsadas;
int rc = setConnection("select p.idPregunta from pregunta p inner join [pregunta-respuesta] pp on p.idPregunta = pp.idPregunta inner join turnoJugador tj on pp.idPR = tj.idPR inner join partida par on tj.idPartida = par.idPartida where tj.idPartida = ?;"); 
    rc = sqlite3_bind_int(res, 1, idPartida);

    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    }

    while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        strcat(preguntasUsadas,sqlite3_column_text(res, 0)); 
        strcat(preguntasUsadas,"$");
    }
    return preguntasUsadas;
}

void retornarPreguntaTurno(int idPartida, int idJugador1, int idJugador2){
    /*
    Si no tiene ningún turno vaya y retorne una pregunta random de la BD y la agrega como primer turno.

    Si ya existen turnos, retorne el último y verifique el estado de la pregunta.
    */

    // Obtener último turno dado en una partida
    struct Pregunta *pregunta;
    int rc = setConnection("select idPP from turnoJugador where idPartida = ? order by idPP desc limit 1;"); 
    rc = sqlite3_bind_int(res, 1, idPartida);

    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    }

    // Solo entra aqui si existe turnos en esa partida. Osea es una partida que ya se había jugado
    if((rc = sqlite3_step(res)) == SQLITE_ROW){ 
        // Retornar pregunta y opciones que sea diferente a los anteriores turnos y la dirección que debería seguir, si es una pregunta para Jugador1 o Jugador2 (Estado Pregunta)

    }else{ // Si es partida nueva conseguir una pregunta y sus opciones para retornarla al cliente
        char* preguntasUsadas;
        preguntasUsadas = armarTuplaPreguntadasUsadas(idPartida);
        pregunta = getPregunta(preguntasUsadas);
    }
}