
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
    int cache = sqlite3_enable_shared_cache(1);
    
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


struct Pregunta* getPregunta(char* tuplaPreguntasUsadas) {
    // select idUsuario from usuarios where idUsuario not in "(1,2,5)";   
    char p1[120] = "SELECT p.idPregunta, p.enunciado, p.puntaje FROM pregunta p where p.idPregunta NOT IN ";
    char p2[120] = " ORDER BY RANDOM() LIMIT 1;";

    strcat(p1, tuplaPreguntasUsadas); strcat(p1, p2);
    int rc = setConnection(p1);

    struct Pregunta *preguntaJuego;
    preguntaJuego = /*(struct Pregunta*)*/malloc(sizeof(struct Pregunta));

    if ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        preguntaJuego->idPregunta = sqlite3_column_int(res, 0);
        strcpy(preguntaJuego->enunciado,(char*)sqlite3_column_text(res, 1));
        preguntaJuego->puntaje = sqlite3_column_int(res, 2); 
    }
    sqlite3_finalize(res);

    rc = setConnection("SELECT pr.idPR, pr.respuesta FROM [pregunta-respuesta] pr where pr.idPregunta = ?;");
    rc = sqlite3_bind_int(res, 1, preguntaJuego->idPregunta);

    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 

    int indice = 0;
    while((rc = sqlite3_step(res)) == SQLITE_ROW) {
        preguntaJuego->opciones[indice].idRespuesta = sqlite3_column_int(res, 0); 
        strcpy(preguntaJuego->opciones[indice].respuesta,(char*)sqlite3_column_text(res, 1));
        indice += 1;
    }
    sqlite3_finalize(res);
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
	sqlite3_finalize(res);
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
        sqlite3_finalize(res);
		return 0; // el usuario ingresado esta opcupado
	}
    sqlite3_finalize(res);

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

char *usuariosRegistrados(char pNombreUsuario[]) {
    int rc = setConnection("SELECT u.nombreUsuario FROM usuarios u where u.nombreUsuario != ?");

    // bind an integer to the parameter placeholder. 
    rc = sqlite3_bind_text(res, 1, pNombreUsuario, -1, 0);

    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 

    char *usuarios; usuarios = malloc(sizeof (char) * 1024);
    int cont = 0;
    memset(usuarios, 0, strlen(usuarios) + 1);
    //bzero(usuarios, sizeof(usuarios));
    strcat(usuarios, "\n" Bold_Blue);

	while ((rc = sqlite3_step(res)) == SQLITE_ROW) {

		strcat(usuarios, sqlite3_column_text(res, 0));
        
        if (cont < 4) {
            strcat(usuarios, Reset_Color " | " Bold_Blue);
        }
        else {
            strcat(usuarios, "\n");
            cont = 0;
        }
        cont += 1;
    }
    strcat(usuarios, Reset_Color);
    sqlite3_finalize(res);

    return usuarios;
}

int getIdJugador(char* nombreUsuario){
    int rc = setConnection("select idUsuario from usuarios where nombreUsuario = ?;");
    rc = sqlite3_bind_text(res, 1, nombreUsuario, -1, 0);

    if ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        //printf("Jugador %s con ID de %d \n",nombreUsuario,sqlite3_column_int(res, 0));
        int id = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
        return id;
	}
    sqlite3_finalize(res);
    return -1;

}

struct Partida verificarExistenciaPartida(int idJugador1, int idJugador2) {
    struct Partida partida = {0,0,0,0,0};
    int rc = setConnection("select idPartida, idJugador1, idJugador2, nivel, puntos from partida where (idJugador1 = ? AND idJugador2 = ?) OR (idJugador2 = ? AND idJugador1 = ?);");
    rc = sqlite3_bind_int(res, 1, idJugador1);
    rc = sqlite3_bind_int(res, 2, idJugador2);
    rc = sqlite3_bind_int(res, 3, idJugador1);
    rc = sqlite3_bind_int(res, 4, idJugador2);

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
    sqlite3_finalize(res);

