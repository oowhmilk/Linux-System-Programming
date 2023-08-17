#ifndef MAIN_H_
#define MAIN_H_

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef STDOUT
	#define STDOUT 1
#endif
#ifndef STDERR
	#define STDERR 2
#endif
#ifndef TEXTFILE
	#define TEXTFILE 3
#endif
#ifndef CFILE
	#define CFILE 4
#endif
#ifndef OVER
	#define OVER 5
#endif
#ifndef WARNING
	#define WARNING -0.1
#endif
#ifndef ERROR
	#define ERROR 0
#endif

#define FILELEN 128
#define BUFLEN 1024
#define SNUM 100
#define QNUM 100
#define ARGNUM 5

#define MAX_PATH 4096

struct ssu_scoreTable{
	char qname[FILELEN];
	double score;
};

void ssu_score(int argc, char *argv[]);
int check_option(int argc, char *argv[]); // 옵션 처리하는 함수
void print_usage(); // Usage 출력 함수

void score_students(); // 모든 학생들의 총점으로 평균을 출력하는 함수
void new_file_score_students(char* new_score_file); // 모든 학생들의 총점으로 평균을 new_score_file 에 출력하는 함수
double score_student_sorting(char* buf, char *id); // 학생 디렉토리에 존재하는 file 모두 검사후 점수 매겨서 buf 에 저장하는 함수
double score_student(int fd, char *id); // 학생 디렉토리에 존재하는 file 모두 검사후 점수 매기는 함수
void write_first_row(int fd); // open 한 파일에 첫 줄에 문제 번호들과 "sum" 작성하는 함수

char *get_answer(int fd, char *result); // 작성한 정답을 get 하는 함수
int score_blank(char *id, char *filename); // 인자로 들어오는 학번, ".txt" filename 으로 만든 file 이 정답 맞았는지 확인하는 함수
double score_program(char *id, char *filename); // 인자로 넣어준 id, filename 으로 compile 하고 결과값으로 프로그램 점수 매기는 함수
double compile_program(char *id, char *filename); // 인자로 들어온 학번, filename 의 ".c" 코드를 compile 해서 compile 결과를 저장하는 함수
int execute_program(char *id, char *filname); // 인자로 넣은 학번, filename 으로 file 생성해서 프로그램 실행해서 출력 결과값을 정답 프로그램 출력 결과값 비교하는 함수
pid_t inBackground(char *name); // "ps | grep 문제번호.stdexe" 형태로 명령어 실행한 해당 프로세스의 PID 값을 return 하는 함수
double check_error_warning(char *filename); // 인자로 들어온 file 내용에 error, warning 검사하는 함수
int compare_resultfile(char *file1, char *file2); // 인자로 들어온 두 file 내의 값을 하나씩 비교해서 같은지 여부 확인하는 함수 == 다를 경우 false , 같을 경우 true

void do_iOption(char (*ids)[FILELEN]); // "-i" 옵션 실행하는 함수
void do_mOption(); // "-m" 옵션 처리 함수

// 구현 추가 함수
void do_nOption(char* new_score_file); // "-n" 옵션 처리 함수
void do_pOption(char *check_id); // "-p" 옵션 실행하는 함수
void do_eOption(); // "-e" 옵션 실행하는 함수
int is_exist(char (*src)[FILELEN], char *target);
void insert_problem(char* id_num, char* problem_num, double score); // 문제 링크드 리스트로 만들기 
void print_list_problem(); // 문제 링크드 리스트 값 확인하는 함수
void free_linked_list(); // 링크드 리스트 free 하는 함수
void insert_student(char *id, char *buf, double score); // 학생 링크드 리스트로 만들기 
void print_list_problem(char *id); // 학생 링크드 리스트 값 확인하는 함수
int count_list_student(); // 학생 링크드 리스트 개수 
void sort_score_ascending(); // 학생 링크드 리스트를 점수로 오름차순 정렬
void sort_score_descending(); // 학생 링크드 리스트를 점수로 내림차순 정렬
void sort_id_descending(); // 학생 링크드 리스트를 학번으로 내림차순 정렬
void save_sorting_score(); // 정렬한 값을 ".csv" file 에 저장하는 함수
int check_student_dir(char *stuDir, char (*ids)[FILELEN]); // 인자로 넣은 학번이 존재하는 학번인지 확인하는 함수
double return_score_student(char *id); // 학생 디렉토리에 존재하는 file 모두 검사후 점수 매기는 함수

int is_thread(char *qname); // threadFiles 에 인자로 넣어준 파일과 같은 이름이 있는 file 유무 확인하는 함수
void redirection(char *command, int newfd, int oldfd); // 기존 출력을 저장하는 old file descriptor 에서 new file descriptor 로 에 출력을 저장
int get_file_type(char *filename); // 인자로 들어온 파일 유형 확인하는 함수
void rmdirs(const char *path); // path 경로 내에 있는 디렉토리와 파일 모두 삭제
void to_lower_case(char *c); // 소문자로 바꾸는 함수

void set_scoreTable(char *ansDir); // score_table 을 set 하는 함수
void read_scoreTable(char *path); // score_table 에 qname, 점수 저장 함수
void make_scoreTable(char *ansDir); // 인자로 받은 ansDir 로 score_table 만드는 함수
void write_scoreTable(char *filename); // score_table 에 있는 값을 인자로 들어온 filename 의 file 에 작성하는 함수
void set_idTable(char *stuDir); // id_table 을 set 하는 함수
int get_create_type(); // score_table 유형 정하는 함수

void sort_idTable(int size); // id_table 을 정렬 하는 함수
void sort_scoreTable(int size); // score_talbe 정렬 함수
void get_qname_number(char *qname, int *num1, int *num2); // filename 에서 숫자 얻는 함수

#endif
