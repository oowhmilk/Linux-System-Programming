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

#include "ssu_daemon.h"

char gwd[MAX_PATH]; // 현재 작업 디렉토리 저장 변수
char monitor_path[MAX_PATH]; // monitor 할 directory 경로 저장 변수
char log_path[MAX_PATH]; // "log.txt" file 경로 저장 변수
change_file change_list[BUFFER_SIZE]; // 변경 사항 저장 변수

void monitoring(char *monitor_path, int sleep_time)
{
	FILE *fp;
	file_node *old_list, *new_list; 
	int change_cnt;

	getcwd(gwd, MAX_PATH);

	//strcat(log_path, gwd);
	memset(log_path, 0, MAX_PATH);
	strcat(log_path, monitor_path);
	strcat(log_path, "/");
	strcat(log_path, "log.txt");

	if(access(log_path, F_OK) != 0)
    {
		if((fp = fopen(log_path, "w+")) < 0) 
		{
			fprintf(stderr, "fopen error for %s\n", log_path);
			exit(1);
		}
	}
	//fclose(fp);
	char monitor_list[MAX_PATH]; // montior_list.txt 경로 저장 변수
	memset(monitor_list, 0, BUFFER_SIZE);
	strcat(monitor_list, gwd);
	strcat(monitor_list, "/");
	strcat(monitor_list, "monitor_list.txt");

	ssu_daemon_init(monitor_list);

	old_list = make_list(monitor_path);

	while(1) 
	{
		change_cnt = 0;

		new_list = make_list(monitor_path); // 현재 파일 목록 및 상태 저장

		compare_list(new_list->child, old_list->child); // 파일 목록 트리 비교

		change_cnt = write_change_list(new_list->child, change_cnt, CREATE); // 생성된 파일 확인
		change_cnt = write_change_list(old_list->child, change_cnt, DELETE); // 삭제된 파일 확인

		write_change_log(change_cnt);

		free_list(old_list);

		old_list = new_list;
		init_list_status(old_list->child, UNCHECK);

		sleep(sleep_time);
	}

}

file_node* make_list(char *path) // 디렉토리 파일 목록 트리화하는 함수
{
	file_node *head, *current;
	int file_count;
	int is_dir = 1;
	
	head = make_node(); 
	current = head;

	strcpy(head->name, path);
	stat(head->name, &(head->statbuf));

	file_count = scandir(head->name, &(head->namelist), NULL, alphasort); 
	for(int i = 0; i < file_count; i++) 
	{

		if(!strcmp(head->namelist[i]->d_name, ".") || !strcmp(head->namelist[i]->d_name, ".."))
			continue;

		file_node *new = make_node(); 
		sprintf(new->name, "%s/%s", path, head->namelist[i]->d_name); 
		stat(new->name, &(new->statbuf));

		if(S_ISDIR(new->statbuf.st_mode))
		{
			new = make_list(new->name);
		} 

		if(is_dir) 
		{ 
			current->child = new;
			current = current->child;
			is_dir = 0;
		} 
		else 
		{
			current->next = new;
			current = current->next;
		}
	}

	return head;
}


void compare_list(file_node *new_list, file_node *old_list) // 파일 목록 트리 비교하는 함수
{
	file_node *current;

	if(new_list == NULL || old_list == NULL) // 둘중 하나라도 비교 대상이 존재하지 않을 경우
	{
		return;
	}

	current = old_list;

	while(current != NULL) 
	{	

		compare_file(new_list, current);

		if(current->child != NULL)
		{
			compare_list(new_list, current->child);
		}

		current = current->next;
	}
}

int compare_file(file_node *new_file, file_node *old_file) // 파일 정보 비교하는 함수
{
	file_node *current;

	current = new_file;

	while(current != NULL) {

		if(!strcmp(current->name, old_file->name)) // 해당 이름을 가진 파일이 기존에 이미 존재할 경우
		{
			current->status = CHECK;

			if(current->statbuf.st_mtime != old_file->statbuf.st_mtime) // 해당 파일이 수정되었을 경우
			{
					current->status = MODIFY;
			}
		
			old_file->status = CHECK;
			return 1;
		}

		if(current->child != NULL) // 디렉토리 안에 파일이 존재할 경우
		{
			if(compare_file(current->child, old_file)) 
				break;
		}

		current = current->next;
	}

	return 0;
}

