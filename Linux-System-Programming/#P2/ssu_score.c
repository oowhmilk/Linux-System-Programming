#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "blank.h"
#include "ssu_score.h"

extern struct ssu_scoreTable score_table[QNUM]; // 점수 테이블 구조체 저장 변수
extern char id_table[SNUM][10]; // 학생

struct ssu_scoreTable score_table[QNUM]; // 점수 테이블 구조체 저장 변수
char id_table[SNUM][10];

char stuDir[BUFLEN]; // stuDir 저장 변수
char ansDir[BUFLEN]; // ansDir 저장 변수
char errorDir[BUFLEN]; // errorDir 저장 변수 == "-e" 옵션 처리할때 옵션 뒤에 나오는 인자들 저장 변수
char threadFiles[ARGNUM][FILELEN]; // threadFiles 저장 변수 == threadFiles 에 들어간 file 들은 컴파일시 "-lpthread" 옵션 추가해서 컴파일 
char pIDs[ARGNUM][FILELEN]; // "-i" 옵션에서 인자로 넣은 학번을 저장 변수
char cIDs[ARGNUM][FILELEN]; // "-c" 옵션에서 인자로 넣은 학번을 저장 변수

// 구현 변수
char* new_score_file = NULL; // "-n" 옵션 뒤에 나오는 인자 저장 변수
char* result_path = NULL; // 결과값 저장하는 ".csv" file 저장 변수
char* score_table_path = NULL; // 점수 저장하는 "score_table.csv" file 저장 변수
char* error_dir_path = NULL; // error directory 경로 저장 변수
char* category = NULL; // 정렬 종류
char* which_sort; // 정렬 방법
char* buf; // 학생마다 각 문제마다 점수 저장 변수

// 사용 옵션
int eOption = false; // "-e" 옵션 저장 변수
int tOption = false; // "-t" 옵션 저장 변수
int mOption = false; // "-m" 옵션 저장 변수 == 점수 테이블 파일 내의 특정 문제 번호의 배점 수정하는 옵션

// 구현 옵션
int nOption = false; // "-n" 옵션 저장 변수 == 새로운 ".csv" file 에 점수 저장하는 옵션
int cOption = false; // "-c" 옵션 저장 변수 == 인자로 입력받은 학생들의 점수 출력하는 옵션
int pOption = false; // "-p" 옵션 저장 변수 == 틀린 문제 출력하는 옵션
int sOption = false; // "-s" 옵션 저장 변수 == sort 하는 옵션
int sortOption = false; // "-sortOption" 옵션 저장 변수 == 오름차순, 내림차순 저장하는 옵션

int cOptind; // "-c" 옵션 명령어에서 위치
int pOptind; // "-p" 옵션 명령어에서 위치

// 링크드 리스트 Node 
typedef struct Node {
	char* id_num;
    char* problem_num;
	double score;
    struct Node* prev;
    struct Node* next;
} Node;

Node* head = NULL;
Node* tail = NULL;

typedef struct Student{
    char* id_num;
	char buffer[BUFLEN];
	double student_score;
    struct Student* prev;
    struct Student* next;
} Student;

Student* first = NULL;
Student* last = NULL;

void ssu_score(int argc, char *argv[])
{
	char saved_path[BUFLEN]; // 현재 작업 디렉토리 경로 저장 변수
	int i;

	for(i = 0; i < argc; i++){ // "-h" 옵션 처리
		if(!strcmp(argv[i], "-h")){
			print_usage();
			return;
		}
	}

	memset(saved_path, 0, BUFLEN);
	if(argc >= 3 && strcmp(argv[1], "-i") != 0){ // "-i" 옵션 아닌 경우
		strcpy(stuDir, argv[1]);
		strcpy(ansDir, argv[2]);
	}

	// 결과값 저장하는 result_path 경로만들기
	result_path = malloc(sizeof(char) * MAX_PATH); 
	char cwd[MAX_PATH];
	getcwd(cwd, sizeof(cwd));
	strcat(cwd, "/");
	strcat(cwd, ansDir);
	strcat(cwd, "/");
	strcat(cwd, "score.csv");
	strcpy(result_path, cwd);

	//점수 저장하는 score_table_path 경로만들기
	score_table_path = malloc(sizeof(char) * MAX_PATH); 
	getcwd(cwd, sizeof(cwd));
	strcat(cwd, "/");
	strcat(cwd, ansDir);
	strcat(cwd, "/");
	strcat(cwd, "score_table.csv");
	strcpy(score_table_path, cwd);


	if(!check_option(argc, argv)) // 옵션 처리하기
		exit(1);

	if(mOption && !eOption && !tOption && !pOption ){ // "-e", "-t", "-p" 없을 경우, "-m" 만 있을 경우
		if(access(score_table_path, F_OK) < 0) // "score_table_path" 해당 경로 file 이 없을 경우
		{
			printf("score_table.csv doesn't exist\n");
			return;
		}
	}

	getcwd(saved_path, BUFLEN); // 현재 작업 saved_path 에 디렉토리 저장

	if(chdir(stuDir) < 0){ // stuDir 없을 경우
		fprintf(stderr, "<STD_DIR> or <ANS_DIR> doesn't exist\n");
		return;
	}

	getcwd(stuDir, BUFLEN); // 현재 작업 stuDir 에 디렉토리 저장

	chdir(saved_path); // saved_path 로 현재 작업 디렉토리 이동

	if(chdir(ansDir) < 0){ // ansDir 없을 경우
		fprintf(stderr, "<STD_DIR> or <ANS_DIR> doesn't exist\n");
		return;
	}

	getcwd(ansDir, BUFLEN); // 현재 작업 ansDir 에 디렉토리 저장

	chdir(saved_path); // saved_path 로 현재 작업 디렉토리 이동

	set_scoreTable(ansDir); // score_table 을 set 하는 함수
	set_idTable(stuDir); // id_table 을 set 하는 함수

	if(nOption) // "-n" 옵션 처리
	{
		do_nOption(new_score_file); // "-n" 옵션 함수 실행하기
		if(eOption)
		{
			do_eOption();
			printf("error saved.. (%s/)\n", error_dir_path);
		}
		return;
	}

	if(mOption) // "-m" 옵션 처리
	{
		do_mOption(ansDir); // "-m" 옵션 처리 함수
		printf("grading student's test papers..\n");
		score_students(); // 학생들의 총점을 출력하는 함수
		printf("result saved.. (%s)\n", result_path);
		if(eOption)
		{
			do_eOption();
			printf("error saved.. (%s/)\n", error_dir_path);
		}
		return;
	}

	if(cOption) // "-c" 옵션 처리
	{
		if(pOption)
		{
			if(cOptind + 1 != pOptind) // "-c" 옵션 뒤에 바로 "-p" 가 오지 않는 경우
			{
				if(strlen(pIDs[0]) != 0 && strlen(cIDs[0]) != 0)
				{
					printf("-c option, -p option 뒤에 모두 학생인자가 들어왔다.\n");
					return;
				}
				else if(strlen(cIDs[0]) != 0)
				{
					for(int i = 0 ; i < ARGNUM ; i++)
					{
						strcpy(pIDs[i], cIDs[i]);
					}
				}
				else if(strlen(pIDs[0]) != 0)
				{
					for(int i = 0 ; i < ARGNUM ; i++)
					{
						strcpy(cIDs[i], pIDs[i]);
					}
				}
			}

			if(strlen(pIDs[0]) != 0)
			{
				if(check_student_dir(stuDir, pIDs) == 0)
				{
					printf("인자로 입력받은 학생이 없다\n");
					return;
				}
				else
				{
					for(int i = 0 ; i < ARGNUM ; i++)
					{
						strcpy(cIDs[i], pIDs[i]);
					}
				}
			}
			
		}

		if(tOption)
		{
			int check_num = 0;

			for(int i = 0 ; i < sizeof(threadFiles) / sizeof(threadFiles[0]) ; i++)
			{
				if(strlen(threadFiles[i]) != 0)
				{
					int j;
					for(j = 0; j < sizeof(score_table) / sizeof(score_table[0]); j++)
					{
						if(strlen(score_table[j].qname) != 0)
						{
							char *ptr = malloc(sizeof(char) * MAX_PATH);; 
							strcpy(ptr, score_table[j].qname); // score_table 에서 filename 저장하기
							ptr = strtok(ptr, "."); // "." 으로 filename 에서 문제 번호 저장하기
							if(strcmp(ptr, threadFiles[i]) == 0)
							{
								break;
							}
							memset(ptr, '\0', sizeof(char) * MAX_PATH);
						}
						check_num++;
					}

					if(check_num == (sizeof(score_table) / sizeof(score_table[0])) && (strlen(threadFiles[0]) != 0))
					{
						printf("Not exist file\n");
						return;
					}
				}

				check_num = 0;
			}
		}

		if(strlen(cIDs[0]) != 0)
		{
			if(check_student_dir(stuDir, cIDs) == 0)
			{
				printf("인자로 입력받은 학생이 없다\n");
				return;
			}
		}

		printf("grading student's test papers..\n");
		score_students(); // 학생들의 총점을 출력하는 함수
		printf("result saved.. (%s)\n", result_path);
		if(eOption)
		{
			do_eOption();
			printf("error saved.. (%s/)\n", error_dir_path);
		}
		return;
	}

	if(pOption) // "-p" 옵션 처리
	{
		if(strlen(pIDs[0]) != 0)
		{
			if(check_student_dir(stuDir, pIDs) == 0)
			{
				printf("인자로 입력받은 학생이 없다\n");
				return;
			}
		}

		printf("grading student's test papers..\n");
		score_students(); // 학생들의 총점을 출력하는 함수
		//print_list_problem();
		printf("result saved.. (%s)\n", result_path);
		if(eOption)
		{
			do_eOption();
			printf("error saved.. (%s/)\n", error_dir_path);
		}
		return;
	}

	if(tOption) // "-t" 옵션 처리
	{
		int check_num = 0;

		for(int i = 0 ; i < sizeof(threadFiles) / sizeof(threadFiles[0]) ; i++)
		{
			if(strlen(threadFiles[i]) != 0)
			{
				int j;
				for(j = 0; j < sizeof(score_table) / sizeof(score_table[0]); j++)
				{
					if(strlen(score_table[j].qname) != 0)
					{
						char *ptr = malloc(sizeof(char) * MAX_PATH);; 
						strcpy(ptr, score_table[j].qname); // score_table 에서 filename 저장하기
						ptr = strtok(ptr, "."); // "." 으로 filename 에서 문제 번호 저장하기
						if(strcmp(ptr, threadFiles[i]) == 0)
						{
							break;
						}
						memset(ptr, '\0', sizeof(char) * MAX_PATH);
					}
					check_num++;
				}

				if(check_num == (sizeof(score_table) / sizeof(score_table[0])) && (strlen(threadFiles[0]) != 0))
				{
					printf("Not exist file\n");
					return;
				}
			}

			check_num = 0;
		}
	}

	if(eOption) // "-e" 옵션 처리
	{
		printf("grading student's test papers..\n");
		score_students(); // 학생들의 총점을 출력하는 함수
		do_eOption();
		printf("result saved.. (%s)\n", result_path);
		printf("error saved.. (%s/)\n", error_dir_path);
		return;
	}

	if(sOption) // "-s" 옵션 처리
	{
		if(strcmp(category, "stdid") != 0 && strcmp(category, "score") != 0) // category stdid, score 예외처리
		{
			printf("<CATEGORY> is %s\n",category);
			return;
		}

		if(strcmp(which_sort, "1") != 0 && sortOption != true) // sort 는 -1, 1 예외처리
		{
			printf("<SORTING> is %s\n",which_sort);
			return;
		}

		printf("grading student's test papers..\n");
		score_students(); // 학생들의 총점을 출력하는 함수

		if(strcmp(category, "stdid") == 0) // 학번에 대한 정렬
		{
			if(sortOption)
			{
				sort_id_descending() ;
			}
		}
		else if(strcmp(category, "score") == 0) // 학점에 대한 정렬
		{
			if(strcmp(which_sort, "1") == 0) // 오름차순 정렬하기
			{
				sort_score_ascending();
			}
			else if(sortOption) // 내림차순 정렬하기
			{
				sort_score_descending();
			}
		}

		save_sorting_score(); // ".csv file 에 저장하기"

		printf("result saved.. (%s)\n", result_path);
		return;
	}	


	printf("grading student's test papers..\n");
	score_students(); // 학생들의 총점을 출력하는 함수
	printf("result saved.. (%s)\n", result_path);

	return;
}

