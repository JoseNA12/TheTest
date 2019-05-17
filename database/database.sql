create table pregunta(
    idPregunta INTEGER primary key AUTOINCREMENT,
    enunciado nvarchar(300),
    puntaje INTEGER
);

GO

create table [pregunta-respuesta](
    idPR INTEGER primary key AUTOINCREMENT,
    idPregunta INTEGER,
    respuesta nvarchar(300)
);

GO

create table usuarios(
    idUsuario INTEGER primary key AUTOINCREMENT,
    nombreUsuario nvarchar(50),
    contrasenia nvarchar(30),
    correo nvarchar(30)
);

GO

create table partida(
    idPartida INTEGER primary key AUTOINCREMENT,
    idJugador1 INTEGER,
    idJugador2 INTEGER,
    nivel INTEGER,
    puntos INTEGER
);

GO

create table turnoJugador(
    idPP INTEGER primary key AUTOINCREMENT,
    idPartida INTEGER,
    idJugador INTEGER,
    idPR INTEGER,
    idEstado INTEGER
);

GO

create table estadoPregunta(
    idEstado INTEGER primary key AUTOINCREMENT,
    nombre nvarchar(20)
);