    /*printf("Verificacion %d \n",partida.idPartida); printf("Verificacion %d \n",partida.idJugador1);
    printf("Verificacion %d \n",partida.idJugador2); printf("Verificacion %d \n",partida.nivel);
    printf("Verificacion %d \n",partida.puntaje);*/
    return partida;
}

struct Partida* crearNuevaPartida(int idJugador1, int idJugador2) {
    struct Partida *partida;
	partida = /*(struct Partida*)*/malloc(sizeof(struct Partida));

    char *zErrMsg = 0;
    sqlite3_stmt *stmt;
    const char *pzTest;
    char *szSQL;

    szSQL = "INSERT INTO partida(idJugador1, idJugador2, nivel, puntos) values(?,?,?,?);";

    int rc = sqlite3_prepare(db, szSQL, strlen(szSQL), &stmt, &pzTest);

    if (rc == SQLITE_OK) { // bind the value
        sqlite3_bind_int(stmt, 1, idJugador1);
        sqlite3_bind_int(stmt, 2, idJugador2);
        sqlite3_bind_int(stmt, 3, 0);
        sqlite3_bind_int(stmt, 4, 0);

        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        *partida = verificarExistenciaPartida(idJugador1, idJugador2);
    }
    return partida;
}

struct Pregunta* getPreguntaTurnoAnterior(int idPartida) {
    struct Pregunta *preguntasRival; // 
    preguntasRival = malloc(sizeof(struct Pregunta) * 3); //malloc(sizeof(struct Pregunta) * 2);

    int i_preguntas, i_opciones;

    int rc = setConnection("select p.idPregunta, p.enunciado, p.puntaje from turnoJugador tp inner join [pregunta-respuesta] pr on pr.idPR = tp.idPR inner join pregunta p on p.idPregunta = pr.idPregunta where tp.idPartida = ? order by tp.idPP desc limit 2;"); 
    rc = sqlite3_bind_int(res, 1, idPartida);

    struct Pregunta *_struct_ptr = malloc(sizeof(struct Pregunta));
    preguntasRival[0] = *_struct_ptr;

    while((rc = sqlite3_step(res)) == SQLITE_ROW) {
        preguntasRival[i_preguntas].idPregunta = sqlite3_column_int(res, 0);
        strcpy(preguntasRival[i_preguntas].enunciado,(char*)sqlite3_column_text(res, 1));
        preguntasRival[i_preguntas].puntaje = sqlite3_column_int(res, 2); 
        i_preguntas += 1; 
    }

    sqlite3_finalize(res);

    i_preguntas = 0;
    i_opciones = 0;
    rc = setConnection("SELECT pr.idPR, pr.respuesta FROM [pregunta-respuesta] pr where pr.idPregunta = ?;");
    rc = sqlite3_bind_int(res, 1, preguntasRival[i_preguntas].idPregunta);
    while((rc = sqlite3_step(res)) == SQLITE_ROW) {
        preguntasRival[i_preguntas].opciones[i_opciones].idRespuesta = sqlite3_column_int(res, 0); 
        strcpy(preguntasRival[i_preguntas].opciones[i_opciones].respuesta,(char*)sqlite3_column_text(res, 1));
        i_opciones += 1;
    }

    sqlite3_finalize(res);

    i_preguntas = 1;
    i_opciones = 0;
    rc = setConnection("SELECT pr.idPR, pr.respuesta FROM [pregunta-respuesta] pr where pr.idPregunta = ?;");
    rc = sqlite3_bind_int(res, 1, preguntasRival[i_preguntas].idPregunta);
    while((rc = sqlite3_step(res)) == SQLITE_ROW) {
        preguntasRival[i_preguntas].opciones[i_opciones].idRespuesta = sqlite3_column_int(res, 0); 
        strcpy(preguntasRival[i_preguntas].opciones[i_opciones].respuesta,(char*)sqlite3_column_text(res, 1));
        i_opciones += 1;
    }
    sqlite3_finalize(res);

    return preguntasRival;
}