int check_option(int argc, char *argv[]) // 옵션 처리하는 함수
{
	int i, j, k;
	int c;
	int exist = 0;

	while((c = getopt(argc, argv, "n:mcpts:1e:h")) != -1) // getopt() 함수로 옵션 구별
	{
		switch(c)
		{
			case 'e': // "-e" 옵션 처리
				eOption = true; // "-e" 옵션 저장 변수 TRUE
				strcpy(errorDir, optarg); // optarg 에 있는 디렉토리 변수 errorDir 에 저장

				if(access(errorDir, F_OK) < 0) // errorDir 디렉토리 존재하지 않을 경우
					mkdir(errorDir, 0755); // errorDir 디렉토리 생성
				else{ // errorDir 존재할 경우
					rmdirs(errorDir); // errorDir 디렉토리 하위 디렉코리 까지 모두 삭제
					mkdir(errorDir, 0755); // errorDir 디렉토리 생성
				}
				break;

			case 't': // "-t" 옵션 처리
				tOption = true; // "-t" 옵션 저장 변수 TRUE
				i = optind; // 다음번 처리될 옵션의 인덱스 저장
				j = 0;

				while(i < argc && argv[i][0] != '-'){ // "-t" 옵션 뒤에 "-" 제외 값 들어오는 경우
					if(j >= ARGNUM) // argument 개수 초과한 경우
					{
						if(j == ARGNUM)
						{
							printf("Maximum Number of Argument Exceeded. :: %s", argv[i]);
						}
						else
						{
							printf(" %s", argv[i]);
						}
					}
					else{
						strcpy(threadFiles[j], argv[i]); // threadFiles 에 옵션 인자 뒤에 나오는 모든 인자를 저장 
					}
					i++; 
					j++;
				}
				if(j > ARGNUM) // argument 개수 초과한 경우
					printf("\n");
				break;

			case 'm': // "-m" 옵션 처리
				mOption = true; // "-m" 옵션 저장 변수 TRUE
				break;

			// 구현 부분
			case 'n': // "-n" 옵션 처리
				nOption = true;
				new_score_file = malloc(sizeof(char) * MAX_PATH);
				strcpy(new_score_file, optarg); // optarg 에 있는 디렉토리 변수 new_score_file 에 저장

				char temp[MAX_PATH];
				char cwd[MAX_PATH];
				getcwd(cwd, sizeof(cwd));

				// 상대경로 입력 시
				if (new_score_file[0] != '/')
				{
					if(new_score_file[0] == '~') // ~ 입력 시
					{
						strcpy(temp, new_score_file + 2);
						strcpy(new_score_file, temp);
					}

					strcat(temp,"/");
					strcat(temp,new_score_file);
					strcat(cwd, temp);
					strcpy(new_score_file, cwd);
				}
				// 절대경로 입력 시
				else 
				{
					
				}
				break;

			case 'c': // "-c" 옵션 처리
				cOption = true;
				cOptind = optind;
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){
					if(j >= ARGNUM) // argument 개수 초과한 경우
					{
						if(j == ARGNUM)
						{
							printf("Maximum Number of Argument Exceeded. :: %s", argv[i]);
						}
						else
						{
							printf(" %s", argv[i]);
						}
					}
					else
						strcpy(cIDs[j], argv[i]);
					i++; 
					j++;
				}
				if(j > ARGNUM) 
					printf("\n");
				break;

			case 'p': // "-i" 옵션 처리
				pOption = true; // "-i" 옵션 저장 변수 TRUE
				pOptind = optind;
				i = optind; // 다음번 처리될 옵션의 인덱스 저장
				j = 0;

				while(i < argc && argv[i][0] != '-'){ // "-i" 옵션 뒤에 "-" 제외 값 들어오는 경우
					if(j >= ARGNUM) // argument 개수 초과한 경우
					{
						if(j == ARGNUM)
						{
							printf("Maximum Number of Argument Exceeded. :: %s", argv[i]);
						}
						else
						{
							printf(" %s", argv[i]);
						}
					}
					else
						strcpy(pIDs[j], argv[i]); // pIDs 에 옵션 인자 뒤에 나오는 모든 인자를 저장
					i++;
					j++;
				}
				if(j > ARGNUM) // argument 개수 초과한 경우
					printf("\n");
				break;

			case 's': // "-s" 옵션 처리
				sOption = true; // "-i" 옵션 저장 변수 TRUE
				i = optind; // 다음번 처리될 옵션의 인덱스 저장
				j = 0;

				category = malloc(sizeof(char) * BUFLEN);
				which_sort = malloc(sizeof(char) * 1);

				strcpy(category, argv[i - 1]);
				i++;
				which_sort = argv[i - 1];
				break;
			
			case '1': // "-1" 옵션 처리
				sortOption = true; // "-i" 옵션 저장 변수 TRUE
				break;

			case '?': // 옵션 예외 처리
				printf("Unknown option %c\n", optopt);
				return false;
		}
	}

	return true; // 옵션 처리 성공 return true;
}

void do_iOption(char (*ids)[FILELEN]) // "-i" 옵션 실행하는 함수
{
	FILE *fp; // "./score.csv" 파일 FILE 스트림 형 구조체 저장 변수
	char tmp[BUFLEN]; // 파일 내용 저장하는 변수
	char qname[QNUM][FILELEN]; // 문제 마다 학생이 받은 점수 저장 변수
	char *ptr; // score_table 에서 filename 저장 변수
	ptr = malloc(sizeof(char) * FILELEN);
	char *p, *id; // id : 학번 저장하는 변수
	int i, j;
	char first, exist;

	if((fp = fopen(result_path, "r")) == NULL){ // "score.csv" 파일 읽기 전용으로 fopen 실패한 경우
		fprintf(stderr, "score.csv file doesn't exist\n");
		return;
	}

	// get qnames
	i = 0;
	fscanf(fp, "%s\n", tmp); // open 한 파일에서 개행까지 읽어서 한 줄씩 tmp 에 저장
	strcpy(qname[i++], strtok(tmp, ",")); // tmp 에서 "," 로 구분해서 qname[i] 에 저장
	
	while((p = strtok(NULL, ",")) != NULL) // "," 구분해서 open한 파일의 첫번째 줄만 저장
		strcpy(qname[i++], p);

	// print result
	i = 0;
	while(i++ <= ARGNUM - 1)
	{
		exist = 0;
		fseek(fp, 0, SEEK_SET); // 현재 위치를 파일 시작점으로 옮기기
		fscanf(fp, "%s\n", tmp); // open 한 파일에서 읽어서 tmp 에 저장

		while(fscanf(fp, "%s\n", tmp) != EOF) // open 한 파일에서 개행까지 읽어서 한 줄씩 tmp 에 저장
		{ 
			id = strtok(tmp, ","); // tmp 한 줄에서 첫번째 "," 전의 값인 학번 저장 변수

			if(!strcmp(ids[i - 1], id)) // 함수 인자로 받은 ids[i - 1] 와 id 비교해서 같을 경우
			{ 
				exist = 1; 
				j = 0;
				first = 0;
				while((p = strtok(NULL, ",")) != NULL)  // "," 구분해서 p 에 저장
				{
					if(atof(p) == 0) // char* p 를 부동 소수점으로 변환
					{ 
						if(!first)
						{ 
							printf("%s is finished.. wrong problem : ", id);
							first = 1;
						}
						if(strcmp(qname[j], "sum"))
						{
							for(int k = 0; k < sizeof(score_table) / sizeof(score_table[0]); k++)
							{
								if(!strcmp(score_table[k].qname, qname[j]))// 수정할 file 과 ptr 비교해서 같은 경우
								{ 
									strcpy(ptr, score_table[k].qname); // score_table 에서 filename 저장하기
									//ptr = strtok(ptr, "."); // "." 으로 filename 에서 문제 번호 저장하기
									printf("%s(%.2f), ", ptr, score_table[k].score); // 해당 문제 점수 출력
									break;
								}
							}
						}
					}
					j++;
				}
				printf("\n");
			}
		}

		if(!exist)
			printf("%s doesn't exist!\n", id);
	}

	fclose(fp);
}

