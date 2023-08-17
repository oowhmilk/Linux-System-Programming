ssu_monitor : ssu_monitor.o ssu_daemon.o
	gcc ssu_monitor.o ssu_daemon.o -o ssu_monitor

ssu_monitor.o : ssu_monitor.c ssu_monitor.h ssu_daemon.h
	gcc -c ssu_monitor.c

ssu_daemon.o : ssu_daemon.c ssu_daemon.h
	gcc -c ssu_daemon.c

clean :
	rm *.o
	rm ssu_monitor
