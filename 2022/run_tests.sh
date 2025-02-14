#!/bin/bash

EXE="./challenge2022"
counter=0

FILES=(
"00-example.txt"
"01-the-cloud-abyss.txt"
"02-iot-island-of-terror.txt"
"03-etheryum.txt"
"04-the-desert-of-autonomous-machines.txt"
"05-androids-armageddon.txt"
)

for file in "${FILES[@]}"; do
    echo "Executing $EXE with $file..."
    $EXE < "$file" > "output0$counter.txt"

    ((counter++))
done