void do_pOption(char *check_id) // "-p" 옵션 실행하는 함수
{
	FILE *fp; // "./score.csv" 파일 FILE 스트림 형 구조체 저장 변수
	char tmp[BUFLEN]; // 파일 내용 저장하는 변수
	char qname[QNUM][FILELEN]; // 문제 마다 학생이 받은 점수 저장 변수
	char *ptr; // score_table 에서 filename 저장 변수
	ptr = malloc(sizeof(char) * FILELEN);
	char *p, *id; // id : 학번 저장하는 변수
	int i, j;
	char first, exist;
	char wrong_problems[ARGNUM - 1][BUFLEN]; 

	if((fp = fopen(result_path, "r")) == NULL){ // "score.csv" 파일 읽기 전용으로 fopen 실패한 경우
		fprintf(stderr, "score.csv file doesn't exist\n");
		return;
	}

	// get qnames
	i = 0;
	fscanf(fp, "%s\n", tmp); // open 한 파일에서 개행까지 읽어서 한 줄씩 tmp 에 저장
	strcpy(qname[i++], strtok(tmp, ",")); // tmp 에서 "," 로 구분해서 qname[i] 에 저장
	
	while((p = strtok(NULL, ",")) != NULL) // "," 구분해서 open한 파일의 첫번째 줄만 저장
		strcpy(qname[i++], p);


	exist = 0;
	int l = 0 ; // "," 를 위한 변수
	fseek(fp, 0, SEEK_SET); // 현재 위치를 파일 시작점으로 옮기기
	fscanf(fp, "%s\n", tmp); // open 한 파일에서 읽어서 tmp 에 저장

	while(fscanf(fp, "%s\n", tmp) != EOF) // open 한 파일에서 개행까지 읽어서 한 줄씩 tmp 에 저장
	{ 
		id = strtok(tmp, ","); // tmp 한 줄에서 첫번째 "," 전의 값인 학번 저장 변수

		if(!strcmp(check_id, id)) // 함수 인자로 받은 ids[i - 1] 와 id 비교해서 같을 경우
		{ 
			exist = 1; 
			j = 0;
			first = 0;
			while((p = strtok(NULL, ",")) != NULL)  // "," 구분해서 p 에 저장
			{
				if(atof(p) == 0) // char* p 를 부동 소수점으로 변환해서 값이 0 인 경우
				{ 
					if(!first)
					{ 
						if(cOption)
						{
							printf(" ,wrong problem : ");
						}
						else
						{
							printf("%s is finished.. wrong problem : ", id);
						}
						first = 1;
					}
					if(strcmp(qname[j], "sum"))
					{
						for(int k = 0; k < sizeof(score_table) / sizeof(score_table[0]); k++)
						{
							if(!strcmp(score_table[k].qname, qname[j]))
							{ 
								strcpy(ptr, score_table[k].qname); // score_table 에서 filename 저장하기

								char *p_ext = strrchr(ptr, '.'); // 마지막 '.' 위치 찾기
								if (p_ext != NULL) { // '.'이 있을 경우
									*p_ext = '\0'; // '.'을 '\0'으로 변경하여 파일 확장자를 삭제
								}

								if(l == 0) // 마지막 "," 제거하기
								{		
									insert_problem(id, ptr, score_table[k].score); 
								}
								else
								{
									insert_problem(id, ptr, score_table[k].score);
								}	
								l++;
								break;
							}
						}
					}
				}
				j++;
			}
			//printf("\n");
			insert_problem("\n","\n",0.0);
		}
	}
	fclose(fp);
}

void do_mOption(char *ansDir) // "-m" 옵션 처리 함수
{
	double newScore; // 새로 바꿀 점수 저장 변수
	char modiName[FILELEN]; // 수정할 file 이름 저장 변수
	char filename[FILELEN];
	char *ptr; // score_table 에서 filename 저장 변수
	int i;

	ptr = malloc(sizeof(char) * FILELEN);

	while(1){

		printf("Input question's number to modify >> ");
		scanf("%s", modiName); // 수정할 file 이름 저장 

		if(strcmp(modiName, "no") == 0) // 수정할 file 이 없을 경우
			break;

		for(i = 0; i < sizeof(score_table) / sizeof(score_table[0]); i++)
		{
			strcpy(ptr, score_table[i].qname); // score_table 에서 filename 저장하기
			ptr = strtok(ptr, "."); // "." 으로 filename 에서 문제 번호 저장하기

			if(!strcmp(ptr, modiName)){ // 수정할 file 과 ptr 비교해서 같은 경우
				printf("Current score : %.2f\n", score_table[i].score); // 현재 점수 출력
				printf("New score : ");
				scanf("%lf", &newScore); // 새로 바꿀 점수 입력
				getchar(); // "\n" 비워주기
				score_table[i].score = newScore; // 새로 바꿀 점수로 score_table 점수 바꾸기
				break;
			}
		}
	}

	sprintf(filename, "%s", score_table_path);

	write_scoreTable(filename); // score_table 에 있는 값을 인자로 들어온 filename 의 file 에 작성하는 함수
	free(ptr);

}

void do_nOption(char* new_score_file) // "-n" 옵션 처리 함수
{
	char* extension; // ".csv" 확장자 확인 변수
	extension = strrchr(new_score_file, '.');

    // 확장자가 .csv가 아닌 경우 예외 처리합니다.
    if (extension == NULL || strcmp(extension, ".csv") != 0) {
        printf("Error: this file is not \".csv\" file.\n");
        return ;
    }
	else
	{
		printf("grading student's test papers..\n");

		char *temp_path = malloc(sizeof(char) * MAX_PATH); 

		strcpy(temp_path, new_score_file);

		char** path_tokens = NULL; // 토큰들을 저장할 메모리 공간 동적 할당
		char* path_token = NULL; // 첫 번째 토큰 추출
		int path_token_count = 0; // 토큰의 개수를 저장하는 변수

		path_tokens = (char**) malloc(sizeof(char*) * 256); // 토큰들을 저장할 메모리 공간 동적 할당
		path_token = strtok(temp_path, "/"); // 첫 번째 토큰 추출
		path_token_count = 0;
		
		while (path_token != NULL) // 토큰이 더 이상 없을 때까지 반복
		{ 
			path_tokens[path_token_count] = path_token; // 현재 토큰을 저장
			path_token_count++;
			path_token = strtok(NULL, "/"); // 다음 토큰 추출
		}

		char *current_path = malloc(sizeof(char) * MAX_PATH); 
		for(int i = 0 ; i < path_token_count - 1 ; i++)
		{
			strcat(current_path,"/");
			strcat(current_path,path_tokens[i]);

			DIR* dir = opendir(current_path);
			if (dir == NULL) // 디렉토리 확인하면서 없는 디렉토리 생성하기 
			{
				mkdir(current_path, 0777);
			}
		}
		
		new_file_score_students(new_score_file); // 학생들의 총점을 출력하는 함수
		char *print_path = malloc(sizeof(char) * MAX_PATH); 
		realpath(new_score_file, print_path);
		printf("result saved.. (%s)\n", print_path);
		free(temp_path);
		free(path_tokens);
		free(current_path);
		free(print_path);
	}
}

void do_cOption(char (*ids)[FILELEN])
{
	FILE *fp;
	char tmp[BUFLEN];
	int i = 0;
	char *p, *saved;

	if((fp = fopen(result_path, "r")) == NULL){
		fprintf(stderr, "file open error for score.csv\n");
		return;
	}

	fscanf(fp, "%s\n", tmp);

	while(fscanf(fp, "%s\n", tmp) != EOF)
	{
		p = strtok(tmp, ",");

		if(!is_exist(ids, tmp))
			continue;

		printf("%s's score : ", tmp);

		while((p = strtok(NULL, ",")) != NULL)
			saved = p;

		printf("%s\n", saved);
	}
	fclose(fp);
}

void do_eOption()
{
	error_dir_path = malloc(sizeof(char) * MAX_PATH);
	char *temp_path = malloc(sizeof(char) * MAX_PATH); 
	char cwd[MAX_PATH];
	getcwd(cwd, sizeof(cwd));
	strcpy(error_dir_path, errorDir);

	// 상대경로 입력 시
	if (error_dir_path[0] != '/')
	{
		if(error_dir_path[0] == '~') // ~ 입력 시
		{
			strcpy(temp_path, error_dir_path + 2);
			strcpy(error_dir_path, temp_path);
		}

		strcat(temp_path,"/");
		strcat(temp_path,error_dir_path);
		strcat(cwd, temp_path);
		strcpy(error_dir_path, cwd);
	}
	// 절대경로 입력 시
	else 
	{
		
	}

	strcpy(temp_path, error_dir_path);

	char** path_tokens = NULL; // 토큰들을 저장할 메모리 공간 동적 할당
	char* path_token = NULL; // 첫 번째 토큰 추출
	int path_token_count = 0; // 토큰의 개수를 저장하는 변수

	path_tokens = (char**) malloc(sizeof(char*) * 256); // 토큰들을 저장할 메모리 공간 동적 할당
	path_token = strtok(temp_path, "/"); // 첫 번째 토큰 추출
	path_token_count = 0;
	
	while (path_token != NULL) // 토큰이 더 이상 없을 때까지 반복
	{ 
		path_tokens[path_token_count] = path_token; // 현재 토큰을 저장
		path_token_count++;
		path_token = strtok(NULL, "/"); // 다음 토큰 추출
	}

	char *current_path = malloc(sizeof(char) * MAX_PATH); 
	for(int i = 0 ; i < path_token_count ; i++)
	{
		strcat(current_path,"/");
		strcat(current_path,path_tokens[i]);

		DIR* dir = opendir(current_path);
		if (dir == NULL) // 디렉토리 확인하면서 없는 디렉토리 생성하기 
		{
			mkdir(current_path, 0777);
		}
	}
}

