#!/bin/bash
echo "Inizio test"

./supervisor 8 &>supervisorout.log &

echo "Supervisor e i relativi server avviati"
x=$!

sleep 2

echo "Eseguo i Client"
#inizio un ciclo dove faccio partire coppie di client a distanza di un secondo l'una dall'altra
for ((i = 0; i < 10; ++i)); do
	./client 5 8 20 >>clientout.log &
	./client 5 8 20 >>clientout.log &
	sleep 1

done

#inizio il cliclo dove ogni 10 secondi mando un sigint in modo che il supervisor possa stampare le stime ricevute fino a quel momento
for ((i = 0; i < 6; ++i)); do
	sleep 10
	time=$(($i + 1))
	echo "Sono trascorsi $(($time * 10)) secondi"
	kill -SIGINT $x

done

kill -SIGINT $x
kill -SIGINT $x

echo "Supervisor e Server chiusi"

bash ./misura.sh supervisorout.log clientout.log
