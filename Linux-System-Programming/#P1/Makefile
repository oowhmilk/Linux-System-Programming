all : ssu_backup ssu_backup_add ssu_backup_recover ssu_backup_remove ssu_backup_ls ssu_backup_vi ssu_backup_help

ssu_backup : ssu_backup.o 
	gcc -g ssu_backup.o -o ssu_backup

ssu_backup.o : ssu_backup.c
	gcc -c ssu_backup.c

ssu_backup_add : ssu_backup_add.o
	gcc -g ssu_backup_add.o -o ssu_backup_add -lssl -lcrypto

ssu_backup_add.o : ssu_backup_add.c 
	gcc -c ssu_backup_add.c

ssu_backup_remove : ssu_backup_remove.o
	gcc -g ssu_backup_remove.o -o ssu_backup_remove

ssu_backup_remove.o : ssu_backup_remove.c 
	gcc -c ssu_backup_remove.c

ssu_backup_recover : ssu_backup_recover.o
	gcc -g ssu_backup_recover.o -o ssu_backup_recover -lssl -lcrypto

ssu_backup_recover.o : ssu_backup_recover.c 
	gcc -c ssu_backup_recover.c

ssu_backup_ls : ssu_backup_ls.o
	gcc -g ssu_backup_ls.o -o ssu_backup_ls

ssu_backup_ls.o : ssu_backup_ls.c
	gcc -c ssu_backup_ls.c


ssu_backup_vi : ssu_backup_vi.o
	gcc -g ssu_backup_vi.o -o ssu_backup_vi

ssu_backup_vi.o : ssu_backup_vi.c
	gcc -c ssu_backup_vi.c


ssu_backup_help : ssu_backup_help.o
	gcc -g ssu_backup_help.o -o ssu_backup_help

ssu_backup_help.o : ssu_backup_help.c
	gcc -c ssu_backup_help.c
	

clean :
	rm ssu_backup.o
	rm ssu_backup_add.o
	rm ssu_backup_recover.o
	rm ssu_backup_remove.o
	rm ssu_backup_ls.o
	rm ssu_backup_vi.o
	rm ssu_backup_help.o