int is_exist(char (*src)[FILELEN], char *target) //
{
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM)
			return false;
		else if(!strcmp(src[i], ""))
			return false;
		else if(!strcmp(src[i++], target))
			return true;
	}
	return false;
}

void set_scoreTable(char *ansDir) // score_table 을 set 하는 함수
{
	char filename[FILELEN]; // filename 저장 변수

	sprintf(filename, "%s", score_table_path); // filename 변수 만들기

	// check exist
	if(access(filename, F_OK) == 0) // filename 의 file 이 존재한 경우
		read_scoreTable(filename); // filename 의 file 로 score_table 만들기
	else // filename 의 file 이 존재하지 않는 경우
	{ 
		make_scoreTable(ansDir); // score_table 생성하는 함수
		write_scoreTable(filename); // score_table 에 있는 값을 인자로 들어온 filename 의 file 에 작성하는 함수
	}
}

void read_scoreTable(char *path) // score_table 에 qname, 점수 저장 함수
{
	FILE *fp;
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){ // path 파일 읽기 전용으로 fopen 실패한 경우
		fprintf(stderr, "file open error for %s\n", path);
		return ;
	}

	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){ // fp 쉼표 이전의 문자열을 qname , 쉼표 이후의 문자열을 score 에 저장
		strcpy(score_table[idx].qname, qname); // score_table 에 qname 저장
		score_table[idx++].score = atof(score); // score_table 에 점수 저장
	}

	fclose(fp);
}

void make_scoreTable(char *ansDir) // 인자로 받은 ansDir 로 score_table 만드는 함수
{
	int type, num;
	double score, bscore, pscore;
	struct dirent *dirp, *c_dirp;
	DIR *dp, *c_dp;
	char *tmp;
	int idx = 0;
	int i;

	num = get_create_type(); // score_table 유형 저장하는 변수

	if(num == 1)
	{
		printf("Input value of blank question : ");
		scanf("%lf", &bscore);
		printf("Input value of program question : ");
		scanf("%lf", &pscore);
	}

	if((dp = opendir(ansDir)) == NULL){ // ansDir open error 발생한 경우
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}

	while((dirp = readdir(dp)) != NULL){ // ansDir 디렉토리 내의 파일을 가져오기

		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) // 가져온 파일명이 "." , ".." 인 경우
			continue;

		if((type = get_file_type(dirp->d_name)) < 0) // 파일 유형 확인해서 ".c" , ".txt" 아닌 경우
			continue;

		strcpy(score_table[idx].qname, dirp->d_name); // score_table 에 파일명 저장하기

		idx++;
	}

	closedir(dp); // open 한 directory close 
	sort_scoreTable(idx); // score_table 정렬하기

	for(i = 0; i < idx; i++)
	{
		type = get_file_type(score_table[i].qname); // score_table 에서 qname 으로 파일 유형 저장 변수

		if(num == 1)
		{
			if(type == TEXTFILE) // file type 이 .txt 인 경우
				score = bscore; // 모든 .txt 점수 설정
			else if(type == CFILE) // file type 이 .c 인 경우
				score = pscore; // 모든 .txt 점수 설정
		}
		else if(num == 2)
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score); // 사용자가 파일마다 점수 설정
		}

		score_table[i].score = score; // 설정한 점수 저장
	}
}

void write_scoreTable(char *filename) // score_table 에 있는 값을 인자로 들어온 filename 의 file 에 작성하는 함수
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]); // score_table 에 존재하는 ssu_scoreTable 구조체 수 저장 변수

	if((fd = creat(filename, 0666)) < 0){ // 인자로 들어온 filename 으로 file 생성
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++) 
	{
		if(score_table[i].score == 0) // score_table 점수가 0 인 경우 
			break; // 반복문 멈춤

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score); // tmp 에 qname , 점수 저장
		write(fd, tmp, strlen(tmp)); // open 한 파일에 저장된 tmp 값을 저장
	}

	close(fd);
}

void set_idTable(char *stuDir) // id_table 을 set 하는 함수
{
	struct stat statbuf; // stat 구조체 저장 변수
	struct dirent *dirp; // dirent 구조체 저장 변수
	DIR *dp; 
	char tmp[BUFLEN]; // filename 명 임시 저장 변수
	int num = 0;

	if((dp = opendir(stuDir)) == NULL){ // stuDir 디렉토리 open 하기
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}

	while((dirp = readdir(dp)) != NULL){ // open 한 디렉토리의 DIR 구조체로 readdir 하기
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) // dirent 구조체의 filename 이 "." , ".." 아닌 경우
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name); // tmp 에 filename 저장하기
		stat(tmp, &statbuf); // tmp 에 저장된 filename 으로 stat 구조체 구하기

		if(S_ISDIR(statbuf.st_mode)) // filename 이 directory 인 경우
			strcpy(id_table[num++], dirp->d_name); // id_table 에 학번 저장
		else
			continue;
	}
	closedir(dp);

	sort_idTable(num); // id_table 정렬하기
}

void sort_idTable(int size) // id_table 을 정렬 하는 함수
{
	int i, j;
	char tmp[10]; // 학번 임시 저장 변수

	for(i = 0; i < size - 1; i++){ // 저장된 학번으로 정렬
		for(j = 0; j < size - 1 -i; j++){
			if(strcmp(id_table[j], id_table[j+1]) > 0){
				strcpy(tmp, id_table[j]);
				strcpy(id_table[j], id_table[j+1]);
				strcpy(id_table[j+1], tmp);
			}
		}
	}
}

void sort_scoreTable(int size) // score_talbe 정렬 함수
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 - i; j++){

			get_qname_number(score_table[j].qname, &num1_1, &num1_2); // 파일 이름의 숫자를 저장하기
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2); // 파일 이름의 숫자를 저장하기

			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){ // 저장한 숫자 비교하기

				memcpy(&tmp, &score_table[j], sizeof(score_table[0]));
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			}
		}
	}
}

void get_qname_number(char *qname, int *num1, int *num2) // filename 에서 숫자 얻는 함수
{
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname)); // qname 을 dup 에 저장
	*num1 = atoi(strtok(dup, "-.")); // dup 에 "." , "-" 이전 string 을 num1 에 저장
	
	p = strtok(NULL, "-."); // 나머지 string 에서 "." , "-" 이전 string 을 num2 에 저장
	if(p == NULL) 
		*num2 = 0;
	else
		*num2 = atoi(p);
}

int get_create_type() // score_table 유형 정하는 함수
{
	int num;

	while(1)
	{
		printf("score_table.csv file doesn't exist in \"%s\"!\n", ansDir);
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num);

		if(num != 1 && num != 2)
			printf("not correct number!\n");
		else
			break;
	}

	return num;
}

