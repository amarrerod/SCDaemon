#!/bin/sh

prog="daemon"
path="/test_folder/"

start(){
 echo -n "Starting service\t"
 ./vagrant_data/daemon_monitor/daemon $path localhost 1890
}

stop(){
 echo -n $"Stopping $prog\t"
 pkill $prog
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
