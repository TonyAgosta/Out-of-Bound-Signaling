#!/bin/bash
echo "Inizio test"

./supervisor 8 &> supervisorout.log &

echo "Supervisor e i relativi server avviati"
x=$!

sleep 2

echo "Eseguo i Client"

for(( i = 0 ; i < 10 ; ++i));do
	./client 5 8 20 >> clientout.log &
	./client 5 8 20 >> clientout.log &
	sleep 1
	
done

for((i = 0; i < 6 ; ++i ));do
	sleep 10
	echo "Segnale $i"
	kill -SIGINT $x
	
done

kill -SIGINT $x
kill -SIGINT $x

echo "Supervisor e Server chiusi"

bash ./misura.sh supervisorout.log clientout.log