void score_students() // 모든 학생들의 총점으로 평균을 출력하는 함수
{
	double total_score = 0; // 평균을 구하기위한 점수 저장 변수
	double score = 0; // 각 학생마다 점수 저장 변수
	int num;
	int average_num = 0;
	int fd; // open 한 file descriptor 저장 변수
	char tmp[BUFLEN]; // 학번 임시 저장 변수
	int size = sizeof(id_table) / sizeof(id_table[0]); // id_table 에 저장된 학생명 수

	if((fd = creat(result_path, 0666)) < 0){ // "score.csv" file 생성
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	write_first_row(fd); // open 한 파일에 첫 줄에 문제 번호들과 "sum" 작성하는 함수

	for(num = 0; num < size; num++) 
	{
		if(!strcmp(id_table[num], "")) // id_table 값 없을 경우
			break;

		if(sOption)
		{
			buf = malloc(sizeof(char) * BUFLEN);

			score = score_student_sorting(buf, id_table[num]);
		}
		else
		{
			sprintf(tmp, "%s,", id_table[num]); // id_table 에 저장된 학번 tmp 에 저장
			write(fd, tmp, strlen(tmp)); // tmp 저장된 값 open 한 file 에 작성
			score = score_student(fd, id_table[num]); // 학번 저장된 id_table 에서 학번
		}
		
		if(cOption)
		{
			if(strlen(cIDs[0]) != 0)
			{
				for(int i = 0 ; i < ARGNUM ; i++) 
				{
					if(strcmp(id_table[num], cIDs[i]) == 0)
					{
						total_score += score;
						average_num++;
					}
				}
			}
			else
				total_score += score;
				
		}

		if(sOption)
		{
			insert_student(id_table[num], buf, score); // 링크드 리스트 만들기
		}

	}

	if(cOption)
	{
		if(strlen(cIDs[0]) != 0)
		{
			printf("Total average : %.2f\n", total_score / average_num); // 학생들의 총점을 가지고 평균 저장
		}
		else
		{
			printf("Total average : %.2f\n", total_score / num); // 모든 학생들의 총점을 가지고 평균 저장
		}
	}

	close(fd); // "score.csv" file close
}

void new_file_score_students(char* new_score_file) // 모든 학생들의 총점으로 평균을 new_score_file 에 출력하는 함수
{
	double total_score = 0; // 평균을 구하기위한 점수 저장 변수
	double score = 0;
	int num;
	int average_num = 0;
	int fd; // open 한 file descriptor 저장 변수
	char tmp[BUFLEN]; // 학번 임시 저장 변수
	int size = sizeof(id_table) / sizeof(id_table[0]); // id_table 에 저장된 학생명 수

	if((fd = creat(new_score_file, 0666)) < 0) 
	{ 
		fprintf(stderr, "creat error for %s", new_score_file);
		return;
	}
	write_first_row(fd); // open 한 파일에 첫 줄에 문제 번호들과 "sum" 작성하는 함수

	for(num = 0; num < size; num++) 
	{
		if(!strcmp(id_table[num], "")) // id_table 값 없을 경우
			break;

		sprintf(tmp, "%s,", id_table[num]); // id_table 에 저장된 학번 tmp 에 저장
		write(fd, tmp, strlen(tmp)); // tmp 저장된 값 open 한 file 에 작성

		if(sOption)
		{
			buf = malloc(sizeof(char) * BUFLEN);

			score = score_student_sorting(buf, id_table[num]);
		}
		else
		{
			score = score_student(fd, id_table[num]); // 학번 저장된 id_table 에서 학번
		}

		if(cOption)
		{
			if(strlen(cIDs[0]) != 0)
			{
				for(int i = 0 ; i < ARGNUM ; i++) 
				{
					if(strcmp(id_table[num], cIDs[i]) == 0)
					{
						total_score += score;
					}
				}
			}
			else
				total_score += score;
				
		}

		if(sOption)
		{
			insert_student(id_table[num], buf, score); // 링크드 리스트 만들기
		}
	}

	if(cOption)
	{
		if(strlen(cIDs[0]) != 0)
		{
			printf("Total average : %.2f\n", total_score / average_num); // 모든 학생들의 총점을 가지고 평균 저장
		}
		else
		{
			printf("Total average : %.2f\n", total_score / num); // 모든 학생들의 총점을 가지고 평균 저장
		}
	}

	close(fd); // "score.csv" file close
}


double score_student(int fd, char *id) // 학생 디렉토리에 존재하는 file 모두 검사후 점수 매기는 함수
{
	int type;
	double result;
	double score = 0; // 점수 저장 변수
	int i;
	char tmp[BUFLEN * 4]; // 경로 임시 저장 변수
	int size = sizeof(score_table) / sizeof(score_table[0]); // score_table 저장된 문제 개수 저장 변수

	for(i = 0; i < size ; i++) // score_table 에 저장된 문제 개수 만큼 반복해서 총 score 저장하는 반복문
	{
		if(score_table[i].score == 0) // score_table 점수가 0 인 경우
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname); // stuDir, 인자로 받은 학번, score_table 에 저장된 문제 이름으로 경로 만들어서 tmp 에 저장 

		if(access(tmp, F_OK) < 0) // tmp 경로에 파일이 존재하지 않는 경우
			result = false;
		else // tmp 경로에 파일이 존재하지 않는 경우
		{
			if((type = get_file_type(score_table[i].qname)) < 0) // score_table 존재하는 파일의 file type 저장
				continue;
			
			if(type == TEXTFILE) // file type 이 ".txt" 인 경우
				result = score_blank(id, score_table[i].qname); // 인자로 넣은 id, .txt file 로 맞을 경우 result = true , 틀렸을 경우 result = false 로 저장
			else if(type == CFILE) // file type 이 ".c" 인 경우
				result = score_program(id, score_table[i].qname); // 인자로 넣은 id, .c file 실행하고 결과값 확인해서 double 형인 result 점수 저장
		}

		if(result == false) // result == false 인 경우
			write(fd, "0,", 2); // fd 에 "0," 값을 저장
		else // result != false 인 경우
		{ 
			if(result == true) // result == true 인 경우
			{ 
				score += score_table[i].score; // score_table 에 문제마다 지정된 점수를 score 에 저장
				sprintf(tmp, "%.2f,", score_table[i].score); // score_table 값을 소수점 아래 두자리까지 tmp 에 저장
			}
			else if(result < 0) // result < 0 인 경우
			{
				score = score + score_table[i].score + result; // score_table 에 문제마다 지정된 점수와 ".c" 검사하고 깍인 점수를 더해서 score 에 저장
				sprintf(tmp, "%.2f,", score_table[i].score + result); // score_table 값을 소수점 아래 두자리까지 tmp 에 저장
			}
			write(fd, tmp, strlen(tmp)); // tmp 에 저장된 값을 fd_file 에 write 
		}
	}

	if(cOption && !pOption)
	{
		if(strcmp("",cIDs[0]) == 0)
		{
			printf("%s is finished.. score : %.2f\n", id, score); 
		}
		else
		{
			int i;
			for(i = 0 ; i < ARGNUM ; i++)
			{
				if(strcmp(id, cIDs[i]) == 0)
				{
					printf("%s is finished.. score : %.2f\n", id, score); 
					break;
				}
			}
			if(i == ARGNUM)
				printf("%s is finished..\n", id);
		}
	}
	else if(!cOption && pOption)
	{
		if(strcmp("",pIDs[0]) == 0)
		{
			do_pOption(id); 
			print_list_problem(id);
			free_linked_list();
			printf("\n"); 
		}
		else
		{
			int i;
			for(i = 0 ; i < ARGNUM ; i++)
			{
				if(strcmp(id, pIDs[i]) == 0)
				{
					do_pOption(id);
					print_list_problem(id);
					free_linked_list();
					printf("\n"); 
					break;
				}
			}
			if(i == ARGNUM)
				printf("%s is finished..\n", id);
		}
	}
	else if(cOption && pOption)
	{
		if(strcmp("", pIDs[0]) == 0)
		{
			printf("%s is finished.. score : %.2f", id, score);
			do_pOption(id);
			print_list_problem(id);
			free_linked_list();
			printf("\n"); 
		}
		else
		{
			int i;
			for(i = 0 ; i < ARGNUM ; i++)
			{
				if(strcmp(id, pIDs[i]) == 0)
				{
					printf("%s is finished.. score : %.2f", id, score); 
					do_pOption(id);
					print_list_problem(id);
					free_linked_list();
					printf("\n"); 
					break;
				}
			}
			if(i == ARGNUM)
				printf("%s is finished..\n", id);
		}
	}
	else
	{
		printf("%s is finished..\n", id); 
	}

	sprintf(tmp, "%.2f\n", score);
	write(fd, tmp, strlen(tmp));

	return score;
}

double score_student_sorting(char* buf, char *id) // 학생 디렉토리에 존재하는 file 모두 검사후 점수 매겨서 buf 에 저장하는 함수
{
	int type;
	double result;
	double score = 0; // 점수 저장 변수
	int i;
	char tmp[BUFLEN * 4]; // 경로 임시 저장 변수
	int size = sizeof(score_table) / sizeof(score_table[0]); // score_table 저장된 문제 개수 저장 변수
	char *zero = "0,";

	memset(buf, '\0', sizeof(char) * BUFLEN);

	for(i = 0; i < size ; i++) // score_table 에 저장된 문제 개수 만큼 반복해서 총 score 저장하는 반복문
	{
		if(score_table[i].score == 0) // score_table 점수가 0 인 경우
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname); // stuDir, 인자로 받은 학번, score_table 에 저장된 문제 이름으로 경로 만들어서 tmp 에 저장 

		if(access(tmp, F_OK) < 0) // tmp 경로에 파일이 존재하지 않는 경우
			result = false;
		else // tmp 경로에 파일이 존재하지 않는 경우
		{
			if((type = get_file_type(score_table[i].qname)) < 0) // score_table 존재하는 파일의 file type 저장
				continue;
			
			if(type == TEXTFILE) // file type 이 ".txt" 인 경우
				result = score_blank(id, score_table[i].qname); // 인자로 넣은 id, .txt file 로 맞을 경우 result = true , 틀렸을 경우 result = false 로 저장
			else if(type == CFILE) // file type 이 ".c" 인 경우
				result = score_program(id, score_table[i].qname); // 인자로 넣은 id, .c file 실행하고 결과값 확인해서 double 형인 result 점수 저장
		}

		if(result == false) // result == false 인 경우
			strncat(buf, zero, 2);
		else // result != false 인 경우
		{ 
			if(result == true) // result == true 인 경우
			{ 
				score += score_table[i].score; // score_table 에 문제마다 지정된 점수를 score 에 저장
				sprintf(tmp, "%.2f,", score_table[i].score); // score_table 값을 소수점 아래 두자리까지 tmp 에 저장
			}
			else if(result < 0) // result < 0 인 경우
			{
				score = score + score_table[i].score + result; // score_table 에 문제마다 지정된 점수와 ".c" 검사하고 깍인 점수를 더해서 score 에 저장
				sprintf(tmp, "%.2f,", score_table[i].score + result); // score_table 값을 소수점 아래 두자리까지 tmp 에 저장
			}
			strncat(buf, tmp, strlen(tmp));
		}
	}

	printf("%s is finished..\n", id); 

	sprintf(tmp, "%.2f\n", score);
	strncat(buf, tmp, strlen(tmp));

	return score;
}

void write_first_row(int fd) // open 한 파일에 첫 줄에 문제 번호들과 "sum" 작성하는 함수
{
	int i; // 반복문을 위한 변수
	char tmp[BUFLEN]; // qname 임시 저장 변수
	int size = sizeof(score_table) / sizeof(score_table[0]); // score_table 저장된 문제 수

	write(fd, ",", 1); // open 한 파일에 "," 작성

	for(i = 0; i < size; i++){ 
		if(score_table[i].score == 0) // score_table 점수가 0 일 경우
			break;
		
		sprintf(tmp, "%s,", score_table[i].qname); // score_table 에 존재하는 qname 을 tmp 에 저장
		write(fd, tmp, strlen(tmp)); // tmp 저장된 값 open 한 file 에 작성
	}
	write(fd, "sum\n", 4); // open 한 file 맨 마지막에 "sum\n" 작성
}

char *get_answer(int fd, char *result) // 작성한 정답을 get 하는 함수
{
	char c; // 한 byte 씩 읽어서 저장 변수
	int idx = 0;

	memset(result, 0, BUFLEN);
	while(read(fd, &c, 1) > 0) // open 한 file 에서 한 byte 씩 읽어서 c 에 저장
	{
		if(c == ':') // c 가 ":" 인 경우
			break;
		
		result[idx++] = c; // result 에 c 에 저장된 값 하나씩 저장
	}
	if(result[strlen(result) - 1] == '\n') // result 의 마지막 값이 "\n" 인 경우
		result[strlen(result) - 1] = '\0'; // result 의 마지막 값을 "\0" 대입

	return result;
}

