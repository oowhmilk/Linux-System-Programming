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

extern int errno;

char* change_path(char* input_path, char* result_path);

char* get_time(char* tmp_text);

int correct_path(char* path);

int filecopy(const char* input_path, const char* backup_path);

int check_hash_code(char* input_path, char* check_path, char* hash);

char* cut_time_path(char *path, char* temp_path);

char* return_hash_value_md5(char* path);

char* return_hash_value_sha(char* path);

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

// 홈 디렉토리 경로 전역변수
char* home_path;


int main(int argc, char** tokens)
{
    char* input_path = NULL;
    char* backup_path = NULL;
    char* temp_path = NULL;
    char* current_path = NULL;
    char* check_path = NULL;

    // hash 함수 저장 변수 
    char* hash = (char*) malloc(sizeof(char) * 5);
    hash = tokens[argc - 1];

    // 현재 경로에 디렉토리만 존재하는 경우를 알기윈한 변수
    int directory_num = 0;

    if (tokens != NULL )
    {
        fflush(stdout);
    }

    if ((strcmp(tokens[1], "md5") == 0 || strcmp(tokens[1], "sha1") == 0)) // 첫 번째 인자 입력이 없는 경우
    {
        printf("Usage : add <FILENAME> [OPTION]\n");
        printf("  -d : add directory recursive\n");
    }
    else
    {
        if (strlen(tokens[1]) < MAX_PATH ) // 경로 길이가 제한을 넘을 경우
        {
            input_path = (char*) malloc(sizeof(char) * MAX_PATH); // 입력받은 경로를 저장하는 변수
            backup_path = (char*) malloc(sizeof(char) * MAX_PATH); // 결과 경로를 저장하는 변수
            temp_path = (char*) malloc(sizeof(char) * MAX_PATH); // 임시로 경로를 저장하는 변수
            current_path = (char*) malloc(sizeof(char) * MAX_PATH); // 현재 작업중인 경로를 저장하는 변수
            check_path = (char*) malloc(sizeof(char) * MAX_PATH); // 확인하기 위한 경로를 저장하는 변수 
            strcpy(input_path, tokens[1]); // 입력받은 경로 저장하기 

            char** path_tokens = NULL; // 토큰들을 저장할 메모리 공간 동적 할당
            char* path_token = NULL; // 첫 번째 토큰 추출
            int path_token_count = 0; // 토큰의 개수를 저장하는 변수

            // 홈 디렉토리 찾기 
            char cwd[4096];
            getcwd(cwd, sizeof(cwd));
            home_path = (char*) malloc(sizeof(char) * 256); 

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

            // 상대경로 입력 시
            if (input_path[0] != '/')
            {
                if(input_path[0] == '~') // ~ 입력 시
                {
                    strcpy(temp_path, input_path + 2);
                    strcpy(input_path, temp_path);
                    
                } 
                realpath(input_path, temp_path); // 상대경로를 절대경로를 바꾸기
                strcpy(input_path, temp_path);
            }
            // 절대경로 입력 시
            else 
            {
                
            }

            //디렉토리 백업하기
            struct stat check; // 파일 정보를 저장할 구조체 
            stat(input_path, &check); // stat 함수로 파일 정보를 가져오기

            if(correct_path(input_path) == 0) // 제대로 된 경로인 경우 
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
                        struct stat check; // 파일 정보를 저장할 구조체 
                        stat(input_path, &check); // stat 함수로 파일 정보를 가져오기

                        // 파일 링크드리스트로 구현하기
                        insert(input_path, S_ISDIR(check.st_mode));

                        if(S_ISDIR(check.st_mode) == 1)
                        {
                            traverse_directory(input_path);
                        }
                        // 반복횟수 지정해주기
                        int num = count_list();

                        Node* current = tail; // 링크드 리스트 뒤에서 부터 확인하기 
                        
                        for(int i = 0 ; i < num ; i++)
                        {
                            
                            strcpy(input_path, current->path);
                            current = current->prev; // 링크드 리스크로 구현된 파일 구조 확인하기 

                            struct stat check; // 파일 정보를 저장할 구조체 
                            stat(input_path, &check); // stat 함수로 파일 정보를 가져오기

                            if (strstr(input_path, "/backup") != NULL) // input_path 경로에 "backup" 가 포함되어있는지 확인하기 
                            {
                                printf("\"%s\" can't be backuped\n",input_path); // input_path 경로에 "backup" 포함된 경우 
                            }
                            else
                            {
                                if (S_ISDIR(check.st_mode) == 1) // 입력받은 경로 파일 유형이 디렉토리인 경우 
                                {

                                    if (tokens[2] != NULL && (strcmp(tokens[2], "md5") != 0 && strcmp(tokens[2], "sha1") != 0)) // 첫번째 인자가 디렉토리일 경우 옵션을 넣었을 경우
                                    {

                                        if (strcmp(tokens[2], "-d") == 0)
                                        {
                                            directory_num ++;

                                            // 백업할 경로 구하기
                                            backup_path = change_path(input_path, backup_path);

                                            // 백업할 경로 임시 저장 경로에 저장하기 
                                            strcpy(temp_path, backup_path);

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

                                            // 사용자 홈 디렉토리 경로 만들기 
                                            strcat(current_path,"/");
                                            strcat(current_path, path_tokens[0]);
                                            strcat(current_path,"/");
                                            strcat(current_path, path_tokens[1]);

                                            // 사용자 홈 디렉토리부터 디렉토리 확인하기
                                            for(int i = 2 ; i < path_token_count ; i++)
                                            {
                                                strcat(current_path,"/");
                                                strcat(current_path,path_tokens[i]);

                                                DIR* dir = opendir(current_path);
                                                if (dir == NULL) // 디렉토리 확인하면서 없는 디렉토리 생성하기 
                                                {
                                                    mkdir(current_path, 0777); // 디렉토리 mode_t 설정하기
                                                }
                                            }
                                            
                                            memset(current_path, '\0', sizeof(char) * MAX_PATH); // 디렉토리 확인한 경로 비워주기
                                        }
                                        else // 두번째 인자 옵션이 제대로 안 들어왔을 때
                                        {
                                            printf("Usage : add <FILENAME> [OPTION]\n"); 
                                            printf("  -d : add directory recursive\n");
                                            exit(0);
                                        }
                                        
                                        // 동적 할당된 메모리 해제
                                        free(path_tokens);
                                        free(path_token);

                                    }
                                    else if((strcmp(tokens[2], "md5") == 0 || strcmp(tokens[2], "sha1") == 0)) // 첫번째 인자가 디렉토리일 경우 옵션을 안 넣었을 경우
                                    {
                                        printf("\"%s\" is a directory file\n",input_path);
                                        exit(0);
                                    }
                                    
                                } 
                                else if (S_ISDIR(check.st_mode) != 1) // 입력받은 경로 파일 유형이 디렉토리가 아닌 경우 
                                {
                                    if (correct_path(input_path) == 0)
                                    {
                                        if (tokens[2] != NULL && strcmp(tokens[2], "-help") == 0 )
                                        {
                                            printf("Usage : add <FILENAME> [OPTION]\n"); 
                                            printf("  -d : add directory recursive\n");
                                        }
                                        else if(strcmp(tokens[2], "-d") == 0 || strcmp(tokens[2], "md5") == 0 || strcmp(tokens[2], "sha1") == 0) // 입력받은 경로가 파일이므로 두번째 인자까지만 있으므로 세번째 인자는 해시 값
                                        {
                                            // 백업할 경로 구하기 
                                            backup_path = change_path(input_path, backup_path);

                                            // 현재 시간 구하기
                                            char* tmp_text = (char*) malloc(sizeof(char) * 16); // text 임시 저장 변수 
                                            tmp_text = get_time(tmp_text);
                                    
                                            // 백업한 파일 경로 파일명 
                                            strcat(backup_path, "_");
                                            strcat(backup_path, tmp_text);

                                            if (check_hash_code(input_path, check_path, hash) == -1) // 백업할 파일과 같은 해쉬값을 가진 파일이 존재하는 경우 
                                            {
                                                
                                            }
                                            else // 백업할 파일과 같은 해쉬값을 가진 파일이 존재하지 않는 경우 
                                            {
                                                
                                                strcpy(temp_path, backup_path);

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

                                                // 사용자 홈 디렉토리 경로 만들기 
                                                strcat(current_path,"/");
                                                strcat(current_path, path_tokens[0]);
                                                strcat(current_path,"/");
                                                strcat(current_path, path_tokens[1]);

                                                // 사용자 홈 디렉토리부터 디렉토리 확인하기
                                                for(int i = 2 ; i < path_token_count - 1 ; i++)
                                                {
                                                    strcat(current_path,"/");
                                                    strcat(current_path,path_tokens[i]);

                                                    DIR* dir = opendir(current_path);
                                                    if (dir == NULL) // 디렉토리 확인하면서 없는 디렉토리 생성하기 
                                                    {
                                                        mkdir(current_path, 0777);
                                                    }
                                                }
                                                
                                                // 파일 백업하기 
                                                if(filecopy(input_path, backup_path) == 0) // 파일 복사가 성공한 경우
                                                {
                                                    printf("\"%s\" backuped\n", backup_path);
                                                }

                                                // 동적 할당된 메모리 해제
                                                free(path_tokens);
                                                free(path_token);

                                            }
                                            // 동적 할당된 메모리 해제
                                            free(tmp_text);
                                        }
                                        else if(strcmp(tokens[2], "-d") != 0 || strcmp(tokens[2], "md5") == 0 || strcmp(tokens[2], "sha1") == 0)
                                        {
                                            printf("Usage : add <FILENAME> [OPTION]\n"); 
                                            printf("  -d : add directory recursive\n");
                                        }
                                    }
                                    else
                                    {
                                        return 0;
                                    }
                                }
                            }

                            
                        }
                        if((num - directory_num) == 0)
                        {   
                            delete_list();
                            struct stat check; // 파일 정보를 저장할 구조체 
                            stat(backup_path, &check); // stat 함수로 파일 정보를 가져오기

                            // 파일 링크드리스트로 구현하기
                            insert(backup_path, S_ISDIR(check.st_mode));
                            if (S_ISDIR(check.st_mode) == 1)
                            {
                                traverse_directory(backup_path);
                            }
                            
                            Node* current = head;
                            while (current != NULL) 
                            {
                                rmdir(current->path);
                                current = current->next;
                            }
                            printf("No file in this Directory\n");
                        }
                    }

                }    
            }
            else // 제대로 된 경로가 아닌 경우 
            {
                return 0;
            }

            
            
        // 동적 할당된 메모리 해제
        free(current_path);
        }              
        else
        {
            printf("Limit Path Length\n"); // 경로 길이 제한
        }
    }

    // 동적 할당된 메모리 해제
    free(input_path);
    free(backup_path);
    free(temp_path);

    exit(0);

    return 0;
}


