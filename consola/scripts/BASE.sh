#!/bin/bash

../consola ../../tests/BASE/BASE_1 ../consola.config &
sleep 1; ../consola ../../tests/BASE/BASE_2 ../consola.config &
sleep 1; ../consola ../../tests/BASE/BASE_2 ../consola.config