int score_blank(char *id, char *filename) // 인자로 들어오는 학번, ".txt" filename 으로 만든 file 이 정답 맞았는지 확인하는 함수
{
	char tokens[TOKEN_CNT][MINLEN];
	node *std_root = NULL, *ans_root = NULL; // std_root : s_answer 에 대해 tree 의 root node, ans_root : a_answer 에 대해 tree 의 root node 
	int idx, start;
	char tmp[BUFLEN * 4]; // filename 임시 저장 변수
	char s_answer[BUFLEN], a_answer[BUFLEN]; // s_answer : 학생이 작성한 답안 저장 변수 , a_answer : 정답지에 작성된 정답 저장 변수
	char qname[FILELEN];
	int fd_std, fd_ans; // fd_std : stuDir 에서 학생 학번 디렉토리에 존재하는 파일 open 한 file descriptor 저장 변수 , fd_ans : ansDir 에서 .txt file open 한 file descriptor 저장 변수
	int result = true; // 결과 저장 변수
	int has_semicolon = false; // semicolon 여부 확인 저장 변수;

	memset(qname, 0, sizeof(qname)); // qname 비우기
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); // filename 에서 확장자 제거하고 파일 이름만 qname 에 저장

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename); // stuDir, id : 인자로 받은 학번, filename 으로 경로 만들어서 tmp 에 저장 == "stuDir/학번/filename" 형태
	fd_std = open(tmp, O_RDONLY); // tmp 에 저장된 파일 이름으로 file open 
	strcpy(s_answer, get_answer(fd_std, s_answer)); // 학생이 작성한 정답을 get 해서 s_answer 저장

	if(!strcmp(s_answer, "")){ // s_answer 값이 없을 경우
		close(fd_std);
		return false;
	}

	if(!check_brackets(s_answer)){ // s_answer 에서 "(" , ")" 개수 다를 경우
		close(fd_std);
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer))); // s_answer 좌우 공백 제거

	if(s_answer[strlen(s_answer) - 1] == ';'){ // s_answer 마지막 값이 ";" 인 경우
		has_semicolon = true; // semicolon 여부 확인 변수 true
		s_answer[strlen(s_answer) - 1] = '\0'; // s_answer 마지막 값에 "\0" 대입
	}

	if(!make_tokens(s_answer, tokens)){ // s_answer 를 문자열 분리해서 tokens 에 저장하기
		close(fd_std); // open 한 학생이 작성한 파일 닫기
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0); // std_root 로 트리 생성후 root 노드 저장 (&idx 로 증가된 idx 값 사용)

	sprintf(tmp, "%s/%s", ansDir, filename); // tmp 에 "ansDir/filename" 경로를 저장
	fd_ans = open(tmp, O_RDONLY); // tmp 에 저장된 file 을 open 해서 file descriptor 저장 

	while(1)
	{
		ans_root = NULL; // 
		result = true; // result 결과 true 저장

		for(idx = 0; idx < TOKEN_CNT; idx++) 
			memset(tokens[idx], 0, sizeof(tokens[idx])); // tokens 비우기

		strcpy(a_answer, get_answer(fd_ans, a_answer)); // fd_ans file descriptor file 에서 a_answer 에 작성된 정답을 저장

		if(!strcmp(a_answer, "")) // a_answer 가 null 인 경우
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer))); // a_answer 좌 우 공백 제거후 저장

		if(has_semicolon == false) // semicolon 여부 확인 변수 false 인 경우
		{
			if(a_answer[strlen(a_answer) -1] == ';') // a_answer 마지막 값이 ";" 인 경우
				continue;
		}

		else if(has_semicolon == true) // semicolon 여부 확인 변수 true 인 경우
		{
			if(a_answer[strlen(a_answer) - 1] != ';') // a_answer 마지막 값이 ";" 아닌 경우
				continue;
			else // a_answer 마지막 값이 ";" 인 경우
				a_answer[strlen(a_answer) - 1] = '\0'; // a_answer 마지막 값에 '\0' 저장
		}

		if(!make_tokens(a_answer, tokens)) // a_answer 를 문자열 분리해서 tokens 에 저장하기
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0); // ans_root 로 트리 생성후 root 노드 저장 (&idx 로 증가된 idx 값 사용)

		compare_tree(std_root, ans_root, &result); // std_root , ans_root node 로 만들어진 tree 비교하기 

		if(result == true) // result 가 true 인 경우 == std_root, ans_root 트리가 서로 같은 경우 result = true
		{
			close(fd_std); 
			close(fd_ans);

			if(std_root != NULL) // std_root 가 NULL 아닐 경우
				free_node(std_root); // std_root 트리를 제거하기
			if(ans_root != NULL) // ans_root 가 NULL 일 경우
				free_node(ans_root); // ans_root 트리를 제거하기
			return true;
		}
	}
	
	close(fd_std); // fd_std file close 하기
	close(fd_ans); // fd_ans file close 하기

	if(std_root != NULL) // std_root 가 NULL 아닐 경우
		free_node(std_root); // std_root 트리를 제거하기
	if(ans_root != NULL) // ans_root 가 NULL 일 경우
		free_node(ans_root); // ans_root 트리를 제거하기

	return false;
}

double score_program(char *id, char *filename) // 인자로 넣어준 id, filename 으로 compile 하고 결과값으로 프로그램 점수 매기는 함수
{
	double compile;
	int result;

	compile = compile_program(id, filename); // 인자로 넣어준 id, filename 의 ".c" 코드를 compile 해서 compile 성공 여부를 확인

	if(compile == ERROR || compile == false) // compile 값이 ERROR, false 중에 하나인 경우
		return false;
	
	result = execute_program(id, filename); // 학번, filename 으로 인자로 넣어준 file 실행해서 결과값을 정답 결과값을 비교해서 저장

	if(!result)
		return false;

	if(compile < 0) // compile 음수일 경우
		return compile; 

	return true;
}

int is_thread(char *qname) // threadFiles 에 인자로 넣어준 파일과 같은 이름이 있는 file 유무 확인하는 함수
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]);

	if(tOption && strlen(threadFiles[0]) == 0)
	{
		return true;
	}

	for(i = 0; i < size; i++){
		if(!strcmp(threadFiles[i], qname)) // threadFiles 에 인자로 넣어준 파일과 같은 이름이 있는 file 있을 경우
		{
			return true;
		}
			
	}
	return false;
}

double compile_program(char *id, char *filename) // 인자로 들어온 학번, filename 의 ".c" 코드를 compile 해서 compile 결과를 저장하는 함수
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN]; // tmp_f : filename 그대로 저장 변수 , tmp_e : filename 을 compile 할수있게 .exe file 로 바꿔서 저장 변수
	char *tmp_f_use = tmp_f;
	char *tmp_e_use = tmp_e;
	char command[BUFLEN * 4];
	char qname[FILELEN]; // 확장자 제외된 filename 저장 변수
	int isthread;
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname)); 
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); // filename 에서 확장자 제외하고 qname 에 저장하기
	
	isthread = is_thread(qname); // qname 과 동일한 file 을 처음 명령어 실행시 넣어줬을 경우 isthread = true 저장

	sprintf(tmp_f_use, "%s/%s", ansDir, filename);  // filename 그대로 저장
	sprintf(tmp_e_use, "%s/%s.exe", ansDir, qname); // filename 에서 ".c" 지우고 ".exe" file 로 바꿔서 저장

	if(tOption && isthread) // tOption && isthread 가 true 일 경우
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e_use, tmp_f_use); // "gcc" 컴파일 명령어 만들기
	else
		sprintf(command, "gcc -o %s %s", tmp_e_use, tmp_f_use); // "gcc" 컴파일 명령어 만들기

	sprintf(tmp_e_use, "%s/%s_error.txt", ansDir, qname); // tmp_e 에 저장된 값을 덮어쓰기 해서 error.txt file 명으로 tmp_e 에 저장
	fd = creat(tmp_e_use, 0666); // tmp_e file 생성

	redirection(command, fd, STDERR); // command 명령어 실행 후 표준 에러에 출력되는 값을 fd 에 출력
	size = lseek(fd, 0, SEEK_END); // fd file size 저장 변수
	close(fd); // fd file close 
	unlink(tmp_e_use); // "ansDir/filename_error.txt" 인 tmp_e 를 unlink

	if(size > 0)
		return false;

	sprintf(tmp_f_use, "%s/%s/%s", stuDir, id, filename); // "stuDir/학번/filename.c" 형태로 tmp_f 저장
	sprintf(tmp_e_use, "%s/%s/%s.stdexe", stuDir, id, qname); // "stuDir/학번/filename.stdexe" 형태로 tmp_e 저장

	if(tOption && isthread) // tOption == true && isthread == true 인 경우
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e_use, tmp_f_use); // 
	else
		sprintf(command, "gcc -o %s %s", tmp_e_use, tmp_f_use);

	sprintf(tmp_f_use, "%s/%s/%s_error.txt", stuDir, id, qname); // "stuDir/학번/filename_error.txt" 형태로 tmp_f 저장
	fd = creat(tmp_f_use, 0666); // "stuDir/학번/filename_error.txt" 파일 생성

	redirection(command, fd, STDERR); // 표준 에러로 출력하는 것을 인자 fd 에 출력
	size = lseek(fd, 0, SEEK_END); // fd 의 file 크기를 저장
	close(fd); // fd file close 

	if(size > 0) // fd file 크기 > 0 == stderr 로 발생된 error 가 있을 경우
	{ 
		if(eOption) // eOption 값이 true 인 경우 == eOption 들어온 경우
		{
			sprintf(tmp_e_use, "%s/%s", errorDir, id); // error.txt 저장하는 dir 인 errorDir, 학번인 id 로 tmp_e 저장
			if(access(tmp_e_use, F_OK) < 0) // tmp_e directory 가 존재하지 않을 경우
				mkdir(tmp_e_use, 0755); // tmp_e directory 생성

			sprintf(tmp_e_use, "%s/%s/%s_error.txt", errorDir, id, qname); // "error/학번/문제번호_error.txt" 형태로 tmp_e 저장
			rename(tmp_f_use, tmp_e_use); // tmp_f 이름을 tmp_e 로 저장

			result = check_error_warning(tmp_e_use); // tmp_e file 의 error, warning 검사해서 저장
		}
		else //  eOption 값이 false 인 경우 == eOption 안 들어온 경우
		{ 
			result = check_error_warning(tmp_f_use); // tmp_f file 의 error, warning 검사해서 저장
			unlink(tmp_f_use); // tmp_f file 삭제
		}

		return result; // file 실행 후 error, warning 으로 매긴 점수 return
	}

	unlink(tmp_f_use); // tmp_f file 삭제
	return true;
}

