#define BUFFER_SIZE 1024
#define MAX_PATH 4096

int get_command(char *command, int argc, char (*argv)[BUFFER_SIZE]); // 내장 명령어 구하는 함수
void add(char (*argv)[BUFFER_SIZE]); // add 실행 함수
int ssu_daemon_init(char *monitor_list); // daemon process 생성함수 
int exist_directory(char *dir_path); // directory 존재 여부 확인하는 함수
int check_monitoring_dir(char *dir_path); // monitor_list.txt 에 존재 여부 확인하는 함수

void delete(char (*argv)[BUFFER_SIZE]); // delete 실행 함수
int check_monitoring_pid(char *pid); // monitor_list.txt 에 존재 여부 확인하는 함수
void ssu_signal_handler(int signo);
void delete_monitor_list(char *delete_line); // monitor_list.txt 에서 해당 줄 삭제하기 

void tree(char (*argv)[BUFFER_SIZE]); // tree 실행 함수 
void draw_tree(char *dir_path, int level); // tree 그리는 함수
void help();