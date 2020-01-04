#!/bin/bash/

#Tony Agosta 544090

echo "Inizia la misura"

MAXERR=25              # una stima può differire al massimo di 25 unità
esatte=0               #per contaare il numero di stime esatte
errore=0               #per calcolare l'errore totale dato dalla somma degli errori per ogni singola stima
nerrori=0              #per contare il nuemro di stime errate
errmedio=0             #per calcolare l'errore medio
stime=0                #per contare il numero di stime fatte

declare -A arraystime  #array di stringhe per associare ad ogni client la stima del supervisor
declare -A arraysecret #array di stringhe per associare ad client il suo secret

for FILE in $@; do
	while read line; do
		RIGA=($line) #alla variabile di tipo stringa assegno volta per volta l'intera riga letta
		case $line in
		*"BASED "*) arraystime[${RIGA[4]}]=${RIGA[2]} ;; #associo il secret stimato dal supervisor al client
		"CLIENT "*" SECRET "*) arraysecret[${RIGA[1]}]=${RIGA[3]} ;; #associo il secret effettivo del client all id del client stesso
		esac
	done <$FILE #uso il file in input come argomento del while
done

for I in "${!arraysecret[@]}"; do
	Y=$((arraystime[$I] - arraysecret[$I])) #faccio la differenza tra il secret stimato dal supervisor per un client e il secret del client
	if (($Y < 0)); then
		Y=$((-$Y)) #se la stima e` negativa cambio il segno
	fi
	if (($Y <= $MAXERR)); then #se la differenza e` minore o uguale di 25 allora incremento il numero di stime esatte
		esatte=$(($esatte + 1))
	fi
	if (($Y > $MAXERR)); then
		nerrori=$(($nerrori + 1)) #se la differenza e` maggiore di 25 allora incremento il numero di stime errate
	fi
	errore=$(($errore + $Y)) #l'errore totale e` dato da tutti gli errori per ogni singola stima
	stime=$(($stime + 1))    #incremento il numero delle stime effettuate
done

errmedio=$(($errore / $stime)) #l'errore medio e` dato dall'errore totale diviso il numero di stime

echo "stime effetuate: " $stime
echo "stime esatte: " $esatte
echo "errore medio: " ${errmedio}
