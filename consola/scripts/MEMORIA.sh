#!/bin/bash

../consola ../../tests/MEMORIA/MEMORIA_1 ../consola.config &
sleep 3; ../consola ../../tests/MEMORIA/MEMORIA_2 ../consola.config &
sleep 3; ../consola ../../tests/MEMORIA/MEMORIA_3 ../consola.config
