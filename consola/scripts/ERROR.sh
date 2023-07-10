#!/bin/bash

../consola ../../tests/ERROR/ERROR_1 ../consola.config &
sleep 0.2; ../consola ../../tests/ERROR/ERROR_2 ../consola.config &
sleep 0.2; ../consola ../../tests/ERROR/ERROR_3 ../consola.config &
sleep 0.2; ../consola ../../tests/ERROR/ERROR_4 ../consola.config 