int comprobarMiTurno(int idPartida, int idJugadorActual) { 
    int ultimoJugador;
    int rc = setConnection("select idJugador from turnoJugador where idPartida = ? order by idPP desc limit 1;"); 
    rc = sqlite3_bind_int(res, 1, idPartida);

    if((rc = sqlite3_step(res)) == SQLITE_ROW) { 
        ultimoJugador = sqlite3_column_int(res, 0);
    }
    sqlite3_finalize(res);

    if(idJugadorActual == ultimoJugador){
        return 0; // Turno jugador actual
    }
    return 1; // Turno del jugador rival
    
}

char* armarTuplaPreguntadasUsadas(int idPartida) { // se retorna: () ó (1) ó (1, 4, ...)
    char* preguntasUsadas;
    int rc = setConnection("select p.idPregunta from pregunta p inner join [pregunta-respuesta] pp on p.idPregunta = pp.idPregunta inner join turnoJugador tj on pp.idPR = tj.idPR inner join partida par on tj.idPartida = par.idPartida where tj.idPartida = ?;"); 
    rc = sqlite3_bind_int(res, 1, idPartida);

    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    }

    preguntasUsadas = malloc(sizeof (char) * 1024);

    strcat(preguntasUsadas, "(");
    while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        strcat(preguntasUsadas,sqlite3_column_text(res, 0)); 
        strcat(preguntasUsadas,",");
    }
    sqlite3_finalize(res);
    
    if (preguntasUsadas[strlen(preguntasUsadas) - 1] == ',') {
        preguntasUsadas[strlen(preguntasUsadas) - 1] = '\0';
    }

    strcat(preguntasUsadas, ")");
    return preguntasUsadas;
}

void insert_turno(int idPartida, int idJugador, int idPreguntaRespuesta){
    int rc = setConnection("INSERT INTO turnoJugador(idPartida, idJugador, idPR) values (?,?,?)");
    rc = sqlite3_bind_int(res, 1, idPartida);
    rc = sqlite3_bind_int(res, 2, idJugador);
    rc = sqlite3_bind_int(res, 3, idPreguntaRespuesta);
    
    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 

	sqlite3_step(res);
    sqlite3_finalize(res);
}

struct Opcion *getRespuestaCorrecta(int idPartida, int idJugadorActual, int idPregunta) {
    struct Opcion *opcionCorrecta = /*{0,0};//*/(struct Opcion*)malloc(sizeof(struct Opcion));

    int rc = setConnection("select pr.idPR, pr.respuesta from turnoJugador tj inner join [pregunta-respuesta] pr on pr.idPR = tj.idPR inner join pregunta p on p.idPregunta = pr.idPregunta where tj.idPartida = ? and tj.idJugador != ? and p.idPregunta = ?;");
    rc = sqlite3_bind_int(res, 1, idPartida);
    rc = sqlite3_bind_int(res, 2, idJugadorActual);
    rc = sqlite3_bind_int(res, 3, idPregunta);

    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);

    }
    while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        opcionCorrecta->idRespuesta = sqlite3_column_int(res, 0); 
        strcpy(opcionCorrecta->respuesta, (char*)sqlite3_column_text(res, 1));
    }

    sqlite3_finalize(res);
    return opcionCorrecta;    
}

struct Partida *getPuntosYNivelPartida(int idPartida) {
    struct Partida *partida;
    partida = malloc(sizeof(struct Partida));

    int rc = setConnection("SELECT nivel, puntos FROM partida WHERE idPartida = ?;");
    rc = sqlite3_bind_int(res, 1, idPartida);
    
    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 

    while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        partida->nivel = sqlite3_column_int(res, 0); 
        partida->puntaje = sqlite3_column_int(res, 1); 
    }

    sqlite3_finalize(res);
    return partida;
}

void establecerRespuestaAcertada(int idPR_Marcado, int idPartida){
    int rc = setConnection("UPDATE turnoJugador SET resultadoTurno = 1 WHERE idPR = ? and idPartida = ?;");
    rc = sqlite3_bind_int(res, 1, idPR_Marcado);
    rc = sqlite3_bind_int(res, 2, idPartida);

    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 

    sqlite3_step(res);
    sqlite3_finalize(res);
}

