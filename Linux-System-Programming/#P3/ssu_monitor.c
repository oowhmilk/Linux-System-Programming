#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <sys/wait.h>

#include "ssu_monitor.h"
#include "ssu_daemon.h"

int tOption;
char temp[MAX_PATH]; // 임시 경로 저장 변수
char cwd[MAX_PATH]; // 현재 작업 디렉토리 경로 저장 변수
char monitor_list[MAX_PATH]; // montior_list.txt 경로 저장 변수

int main(void)
{

	getcwd(cwd, sizeof(cwd));

    memset(temp, 0, sizeof(temp));
    strcat(temp, cwd);
    strcat(temp, "/monitor_list.txt");
    strcpy(monitor_list, temp);

	
	while(1)
	{
		char command[BUFFER_SIZE]; 
		char argv[10][BUFFER_SIZE];
		int argc = 0;

		memset(argv, 0, sizeof(argv));

		printf("20192393>");
		fgets(command, sizeof(command), stdin);

		argc = get_command(command, argc, argv);

		int i = 0;

        while(i < argc)
        {
            if(argv[i][0] == '-')
			{
				int c = argv[i][1];

				switch(c)
				{
					case 't':
                    {
                        tOption = 1;
                        break;
                    }
			    }
            }

            i++;
        }

        if(strcmp(argv[0], "add") == 0)
        {
            if(argc < 2) // 첫번째 입력이 안 들어온 경우
            {
                printf("Usage : add <DIRPATH> [OPTION]\n");
            }
            else
            {
                add(argv);
            }
        }
        else if(strcmp(argv[0], "delete") == 0)
        {   
            if(argc < 2) // 첫번째 입력이 안 들어온 경우
            {
                printf("Usage : delete <DAEMON_PID>\n");
            }
            else
            {
                delete(argv);
            }
        }
        else if(strcmp(argv[0], "tree") == 0)
        {
            if(argc < 2) // 첫번째 입력이 안 들어온 경우
            {
                printf("Usage : tree <DIRPATH>\n");
            }
            else
            {
                tree(argv);
            }
        }
        else if(strcmp(argv[0], "help") == 0)
        {
            help();
        }
        else if(strcmp(argv[0], "exit") == 0)
        {
            break;
        }
        else if(strcmp(argv[0], "\0") == 0)
        {
            ;
        }
        else
        {
            help();
        }
    }

	exit(0);
}



int get_command(char *command, int argc, char (*argv)[BUFFER_SIZE])
{
	for(int i = 0; i < (int)strlen(command) ; i++)
	{
		for(int j = 0;;j++, i++)
		{
			if (command[i] == ' ' || command[i] == '\n')
			{
				argc++;
				argv[argc][j] = '\0';
				break;
			}

			if(command[i] == '\n')
				break;

			argv[argc][j] = command[i];
		}
	}

	return argc;
}


void add(char (*argv)[BUFFER_SIZE]) // add 실행 함수
{
    char dir_path[BUFFER_SIZE];
    memset(dir_path, 0, sizeof(dir_path));
    strcpy(dir_path, argv[1]);

    char temp[MAX_PATH];
    memset(temp, 0, sizeof(temp));

    // 상대경로 입력시
    if (argv[1][0] != '/')
    {
        strcat(temp, cwd);
        strcat(temp, "/");
        strcat(temp, dir_path);
    }
    // 절대경로 입력 시
    else 
    {
        strcpy(temp, dir_path);
    }

    int sleep_time = 0 ;

    if(tOption)
    {
        sleep_time = atoi(argv[3]);
        tOption = 0;
    }
    else
    {
        sleep_time = 1;
        tOption = 0;
    }

    if(sleep_time < 1)
    {
        printf("Usage : add <DIRPATH> [OPTION]\n");
    }
    else
    {
        if(exist_directory(temp)) // 인자로 입력한 directory 존재할 경우
        {
            if(check_monitoring_dir(temp)) // monitor_list.txt 에 존재하는 경우
            {
                printf("%s exists in monitor_list.txt\n", temp);
            }
            else
            {
                char monitor_path[MAX_PATH];
                snprintf(monitor_path, MAX_PATH,"%s", temp);

                int fd = open(monitor_list, O_WRONLY | O_APPEND);
                write(fd, monitor_path, strlen(monitor_path));
                write(fd, " ", 1);

                pid_t pid;
                pid_t cpid;
                int status;

                if((pid = fork()) < 0)
                {
                    printf("fork error\n");
                    exit(1);
                }
                else if(pid == 0)
                {
                    monitoring(temp, sleep_time);
                }
                else
                {
                    wait(&status);
                }

                close(fd);
                
                printf("monitoring started (%s)\n", temp);
            }
        }
        else
        {
            printf("Usage : add <DIRPATH> [OPTION]\n");
        }
    }

    
}

int exist_directory(char *dir_path) // directory 존재 여부 확인하는 함수
{
    int return_num = 0;

	// check exist
	if(access(dir_path, F_OK) == 0)
    {
        struct stat statbuf;

        if (stat(dir_path, &statbuf) < 0) {
            fprintf(stderr, "stat error\n");
            exit(1);
        }

        if(S_ISDIR(statbuf.st_mode))
        {
            return_num = 1;
        }
    }

    return return_num;
}

