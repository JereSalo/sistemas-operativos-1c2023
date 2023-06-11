#!/bin/bash

../consola ../../tests/DEADLOCK/DEADLOCK_1 ../consola.config &
sleep 1; ../consola ../../tests/DEADLOCK/DEADLOCK_2 ../consola.config &
sleep 1; ../consola ../../tests/DEADLOCK/DEADLOCK_3 ../consola.config