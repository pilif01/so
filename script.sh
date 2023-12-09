#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Utilizare: $0 <c>"
    exit 1
fi

filterChar=$1
correctSentences=0

while IFS= read -r sentence; do
    # Verifica daca propozitia indeplineste toate conditiile
    if [[ "$sentence" =~ ^[A-Z][A-Za-z0-9, .!?]+[?!.]$ && ! "$sentence" =~ ,.*\sși && "$sentence" =~ $filterChar ]]; then
        ((correctSentences++))
    fi
done

echo $correctSentences