int write_change_list(file_node *head, int change_cnt, int status) // 변경사항 목록 작성하는 함수
{
	file_node *current;

	current = head;

	while(current != NULL) 
	{
		if(current->status == UNCHECK)
		{
			change_list[change_cnt].time = time(NULL);
			strcpy(change_list[change_cnt].name, current->name);
			change_list[change_cnt].status = status;
			change_cnt++;
		}
		else if(current->status == MODIFY)
		{
			char temp_path[MAX_PATH];
			strcpy(temp_path, current->name);
			char** path_tokens = NULL; // 토큰들을 저장할 메모리 공간 동적 할당
			char* path_token = NULL; // 첫 번째 토큰 추출
			int path_token_count = 0; // 토큰의 개수를 저장하는 변수

			// 백업할 경로를 "/" 로 나누기 (나눠서 각 디렉토리 보기)
			path_tokens = (char**) malloc(sizeof(char*) * 256); // 토큰들을 저장할 메모리 공간 동적 할당
			path_token = strtok(temp_path, "/"); // 첫 번째 토큰 추출
			path_token_count = 0;
			
			while (path_token != NULL) // 토큰이 더 이상 없을 때까지 반복
			{ 
				path_tokens[path_token_count] = path_token; // 현재 토큰을 저장
				path_token_count++;
				path_token = strtok(NULL, "/"); // 다음 토큰 추출
			}

			if(strcmp(path_tokens[path_token_count - 1], "log.txt") == 0)
			{
				;
			}
			else
			{
				change_list[change_cnt].time = current->statbuf.st_mtime;
				strcpy(change_list[change_cnt].name, current->name);
				change_list[change_cnt].status = MODIFY;
				change_cnt++;
			}
		}
		

		if(current->child != NULL)
		{
			change_cnt = write_change_list(current->child, change_cnt, status);
		}
		
		current = current->next;
	}

	return change_cnt;
}

void write_change_log(int change_cnt) // 변경사항 파일 기록하는 함수
{
	char file_name[MAX_PATH];
	char *time_format;
	struct tm time;
	FILE *fp;
	char *tmp;

	if((fp = fopen(log_path, "rw+")) < 0) 
	{
		fprintf(stderr, "fopen error for %s\n", log_path);
		exit(1);
	}

	fseek(fp, 0, SEEK_END); 

	for(int i = 0 ; i < change_cnt ; i++) 
	{

		time = *localtime(&change_list[i].time);
		time_format = make_time_format(time);

		struct stat statbuf;
		stat(change_list[i].name, &statbuf);

		if(change_list[i].status == CREATE)
		{
			if(!S_ISDIR(statbuf.st_mode))
			{
				fprintf(fp, "[%s][%s][%s]\n", time_format, "create", change_list[i].name);
			}
		}
		else if(change_list[i].status == DELETE)
		{
			fprintf(fp, "[%s][%s][%s]\n", time_format, "remove", change_list[i].name);
		}
		else if(change_list[i].status == MODIFY)
		{
			if(!S_ISDIR(statbuf.st_mode))
			{
				fprintf(fp, "[%s][%s][%s]\n", time_format, "modify", change_list[i].name);
			}
		}	
	}
	fclose(fp);
}


void init_list_status(file_node *head, int status) // 모니터링 파일 상태 초기화하는 함수
{
	file_node *current;

	current = head;

	while(current != NULL) {

		current->status = status;

		if(current->child != NULL)  
			init_list_status(current->child, status); 

		current = current->next;
	}
}


file_node* make_node() // 새로운 file_node 생성하는 함수
{
	file_node *new = (file_node*)malloc(sizeof(file_node));

	memset(new->name, 0, BUFFER_SIZE);
	new->next = NULL;
	new->child = NULL;
	new->namelist = NULL;
	new->status = UNCHECK;

	return new;
}

void free_list(file_node *head) // 모니터링 파일 목록 메모리 할당 해제하는 함수
{
	if(head->child != NULL) 
	{
		free_list(head->child);
	}

	if(head->next != NULL) 
	{
		free_list(head->next);
	}

	free(head->namelist);
	free(head); 
}

char* make_time_format(struct tm time) // 시간 형식 문자열 생성하는 함수
{
	static char time_format[BUFFER_SIZE];

	sprintf(time_format, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
			time.tm_year + 1900,
			time.tm_mon + 1,
			time.tm_mday,
			time.tm_hour,
			time.tm_min,
			time.tm_sec);

	return (char*)time_format;
}


int ssu_daemon_init(char *monitor_list)
{
	pid_t pid, d_pid;
	int fd, maxfd, monitor_fd, log_fd;
	
	if((pid = fork()) < 0)
	{
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if(pid != 0)
	{
		exit(0);
	}

	d_pid = getpid();

	monitor_fd = open(monitor_list, O_WRONLY | O_APPEND);

	char process_id[10];
	sprintf(process_id, "%d", d_pid);
	write(monitor_fd, process_id, strlen(process_id));
	write(monitor_fd, "\n", 1);

	setsid();
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	maxfd = getdtablesize();

	for(fd = 0; fd < maxfd; fd++)
		close(fd);

	umask(0);
	chdir("/");
	fd = open("/dev/null", O_RDWR);
	dup(0);
	dup(0);

    return 0;
}