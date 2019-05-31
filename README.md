Proyecto 1 - The Test - Principios de Sistemas Operativos

---------------------------------------
	Proyecto 1 - The Test 
---------------------------------------

Implementación del Juego The Test (facebook messenger)

Este es un Juego multiusuario cliente-servidor, en el que debe usar herramientas básicas tales como sockets, semáforos, threads y otros que requiera necesarios.


	Aspectos logísticos
---------------------------------------

El proyecto debe realizarse y correr todo en Linux, programando el servidor en C; los clientes los puede programar en el lenguaje y plataformas libre sobre Linux que prefiera (consola, web, App, etc). Todas las comunicaciones al servidor se deben hacer sobre sockets. 


	Sobre la Implementación
----------------------------------------------

The Test es un juego de preguntas entre un par de usuarios, el cual hace un par de preguntas a un primer usuario, brindando 2 o 3 posibles respuestas, luego el usuario las contesta y el servidor guarda las respuestas y ahora pasa el turno de juego a un segundo jugador. Este segundo jugador verá las mismas preguntas y debe adivinar las respuestas del jugador 1 (ambos usuarios saben con quién están jugando). Por cada respuesta que adivine ambos ganan puntos, no hay penalidad por las respuestas fallidas. El jugador 2 recibe un par de preguntas nuevas que debe contestar, de igual manera se le presentan de 2 a 3 opciones para contestar según la pregunta planteada.

En el siguiente turno el primer usuario debe adivinar ahora las respuestas de las preguntas dadas al usuario 2, verá las respuestas adivinadas por el otro usuario sobre sus preguntas originales y recibirá un par de preguntas nuevas. De esta forma se le pasa el turno al otro usuario y tendrá el mismo proceso en estos 3 pasos: adivinar, ver respuestas adivinadas, contestar nuevas preguntas.

Los usuarios irán avanzando en niveles según los puntos que vayan ganando en cada pregunta acertada. Los valores de cada pregunta se deben definir al crear las preguntas.

+ Cada usuario en el sistema puede realizar múltiples juegos con diversos jugadores, uno a la vez. 
+ El programa servidor debe generar preguntas aleatorias en cada turno, para que el juego no lleve preguntas en orden y los usuarios no sepan de ante mano qué se va a preguntar a cada jugador. 
+ El programa servidor debe recordar las preguntas realizadas a cada par de usuarios y no repetir preguntas en la misma dirección usuario 1 a usuario 2, pero si se puede usar la misma pregunta en el sentido usuario 2 a usuario 1 y ya luego no debe usarse más entre este par de usuarios.
+ El programa servidor debe llevar el control de puntos entre cada par de usuarios.
+ El programa servidor debe llevar la sesión de cada juego, si una sesión se cierra a mitad del juego, al reconectarse el usuario, el programa deberá mostrarle de nuevo las preguntas y el paso por donde había quedado el juego 
+ Cada usuario jugará su turno una vez que el otro usuario haya contestado su parte, en caso contrario el usuario debe esperar la respuesta del otro usuario para continuar jugando; o bien seleccionar otro usuario disponible para jugar. 
+ El programa servidor puede ser consultado directamente para ver el estado de los juegos actuales en el sistema, jugadores en juego activo y estadísticas tales como : usuarios, número de preguntas, número de usos, aciertos y fallos totales del sistema y por cada pregunta, puntajes entre todos los pares de usuarios en juego como un ranking.
+ No es necesario hacer un sistema para ingreso de las preguntas al servidor, se considerará con 20 puntos extra si realiza un programa para dar mantenimiento remoto a las preguntas en el servidor: crear preguntas, con sus opciones y puntajes, visualizar las preguntas existentes, modificarlas y ver las estadísticas de cantidad de usos, aciertos y fallos asociadas a cada pregunta.
+ Para el día del demo deben haber ya una serie de jugadores registrados y jugadas realizadas entre estos, para poder ver las estadísticas, se harán partidas nuevas durante la revisión. Puede pedir a compañeros o amigos que usen su programa para registrar múltiples datos de juegos de varios usuarios.
+ Los tipos de preguntas permitidas son al estilo del juego The Test, las mismas no pueden ser ofensivas, discriminatorias o de datos personales, pues será posible ver las respuestas de los usuarios y no habrá control sobre la privacidad de los datos de los usuarios. 
+ El poder enviar un archivo de texto a un usuario por correo con las preguntas que ha contestado y sus respuestas con un usuario particular (es decir el histórico de juego entre un par de usuarios) será considerado con 20 puntos extra. El envío lo debe hacer el programa servidor usando comandos de Linux de forma transparente para el usuario. 
+ Si lo desea puede usar algún sistema tipo base de datos para almacenar la información de las preguntas, jugadores, puntos y respuestas; o bien puede usar archivos planos de texto, XML o similar y crear sus propia estructura de base de datos

----------------------------------------------