// 입력받은 경로가 제대로 된 경로인지 확인하는 함수
int correct_path(char* path)
{
    struct stat check; // 파일 정보를 저장할 구조체 
    stat(path, &check); // stat 함수로 파일 정보를 가져옴 

    if(access(path, F_OK) == -1 && errno == ENOENT) // 해당 경로에 파일 존재 여부 확인하기 
    {
        printf("Usage : add <FILENAME> [OPTION]\n"); // 해당 경로에 파일이 존재하지 않을때 
        printf("  -d : add directory recursive\n");
        return -1;
    }
    else
    {
        if(S_ISREG(check.st_mode) || S_ISDIR(check.st_mode)) // 해당 경로가 일반 파일 , 디렉토리 인지 확인하기
        {
            if((check.st_mode & S_IRWXU) != 0) // 파일에 대한 접근 권한 확인하기 
            {
                return 0;
            }
            else
            {
                printf("Access Control\n"); // 파일에 대한 접근 권한 없을때 
                return -1;
            }
        }
        else
        {
            printf("Not Regular File , Not Directory\n"); // 일반 파일 , 디렉토리가 아닐때 
            return -1;
        }

    }
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




// 현재 시간 구하는 함수
char* get_time(char* tmp_text)
{
    time_t current_time = time(NULL); // 현재 시간을 저장하는 변수 
        struct tm local_time = *localtime(&current_time); // 현재 시간을 구하는 변수 
        long tmp_num = (local_time.tm_year + 1900 - 2000) * 10000000000 + (local_time.tm_mon + 1) * 100000000 + (local_time.tm_mday) * 1000000 + (local_time.tm_hour) * 10000 + (local_time.tm_min) * 100 + local_time.tm_sec;
        sprintf(tmp_text, "%ld", tmp_num);

    return tmp_text;
}


// 파일 복사하는 함수 
int filecopy(const char* input_path, const char* backup_path) {

    FILE* exist_file;
    FILE* backup_file;

    exist_file = fopen(input_path, "a+"); // 원본 파일 확인하기 
    backup_file = fopen(backup_path, "a+"); // 백업할 파일 만들기

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


// 해쉬함수로 파일 동일 여부 확인하는 함수 
int check_hash_code(char* input_path, char* check_path, char* hash)
{
    int return_num; // return 할 갑 저장 변수

    // 입력받은 경로 백업할 경로로 바꾸기  
    check_path = change_path(input_path, check_path);

    // backup 디렉토리 경로 만들기 
    char* backup_directory_path = (char*) malloc(sizeof(char) * MAX_PATH);
    backup_directory_path = change_path(home_path, backup_directory_path);

    struct stat check; // 파일 정보를 저장할 구조체 
    stat(backup_directory_path, &check); // stat 함수로 파일 정보를 가져오기

    // 파일 링크드리스트로 구현하기
    insert(backup_directory_path, S_ISDIR(check.st_mode));

    if (S_ISDIR(check.st_mode) == 1)
    {
        traverse_directory(backup_directory_path);
    }

    Node* check_node = head; // 링크드 리스트 뒤에서 부터 확인하기 

    while (check_node != NULL) 
    {
        struct stat check; // 파일 정보를 저장할 구조체 
        stat(check_node->path, &check); // stat 함수로 파일 정보를 가져오기

        if (S_ISDIR(check.st_mode) == 0) // 입력받은 경로 파일 유형이 디렉토리가 아닌 경우 
        {
            char* temp = NULL; // 임시로 경로를 저장하는 변수
            temp = cut_time_path(check_node->path, temp); 

            if (strcmp(temp, check_path) == 0) // 새로 저장할 경로 랑 이미 저장한 경로랑 같은것이 있을 경우 
            {

                if(strcmp(hash, "md5") == 0)
                {
                    if (strcmp(return_hash_value_md5(check_node->path), return_hash_value_md5(input_path)) != 0) // 두 경로에 따른 파일의 해시값이 다른 경우 
                    {
                        return_num = 0;
                    }
                    else  // 두 경로에 따른 파일의 해시 값이 같은 경우
                    {
                        printf("\"%s\" is already backuped\n", check_node -> path);
                        return return_num = -1;
                        break;
                    }
                }
                else
                {
                    if (strcmp(return_hash_value_sha(check_node->path), return_hash_value_sha(input_path)) != 0) // 두 경로에 따른 파일의 해시값이 다른 경우 
                    {
                        return_num = 0;
                    }
                    else  // 두 경로에 따른 파일의 해시 값이 같은 경우
                    {
                        printf("\"%s\" is already backuped\n", check_node -> path);
                        return return_num = -1;
                        break;
                    }
                }
            }
        }
        else
        {
            
        }
        check_node = check_node->next; // 링크드 리스크로 구현된 파일 구조 확인하기 
    }

    return return_num;
}

// backup path 에서 뒤에 14자리 자르는 함수
char* cut_time_path(char *path, char* temp)
{
    struct stat check; // 파일 정보를 저장할 구조체 
    stat(path, &check); // stat 함수로 파일 정보를 가져오기

    // 파일 링크드리스트로 구현하기
    insert(path, S_ISDIR(check.st_mode));

    if(S_ISDIR(check.st_mode) != 1) // 디렉토리인 경우
    {
        temp = (char*) malloc(sizeof(char) * (int)strlen(path) - 13); // 임시로 경로를 저장하는 변수 길이를 지정하기 
        for(int i = 0 ; i < (int)strlen(path) - 13 ; i++)
        {
            temp[i] = path[i];
        }
        return temp;
    }
    else
    {
        strcpy(temp, path);
        return temp;
    }
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

// 링크드 리스트에 현재 존재하는 파일 개수 확인하는 함수
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
