#!/bin/bash
echo "inizio test"

./supervisor 8 &> supervisorout.log &

echo "dopo supervisor avviato"
x=$!

sleep 2

echo "partono i client"

for((i=0;i<5;i++));do
	./client 5 8 20 >> clientout.log &
	./client 5 8 20 >> clientout.log &
	sleep 1
	
done

for((i=0;i<6;i++));do
	kill -SIGINT $x
	sleep 10
	
done

kill -SIGINT $x
sleep 0.5
kill -SIGINT $x
