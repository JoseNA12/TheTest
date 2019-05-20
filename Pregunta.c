struct Opcion {
    int idRespuesta;
    char respuesta[256];
};

struct Pregunta {
    int idPregunta;
    char enunciado[256];
    int puntaje;
    struct Opcion opciones[3]; // struct Opcion *opciones[3];
};
