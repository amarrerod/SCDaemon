#!/bin/sh

prog="server"

start(){
 echo -n "Starting server\t"
 /vagrant_data/daemon_monitor/server 1890
}

stop(){
 echo -n $"Stopping $prog\t"
 pkill $prog
 fuser -k 1890
 echo
}

restart(){
	stop
	start
}

status(){
	status $prog
}

case "$1" in
 start)
  start
  ;;
 stop)
  stop
  ;;
 restart)
	restart
	;;
 *) 
	echo $"Usage: $0 {start|stop|restart}"
  exit 2
esac

exit 0
