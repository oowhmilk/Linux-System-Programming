#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/wait.h>

#define MAX_PATH 4096

int correct_path(char* backup_path, char* temp_path);

char* cut_time_path(char *path, char* temp);

char* cut_path(char *path, char* temp, int backup_path_length);

char* change_path(char* input_path, char* backup_path);

char* return_num(long num, char* change_num);

char* make_realpath(char* path);

void insert(char* path, int file_type);

int count_list(); 

void print_list(); 

void delete_list();

void traverse_directory(char* dir_path);

// 링크드 리스트 Node 
typedef struct Node {
    char* path;
    int file_type;
    struct Node* prev;
    struct Node* next;
} Node;

Node* head = NULL;
Node* tail = NULL;

int main(int argc, char** tokens)
{
    char* input_path = NULL;
    char* backup_path = NULL;
    char* temp_path = NULL;
    char* current_path = NULL;
    char* backup_directory_path = NULL;

    if (tokens != NULL )
    {
        fflush(stdout);
    }

    input_path = (char*) malloc(sizeof(char) * MAX_PATH); // 입력받은 경로를 저장하는 변수
    backup_path = (char*) malloc(sizeof(char) * MAX_PATH); // 결과 경로를 저장하는 변수
    temp_path = (char*) malloc(sizeof(char) * MAX_PATH); // 임시로 경로를 저장하는 변수
    current_path = (char*) malloc(sizeof(char) * MAX_PATH); // 현재 작업중인 경로를 저장하는 변수
    backup_directory_path = (char*) malloc(sizeof(char) * MAX_PATH);// 백업 디렉토리 경로를 저장하는 변수

    char** path_tokens = NULL; // 토큰들을 저장할 메모리 공간 동적 할당
    char* path_token = NULL; // 첫 번째 토큰 추출
    int path_token_count = 0; // 토큰의 개수를 저장하는 변수

    // 홈 디렉토리 찾기 
    char cwd[4096];
    getcwd(cwd, sizeof(cwd));
    strcpy(current_path, cwd);
    char* home_path = (char*) malloc(sizeof(char) * 256); 

    // 백업할 경로를 "/" 로 나누기 (나눠서 각 디렉토리 보기)
    path_tokens = (char**) malloc(sizeof(char*) * 256); // 토큰들을 저장할 메모리 공간 동적 할당
    path_token = strtok(cwd, "/"); // 첫 번째 토큰 추출
    path_token_count = 0;
    
    while (path_token != NULL) // 토큰이 더 이상 없을 때까지 반복
    { 
        path_tokens[path_token_count] = path_token; // 현재 토큰을 저장
        path_token_count++;
        path_token = strtok(NULL, "/"); // 다음 토큰 추출
    }

    // 사용자 홈 디렉토리 경로 만들기 
    strcat(home_path,"/");
    strcat(home_path, path_tokens[0]);
    strcat(home_path,"/");
    strcat(home_path, path_tokens[1]);

    if (strcmp(tokens[1], "md5") == 0 || strcmp(tokens[1], "sha1") == 0) // 첫 번째 인자 입력이 없는 경우 
    {
        printf("Usage : remove <FILENAME> [OPTION]\n");
        printf("  -a : remove all file(recursive)\n");
        printf("  -c : clear backup directory\n");
    }
    else
    {
        strcpy(input_path, tokens[1]); // 입력받은 경로 저장하기 

        // 상대경로 입력 시
        if (input_path[0] != '/')
        {
            if(input_path[0] == '~')
            {
                strcpy(temp_path, home_path);
                strcat(temp_path, input_path + 1);
                strcpy(input_path, temp_path);
            }
            else
            {
                strcpy(temp_path, current_path);
                strcat(temp_path, "/");
                strcat(temp_path, input_path);
                strcpy(input_path, temp_path);
            }
        } 
        // 절대경로 입력 시
        else 
        {
            
        }

        // backup 디렉토리 경로 만들기 
        backup_directory_path = change_path(home_path, backup_directory_path);

        // 백업했던 경로 구하기
        backup_path = change_path(input_path, backup_path);

        // 백업했던 경로 realpath() 함수 사용한것처럼 만들어주기
        backup_path = make_realpath(backup_path);

        struct stat backup_path_check; // 파일 정보를 저장할 구조체 
        stat(backup_path, &backup_path_check); // stat 함수로 파일 정보를 가져오기      

        // 파일 링크드리스트로 구현하기
        if (S_ISDIR(backup_path_check.st_mode) == 1)
        {
            insert(backup_path, S_ISDIR(backup_path_check.st_mode));
            traverse_directory(backup_path);
        }
        else
        {
            traverse_directory(backup_directory_path);
        }

        if (strlen(tokens[1]) < MAX_PATH && strcmp(tokens[1], "-c") != 0) // 경로 길이가 제한을 넘을 경우
        {
            
            if(strstr(input_path, home_path) == NULL) // input_path 경로에 홈 디렉토리 가 포함되었는지 확인하기 
            {
                printf("\"%s\" can't be backuped\n",input_path); // input_path 경로에 홈 디렉토리가 포함되지 않은 경우 
            }
            else
            {
                if (strstr(input_path, "/backup") != NULL) // input_path 경로에 "backup" 가 포함되어있는지 확인하기 
                {
                    printf("\"%s\" can't be backuped\n",input_path); // input_path 경로에 "backup" 포함된 경우 
                }
                else
                {
                    if(correct_path(backup_path, temp_path) == 0) // 제대로 된 경로인 경우 
                    {
                        struct stat check; // 파일 정보를 저장할 구조체 
                        stat(backup_path, &check); // stat 함수로 파일 정보를 가져오기

                        if (S_ISDIR(check.st_mode) == 1) // 입력받은 경로 파일 유형이 디렉토리인 경우 
                        {
                            if(tokens[2] != NULL && strcmp(tokens[2], "md5") != 0 && strcmp(tokens[2], "sha1") != 0) // 세번째 인자로 옵션이 들어온 경우
                            {
                                if(strcmp(tokens[2], "-a") == 0 && (strcmp(tokens[3], "md5") == 0 || strcmp(tokens[3], "sha1") == 0)) // 세번째 인자가 -a 인 경우 인자는 최대 세번째까지 받을수있는 경우
                                {
                                    Node* current = head;
                                    while (current != NULL) 
                                    {
                                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                        temp = cut_path(current->path, temp, (int)strlen(backup_path)); // 백업했던 파일 중에 input_path 랑 똑같은 디렉토리에 존재하는 파일 저장 변수

                                        struct stat check; // 파일 정보를 저장할 구조체 
                                        stat(current->path, &check); // stat 함수로 파일 정보를 가져오기

                                        if(strcmp(backup_path, temp) == 0)
                                        {
                                            if(remove(current->path) == 0) // 파일 삭제 성공한 경우
                                            {
                                                if(S_ISDIR(check.st_mode) == 1) // 입력받은 경로 파일 유형이 디렉토리인 경우
                                                {
                                                    
                                                }
                                                else // 입력받은 경로 파일 유형이 디렉토리가 아닌 경우
                                                {
                                                    printf("\"%s\" backup file removed\n",current->path); // 입력받은 경로 파일 유형이 디렉토리가 아닌 경우만 삭제한 파일 출력하기
                                                }
                                            }
                                            else
                                            {
                                                printf("fail file remove\n");
                                            }
                                        }
                                        current = current->next; // 링크드 리스크로 구현된 파일 구조 확인하기
                                    }
                                    
                                }
                                else if(strcmp(tokens[2], "-a") != 0 && (strcmp(tokens[3], "md5") == 0 || strcmp(tokens[3], "sha1") == 0)) // 세번째 인자가 -a 가 아닌 다른 값이 들어온 경우 
                                {
                                    printf("Usage : remove <FILENAME> [OPTION]\n");
                                    printf("  -a : remove all file(recursive)\n");
                                    printf("  -c : clear backup directory\n");
                                }
                            }
                            else if(strcmp(tokens[2], "md5") == 0 || strcmp(tokens[2], "sha1") == 0) // 세번째 인자가 안들어온 경우
                            {
                                printf("Usage : remove <FILENAME> [OPTION]\n");
                                printf("  -a : remove all file(recursive)\n");
                                printf("  -c : clear backup directory\n");
                            }
                        }

                        else if (S_ISDIR(check.st_mode) != 1) // 입력받은 경로 파일 유형이 디렉토리가 아닌 경우 
                        {
                            if(tokens[2] != NULL && (strcmp(tokens[2], "md5") != 0 && strcmp(tokens[2], "sha1") != 0)) // 세번째 인자로 옵션이 들어온 경우
                            {
                                if(strcmp(tokens[2], "-a") == 0 && (strcmp(tokens[3], "md5") == 0 || strcmp(tokens[3], "sha1") == 0)) // 세번째 인자가 -a 인 경우 인자는 최대 세번째까지 받을수있는 경우
                                {
                                    Node* current = head;
                                    while (current != NULL) 
                                    {
                                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                        temp = cut_time_path(current->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기

                                        if(strcmp(backup_path, temp) == 0)// 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                        {
                                            if(remove(current->path) == 0) // 파일 삭제 성공한 경우
                                            {
                                                printf("\"%s\" backup file removed\n",current->path);
                                            }
                                            else
                                            {
                                                printf("fail file remove\n");
                                            }
                                        }
                                        
                                        current = current->next; // 링크드 리스크로 구현된 파일 구조 확인하기
                                    }
                                }
                                else if(strcmp(tokens[2], "-a") != 0 && (strcmp(tokens[3], "md5") == 0 || strcmp(tokens[3], "sha1") == 0)) // 세번째 인자가 -a 가 아닌 옵션이 들어온 경우
                                {
                                    printf("Usage : remove <FILENAME> [OPTION]\n");
                                    printf("  -a : remove all file(recursive)\n");
                                    printf("  -c : clear backup directory\n");
                                }
                            }
                            else if((strcmp(tokens[2], "md5") == 0 || strcmp(tokens[2], "sha1") == 0))
                            {
                                Node* current = head; // 링크드 리스트 확인하기 

                                int file_num = 0; // 존재하는 파일 개수를 저장하는 변수

                                char clock[13]; // 백업했던 파일의 시간 저장하는 변수

                                while (current != NULL) 
                                {
                                    char* temp = (char*) malloc(sizeof(char) * MAX_PATH); // 임시로 경로를 저장하는 변수
                                    temp = cut_time_path(current->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기
                                    if(strcmp(backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                    {
                                        file_num++;
                                    }
                                    else
                                    {
                                        
                                    }
                                    current = current->next; // 링크드 리스크로 구현된 파일 구조 확인하기
                                }

                                if(file_num == 1)
                                {
                                    Node* check = head;
                                    int check_num = 0 ; 
                                    while (check != NULL)
                                    {
                                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                        temp = cut_time_path(check->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기

                                        if(strcmp(backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                        {
                                            if(remove(check->path) == 0) // 파일 삭제 성공한 경우
                                            {
                                                printf("\"%s\" backup file removed\n",check->path);
                                            }
                                            else
                                            {
                                                printf("fail file remove\n");
                                            }
                                        }
                                        check = check->next;
                                    }

                                }
                                else
                                {
                                    current = head; // 링크드리스트 순회를 위해 current 재설정

                                    int print_num = 0; // 같은 파일 출력용 번호 변수

                                    printf("backup file list of \"%s\"\n",input_path);
                                    printf("0. exit\n");

                                    while (current != NULL) 
                                    {
                                        temp_path = strcpy(temp_path, current->path);
                                        
                                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH); // 임시로 경로를 저장하는 변수
                                        
                                        char* clock = (char*) malloc(sizeof(char) * 13); // 백업했던 파일의 시간 저장하기 
                                        
                                        for(int i = (int)strlen(current->path) - 12 , num = 0; i < (int)strlen(current->path)  ; i++ , num++)
                                        {
                                            clock[num] = current->path[i];
                                        }
                                        clock[12] = '\0';

                                        temp = cut_time_path(current->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기

                                        FILE *fp;
                                        long file_size;

                                        fp = fopen(current->path, "rb"); // 파일을 바이너리 모드로 읽기 위해 "rb" 모드로 오픈

                                        if (fp == NULL) 
                                        {
                                            printf("파일 열기에 실패하였습니다.\n");
                                            exit(1);
                                        }

                                        fseek(fp, 0, SEEK_END); // 파일 포인터를 파일 끝으로 이동
                                        file_size = ftell(fp); // 파일 포인터 위치를 파일 크기로 변환
                                        fclose(fp);

                                        if(strcmp(backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                        {
                                            print_num++;
                                            printf("%d. %s\t\t",print_num, clock);

                                            char* change_num = (char*) malloc(sizeof(char) * 20);
                                            change_num = return_num(file_size, change_num);

                                            printf("%s\n",change_num);

                                        }
                                        else
                                        {
                                            
                                        }

                                        current = current->next; // 링크드 리스크로 구현된 파일 구조 확인하기
                                    }

                                    printf("choose file to remove\n");
                                    printf(">> ");

                                    char* choose_file = (char*) malloc(sizeof(char) * file_num); // 고를 파일을 저장하는 변수
                                    gets(choose_file); // 표준 입력을 입력 받기 
                                    int choose_num = atoi(choose_file); // 고른 파일 번호 저장하는 변수

                                    // choose_num 으로 고른 파일을 링크드 리스트 순회 방법으로 같은 위치에 있는 파일을 찾기위해 while 
                                    Node* check = head;
                                    int check_num = 0 ; 
                                    while (check != NULL)
                                    {
                                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                        temp = cut_time_path(check->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기

                                        if(strcmp(backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                        {
                                            check_num++;
                                            if(check_num == choose_num)
                                            {
                                                if(remove(check->path) == 0) // 파일 삭제 성공한 경우
                                                {
                                                    printf("\"%s\" backup file removed\n",check->path);
                                                }
                                                else
                                                {
                                                    printf("fail file remove\n");
                                                }
                                                
                                            }
                                        }
                                        
                                        check = check->next;
                                    }

                                }

                                
                            }
                        }
                    }
                    else // 제대로 된 경로가 아닌 경우 
                    {
                        printf("Usage : remove <FILENAME> [OPTION]\n");
                        printf("  -a : remove all file(recursive)\n");
                        printf("  -c : clear backup directory\n");
                    }    
                }
            }
        }
        else if(strlen(tokens[1]) < MAX_PATH && strcmp(tokens[1], "-c") == 0) // backup 디렉토리 내에 있는 모든 파일을 삭제하는 경우
        {
            if(strcmp(tokens[2], "md5") != 0 && strcmp(tokens[2], "sha1") != 0) 
            {
                printf("Usage : remove <FILENAME> [OPTION]\n");
                printf("  -a : remove all file(recursive)\n");
                printf("  -c : clear backup directory\n");
            }
            else
            {
                int directory_num = 0;
                int file_num = 0;

                Node* current = head;
                while (current != NULL) 
                {
                    struct stat check; // 파일 정보를 저장할 구조체 
                    stat(current->path, &check); // stat 함수로 파일 정보를 가져오기

                    if (S_ISDIR(check.st_mode) == 1) // 입력받은 경로 파일 유형이 디렉토리인 경우 
                    {
                        directory_num++;
                    }
                    else
                    {
                        file_num++;
                    }


                    if(strcmp(backup_directory_path, current->path) == 0) // backup_directory_path 와 current->path 같아서 backup directroy 삭제하지 않는 경우
                    {

                    }
                    else
                    {
                        if(remove(current->path) == 0) // 파일 삭제 성공한 경우
                        {
                            //printf("\"%s\" backup file removed\n",current->path);
                        }
                        else
                        {
                            printf("fail file remove\n");
                        }
                    }

                    current = current->next; // 링크드 리스크로 구현된 파일 구조 확인하기
                }

                if(directory_num == 0 && file_num == 0) // backup 디렉토리내에 아무것도 아무 파일이 없는 경우
                {
                    printf("no file(s) in the backup\n");
                }
                else
                {
                    printf("backup directory cleared(%d regular files and %d subdirectories totally).\n",file_num, directory_num);
                }
            }
        }        
        else
        {
            printf("Limit Path Length\n"); // 경로 길이 제한
        }
    }

    exit(0);
    return 0;
}


// 입력받은 경로가 제대로 된 경로인지 확인하는 함수
int correct_path(char* backup_path, char* temp_path)
{
    Node* current = head; // 링크드 리스트 확인하기

    while (current != NULL) 
    {
        temp_path = strcpy(temp_path, current->path);
        
        char* temp = NULL; // 임시로 경로를 저장하는 변수

        temp = cut_time_path(current->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기

        if(strcmp(backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
        {
            return 0;
            break;
        }
        
        current = current->next; // 링크드 리스크로 구현된 파일 구조 확인하기
    }
    return -1;
}


// 입력받은 경로를 수정하는 함수 
char* change_path(char* input_path, char* backup_path)
{
    char* input = NULL;
    char** front_tokens = NULL;
    char** back_tokens = NULL;
    char* token = NULL;

    input = (char*) malloc(sizeof(char) * MAX_PATH);
    strcpy(input, input_path);
    front_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
    back_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
    token = strtok(input, "/"); // 첫 번째 토큰 추출

    int token_count = 0 ; // 토큰의 개수를 저장하는 변수
    while (token != NULL) // 토큰이 더 이상 없을 때까지 반복
    {
        if(token_count < 2)
        {
            front_tokens[token_count] = token; // 토큰들 중에서 앞에 2개의 토큰을 저장
        }
        else
        {
            back_tokens[token_count] = token; // 토큰들 중에서 앞에 2개를 제외한 토큰을 저장 
        }
        token_count++;
        token = strtok(NULL, "/"); // 다음 토큰 추출
    }

    memset(backup_path, '\0', sizeof(char) * MAX_PATH);

    // 결과 경로를 저장
    for(int i = 0 ; i < 2 ; i++)
    {
        strcat(backup_path, "/");
        strcat(backup_path, front_tokens[i]);
    }
    strcat(backup_path, "/backup");
    for(int i = 2 ; i < token_count ; i++)
    {
        strcat(backup_path, "/");
        strcat(backup_path, back_tokens[i]);
    }
    
    return backup_path;

}


// backup path 에서 뒤에 14자리 자르는 함수
char* cut_time_path(char *path, char* temp)
{
    struct stat check; // 파일 정보를 저장할 구조체 
    stat(path, &check); // stat 함수로 파일 정보를 가져오기

    if(S_ISDIR(check.st_mode) == 1) // 입력받은 경로 파일 유형이 디렉토리인 경우 
    {
        return path;
    }
    else
    {
        temp = (char*) malloc(sizeof(char) * (int)strlen(path) - 12); // 임시로 경로를 저장하는 변수 길이를 지정하기 
    
        for(int i = 0 ; i < (int)strlen(path) - 13 ; i++)
        {
            temp[i] = path[i];
        }
        temp[(int)strlen(path) - 13] ='\0';

        return temp;
    }
}


// backup path 랑 똑같게 자르는 함수
char* cut_path(char *path, char* temp, int backup_path_length)
{
    temp = (char*) malloc(sizeof(char) * (backup_path_length + 1)); // 임시로 경로를 저장하는 변수 길이를 지정하기 
    
    for(int i = 0 ; i < backup_path_length ; i++)
    {
        temp[i] = path[i];
    }
    temp[backup_path_length] ='\0';

    return temp;
}


// 숫자 입력시 숫자 3자리씩 끊어서 보여주는 함수
char* return_num(long num, char* change_num)
{
    char* str = (char*) malloc(sizeof(char) * 20);

	sprintf(str, "%ld", num); // 입력된 정수를 문자열로 변환
	int len = strlen(str);
    int count = 0;
	
	if (len > 3)
	{
		for (int i = 0; i < len; i++)
        {
            if ((i != 0) && ((len - i) % 3 == 0))
            {
                change_num[count++] = ',';
            }	
            change_num[count++] = str[i];
        }
	}

	else
	{
		strcpy(change_num,str);
	}

    return change_num;
}


// realpath 처럼 만들어주는 함수
char* make_realpath(char* path) 
{
    size_t path_len = strlen(path);
    char* result = (char*) malloc((path_len + 1) * sizeof(char));
    int i, j;

    if (strncmp(path, "./", 2) == 0) // "./" 로 시작할때 무시하고 시작하기
    {
        i = 2;
        j = 0;
        while (path[i] != '\0') 
        {
            if (path[i] == '/') 
            {
                i++;
                continue;
            }
            result[j++] = path[i++];
        }
        result[j] = '\0';
        path_len = j;
        path = result;
    }

    i = 0;
    j = 0;
    while (i < path_len) 
    {
        if (strncmp(&path[i], "/../", 4) == 0) 
        { // "/../" 값이 존재할때 이전 디렉토리 로 result 저장하기

            int prev_dir_pos = j - 1; // 이전 디렉토리 위치

            while (prev_dir_pos >= 0 && result[prev_dir_pos] != '/') 
            {
                prev_dir_pos--;
            }
            if (prev_dir_pos < 0) 
            {
                prev_dir_pos = 0;
            }

            j = prev_dir_pos;
            i += 3;
        }
        else if (strncmp(&path[i], "/./", 3) == 0) 
        { // "/./" 값이 존재할때 무시하고 지나가기
            i += 2;
        }
        else {
            result[j++] = path[i++];
        }
    }
    result[j] = '\0';

    if (strcmp(&result[j - 2], "/.") == 0) // "./" 로 끝날때 무시하기
    { 
        result[j - 1] = '\0';
    }


    return result;
}


// 파일 관리 링크드 리스트로 만들기 
void insert(char* path, int file_type) 
{
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->path = (char*)malloc((strlen(path) + 1) * sizeof(char));
    strcpy(new_node->path, path);
    new_node->file_type = file_type;
    new_node->prev = NULL;
    new_node->next = head;
    if (head != NULL) 
    {
        head->prev = new_node;
    }
    head = new_node;
    if (tail == NULL) 
    {
        tail = new_node;
    }
}

// 링크드 리스트 값 뒤에서 부터 확인하는 함수
int count_list() 
{
    Node* current = tail;
    int i = 0 ;
    while (current != NULL) 
    {
        i++;
        current = current->prev;
    }
    return i ;
}

// 링크드 리스트 값 앞에서 부터 확인하는 함수
void print_list() 
{
    Node* current = head;
    while (current != NULL) 
    {
        if (current->file_type == 1) 
        {
            printf("Directory: %s\n", current->path);
        } 
        else 
        {
            printf("File: %s\n", current->path);
        }
        current = current->next;
    }
}

// 링크드 리스트 삭제하는 함수
void delete_list() 
{
    Node* current = head;
    while (current != NULL) 
    {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL;
}

// 링크드 리스트 재귀로 해당 디렉토리에 존재한는 파일 저장하는 함수
void traverse_directory(char* dir_path) 
{
    DIR* dir = opendir(dir_path);
    if (dir == NULL) 
    {
        mkdir(dir_path, 0755);
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
        {
            continue;
        }
        char path[MAX_PATH];
        sprintf(path, "%s/%s", dir_path, entry->d_name);
        struct stat st;
        if (stat(path, &st) == -1) 
        {
            perror("stat");
            continue;
        }
        if (S_ISDIR(st.st_mode)) 
        {
            insert(path, S_ISDIR(st.st_mode));
            traverse_directory(path);
        } 
        else 
        {
            insert(path, S_ISDIR(st.st_mode));
        }
    }
    closedir(dir);
}