double check_error_warning(char *filename) // 인자로 들어온 file 내용에 error, warning 검사하는 함수
{
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;

	if(access(filename, F_OK) < 0)
	{
		printf("error directory doesn't exist\n");
		exit(1);
	}

	if((fp = fopen(filename, "r")) == NULL) // 인자로 들어온 filename file fopen 해서 에러 발생한 경우
	{
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	while(fscanf(fp, "%s", tmp) > 0) // fopen 한 file 의 내용을 tmp 에 저장
	{
		if(!strcmp(tmp, "error:")) // tmp 값이 "error:" 인 경우
			return ERROR;
		else if(!strcmp(tmp, "warning:")) // tmp 값이 "warning:" 인 경우
			warning += WARNING; // warning = warning - 0.1
	}

	return warning;
}

int execute_program(char *id, char *filename) // 인자로 넣은 학번, filename 으로 file 생성해서 프로그램 실행해서 출력 결과값을 정답 프로그램 출력 결과값 비교하는 함수
{
	char std_fname[BUFLEN * 4], ans_fname[BUFLEN * 4];// std_fname : "stuDir/학번/문제번호.stdout" 형태로 file 저장 변수 == 출력 결과 값 저장 변수 , ans_fname : "ansDir/문제번호.stdout" 형태 file 저장 변수 == 출력 결과 값 저장 변수
	char tmp[BUFLEN * 4]; // file 이름 임시 저장 변수
	char qname[FILELEN * 4];
	time_t start, end; // start : 프로그램 시작 전 시간 저장 변수 , end : 프로그램 시작 후 시간 저장 변수
	pid_t pid;
	int fd;

	memset(qname, 0, sizeof(qname)); // qname 비우기
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); // qname 을 확장자 제외한 filename 으로 저장

	sprintf(ans_fname, "%s/%s.stdout", ansDir, qname); // "ansDir/문제번호.stdout" 형태로 ans_fname 에 저장 
	fd = creat(ans_fname, 0666); // ans_fname file 생성 

	sprintf(tmp, "%s/%s.exe", ansDir, qname); // "ansDir/문제번호.exe" 형태로 tmp 에 저장 
	redirection(tmp, fd, STDOUT); // tmp file 이 .exe file 이므로 실행해서 STDOUT 표준 출력에 출력
	close(fd); // fd file close 

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname); // "stuDir/학번/문제번호.stdout" 형태로 std_fname 에 저장 
	fd = creat(std_fname, 0666); // std_fname file 생성

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname); // "stuDir/학번/문제번호.stdexe &" 형태로 std_fname 에 저장 

	start = time(NULL); // 현재 시각을 저장 == 프로그램 시작 시각
	redirection(tmp, fd, STDOUT); // tmp 명령어를 실행해서 표준출력에 출력할 값을 fd 에 출력
	
	sprintf(tmp, "%s.stdexe", qname); // "문제번호.stdexe" 형태로 tmp 에 저장
	while((pid = inBackground(tmp)) > 0) // 백그라운드에서 실행중인 pid 저장
	{
		end = time(NULL); // 현재 시각을 저장 == 프로그램 종료 시각

		if(difftime(end, start) > OVER){ // program 실행 시간 > OVER 일 경우
			kill(pid, SIGKILL); // pid process 를 강제 종료
			close(fd); // "stuDir/학번/문제번호.stdout" 형태 file close 
			return false;
		}
	}

	close(fd);

	return compare_resultfile(std_fname, ans_fname); // "stuDir/학번/문제번호.stdout", "ansDir/문제번호.stdout" file 내의 값이 같은지 여부 확인하는 함수
}

pid_t inBackground(char *name) // "ps | grep 문제번호.stdexe" 형태로 명령어 실행한 해당 프로세스의 PID 값을 return 하는 함수
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;
	
	memset(tmp, 0, sizeof(tmp)); // tmp 비우기
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); // "background.txt" file 생성해서 open 

	sprintf(command, "ps | grep %s", name); // "ps | grep 문제번호.stdexe" 형태로 command 에 저장
	redirection(command, fd, STDOUT); // "ps | grep 문제번호.stdexe" 형태인 command 명령어 실행 후 표준출력으로 출력된 값을 fd 에 출력

	lseek(fd, 0, SEEK_SET); // fd 파일의 현재 위치를 file 의 맨 처음으로 이동
	read(fd, tmp, sizeof(tmp)); // fd 파일에서 tmp 크기 만큼 읽어서 tmp 에 저장

	if(!strcmp(tmp, "")) // tmp 가 NULL 일 경우
	{
		unlink("background.txt"); // "background.txt" link 끊기
		close(fd); // "background.txt" file close
		return 0;
	}

	pid = atoi(strtok(tmp, " ")); // tmp 를 " " 구분자로 구분해서 나온 값을 정수형을 변환해서 pid 에 저장
	close(fd); // "background.txt" file close

	unlink("background.txt"); // "background.txt" link 끊기
	return pid; 
}

int compare_resultfile(char *file1, char *file2) // 인자로 들어온 두 file 내의 값을 하나씩 비교해서 같은지 여부 확인하는 함수 == 다를 경우 false , 같을 경우 true
{
	int fd1, fd2; // file descriptor 저장 변수
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY); // "stuDir/학번/문제번호.stdout" 형태로 인자로 들어온 file1 을 open 
	fd2 = open(file2, O_RDONLY); // "ansDir/문제번호.stdout" 형태로 인자로 들어온 file2 을 open

	while(1) // open 한 file 내에 있는 값을 한 글자씩 읽어서 비교해서 하나라도 다르면 false
	{
		while((len1 = read(fd1, &c1, 1)) > 0){ // fd1 에서 크기 "1" 만큼 read 해서 &c1 에 저장
			if(c1 == ' ') // c1 값이 ' ' 인 경우 == 공백 무시해서 지나감
				continue;
			else 
				break;
		}
		while((len2 = read(fd2, &c2, 1)) > 0){ // fd2 에서 크기 "1" 만큼 read 해서 &c2 에 저장
			if(c2 == ' ')  // c2 값이 ' ' 인 경우 == 공백 무시해서 지나감
				continue;
			else 
				break;
		}
		
		if(len1 == 0 && len2 == 0) // read 예외처리
			break;

		to_lower_case(&c1); // c1 대문자로 변화하기
		to_lower_case(&c2); // c2 대문자로 변화하기

		if(c1 != c2){ // c1, c2 값이 다를 경우
			close(fd1); // file close
			close(fd2); // file close
			return false;
		}
	}
	close(fd1); // file close
	close(fd2); // file close
	return true;
}

void redirection(char *command, int new, int old) // 기존 출력을 저장하는 old file descriptor 에서 new file descriptor 로 에 출력을 저장
{
	int saved;

	saved = dup(old); // old 인자의 file descriptor 를 saved 에 저장
	dup2(new, old); // old 를 new 로 file descriptor 를 바꾸기

	system(command); // command 명령어 실행

	dup2(saved, old); // old 를 saved 로 file descriptor 를 바꾸기
	close(saved); // saved file descriptor close
}

int get_file_type(char *filename) // 인자로 들어온 파일 유형 확인하는 함수
{
	char *extension = strrchr(filename, '.'); // filename 에서 마지막 "." 이후의 값 저장하는 변수

	if(!strcmp(extension, ".txt"))
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))
		return CFILE;
	else
		return -1;
}

void rmdirs(const char *path) // path 경로 내에 있는 디렉토리와 파일 모두 삭제
{
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[BUFLEN * 4];
	
	if((dp = opendir(path)) == NULL) // path 에 해당하는 directory open 
		return;

	while((dirp = readdir(dp)) != NULL) // open 한 directory 로 dirp 가 있을 경우
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) // dirp 구조체 멤버변수인 filename 이 "." , ".." 둘 중에 하나인 경우
			continue;

		sprintf(tmp, "%s/%s", path, dirp->d_name); // tmp 에 "path/filname" 으로 경로 저장

		if(lstat(tmp, &statbuf) == -1) // tmp 경로에 대해 lstat 함수로 얻은 stat 구조체를 statbuf 에 저장
			continue;

		if(S_ISDIR(statbuf.st_mode)) // stat 구조체인 statbuf 의 파일 유형이 directory 인 경우
			rmdirs(tmp); // 재귀적으로 rmdirs directory 실행
		else // stat 구조체인 statbuf 의 파일 유형이 directory 가 아닌 경우 == file 인 경우
			unlink(tmp); // tmp 경로에 해당하는 file unlink
	}

	closedir(dp); // path 경로의 directory close
	rmdir(path); // path 경로의 directory 삭제
}

void to_lower_case(char *c) // 소문자로 바꾸는 함수
{
	if(*c >= 'A' && *c <= 'Z') // c 가 "A" , "Z" 사이에 있을 경우
		*c = *c + 32; // ASCI 값 더하기
}