int check_monitoring_dir(char *dir_path) // monitor_list.txt 에 존재 여부 확인하는 함수
{

    if(access(monitor_list, F_OK) == 0)
    {
        FILE *fp = fopen(monitor_list, "r");

        char line[BUFFER_SIZE];
        while (fgets(line, sizeof(line), fp)) 
        {
            char* token;
            token = strtok(line, " ");

            char check_token[BUFFER_SIZE];
            strcpy(check_token, token);
            strcat(check_token, "/");

            if(strcmp(token, dir_path) == 0) 
            {
                fclose(fp);
                return 1;
            }

            if(strstr(check_token, dir_path) != NULL) // 포함여부 확인
            {
                char check_dir[MAX_PATH];
                strcpy(check_dir, dir_path);
                strcat(check_dir, "/");
                
                if(strstr(check_token, check_dir) != NULL)
                {
                    fclose(fp);
                    return 1;
                }
            }
            else if(strstr(dir_path, check_token) != NULL) // 포함여부 확인
            {
                fclose(fp);
                return 1;
            }

            
        }
        fclose(fp);
    }
    else
    {
        open(monitor_list, O_CREAT | O_WRONLY, 0644);
    }

    return 0;   
    
}

void delete(char (*argv)[BUFFER_SIZE]) // delete 실행 함수
{
    char input[BUFFER_SIZE];
    strcpy(input, argv[1]);

    if(check_monitoring_pid(input)) // 인자로 입력한 프로세스 번호가 존재할 경우
    {
        int delete_pid;
        delete_pid = atoi(input);

        struct sigaction sig_act;
        sigset_t sig_set;

        sigemptyset(&sig_act.sa_mask);
        sig_act.sa_flags = 0; 
        sig_act.sa_handler = ssu_signal_handler; 
        sigaction(SIGUSR1, &sig_act, NULL); 
        kill(delete_pid, SIGUSR1);

    }
    else
    {
        printf("%s doesn't exist process number\n",input);
    }


}

int check_monitoring_pid(char *pid) // monitor_list.txt 에 존재 여부 확인하는 함수
{

    if(access(monitor_list, F_OK) == 0)
    {
        FILE *fp = fopen(monitor_list, "r");

        char line[BUFFER_SIZE];
        char delete_line[BUFFER_SIZE];

        while (fgets(line, sizeof(line), fp)) 
        {
            strcpy(delete_line, line);
            char *result;
            char* token = strtok(line, " ");

            while (token != NULL) {
                result = token;
                token = strtok(NULL, " ");
            }

            if (result[strlen(result) - 1] == '\n') 
            {
                result[strlen(result) - 1] = '\0';
            }         
        
            if(strcmp(pid, result) == 0)
            {
                delete_monitor_list(delete_line);
                printf("monitoring ended (%s)\n", line);
                return 1;
            }
        }

        fclose(fp);
    }

    return 0;   
}

void ssu_signal_handler(int signo)
{
    ;
}

void delete_monitor_list(char *delete_line) // monitor_list.txt 에서 해당 줄 삭제하기 
{
    if(access(monitor_list, F_OK) == 0)
    {
        FILE *fp_monitor;
        FILE *fp_temp;

        if((fp_monitor = fopen(monitor_list, "r")) < 0) 
        {
            fprintf(stderr, "fopen error for %s\n", monitor_list);
            exit(1);
        }

        if((fp_temp = fopen("temp.txt", "w")) < 0) 
        {
            fprintf(stderr, "fopen error for %s\n", "temp.txt");
            exit(1);
        }

        char line[BUFFER_SIZE];

        while (fgets(line, sizeof(line), fp_monitor)) 
        {
            if(!strcmp(line, delete_line)) // 삭제할 줄이랑 같을 경우
            {
                continue;
            }
            else
            {
                fprintf(fp_temp, "%s", line);
            }
        }

        fclose(fp_monitor);
        fclose(fp_temp);

        if (remove(monitor_list) != 0) 
        {
            fprintf(stderr, "remove error for %s\n", monitor_list);
            exit(1);
        }

        if (rename("temp.txt", monitor_list) != 0) 
        {
            fprintf(stderr, "rename error\n");
            exit(1);
        }

    }
}


void tree(char (*argv)[BUFFER_SIZE]) // tree 실행 함수
{
    char dir_path[BUFFER_SIZE];
    memset(dir_path, 0, sizeof(dir_path));
    strcpy(dir_path, argv[1]);

    char temp[MAX_PATH];
    memset(temp, 0, sizeof(temp));

    // 상대경로 입력시
    if (argv[1][0] != '/')
    {
        strcat(temp, cwd);
        strcat(temp, "/");
        strcat(temp, dir_path);
    }
    // 절대경로 입력 시
    else 
    {
        strcpy(temp, dir_path);
    }


    if(check_monitoring_dir(temp)) // monitor_list.txt 에 존재 여부 확인하는 함수
    {
        printf("%s\n",argv[1]);
        draw_tree(temp, 0);
    }
    else
    {
        printf("%s doesn't exist directory path\n", temp);
    }
}


void draw_tree(char *dir_path, int level) // tree 그리는 함수
{
    DIR *dir;
    struct dirent *dentry;

    if ((dir = opendir(dir_path)) == NULL) 
    {
        fprintf(stderr, "opendir error\n");
        return;
    }

    while ((dentry = readdir(dir)) != NULL) 
    {
        if (strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name, "..") == 0)
            continue;

        for (int i = 0; i < level - 1; i++) 
        {
            printf("|     ");
        }

        if (level > 0) 
        {
            printf("|---- ");
        }

        printf("%s\n", dentry->d_name);

        if (dentry->d_type == DT_DIR) 
        {
            char new_path[BUFFER_SIZE];
            snprintf(new_path, sizeof(new_path), "%s/%s", dir_path, dentry->d_name);
            draw_tree(new_path, level + 1);
        }
    }

    closedir(dir);
}



void help()
{
	printf("Usage : add <DIRPATH> [OPTION]\n");
    printf("Usage : delete <DAEMON_PID>\n");
    printf("Usage : tree <DIRPATH>\n");
    printf("Usage : help\n");
    printf("Usage : exit\n");
}