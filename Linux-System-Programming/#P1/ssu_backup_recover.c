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
#include <openssl/md5.h>
#include <openssl/sha.h>

#define MAX_PATH 4096
#define BUFFER_SIZE 1024 * 16

int correct_path(char* backup_path, char* temp_path);

char* cut_time_path(char *path, char* temp);

char* cut_path(char *path, char* temp, int backup_path_length);

char* cut_backup_path(char* input_path, char* backup_path);

char* change_input_path(char* input_path, char* save_path, char* new_input);

char* change_directory_path(char* input_path, char* save_path, char* new_input);

char* change_path(char* input_path, char* backup_path);

int filecopy(const char* input_path, const char* backup_path);

char* return_hash_value_md5(char* path);

char* return_hash_value_sha(char* path);

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

    // hash 함수 저장 변수 
    char* hash = (char*) malloc(sizeof(char) * 5);
    hash = tokens[argc - 1];

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
    
    if (strcmp(tokens[1], "md5") == 0 || strcmp(tokens[1], "sha1") == 0) // 첫 번째 인자 입력이 없는 경우 , 입력된 인자가 4개 이상일 경우 
    {
        printf("Usage : recover <FILENAME> [OPTION]\n");
        printf("  -d : recover directory recursive\n");
        printf("  -n <NEWNAME> : recover file with new name\n");
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
   
        if (strlen(tokens[1]) < MAX_PATH) // 경로 길이가 제한을 넘을 경우
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
                        struct stat backup_path_check; // 파일 정보를 저장할 구조체 
                        stat(backup_path, &backup_path_check); // stat 함수로 파일 정보를 가져오기

                        if (S_ISDIR(backup_path_check.st_mode) == 1) // 입력받은 경로 파일 유형이 디렉토리인 경우 
                        {
                            if(tokens[2] != NULL && strcmp(tokens[2], "md5") != 0 && strcmp(tokens[2], "sha1") != 0) // 세번째 인자로 옵션이 들어온 경우
                            {
                                if(strcmp(tokens[2], "-d") == 0 && (strcmp(tokens[3], "md5") == 0 || strcmp(tokens[3], "sha1") == 0)) // 세번째 인자가 -d 인 경우 인자는 최대 세번째까지 받을수있는 경우
                                {
                                    Node* diretory_current = tail;
                                    while (diretory_current != NULL) 
                                    {
                                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                        temp = cut_path(diretory_current->path, temp, (int)strlen(backup_path)); // 백업했던 파일 중에 input_path 랑 똑같은 디렉토리에 존재하는 파일 저장 변수

                                        if(strcmp(backup_path, temp) == 0)
                                        {
                                            char* cutted_backup_path = (char*) malloc(sizeof(char) * MAX_PATH);
                                            cutted_backup_path = cut_backup_path(diretory_current->path, cutted_backup_path);

                                            struct stat diretory_current_check; // 파일 정보를 저장할 구조체 
                                            stat(diretory_current->path, &diretory_current_check); // stat 함수로 파일 정보를 가져오기

                                            if(S_ISDIR(diretory_current_check.st_mode) == 1)
                                            {
                                                DIR* dir = opendir(cutted_backup_path);
                                                if (dir == NULL) // 디렉토리 확인하면서 없는 디렉토리 생성하기 
                                                {
                                                    mkdir(cutted_backup_path, 0777); // 디렉토리 mode_t 설정하기
                                                    
                                                }
                                            }
                                            else
                                            {
                                                char* new_input_path = (char*) malloc(sizeof(char) * MAX_PATH);
                                                new_input_path = cut_time_path(cutted_backup_path, new_input_path); // 새로운 input_path 처럼 만들기

                                                char* new_backup_path = (char*) malloc(sizeof(char) * MAX_PATH);
                                                new_backup_path = change_path(new_input_path, new_backup_path); // 새로운 backup_path 처럼 만들기

                                                Node* current = head; // 링크드 리스트 확인하기 

                                                int file_num = 0; // 존재하는 파일 개수를 저장하는 변수

                                                char clock[13]; // 백업했던 파일의 시간 저장하는 변수

                                                while(current != NULL)
                                                {
                                                    char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                                    temp = cut_time_path(current->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기
                                                    if(strcmp(new_backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                                    {
                                                        file_num++;
                                                    }
                                                    else
                                                    {
                                                        
                                                    }

                                                    current = current->next; // 링크드 리스크로 구현된 파일 구조 확인하기
                                                }

                                                if(file_num == 1) // 입력받은 파일과 backup 디렉토리에 동일한 파일이 하나일경우
                                                {
                                                    // choose_num 으로 고른 파일을 링크드 리스트 순회 방법으로 같은 위치에 있는 파일을 찾기위해 while 
                                                    Node* check = head;
                                                    int check_num = 0 ; 
                                                    while (check != NULL)
                                                    {
                                                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                                        temp = cut_time_path(check->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기

                                                        if(strcmp(new_backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                                        {
                                                            if(fopen(new_input_path, "rb") == NULL)
                                                            {
                                                                char* input_path_check = NULL;
                                                                char* result = NULL;
                                                                result = realpath(input_path, input_path_check);
                                                                if(result == NULL)
                                                                {
                                                                    if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                    {
                                                                        printf("\"%s\" backup file recover\n",check->path);
                                                                    }
                                                                    else
                                                                    {
                                                                        printf("fail file recover\n");
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                    {
                                                                        printf("\"%s\" backup recover to \"%s\"\n",check->path, new_input_path);
                                                                    }
                                                                    else
                                                                    {
                                                                        printf("fail file recover\n");
                                                                    }
                                                                }
                                                            }
                                                            else
                                                            {
                                                                if(strcmp(return_hash_value_sha(check->path), return_hash_value_sha(new_input_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                {
                                                                    printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, new_input_path);
                                                                }
                                                                else
                                                                {
                                                                    if(strcmp(hash, "md5") == 0) // 해시 함수 종류가 md5 인 경우
                                                                    {
                                                                        if(strcmp(return_hash_value_md5(check->path), return_hash_value_md5(new_input_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                        {
                                                                            printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, new_input_path);
                                                                        }
                                                                        else
                                                                        {
                                                                            char* input_path_check = NULL;
                                                                            char* result = NULL;
                                                                            result = realpath(input_path, input_path_check);
                                                                            if(result == NULL)
                                                                            {
                                                                                if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                                {
                                                                                    printf("\"%s\" backup file recover\n",check->path);
                                                                                }
                                                                                else
                                                                                {
                                                                                    printf("fail file recover\n");
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                                {
                                                                                    printf("\"%s\" backup recover to \"%s\"\n",check->path, new_input_path);
                                                                                }
                                                                                else
                                                                                {
                                                                                    printf("fail file recover\n");
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                    else // 해시 함수 종류가 sha1 인 경우
                                                                    {
                                                                        if(strcmp(return_hash_value_sha(check->path), return_hash_value_sha(new_input_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                        {
                                                                            printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, new_input_path);
                                                                        }
                                                                        else
                                                                        {
                                                                            char* input_path_check = NULL;
                                                                            char* result = NULL;
                                                                            result = realpath(input_path, input_path_check);
                                                                            if(result == NULL)
                                                                            {
                                                                                if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                                {
                                                                                    printf("\"%s\" backup file recover\n",check->path);
                                                                                }
                                                                                else
                                                                                {
                                                                                    printf("fail file recover\n");
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                                {
                                                                                    printf("\"%s\" backup recover to \"%s\"\n",check->path, new_input_path);
                                                                                }
                                                                                else
                                                                                {
                                                                                    printf("fail file recover\n");
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }

                                                        }
                                                        check = check->next;
                                                    }

                                                }
                                                else
                                                {
                                                    int same_path_num = 0; // 해당 디렉토리에 똑같은 경로에서
                                                    Node* origin = diretory_current; 
                                                    Node* num = diretory_current; // 링크드 리스트에서 현재 보고있는 파일
                                                    while(num != NULL)
                                                    {
                                                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH); // 임시로 경로를 저장하는 변수
                                                        temp = cut_time_path(num->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기
                                                        if(strcmp(new_backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                                        {
                                                            same_path_num++;
                                                        }
                                                        else
                                                        {
                                                            
                                                        }

                                                        num = num->next;
                                                    }

                                                    diretory_current = origin;
                                                    
                                                    if(same_path_num == 1)
                                                    {
                                                        current = head; // 링크드리스트 순회를 위해 current 재설정

                                                        int print_num = 0; // 같은 파일 출력용 번호 변수

                                                        printf("backup file list of \"%s\"\n",new_input_path);
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

                                                            if(strcmp(new_backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
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

                                                        printf("choose file to recover\n");
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

                                                            if(strcmp(new_backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                                            {
                                                                check_num++;
                                                                if(check_num == choose_num)
                                                                {
                                                                    if(fopen(new_input_path, "rb") == NULL)
                                                                    {
                                                                        char* input_path_check = NULL;
                                                                        char* result = NULL;
                                                                        result = realpath(input_path, input_path_check);
                                                                        if(result == NULL)
                                                                        {
                                                                            if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                            {
                                                                                printf("\"%s\" backup file recover\n",check->path);
                                                                            }
                                                                            else
                                                                            {
                                                                                printf("fail file recover\n");
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                            {
                                                                                printf("\"%s\" backup recover to \"%s\"\n",check->path, new_input_path);
                                                                            }
                                                                            else
                                                                            {
                                                                                printf("fail file recover\n");
                                                                            }
                                                                        }                                                                        
                                                                    }
                                                                    else
                                                                    {
                                                                        if(strcmp(return_hash_value_sha(check->path), return_hash_value_sha(new_input_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                        {
                                                                            printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, new_input_path);
                                                                        }
                                                                        else
                                                                        {
                                                                            if(strcmp(hash, "md5") == 0) // 해시 함수 종류가 md5 인 경우
                                                                            {
                                                                                if(strcmp(return_hash_value_md5(check->path), return_hash_value_md5(new_input_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                                {
                                                                                    printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, new_input_path);
                                                                                }
                                                                                else
                                                                                {
                                                                                    char* input_path_check = NULL;
                                                                                    char* result = NULL;
                                                                                    result = realpath(input_path, input_path_check);
                                                                                    if(result == NULL)
                                                                                    {
                                                                                        if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                                        {
                                                                                            printf("\"%s\" backup file recover\n",check->path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            printf("fail file recover\n");
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                                        {
                                                                                            printf("\"%s\" backup recover to \"%s\"\n",check->path, new_input_path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            printf("fail file recover\n");
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                            else // 해시 함수 종류가 sha1 인 경우
                                                                            {
                                                                                if(strcmp(return_hash_value_sha(check->path), return_hash_value_sha(new_input_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                                {
                                                                                    printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, new_input_path);
                                                                                }
                                                                                else
                                                                                {
                                                                                    char* input_path_check = NULL;
                                                                                    char* result = NULL;
                                                                                    result = realpath(input_path, input_path_check);
                                                                                    if(result == NULL)
                                                                                    {
                                                                                        if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                                        {
                                                                                            printf("\"%s\" backup file recover\n",check->path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            printf("fail file recover\n");
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        if(filecopy(check->path, new_input_path) == 0) // 파일 복사 성공한 경우
                                                                                        {
                                                                                            printf("\"%s\" backup recover to \"%s\"\n",check->path, new_input_path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            printf("fail file recover\n");
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }                                                           
                                                                }
                                                            }
                                                            check = check->next;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        diretory_current = diretory_current->prev;
                                    }
                                }
                                else if(strcmp(tokens[2], "-d") == 0 && strcmp(tokens[3], "sha1") != 0 && strcmp(tokens[3], "md5") != 0) // 세번째 인자가 -d 이고 뒤에 인자를 더 받는 경우
                                {
                                    if(strcmp(tokens[3], "-n") == 0 && strcmp(tokens[4], "sha1") != 0 && strcmp(tokens[4], "md5") != 0) // 네번째 인자가 -n 이고 뒤에 새로운 경로를 받은 경우
                                    {
                                        char* newfile_input = (char*) malloc(sizeof(char) * MAX_PATH);
                                        char* newfile_temp = (char*) malloc(sizeof(char) * MAX_PATH);

                                        strcpy(newfile_input, tokens[4]); // 입력받은 경로 저장하기 

                                        // 상대경로 입력 시
                                        if (newfile_input[0] != '/')
                                        {
                                            if(newfile_input[0] == '~')
                                            {
                                                strcpy(newfile_temp, home_path);
                                                strcat(newfile_temp, newfile_input + 1);
                                                strcpy(newfile_input, newfile_temp);
                                            }
                                            else
                                            {
                                                strcpy(newfile_temp, current_path);
                                                strcat(newfile_temp, "/");
                                                strcat(newfile_temp, newfile_input);
                                                strcpy(newfile_input, newfile_temp);
                                            }
                                        }
                                        // 절대경로 입력 시
                                        else 
                                        {
                                            
                                        }

                                        if(strstr(newfile_input, home_path) == NULL) // input_path 경로에 홈 디렉토리 가 포함되었는지 확인하기 
                                        {
                                            printf("\"%s\" can't be backuped\n",input_path); // input_path 경로에 홈 디렉토리가 포함되지 않은 경우 
                                        }
                                        else
                                        {
                                            if (strstr(newfile_input, "/backup") != NULL) // input_path 경로에 "backup" 가 포함되어있는지 확인하기 
                                            {
                                                printf("\"%s\" can't be backuped\n",input_path); // input_path 경로에 "backup" 포함된 경우 
                                            }
                                            else
                                            {
                                                char* new_temp_path = (char*) malloc(sizeof(char) * MAX_PATH);
                                                char* new_path = (char*) malloc(sizeof(char) * MAX_PATH); 

                                                strcpy(new_path, tokens[4]); // 입력받은 경로 저장하기 
                                            
                                                // 상대경로 입력 시
                                                if (new_path[0] != '/')
                                                {
                                                    strcpy(new_temp_path, current_path);
                                                    strcat(new_temp_path, "/");
                                                    strcat(new_temp_path,tokens[4]);
                                                    strcpy(new_path, new_temp_path);
                                                }
                                                // 절대경로 입력 시
                                                else 
                                                {
                                        
                                                }
                                                
                                                Node* diretory_current = tail;
                                                while (diretory_current != NULL) 
                                                {
                                                    struct stat diretory_current_check; // 파일 정보를 저장할 구조체 
                                                    stat(diretory_current->path, &diretory_current_check); // stat 함수로 파일 정보를 가져오기

                                                    char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                                    temp = cut_path(diretory_current->path, temp, (int)strlen(backup_path)); // 백업했던 파일 중에 input_path 랑 똑같은 디렉토리에 존재하는 파일 저장 변수
                            
                                                    if(strcmp(backup_path, temp) == 0)
                                                    {
                                                        char* cutted_backup_path = (char*) malloc(sizeof(char) * MAX_PATH);
                                                        cutted_backup_path = cut_backup_path(diretory_current->path, cutted_backup_path);

                                                    
                                                        if(S_ISDIR(diretory_current_check.st_mode) == 1)
                                                        {
                                                            char* save_path = (char*) malloc(sizeof(char) * MAX_PATH);
                                                            save_path = change_directory_path(cutted_backup_path, save_path, new_path);

                                                            char* temp_save_path = (char*) malloc(sizeof(char) * MAX_PATH);
                                                            char* current_save_path = (char*) malloc(sizeof(char) * MAX_PATH);

                                                            strcpy(temp_save_path, save_path);

                                                            // 백업할 경로를 "/" 로 나누기 (나눠서 각 디렉토리 보기)
                                                            path_tokens = (char**) malloc(sizeof(char*) * 256); // 토큰들을 저장할 메모리 공간 동적 할당
                                                            path_token = strtok(temp_save_path, "/"); // 첫 번째 토큰 추출
                                                            path_token_count = 0;
                                                            
                                                            while (path_token != NULL) // 토큰이 더 이상 없을 때까지 반복
                                                            { 
                                                                path_tokens[path_token_count] = path_token; // 현재 토큰을 저장
                                                                path_token_count++;
                                                                path_token = strtok(NULL, "/"); // 다음 토큰 추출
                                                            }

                                                            // 사용자 홈 디렉토리 경로 만들기 
                                                            strcat(current_save_path,"/");
                                                            strcat(current_save_path, path_tokens[0]);
                                                            strcat(current_save_path,"/");
                                                            strcat(current_save_path, path_tokens[1]);

                                                            // 사용자 홈 디렉토리부터 디렉토리 확인하기
                                                            for(int i = 2 ; i < path_token_count ; i++)
                                                            {
                                                                strcat(current_save_path,"/");
                                                                strcat(current_save_path,path_tokens[i]);

                                                                DIR* dir = opendir(current_save_path);
                                                                if (dir == NULL) // 디렉토리 확인하면서 없는 디렉토리 생성하기 
                                                                {
                                                                    mkdir(current_save_path, 0777);
                                                                }
                                                            }
                                                        }
                                                        else
                                                        {
                                                            char* new_input_path = (char*) malloc(sizeof(char) * MAX_PATH);
                                                            new_input_path = cut_time_path(cutted_backup_path, new_input_path); // 새로운 input_path 처럼 만들기

                                                            char* new_backup_path = (char*) malloc(sizeof(char) * MAX_PATH);
                                                            new_backup_path = change_path(new_input_path, new_backup_path); // 새로운 backup_path 처럼 만들기

                                                            char* save_path = (char*) malloc(sizeof(char) * MAX_PATH);
                                                            save_path = change_directory_path(new_input_path, save_path, new_path); // 새로 저장할 path 만들기

                                                            Node* current = head; // 링크드 리스트 확인하기 

                                                            int file_num = 0; // 존재하는 파일 개수를 저장하는 변수

                                                            char clock[13]; // 백업했던 파일의 시간 저장하는 변수

                                                            while(current != NULL)
                                                            {
                                                                char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                                                temp = cut_time_path(current->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기
                                                                if(strcmp(new_backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                                                {
                                                                    file_num++;
                                                                }
                                                                else
                                                                {
                                                                    
                                                                }

                                                                current = current->next; // 링크드 리스크로 구현된 파일 구조 확인하기
                                                            }

                                                            if(file_num == 1) // 입력받은 파일과 backup 디렉토리에 동일한 파일이 하나일경우
                                                            {
                                                                // choose_num 으로 고른 파일을 링크드 리스트 순회 방법으로 같은 위치에 있는 파일을 찾기위해 while 
                                                                Node* check = head;
                                                                int check_num = 0 ; 
                                                                while (check != NULL)
                                                                {
                                                                    char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                                                    temp = cut_time_path(check->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기

                                                                    if(strcmp(new_backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                                                    {
                                                                        if (fopen(save_path, "rb") == NULL) 
                                                                        {
                                                                            char* input_path_check = NULL;
                                                                            char* result = NULL;
                                                                            result = realpath(input_path, input_path_check);
                                                                            if(result == NULL)
                                                                            {
                                                                                if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                {
                                                                                    printf("\"%s\" backup file recover\n",check->path);
                                                                                }
                                                                                else
                                                                                {
                                                                                    printf("fail file recover\n");
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                {
                                                                                    printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                                }
                                                                                else
                                                                                {
                                                                                    printf("fail file recover\n");
                                                                                }
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            if(strcmp(hash, "md5") == 0) // 해시 함수 종류가 md5 인 경우
                                                                            {
                                                                                if(strcmp(return_hash_value_md5(check->path), return_hash_value_md5(save_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                                {
                                                                                    printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, save_path);
                                                                                }
                                                                                else
                                                                                {
                                                                                    char* input_path_check = NULL;
                                                                                    char* result = NULL;
                                                                                    result = realpath(input_path, input_path_check);
                                                                                    if(result == NULL)
                                                                                    {
                                                                                        if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                        {
                                                                                            printf("\"%s\" backup file recover\n",check->path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            printf("fail file recover\n");
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                        {
                                                                                            printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            printf("fail file recover\n");
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                            else // 해시 함수 종류가 sha1 인 경우
                                                                            {
                                                                                if(strcmp(return_hash_value_sha(check->path), return_hash_value_sha(save_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                                {
                                                                                    printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, save_path);
                                                                                }
                                                                                else
                                                                                {
                                                                                    char* input_path_check = NULL;
                                                                                    char* result = NULL;
                                                                                    result = realpath(input_path, input_path_check);
                                                                                    if(result == NULL)
                                                                                    {
                                                                                        if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                        {
                                                                                            printf("\"%s\" backup file recover\n",check->path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            printf("fail file recover\n");
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                        {
                                                                                            printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            printf("fail file recover\n");
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                    check = check->next;
                                                                }

                                                            }
                                                            else
                                                            {

                                                                int same_path_num = 0; // 해당 디렉토리에 똑같은 경로에서
                                                                Node* origin = diretory_current; 
                                                                Node* num = diretory_current; // 링크드 리스트에서 현재 보고있는 파일
                                                                while(num != NULL)
                                                                {
                                                                    char* temp = (char*) malloc(sizeof(char) * MAX_PATH); // 임시로 경로를 저장하는 변수
                                                                    temp = cut_time_path(num->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기
                                                                    if(strcmp(new_backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                                                    {
                                                                        same_path_num++;
                                                                    }
                                                                    else
                                                                    {
                                                                        
                                                                    }

                                                                    num = num->next;
                                                                }

                                                                diretory_current = origin;
                                                                
                                                                if(same_path_num == 1)
                                                                {
                                                                    current = head; // 링크드리스트 순회를 위해 current 재설정

                                                                    int print_num = 0; // 같은 파일 출력용 번호 변수

                                                                    printf("backup file list of \"%s\"\n",new_input_path);
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

                                                                        if(strcmp(new_backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
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

                                                                    printf("choose file to recover\n");
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

                                                                        if(strcmp(new_backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                                                        {
                                                                            check_num++;
                                                                            if(check_num == choose_num)
                                                                            {
                                                                                if (fopen(save_path, "rb") == NULL) 
                                                                                {
                                                                                    char* input_path_check = NULL;
                                                                                    char* result = NULL;
                                                                                    result = realpath(input_path, input_path_check);
                                                                                    if(result == NULL)
                                                                                    {
                                                                                        if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                        {
                                                                                            printf("\"%s\" backup file recover\n",check->path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            printf("fail file recover\n");
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                        {
                                                                                            printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            printf("fail file recover\n");
                                                                                        }
                                                                                    }
                                                                                }
                                                                                else
                                                                                {
                                                                                    if(strcmp(hash, "md5") == 0) // 해시 함수 종류가 md5 인 경우
                                                                                    {
                                                                                        if(strcmp(return_hash_value_md5(check->path), return_hash_value_md5(save_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                                        {
                                                                                            printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, save_path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            char* input_path_check = NULL;
                                                                                            char* result = NULL;
                                                                                            result = realpath(input_path, input_path_check);
                                                                                            if(result == NULL)
                                                                                            {
                                                                                                if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                                {
                                                                                                    printf("\"%s\" backup file recover\n",check->path);
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                    printf("fail file recover\n");
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                                {
                                                                                                    printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                    printf("fail file recover\n");
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                    else // 해시 함수 종류가 sha1 인 경우
                                                                                    {
                                                                                        if(strcmp(return_hash_value_sha(check->path), return_hash_value_sha(save_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                                        {
                                                                                            printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, save_path);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            char* input_path_check = NULL;
                                                                                            char* result = NULL;
                                                                                            result = realpath(input_path, input_path_check);
                                                                                            if(result == NULL)
                                                                                            {
                                                                                                if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                            {
                                                                                                printf("\"%s\" backup file recover\n",check->path);
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                printf("fail file recover\n");
                                                                                            }
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                                            {
                                                                                                printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                printf("fail file recover\n");
                                                                                            }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }   
                                                                            }
                                                                        }
                                                                        check = check->next;
                                                                    }
                                                                }

                                                            }
                                                        }   
                                                    }
                                                    diretory_current = diretory_current->prev;
                                                }
                                            }
                                        }

                                    }
                                    else // -n 옵션을 넣었을때 뒤에 새로운 경로 넣어주지않을 경우
                                    {
                                        printf("Usage : recover <FILENAME> [OPTION]\n");
                                        printf("  -d : recover directory recursive\n");
                                        printf("  -n <NEWNAME> : recover file with new name\n");
                                    }

                                }
                                else if(strcmp(tokens[2], "-d") != 0 && (strcmp(tokens[3], "md5") == 0 || strcmp(tokens[3], "sha1") == 0)) // 디렉토리일때 옵션 잘못 넣었을 경우
                                {
                                    printf("Usage : recover <FILENAME> [OPTION]\n");
                                    printf("  -d : recover directory recursive\n");
                                    printf("  -n <NEWNAME> : recover file with new name\n");
                                }
                            }
                            else // 디렉토리일때 옵션을 넣어주지 않는 경우
                            {
                                printf("Usage : recover <FILENAME> [OPTION]\n");
                                printf("  -d : recover directory recursive\n");
                                printf("  -n <NEWNAME> : recover file with new name\n");
                            }

                        }
                        else if (S_ISDIR(backup_path_check.st_mode) != 1) // 입력받은 경로 파일 유형이 디렉토리가 아닌 경우 
                        {
                            Node* current = head; // 링크드 리스트 확인하기 

                            int file_num = 0; // 존재하는 파일 개수를 저장하는 변수

                            char clock[13]; // 백업했던 파일의 시간 저장하는 변수

                            while(current != NULL)
                            {
                                char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
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

                            if(tokens[2] != NULL && strcmp(tokens[2], "md5") != 0 && strcmp(tokens[2], "sha1") != 0) // 옵션 "-n" 이 있을 경우
                            {
                                if(strcmp(tokens[2], "-n") == 0 && strcmp(tokens[3], "md5") != 0 && strcmp(tokens[3], "sha1") != 0) // 세번째 인자가 "-n" 일때는 뒤에 NEWNAME 이 존재해야한다
                                {
                                    char* newfile_input = (char*) malloc(sizeof(char) * MAX_PATH);
                                    char* newfile_temp = (char*) malloc(sizeof(char) * MAX_PATH);

                                    strcpy(newfile_input, tokens[3]); // 입력받은 경로 저장하기 

                                    // 상대경로 입력 시
                                    if (newfile_input[0] != '/')
                                    {
                                        if(newfile_input[0] == '~')
                                        {
                                            strcpy(newfile_temp, home_path);
                                            strcat(newfile_temp, newfile_input + 1);
                                            strcpy(newfile_input, newfile_temp);
                                        }
                                        else
                                        {
                                            strcpy(newfile_temp, current_path);
                                            strcat(newfile_temp, "/");
                                            strcat(newfile_temp, newfile_input);
                                            strcpy(newfile_input, newfile_temp);
                                        }
                                    }
                                    // 절대경로 입력 시
                                    else 
                                    {
                                        
                                    }

                                    if(strstr(newfile_input, home_path) == NULL) // input_path 경로에 홈 디렉토리 가 포함되었는지 확인하기 
                                    {
                                        printf("\"%s\" can't be backuped\n",input_path); // input_path 경로에 홈 디렉토리가 포함되지 않은 경우 
                                    }
                                    else
                                    {
                                        if (strstr(newfile_input, "/backup") != NULL) // input_path 경로에 "backup" 가 포함되어있는지 확인하기 
                                        {
                                            printf("\"%s\" can't be backuped\n",input_path); // input_path 경로에 "backup" 포함된 경우 
                                        }
                                        else
                                        {
                                            char* save_path = (char*) malloc(sizeof(char) * MAX_PATH); // 새로 저장할 경로 저장할 변수
                                            save_path = change_input_path(input_path, save_path, tokens[3]);

                                            if(file_num == 1) // 입력받은 파일과 backup 디렉토리에 동일한 파일이 하나일경우
                                            {
                                                // choose_num 으로 고른 파일을 링크드 리스트 순회 방법으로 같은 위치에 있는 파일을 찾기위해 while 
                                                Node* check = head;
                                                int check_num = 0 ; 
                                                while (check != NULL)
                                                {
                                                    char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                                    temp = cut_time_path(check->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기

                                                    if(strcmp(backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                                    {
                                                        if (fopen(save_path, "rb") == NULL) 
                                                        {
                                                            char* input_path_check = NULL;
                                                            char* result = NULL;
                                                            result = realpath(input_path, input_path_check);
                                                            if(result == NULL)
                                                            {
                                                                if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                {
                                                                    printf("\"%s\" backup file recover\n",check->path);
                                                                }
                                                                else
                                                                {
                                                                    printf("fail file recover\n");
                                                                }
                                                            }
                                                            else
                                                            {
                                                                if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                {
                                                                    printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                }
                                                                else
                                                                {
                                                                    printf("fail file recover\n");
                                                                }
                                                            }
                                                        }
                                                        else
                                                        {
                                                            if(strcmp(hash, "md5") == 0) // 해시 함수 종류가 md5 인 경우
                                                            {
                                                                if(strcmp(return_hash_value_md5(check->path), return_hash_value_md5(save_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                {
                                                                    printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, save_path);
                                                                }
                                                                else
                                                                {
                                                                    char* input_path_check = NULL;
                                                                    char* result = NULL;
                                                                    result = realpath(input_path, input_path_check);
                                                                    if(result == NULL)
                                                                    {
                                                                        if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                        {
                                                                            printf("\"%s\" backup file recover\n",check->path);
                                                                        }
                                                                        else
                                                                        {
                                                                            printf("1fail file recover\n");
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                        {
                                                                            printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                        }
                                                                        else
                                                                        {
                                                                            printf("1fail file recover\n");
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                            else // 해시 함수 종류가 sha1 인 경우
                                                            {
                                                                if(strcmp(return_hash_value_sha(check->path), return_hash_value_sha(save_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                {
                                                                    printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, save_path);
                                                                }
                                                                else
                                                                {
                                                                    char* input_path_check = NULL;
                                                                    char* result = NULL;
                                                                    result = realpath(input_path, input_path_check);
                                                                    if(result == NULL)
                                                                    {
                                                                        if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                        {
                                                                            printf("\"%s\" backup file recover\n",check->path);
                                                                        }
                                                                        else
                                                                        {
                                                                            printf("fail file recover\n");
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                        {
                                                                            printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                        }
                                                                        else
                                                                        {
                                                                            printf("fail file recover\n");
                                                                        }
                                                                    }
                                                                }
                                                            }
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

                                                printf("choose file to recover\n");
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
                                                            if (fopen(save_path, "rb") == NULL) 
                                                            {
                                                                char* input_path_check = NULL;
                                                                char* result = NULL;
                                                                result = realpath(input_path, input_path_check);
                                                                if(result == NULL)
                                                                {
                                                                    if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                    {
                                                                        printf("\"%s\" backup file recover\n",check->path);
                                                                    }
                                                                    else
                                                                    {
                                                                        printf("fail file recover\n");
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                    {
                                                                        printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                    }
                                                                    else
                                                                    {
                                                                        printf("fail file recover\n");
                                                                    }
                                                                }
                                                            }
                                                            else
                                                            {
                                                                if(strcmp(hash, "md5") == 0) // 해시 함수 종류가 md5 인 경우
                                                                {
                                                                    if(strcmp(return_hash_value_md5(check->path), return_hash_value_md5(save_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                    {
                                                                        printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, save_path);
                                                                    }
                                                                    else
                                                                    {
                                                                        char* input_path_check = NULL;
                                                                        char* result = NULL;
                                                                        result = realpath(input_path, input_path_check);
                                                                        if(result == NULL)
                                                                        {
                                                                            if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                            {
                                                                                printf("\"%s\" backup file recover\n",check->path);
                                                                            }
                                                                            else
                                                                            {
                                                                                printf("fail file recover\n");
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                            {
                                                                                printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                            }
                                                                            else
                                                                            {
                                                                                printf("fail file recover\n");
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                                else // 해시 함수 종류가 sha1 인 경우
                                                                {
                                                                    if(strcmp(return_hash_value_sha(check->path), return_hash_value_sha(save_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                                    {
                                                                        printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, save_path);
                                                                    }
                                                                    else
                                                                    {
                                                                        char* input_path_check = NULL;
                                                                        char* result = NULL;
                                                                        result = realpath(input_path, input_path_check);
                                                                        if(result == NULL)
                                                                        {
                                                                            if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                            {
                                                                                printf("\"%s\" backup file recover\n",check->path);
                                                                            }
                                                                            else
                                                                            {
                                                                                printf("fail file recover\n");
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            if(filecopy(check->path, save_path) == 0) // 파일 복사 성공한 경우
                                                                            {
                                                                                printf("\"%s\" backup recover to \"%s\"\n",check->path, save_path);
                                                                            }
                                                                            else
                                                                            {
                                                                                printf("fail file recover\n");
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }                                               
                                                        }
                                                    }
                                                    
                                                    check = check->next;
                                                }
                                            }
                                        }
                                    }
                                    
                                }
                                else if(strcmp(tokens[2], "-n") == 0 && (strcmp(tokens[3], "md5") == 0 || strcmp(tokens[3], "sha1") == 0)) // -n 인자 들어온 경우 뒤에 새로운 경로 추가 안 들어온
                                {
                                    printf("Usage : recover <FILENAME> [OPTION]\n");
                                    printf("  -d : recover directory recursive\n");
                                    printf("  -n <NEWNAME> : recover file with new name\n");
                                }
                            }
                            else if(strcmp(tokens[2], "md5") == 0 || strcmp(tokens[2], "sha1") == 0)
                            {
                                if(file_num == 1) // 입력받은 파일과 backup 디렉토리에 동일한 파일이 하나일경우
                                {
                                    // choose_num 으로 고른 파일을 링크드 리스트 순회 방법으로 같은 위치에 있는 파일을 찾기위해 while 
                                    Node* check = head;
                                    int check_num = 0 ; 
                                    while (check != NULL)
                                    {
                                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                                        temp = cut_time_path(check->path, temp); // 백업했던 파일 중에 시간 작성 부분 자르기

                                        if(strcmp(backup_path, temp) == 0) // 입력받아서 구한 백업했던 경로와 백업 디렉토리에 존재하는 파일 같은게 있을 경우
                                        {
                                            if(fopen(input_path, "rb") == NULL)
                                            {
                                                char* input_path_check = NULL;
                                                char* result = NULL;
                                                result = realpath(input_path, input_path_check);
                                                if(result == NULL)
                                                {
                                                    if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                    {
                                                        printf("\"%s\" backup file recover\n",check->path);
                                                    }
                                                    else
                                                    {
                                                        printf("fail file recover\n");
                                                    }
                                                }
                                                else
                                                {
                                                    if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                    {
                                                        printf("\"%s\" backup recover to \"%s\"\n",check->path, input_path);
                                                    }
                                                    else
                                                    {
                                                        printf("fail file recover\n");
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if(strcmp(hash, "md5") == 0) // 해시 함수 종류가 md5 인 경우
                                                {
                                                    if(strcmp(return_hash_value_md5(check->path), return_hash_value_md5(input_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                    {
                                                        printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, input_path);
                                                    }
                                                    else
                                                    {
                                                        char* input_path_check = NULL;
                                                        char* result = NULL;
                                                        result = realpath(input_path, input_path_check);
                                                        if(result == NULL)
                                                        {
                                                            if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                            {
                                                                printf("\"%s\" backup file recover\n",check->path);
                                                            }
                                                            else
                                                            {
                                                                printf("fail file recover\n");
                                                            }
                                                        }
                                                        else
                                                        {
                                                            if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                            {
                                                                printf("\"%s\" backup recover to \"%s\"\n",check->path, input_path);
                                                            }
                                                            else
                                                            {
                                                                printf("fail file recover\n");
                                                            }
                                                        }
                                                    }
                                                }
                                                else // 해시 함수 종류가 sha1 인 경우
                                                {
                                                    if(strcmp(return_hash_value_sha(check->path), return_hash_value_sha(input_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                    {
                                                        printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, input_path);
                                                    }
                                                    else
                                                    {
                                                        char* input_path_check = NULL;
                                                        char* result = NULL;
                                                        result = realpath(input_path, input_path_check);
                                                        if(result == NULL)
                                                        {
                                                            if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                            {
                                                                printf("\"%s\" backup file recover \n",check->path);
                                                            }
                                                            else
                                                            {
                                                                printf("fail file recover\n");
                                                            }
                                                        }
                                                        else
                                                        {
                                                            if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                            {
                                                                printf("\"%s\" backup recover to \"%s\"\n",check->path, input_path);
                                                            }
                                                            else
                                                            {
                                                                printf("fail file recover\n");
                                                            }
                                                        }
                                                    }
                                                }
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

                                    printf("choose file to recover\n");
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
                                                if(fopen(input_path, "rb") == NULL)
                                                {
                                                    char* input_path_check = NULL;
                                                    char* result = NULL;
                                                    result = realpath(input_path, input_path_check);
                                                    if(result == NULL)
                                                    {
                                                        if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                        {
                                                            printf("\"%s\" backup file recover\n",check->path);
                                                        }
                                                        else
                                                        {
                                                            printf("fail file recover\n");
                                                        }
                                                    }
                                                    else
                                                    {
                                                        if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                        {
                                                            printf("\"%s\" backup recover to \"%s\"\n",check->path, input_path);
                                                        }
                                                        else
                                                        {
                                                            printf("fail file recover\n");
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if(strcmp(hash, "md5") == 0) // 해시 함수 종류가 md5 인 경우
                                                    {
                                                        if(strcmp(return_hash_value_md5(check->path), return_hash_value_md5(input_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                        {
                                                            printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, input_path);
                                                        }
                                                        else
                                                        {
                                                            char* input_path_check = NULL;
                                                            char* result = NULL;
                                                            result = realpath(input_path, input_path_check);
                                                            if(result == NULL)
                                                            {
                                                                if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                                {
                                                                    printf("\"%s\" backup file recover\n",check->path);
                                                                }
                                                                else
                                                                {
                                                                    printf("fail file recover\n");
                                                                }
                                                            }
                                                            else
                                                            {
                                                                if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                                {
                                                                    printf("\"%s\" backup recover to \"%s\"\n",check->path, input_path);
                                                                }
                                                                else
                                                                {
                                                                    printf("fail file recover\n");
                                                                }
                                                            }
                                                        }
                                                    }
                                                    else // 해시 함수 종류가 sha1 인 경우
                                                    {
                                                        if(strcmp(return_hash_value_sha(check->path), return_hash_value_sha(input_path)) == 0) // 두 경로에 따른 파일의 해시값이 같은 경우 (파일을 recover 하지 않는 경우)
                                                        {
                                                            printf("\"%s\" is already backuped recover to \"%s\"\n",check->path, input_path);
                                                        }
                                                        else
                                                        {
                                                            char* input_path_check = NULL;
                                                            char* result = NULL;
                                                            result = realpath(input_path, input_path_check);
                                                            if(result == NULL)
                                                            {
                                                                if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                                {
                                                                    printf("\"%s\" backup file recover",check->path);
                                                                }
                                                                else
                                                                {
                                                                    printf("fail file recover\n");
                                                                }
                                                            }
                                                            else
                                                            {
                                                                if(filecopy(check->path, input_path) == 0) // 파일 복사 성공한 경우
                                                                {
                                                                    printf("\"%s\" backup recover to \"%s\"\n",check->path, input_path);
                                                                }
                                                                else
                                                                {
                                                                    printf("fail file recover\n");
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        
                                        check = check->next;
                                    }
                                }
                            }
                            else
                            {
                                printf("Usage : recover <FILENAME> [OPTION]\n");
                                printf("  -d : recover directory recursive\n");
                                printf("  -n <NEWNAME> : recover file with new name\n");
                            }
                        }
                    }
                    else
                    {
                        printf("Usage : recover <FILENAME> [OPTION]\n");
                        printf("  -d : recover directory recursive\n");
                        printf("  -n <NEWNAME> : recover file with new name\n");
                    }
                }
            }
        }
        else
        {
            printf("Limit Path Length\n"); // 경로 길이 제한
        }
    }

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


// 경로에서 "backup" 디렉토리 빼는 함수
char* cut_backup_path(char* input_path, char* backup_path)
{
    char* input = NULL;
    char** tokens = NULL;
    char* token = NULL;

    input = (char*) malloc(sizeof(char) * MAX_PATH);
    strcpy(input, input_path);
    tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
    token = strtok(input, "/"); // 첫 번째 토큰 추출

    int token_count = 0 ; // 토큰의 개수를 저장하는 변수
    while (token != NULL) // 토큰이 더 이상 없을 때까지 반복
    {
        tokens[token_count] = token; // 토큰들 중에서 앞에 2개의 토큰을 저장
        token_count++;
        token = strtok(NULL, "/"); // 다음 토큰 추출
    }

    memset(backup_path, '\0', sizeof(char) * MAX_PATH);

    // 결과 경로를 저장
    for(int i = 0 ; i < 2 ; i++)
    {
        strcat(backup_path, "/");
        strcat(backup_path, tokens[i]);
    }
    
    for(int i = 3 ; i < token_count ; i++)
    {
        strcat(backup_path, "/");
        strcat(backup_path, tokens[i]);
    }
    
    return backup_path;

}

/// 경로에서 "new input" 을 붙이는 함수
char* change_input_path(char* input_path, char* save_path, char* new_input)
{
    char* input = NULL;
    char** tokens = NULL;
    char* token = NULL;
    
    input = (char*) malloc(sizeof(char) * MAX_PATH);
    strcpy(input, input_path);
    tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
    token = strtok(input, "/"); // 첫 번째 토큰 추출

    int token_count = 0 ; // 토큰의 개수를 저장하는 변수
    while (token != NULL) // 토큰이 더 이상 없을 때까지 반복
    {
        tokens[token_count] = token; // 토큰들 중에서 앞에 2개의 토큰을 저장
        token_count++;
        token = strtok(NULL, "/"); // 다음 토큰 추출
    }

    char* new_input_input = NULL;
    char** new_input_tokens = NULL;
    char* new_input_token = NULL;

    new_input_input = (char*) malloc(sizeof(char) * MAX_PATH);
    strcpy(new_input_input, new_input);
    new_input_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
    new_input_token = strtok(new_input_input, "/"); // 첫 번째 토큰 추출

    int new_input_token_count = 0 ; // 토큰의 개수를 저장하는 변수
    while (new_input_token != NULL) // 토큰이 더 이상 없을 때까지 반복
    {
        new_input_tokens[new_input_token_count] = new_input_token; // 토큰들 중에서 앞에 2개의 토큰을 저장
        new_input_token_count++;
        new_input_token = strtok(NULL, "/"); // 다음 토큰 추출
    }

    memset(save_path, '\0', sizeof(char) * MAX_PATH);

    // 결과 경로를 저장
    for(int i = 0 ; i < 2 ; i++)
    {
        strcat(save_path, "/");
        strcat(save_path, tokens[i]);
    }
    for(int i = 2 ; i < token_count - 1 ; i++)
    {
        strcat(save_path, "/");
        strcat(save_path, new_input_tokens[i - 2]);
        DIR* dir = opendir(save_path);
        if (dir == NULL) // 디렉토리 확인하면서 없는 디렉토리 생성하기 
        {
            mkdir(save_path, 0777);
        }
    }
    strcat(save_path, "/");
    strcat(save_path, new_input_tokens[new_input_token_count - 1]);

    return save_path;

}


// 경로에서 "new input" 을 붙이는 함수
char* change_directory_path(char* input_path, char* save_path, char* new_input) // new_input 을 새로 넣어줄 거기에 절대경로 구하는 것 처럼 만든거
{
    char* input = NULL;
    char** tokens = NULL;
    char* token = NULL;

    input = (char*) malloc(sizeof(char) * MAX_PATH);
    strcpy(input, input_path);
    tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
    token = strtok(input, "/"); // 첫 번째 토큰 추출

    int token_count = 0 ; // 토큰의 개수를 저장하는 변수
    while (token != NULL) // 토큰이 더 이상 없을 때까지 반복
    {
        tokens[token_count] = token; // 토큰들 중에서 앞에 2개의 토큰을 저장
        token_count++;
        token = strtok(NULL, "/"); // 다음 토큰 추출
    }

    memset(save_path, '\0', sizeof(char) * MAX_PATH);

    char* new_input_input = NULL;
    char** new_input_tokens = NULL;
    char* new_input_token = NULL;

    new_input_input = (char*) malloc(sizeof(char) * MAX_PATH);
    strcpy(new_input_input, new_input);
    new_input_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
    new_input_token = strtok(new_input_input, "/"); // 첫 번째 토큰 추출

    int new_input_token_count = 0 ; // 토큰의 개수를 저장하는 변수
    while (new_input_token != NULL) // 토큰이 더 이상 없을 때까지 반복
    {
        new_input_tokens[new_input_token_count] = new_input_token; // 토큰들 중에서 앞에 2개의 토큰을 저장
        new_input_token_count++;
        new_input_token = strtok(NULL, "/"); // 다음 토큰 추출
    }


    // 결과 경로를 저장
    for(int i = 0 ; i < new_input_token_count; i++)
    {
        strcat(save_path, "/");
        strcat(save_path, new_input_tokens[i]);
    }

    for(int i = new_input_token_count ; i < token_count; i++)
    {
        strcat(save_path, "/");
        strcat(save_path, tokens[i]);
    }
    
    return save_path;

}


// 파일 복사하는 함수 
int filecopy(const char* input_path, const char* backup_path) {
    FILE* exist_file;
    FILE* backup_file;

    exist_file = fopen(input_path, "rb"); // 원본 파일 확인하기 
    backup_file = fopen(backup_path, "wb"); // 백업할 파일 만들기

    long file_size;
    char* buffer;
    size_t file_contents;
    
    fseek(exist_file, 0, SEEK_END);
    file_size = ftell(exist_file); // 파일 크기 저장하기 
    rewind(exist_file);

    buffer = (char*)malloc(sizeof(char) * file_size);
    if (buffer == NULL) 
    {
        fputs("Memory error", stderr);
        exit(2);
    }


    if (exist_file == NULL) // 원본 파일 열기
    {
        return -2;
    }    
    
    if (backup_file == NULL) // 백업할 파일 만들기
    {
        fclose(exist_file);
        return -3;
    }

    while ( (file_contents = fread( buffer, 1, file_size, exist_file)) > 0 ){
        if ( 0 == fwrite(buffer, 1, file_size, backup_file)) {
            fclose(exist_file);
            fclose(backup_file);
            unlink(backup_path); // 에러난 파일 지우고 종료
            return -4;
        }
    }
    fclose(exist_file);
    fclose(backup_file);

    return 0;
}


// 파일 경로를 넣어줬을 경우 해쉬 값 나오는 함수 
char* return_hash_value_md5(char* path)
{
    FILE *path_file;
    size_t file_contents;
    MD5_CTX md5_context;
    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    int fd;
    unsigned char buffer[BUFFER_SIZE];

    path_file = fopen(path, "rb");
    if (path_file == NULL) 
    {
        printf("Error: could not open file %s\n", path);
        return NULL;
    }

    MD5_Init(&md5_context);

    while ((file_contents = fread(buffer, 1, sizeof(buffer), path_file)) != 0) 
    {
        MD5_Update(&md5_context, buffer, file_contents);
    }

    MD5_Final(md5_hash, &md5_context);

    fclose(path_file);

    // 16진수로 변환된 해시값을 저장할 문자열 버퍼를 할당합니다.
    char* hash_str = malloc(2 * MD5_DIGEST_LENGTH + 1);
    if (hash_str == NULL) 
    {
        printf("Error: memory allocation failed.\n");
        return NULL;
    }

    // 해시 값을 16진수 문자열로 변환합니다.
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) 
    {
        sprintf(hash_str + (2 * i), "%02x", md5_hash[i]);
    }

    hash_str[2 * MD5_DIGEST_LENGTH] = '\0';

    return hash_str;
}


// 파일 경로를 넣어줬을 경우 해쉬 값 나오는 함수 
char* return_hash_value_sha(char* path)
{
    FILE *path_file;
    size_t file_contents;
    SHA_CTX sha_context;
    unsigned char sha_hash[SHA_DIGEST_LENGTH];
    int fd;
    unsigned char buffer[BUFFER_SIZE];

    path_file = fopen(path, "rb");
    if (path_file == NULL) 
    {
        printf("Error: could not open file %s\n", path);
        return NULL;
    }

    SHA1_Init(&sha_context);

    while ((file_contents = fread(buffer, 1, sizeof(buffer), path_file)) != 0) 
    {
        SHA1_Update(&sha_context, buffer, file_contents);
    }

    SHA1_Final(sha_hash, &sha_context);

    fclose(path_file);

    // 16진수로 변환된 해시값을 저장할 문자열 버퍼를 할당합니다.
    char* hash_str = malloc(2 * SHA_DIGEST_LENGTH + 1);
    if (hash_str == NULL) 
    {
        printf("Error: memory allocation failed.\n");
        return NULL;
    }

    // 해시 값을 16진수 문자열로 변환합니다.
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) 
    {
        sprintf(hash_str + (2 * i), "%02x", sha_hash[i]);
    }

    hash_str[2 * SHA_DIGEST_LENGTH] = '\0';

    return hash_str;
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