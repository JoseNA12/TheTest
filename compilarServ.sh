#!/bin/sh
gcc servidor.c sql_handler.c -lsqlite3 -o servidor
./servidor