void sumarPuntos(int idPartida, int puntajePregunta){
    int nivel, puntos, puntajeNuevo;

    int rc = setConnection("SELECT nivel, puntos FROM partida WHERE idPartida = ?;");
    rc = sqlite3_bind_int(res, 1, idPartida);
    
    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 

    while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        nivel = sqlite3_column_int(res, 0); 
        puntos = sqlite3_column_int(res, 1); 
    }

    sqlite3_finalize(res);

    puntajeNuevo = puntos + puntajePregunta;
    if(puntajeNuevo >= 100){
        puntajeNuevo -= 100;
        nivel += 1;
    }

    rc = setConnection("UPDATE partida SET puntos = ?, nivel = ? WHERE idPartida = ?;");
    rc = sqlite3_bind_int(res, 1, puntajeNuevo);
    rc = sqlite3_bind_int(res, 2, nivel);
    rc = sqlite3_bind_int(res, 3, idPartida);
    sqlite3_step(res);
    sqlite3_finalize(res);
}


/*
 El programa servidor puede ser consultado directamente para ver el estado de los juegos actuales en el sistema,
 jugadores en juego activo y estadísticas tales como : usuarios, número de preguntas, número de usos, aciertos y
 fallos totales del sistema y por cada pregunta, puntajes entre todos los pares de usuarios en juego como un ranking.
*/

void jugadoresEnJuegoActivo(char* enunciado_enviar){
    //bzero(enunciado_enviar, sizeof(enunciado_enviar));
    memset(enunciado_enviar, 0, strlen(enunciado_enviar) + 1);
    strcat(enunciado_enviar, "\n" Bold_Yellow);
	strcat(enunciado_enviar, "Jugadores");
	strcat(enunciado_enviar, Reset_Color);
    strcat(enunciado_enviar, Bold_Magenta"\nPartida | Jugador 1 | Jugador 2\n"Reset_Color);
    int rc = setConnection("select p.idPartida,u.nombreUsuario from partida p inner join usuarios u on u.idUsuario = p.idJugador1 OR u.idUsuario = p.idJugador2 order by p.puntos desc");
    int c = 0;
    while((rc = sqlite3_step(res)) == SQLITE_ROW) {
        if(c){ // Info del jugador 2
            strcat(enunciado_enviar, sqlite3_column_text(res, 1));
            strcat(enunciado_enviar, "\n");
            c = 0;
        }else{ // Info de la partida y jugador 1
            strcat(enunciado_enviar, sqlite3_column_text(res, 0));
            strcat(enunciado_enviar, " | ");
            strcat(enunciado_enviar, sqlite3_column_text(res, 1));
            strcat(enunciado_enviar, " | ");
            c+=1;
        } 
    }
    strcat(enunciado_enviar, "\nIngrese cualquier tecla para salir \n>>>");
    sqlite3_finalize(res);
}

void jugadoresRegistrados(char* enunciado_enviar){
    //bzero(enunciado_enviar, sizeof(enunciado_enviar));
    memset(enunciado_enviar, 0, strlen(enunciado_enviar) + 1);
    strcat(enunciado_enviar, "\n" Bold_Yellow);
	strcat(enunciado_enviar, "Jugadores registrados");
	strcat(enunciado_enviar, Reset_Color);
    strcat(enunciado_enviar, "\n");
    int rc = setConnection("select nombreUsuario from usuarios;");
    while((rc = sqlite3_step(res)) == SQLITE_ROW) {
        strcat(enunciado_enviar, sqlite3_column_text(res, 0));
        strcat(enunciado_enviar, "\n");
    }
    strcat(enunciado_enviar, "\nIngrese cualquier tecla para salir \n>>>");
    sqlite3_finalize(res);
}

int numeroPreguntas(){
    int rc = setConnection("select count(idPregunta) from pregunta;");
    int retorno = 0;
    while((rc = sqlite3_step(res)) == SQLITE_ROW){
        retorno = sqlite3_column_int(res, 0); 
    }
    sqlite3_finalize(res);
    return retorno;
}