void print_usage() // Usage 출력 함수
{
	printf("Usage : ssu_score <STD_DIR> <ANS_DIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -n <CSVFILENAME>\n");
	printf(" -m\n");
	printf(" -c [STUDENTIDS ...]\n");
	printf(" -p [STUDENTIDS ...]\n");
	printf(" -t [QNAMES ...]\n");
	printf(" -s <CATEGORY> <1|-1>\n");
	printf(" -e <DIRNAME>\n");
	printf(" -h\n");
}

// 문제 링크드 리스트로 만들기 
void insert_problem(char* id_num, char* problem_num, double score) 
{
    Node* new_node = (Node*)malloc(sizeof(Node));
	new_node->id_num = (char*)malloc((strlen(id_num) + 1) * sizeof(char));
    new_node->problem_num = (char*)malloc((strlen(problem_num) + 1) * sizeof(char));

	strcpy(new_node->id_num, id_num);
    strcpy(new_node->problem_num, problem_num);
	new_node->score = score;
    
	new_node->prev = tail;
    new_node->next = NULL;
    
	if (tail != NULL) 
    {
        tail->next = new_node;
    }
    
	tail = new_node;
    
	if (head == NULL) 
    {
        head = new_node;
    }
}

// 문제 링크드 리스트 값 확인하는 함수
void print_list_problem(char *id) 
{
    Node* current = head;
    while (current != NULL) 
    {
		if(current->next != NULL)
        {
            if(strcmp(current->id_num, id) == 0)
			{
				if(strcmp(current->next->problem_num, "\n") == 0)
				{
					if(current->score - (int)(current->score) == 0)
					{
						printf("%s(%d)",current->problem_num, (int)(current->score));
					}
					else
					{
						printf("%s(%.2f)",current->problem_num, current->score);
					}
					
				}
				else if(strcmp(current->problem_num, "\n") == 0)
				{
					printf("\n");
				}
				else
				{
					if(current->score - (int)(current->score) == 0)
					{
						printf("%s(%d), ",current->problem_num, (int)(current->score));
					}
					else
					{
						printf("%s(%.2f), ",current->problem_num, current->score);
					}
				}
			}
        }
		// printf("problem : %s\n",current->problem_num);
        current = current->next;
    }
}

// 링크드 리스트 free 하는 함수
void free_linked_list()
{
    Node* current = head;
    while (current != NULL)
    {
        Node* next = current->next; // 다음 노드를 임시 변수에 저장
        free(current); // 현재 노드를 해제
        current = next; // 다음 노드로 이동
    }
    head = NULL; // head 포인터를 NULL로 초기화
}

// 학생 링크드 리스트로 만들기 
void insert_student(char *id, char *buf, double score)
{
	Student* new_student = (Student*)malloc(sizeof(Student));
	new_student->id_num = (char*)malloc((strlen(id) + 1) * sizeof(char));
	
	strcpy(new_student->id_num, id);
	strcpy(new_student->buffer, buf);
	new_student->student_score = score;

	new_student->prev = last;
	new_student->next = NULL;

	if (last != NULL) 
    {
        last->next = new_student;
    }

    last = new_student;
    
	if (first == NULL) 
    {
        first = new_student;
    }

}

// 학생 링크드 리스트 값 확인하는 함수
void print_list_student() 
{
    Student* current = first;
    while (current != NULL) 
    {
		printf("id_num : %s , buf : %s , score : %lf\n",current->id_num, current->buffer, current->student_score);
        current = current->next;
    }
}

// 학생 링크드 리스트 개수 
int count_list_student() 
{
    Student* current = first;
    int i = 0 ;
    while (current != NULL) 
    {
        i++;
        current = current->next;
    }

    return i ;
}

// 학생 링크드 리스트를 점수로 오름차순 정렬
void sort_score_ascending()
{
    if (first == NULL)
        return;

    int swapped = 1;
    Student *current;
    Student *end = NULL;

    while (swapped)
    {
        swapped = 0;
        current = first;

        while (current->next != end)
        {
            if (current->student_score > current->next->student_score)
            {
                double tempScore = current->student_score;
                char tempId[10];
				char tempBuffer[BUFLEN];
                strcpy(tempId, current->id_num);
				strcpy(tempBuffer, current->buffer);

                current->student_score = current->next->student_score;
                strcpy(current->id_num, current->next->id_num);
				strcpy(current->buffer, current->next->buffer);

                current->next->student_score = tempScore;
                strcpy(current->next->id_num, tempId);
				strcpy(current->next->buffer, tempBuffer);

                swapped = 1;
            }
            current = current->next;
        }
        end = current;
    }
}



// 학생 링크드 리스트를 점수로 내림차순 정렬
void sort_score_descending() 
{
	if (first == NULL) 
		return;

    int swapped;
    Student* current;
    Student* end = NULL;

    while (swapped)
    {
        swapped = 0;
        current = first;

        while (current->next != end)
        {
            if (current->student_score < current->next->student_score)
            {
                double tempScore = current->student_score;
                char tempId[10];
				char tempBuffer[BUFLEN];
                strcpy(tempId, current->id_num);
				strcpy(tempBuffer, current->buffer);

                current->student_score = current->next->student_score;
                strcpy(current->id_num, current->next->id_num);
				strcpy(current->buffer, current->next->buffer);

                current->next->student_score = tempScore;
                strcpy(current->next->id_num, tempId);
				strcpy(current->next->buffer, tempBuffer);

                swapped = 1;
            }
            current = current->next;
        }
        end = current;
    }
}

// 학생 링크드 리스트를 학번으로 내림차순 정렬
void sort_id_descending() 
{
	if (first == NULL) 
		return;

    int swapped;
    Student* current;
    Student* end = NULL;

	while (swapped)
    {
        swapped = 0;
        current = first;

        while (current->next != end)
        {
            int current_id = atoi(current->id_num);
			int next_id = atoi(current->next->id_num);

            if (current_id < next_id)
            {
                double tempScore = current->student_score;
                char tempId[10];
				char tempBuffer[BUFLEN];
                strcpy(tempId, current->id_num);
				strcpy(tempBuffer, current->buffer);

                current->student_score = current->next->student_score;
                strcpy(current->id_num, current->next->id_num);
				strcpy(current->buffer, current->next->buffer);

                current->next->student_score = tempScore;
                strcpy(current->next->id_num, tempId);
				strcpy(current->next->buffer, tempBuffer);

                swapped = 1;
            }
            current = current->next;
        }
        end = current;
    }
}

// 인자로 넣은 학번이 존재하는 학번인지 확인하는 함수
int check_student_dir(char *stuDir, char (*ids)[FILELEN])
{
    DIR *dirp;
    struct dirent *dirent;
    int i, j, exist = 0;
	int ids_num = 0;

    // 학생 디렉토리 열기
    if((dirp = opendir(stuDir)) == NULL) 
	{
        fprintf(stderr, "Error: cannot open directory %s\n", stuDir);
        exit(1);
    }

    // 디렉토리 내부 검색
    for(i = 0 ; i < ARGNUM ; i++) 
	{
		if(strlen(ids[i]) != 0)
			ids_num++;

        while((dirent = readdir(dirp)) != NULL) 
		{
            // 숨김 파일과 현재 디렉토리, 부모 디렉토리 제외
            if(dirent->d_name[0] == '.') 
				continue;

            // 학생 ID와 일치하는 디렉토리 존재 여부 검사
            if(strcmp(dirent->d_name, ids[i]) == 0) 
			{
                exist++;
                break;
            }
        }
        // 디렉토리 내부 검색을 처음부터 다시 시작
        rewinddir(dirp);
    }

    // 디렉토리 닫기
    closedir(dirp);

	if(ids_num != exist)
		return 0;
	else	
		return 1;
}

// 정렬한 값을 ".csv" file 에 저장하는 함수
void save_sorting_score()
{
	int fd;

	if((fd = creat(result_path, 0666)) < 0){ // "score.csv" file 생성
		fprintf(stderr, "creat error for score.csv");
		return;
	}

	write_first_row(fd); // open 한 파일에 첫 줄에 문제 번호들과 "sum" 작성하는 함수

	Student* current = first;
    while (current != NULL) 
    {
		write(fd, current->id_num, strlen(current->id_num));
		write(fd, ",", 1);
		write(fd, current->buffer, strlen(current->buffer));
		//printf("id_num : %s , buf : %s , score : %lf\n",current->id_num, current->buffer, current->student_score);
        current = current->next;
    }
}


double return_score_student(char *id) // 학생 디렉토리에 존재하는 file 모두 검사후 점수 매기는 함수
{
	int type;
	double result;
	double score = 0; // 점수 저장 변수
	int i;
	int size = sizeof(score_table) / sizeof(score_table[0]); // score_table 저장된 문제 개수 저장 변수

	for(i = 0; i < size ; i++) // score_table 에 저장된 문제 개수 만큼 반복해서 총 score 저장하는 반복문
	{
		if(score_table[i].score == 0) // score_table 점수가 0 인 경우
			break;

		if((type = get_file_type(score_table[i].qname)) < 0) // score_table 존재하는 파일의 file type 저장
			continue;
		
		if(type == TEXTFILE) // file type 이 ".txt" 인 경우
			result = score_blank(id, score_table[i].qname); // 인자로 넣은 id, .txt file 로 맞을 경우 result = true , 틀렸을 경우 result = false 로 저장
		else if(type == CFILE) // file type 이 ".c" 인 경우
			result = score_program(id, score_table[i].qname); // 인자로 넣은 id, .c file 실행하고 결과값 확인해서 double 형인 result 점수 저장

		if(result == false) // result == false 인 경우
		{

		}
		else // result != false 인 경우
		{ 
			if(result == true) // result == true 인 경우
			{ 
				score += score_table[i].score; // score_table 에 문제마다 지정된 점수를 score 에 저장
			}
			else if(result < 0) // result < 0 인 경우
			{
				score = score + score_table[i].score + result; // score_table 에 문제마다 지정된 점수와 ".c" 검사하고 깍인 점수를 더해서 score 에 저장
			}
			
		}
	}

	return score;
}