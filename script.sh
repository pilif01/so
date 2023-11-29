#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Utilizare: $0 <c>"
    exit 1
fi

filterChar=$1
correctSentences=0

while IFS= read -r sentence; do
    # Verifica daca propozitia incepe cu litera mare
    if [[ "$sentence" =~ ^[A-Z] ]]; then
        # Verifica daca propozitia contine doar caractere valide
        if [[ "$sentence" =~ ^[A-Za-z0-9, .!?]+$ ]]; then
            # Verifica daca propozitia se termina cu ? sau ! sau .
            if [[ "$sentence" =~ [?!.]$ ]]; then
                # Verifica daca propozitia nu contine virgula (,) inainte de si
                if [[ ! "$sentence" =~ ,.*\sși ]]; then
                    # Verifica daca propozitia contine caracterul <c>
                    if [[ "$sentence" =~ $filterChar ]]; then
                        ((correctSentences++))
                    fi
                fi
            fi
        fi
    fi
done

echo $correctSentences