int numeroPreguntasPorEstado(int estadoPregunta){ // Correcto o Incorrecto
    int retorno = 0;
    int rc = setConnection("select count(distinct tj.idPR) from turnoJugador tj where tj.resultadoTurno = ?;");
    rc = sqlite3_bind_int(res, 1, estadoPregunta);
    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 
    while((rc = sqlite3_step(res)) == SQLITE_ROW) {
        retorno = sqlite3_column_int(res, 0); 
    }
    sqlite3_finalize(res);
    return retorno;
}

void estadisticasPorPregunta(char* enunciado_enviar, int estadoPregunta){
    char temp[10]; 
    //bzero(enunciado_enviar, sizeof(enunciado_enviar));
    memset(enunciado_enviar, 0, strlen(enunciado_enviar) + 1);
    strcat(enunciado_enviar, "\n" Bold_Yellow);
	strcat(enunciado_enviar, "Estadísticas por pregunta");
	strcat(enunciado_enviar, Reset_Color);
    int rc = setConnection("select p.enunciado, count(tj.idPR) from pregunta p inner join [pregunta-respuesta] pr on pr.idPregunta = p.idPregunta inner join turnoJugador tj on tj.idPR = pr.idPR where tj.resultadoTurno = ? group by p.enunciado;");
    rc = sqlite3_bind_int(res, 1, estadoPregunta);
    if (rc != SQLITE_OK) {
        printf("Failed to bind parameter: %s\n\r", sqlite3_errstr(rc));
        sqlite3_close(db);
    } 
    while((rc = sqlite3_step(res)) == SQLITE_ROW) {
        strcat(enunciado_enviar, "\nPregunta: ");
        strcat(enunciado_enviar, sqlite3_column_text(res, 0));
        if(estadoPregunta){
            strcat(enunciado_enviar, "\nRespuestas correctas: ");
        }else{
            strcat(enunciado_enviar, "\nRespuestas incorrectas: ");
        }
        memset(temp, 0, 10);
        sprintf(temp, "%d", sqlite3_column_int(res, 1)); 
        strcat(enunciado_enviar, temp);
        strcat(enunciado_enviar, "\n");
    }
    sqlite3_finalize(res);
}

void ranking(char* enunciado_enviar){
    //bzero(enunciado_enviar, sizeof(enunciado_enviar));
    memset(enunciado_enviar, 0, strlen(enunciado_enviar) + 1);
    strcat(enunciado_enviar, "\n" Bold_Yellow);
	strcat(enunciado_enviar, "Ranking");
	strcat(enunciado_enviar, Reset_Color);
    strcat(enunciado_enviar, Bold_Magenta "\nPartida | Jugador 1 | Jugador 2 | Nivel | Puntos \n" Reset_Color);
    int rc = setConnection("select p.idPartida,u.nombreUsuario, p.nivel, p.puntos from partida p inner join usuarios u on u.idUsuario = p.idJugador1 OR u.idUsuario = p.idJugador2 order by p.puntos, p.nivel desc");
    int c = 0;
    int rank = 1;
    while((rc = sqlite3_step(res)) == SQLITE_ROW) {
        if(c){ // Info del jugador 2, nivel y puntos
            strcat(enunciado_enviar, sqlite3_column_text(res, 1));
            strcat(enunciado_enviar, " | ");
            strcat(enunciado_enviar, sqlite3_column_text(res, 2));
            strcat(enunciado_enviar, " | ");
            strcat(enunciado_enviar, sqlite3_column_text(res, 3));
            strcat(enunciado_enviar, "\n");
            c = 0;
            rank += 1;
        }else{ // Info de la partida y jugador 1
            strcat(enunciado_enviar, "[");
            char temp[10]; 
		    sprintf(temp, "%d", rank); 
		    strcat(enunciado_enviar, temp);
            strcat(enunciado_enviar, "] ");
            strcat(enunciado_enviar, sqlite3_column_text(res, 0));
            strcat(enunciado_enviar, " | ");
            strcat(enunciado_enviar, sqlite3_column_text(res, 1));
            strcat(enunciado_enviar, " | ");
            c+=1;
        } 
    }
    strcat(enunciado_enviar, "\nIngrese cualquier tecla para salir \n>>> ");
    sqlite3_finalize(res);
}
