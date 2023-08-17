#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/time.h>

#define UNCHECK -1
#define CHECK  0
#define DELETE  1
#define CREATE  2
#define MODIFY  3

#define BUFFER_SIZE     1024
#define MAX_INFO_SIZE   2048
#define MAX_PATH 4096

typedef struct change_file { // 변경사항 구조체
	time_t time; // 변경 시간
	char name[MAX_PATH]; // 파일 이름
	int status; // 변경 상태 
} change_file;

typedef struct file_node{ // 모니터링 파일 목록 구조체
	char name[MAX_PATH]; // 파일 이름
	struct stat statbuf; // 파일 상태 정보
	struct dirent **namelist; // 디렉토리 경우 하위 파일 목록
	struct file_node *next; // 하위 디렉토리 파일 포인터
	struct file_node *child; // 같은 레벨의 다음 파일 포인터
	int status; // 모니터링 확인 상태
} file_node;

void monitoring(char *monitor_path, int sleep_time); // 모니터링 함수
file_node* make_list(char *path); // 디렉토리 파일 목록 트리화하는 함수
void compare_list(file_node *new_list, file_node *old_list); // 파일 목록 트리 비교하는 함수
int compare_file(file_node *new_file, file_node *old_file); // 파일 정보 비교하는 함수
int write_change_list(file_node *head, int change_cnt, int status); // 변경 사항 목록 작성하는 함수
void write_change_log(int change_cnt); // 변경사항 파일 기록하는 함수
void init_list_status(file_node *head, int status); // 모니터링 파일 상태 초기화하는 함수
file_node* make_node(); // 새로운 file_node 생성 함수 
void free_list(file_node *head); // 모니터링 파일 목록 메모리 할당 해제하는 함수
char* make_time_format(struct tm time); // 시간 형식 문자열 생성하는 함수
int ssu_daemon_init(char *monitor_list); // 데몬프로세스 설정