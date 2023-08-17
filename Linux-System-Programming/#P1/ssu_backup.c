#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 4096

int main (int argc , char* argv[]) 
{

    if (argc != 2) // 실행 명령어 올바른지 확인함
    {
        printf("Usage: ssu_backup <md5 | sha1>\n");
        return 1;
    }
    else
    {
        if (strcmp(argv[1], "sha1") == 0 || strcmp(argv[1], "md5") == 0) // 실행 명령어 올바른지 확인함
        {
            char* current_path = (char*) malloc(sizeof(char) * MAX_PATH); // 현재 작업중인 경로를 저장하는 변수
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
            strcat(home_path, "/");
            strcat(home_path,"backup");

            // 디렉토리 생성
            struct stat directory;
            if (stat(home_path, &directory) == -1) // 디렉토리가 존재하지 않을 때
            {  
                if (mkdir(home_path, 0777) == -1) // 디렉토리 생성
                {  
                    perror("mkdir");
                    exit(1);
                }
            }

            // 프로그램 시작 
            while (1) 
            {

                char* input = (char*) malloc(sizeof(char) * MAX_PATH); // 입력받은 문자열을 저장할 메모리 공간 동적 할당
                char* cwd = (char*) malloc(sizeof(char) * MAX_PATH);
                printf("20192393>");
                fgets(input, MAX_PATH, stdin); // 입력받은 문자열 저장

                // 입력 문자열에서 개행 문자를 삭제함
                size_t len = strlen(input);
                if (len > 0 && input[len-1] == '\n') 
                {
                    input[len-1] = '\0'; // 입력 문자열을 NULL 값으로 바꿈
                }

                char** tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
                char* token = strtok(input, " "); // 첫 번째 토큰 추출
                
                int token_count = 0; // 토큰의 개수를 저장하는 변수
                while (token != NULL) // 토큰이 더 이상 없을 때까지 반복
                { 
                    tokens[token_count] = token; // 현재 토큰을 저장
                    token_count++;
                    token = strtok(NULL, " "); // 다음 토큰 추출
                }
                

                // 개행 입력시 프롬프트 재출력
                if (token_count == 0 || strcmp(tokens[0], "") == 0) 
                {
                    // 동적 할당된 메모리 해제
                    free(input);
                    free(tokens);
                    continue;
                }
                else
                {
                    if (strcmp(tokens[0], "add") == 0)
                    {
                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                        char* work = (char*) malloc(sizeof(char) * MAX_PATH); // 현재 작업 디렉토리 저장 변수

                        char** t_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
                        temp = strcpy(temp, argv[0]);
                        char* t_token = strtok(temp, "/"); // 첫 번째 토큰 추출
                        
                        int t_token_count = 0; // 토큰의 개수를 저장하는 변수
                        
                        while (t_token != NULL) // 토큰이 더 이상 없을 때까지 반복
                        { 
                            t_tokens[t_token_count] = t_token; // 현재 토큰을 저장
                            t_token_count++;
                            t_token = strtok(NULL, "/"); // 다음 토큰 추출
                        }

                        for(int i = 0 ; i < t_token_count - 1 ; i++)
                        {
                            strcat(work, t_tokens[i]);
                            strcat(work, "/");
                        }

                        // hash 함수 저장해서 tokens 로 넘기는 변수
                        tokens[token_count] = argv[1];

                        pid_t pid;
                        pid = fork();
                        int a = 0;

                        strcat(work, "ssu_backup_add");

                        if (pid == 0)
                        {
                            execv(work, tokens);
                            exit(0);
                        }
                        else
                        {
                            waitpid(pid, &a, 0);
                        }

                        free(work);
                        
                    }
                    else if (strcmp(tokens[0], "remove") == 0)
                    {
                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                        char* work = (char*) malloc(sizeof(char) * MAX_PATH); // 현재 작업 디렉토리 저장 변수

                        char** t_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
                        temp = strcpy(temp, argv[0]);
                        char* t_token = strtok(temp, "/"); // 첫 번째 토큰 추출
                        
                        int t_token_count = 0; // 토큰의 개수를 저장하는 변수
                        
                        while (t_token != NULL) // 토큰이 더 이상 없을 때까지 반복
                        { 
                            t_tokens[t_token_count] = t_token; // 현재 토큰을 저장
                            t_token_count++;
                            t_token = strtok(NULL, "/"); // 다음 토큰 추출
                        }

                        for(int i = 0 ; i < t_token_count - 1 ; i++)
                        {
                            strcat(work, t_tokens[i]);
                            strcat(work, "/");
                        }

                        // hash 함수 저장해서 tokens 로 넘기는 변수
                        tokens[token_count] = argv[1];

                        pid_t pid;
                        pid = fork();
                        int a = 0;

                        strcat(work, "ssu_backup_remove");

                        if (pid == 0)
                        {
                            execv(work, tokens);
                            exit(0);
                        }
                        else
                        {
                            waitpid(pid, &a, 0);
                        }
                        free(work);
                    }
                    else if (strcmp(tokens[0], "recover") == 0)
                    {
                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                        char* work = (char*) malloc(sizeof(char) * MAX_PATH); // 현재 작업 디렉토리 저장 변수

                        char** t_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
                        temp = strcpy(temp, argv[0]);
                        char* t_token = strtok(temp, "/"); // 첫 번째 토큰 추출
                        
                        int t_token_count = 0; // 토큰의 개수를 저장하는 변수
                        
                        while (t_token != NULL) // 토큰이 더 이상 없을 때까지 반복
                        { 
                            t_tokens[t_token_count] = t_token; // 현재 토큰을 저장
                            t_token_count++;
                            t_token = strtok(NULL, "/"); // 다음 토큰 추출
                        }

                        for(int i = 0 ; i < t_token_count - 1 ; i++)
                        {
                            strcat(work, t_tokens[i]);
                            strcat(work, "/");
                        }

                        // hash 함수 저장해서 tokens 로 넘기는 변수
                        tokens[token_count] = argv[1];

                        pid_t pid;
                        pid = fork();
                        int a = 0;

                        strcat(work, "ssu_backup_recover");

                        if (pid == 0)
                        {
                            execv(work, tokens);
                            exit(0);
                        }
                        else
                        {
                            waitpid(pid, &a, 0);
                        }
                        free(work);
                    }
                    else if (strcmp(tokens[0], "ls") == 0)
                    {
                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                        char* work = (char*) malloc(sizeof(char) * MAX_PATH); // 현재 작업 디렉토리 저장 변수

                        char** t_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
                        temp = strcpy(temp, argv[0]);
                        char* t_token = strtok(temp, "/"); // 첫 번째 토큰 추출
                        
                        int t_token_count = 0; // 토큰의 개수를 저장하는 변수
                        
                        while (t_token != NULL) // 토큰이 더 이상 없을 때까지 반복
                        { 
                            t_tokens[t_token_count] = t_token; // 현재 토큰을 저장
                            t_token_count++;
                            t_token = strtok(NULL, "/"); // 다음 토큰 추출
                        }

                        for(int i = 0 ; i < t_token_count - 1 ; i++)
                        {
                            strcat(work, t_tokens[i]);
                            strcat(work, "/");
                        }

                        pid_t pid;
                        pid = fork();
                        int a = 0;

                        strcat(work, "ssu_backup_ls");

                        if (pid == 0)
                        {
                            execv(work, tokens);
                            exit(1);
                        }
                        else
                        {
                            waitpid(pid, &a, 0);
                        }
                        free(work);
                    }
                    else if (strcmp(tokens[0], "vi") == 0 || strcmp(tokens[0], "vim") == 0)
                    {
                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                        char* work = (char*) malloc(sizeof(char) * MAX_PATH); // 현재 작업 디렉토리 저장 변수

                        char** t_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
                        temp = strcpy(temp, argv[0]);
                        char* t_token = strtok(temp, "/"); // 첫 번째 토큰 추출
                        
                        int t_token_count = 0; // 토큰의 개수를 저장하는 변수
                        
                        while (t_token != NULL) // 토큰이 더 이상 없을 때까지 반복
                        { 
                            t_tokens[t_token_count] = t_token; // 현재 토큰을 저장
                            t_token_count++;
                            t_token = strtok(NULL, "/"); // 다음 토큰 추출
                        }

                        for(int i = 0 ; i < t_token_count - 1 ; i++)
                        {
                            strcat(work, t_tokens[i]);
                            strcat(work, "/");
                        }

                        pid_t pid;
                        pid = fork();
                        int a = 0;

                        strcat(work, "ssu_backup_vi");

                        if (pid == 0)
                        {
                            execv(work, tokens);
                            exit(0);
                        }
                        else
                        {
                            waitpid(pid, &a, 0);
                        }
                        free(work);
                    }
                    else if (strcmp(tokens[0], "help") == 0) 
                    {
                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                        char* work = (char*) malloc(sizeof(char) * MAX_PATH); // 현재 작업 디렉토리 저장 변수

                        char** t_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
                        temp = strcpy(temp, argv[0]);
                        char* t_token = strtok(temp, "/"); // 첫 번째 토큰 추출
                        
                        int t_token_count = 0; // 토큰의 개수를 저장하는 변수
                        
                        while (t_token != NULL) // 토큰이 더 이상 없을 때까지 반복
                        { 
                            t_tokens[t_token_count] = t_token; // 현재 토큰을 저장
                            t_token_count++;
                            t_token = strtok(NULL, "/"); // 다음 토큰 추출
                        }

                        for(int i = 0 ; i < t_token_count - 1 ; i++)
                        {
                            strcat(work, t_tokens[i]);
                            strcat(work, "/");
                        }

                        pid_t pid;
                        pid = fork();
                        int a = 0;

                        strcat(work, "ssu_backup_help");

                        if (pid == 0)
                        {
                            execv(work, tokens);
                            exit(0);
                        }
                        else
                        {
                            waitpid(pid, &a, 0);
                        }
                        free(work);
                    }
                    else if (strcmp(tokens[0], "exit") == 0 && tokens[1] == NULL)
                    {
                        exit(1);
                    }
                    else
                    {
                        char* temp = (char*) malloc(sizeof(char) * MAX_PATH);
                        char* work = (char*) malloc(sizeof(char) * MAX_PATH); // 현재 작업 디렉토리 저장 변수

                        char** t_tokens = (char**) malloc(sizeof(char*) * MAX_PATH); // 토큰들을 저장할 메모리 공간 동적 할당
                        temp = strcpy(temp, argv[0]);
                        char* t_token = strtok(temp, "/"); // 첫 번째 토큰 추출
                        
                        int t_token_count = 0; // 토큰의 개수를 저장하는 변수
                        
                        while (t_token != NULL) // 토큰이 더 이상 없을 때까지 반복
                        { 
                            t_tokens[t_token_count] = t_token; // 현재 토큰을 저장
                            t_token_count++;
                            t_token = strtok(NULL, "/"); // 다음 토큰 추출
                        }

                        for(int i = 0 ; i < t_token_count - 1 ; i++)
                        {
                            strcat(work, t_tokens[i]);
                            strcat(work, "/");
                        }

                        pid_t pid;
                        pid = fork();
                        int a = 0;

                        strcat(work, "ssu_backup_help");
                        
                        if (pid == 0)
                        {
                            execv(work, tokens);
                            exit(0);
                        }
                        else
                        {
                            waitpid(pid, &a, 0);
                        }
                        free(work);
                    }
                    memset(cwd, '\0', sizeof(char) * MAX_PATH); // 디렉토리 확인한 경로 비워주기
                }

                memset(tokens, '\0' , sizeof(char*) * MAX_PATH);
                

                // 동적 할당된 메모리 해제
                free(cwd);
                free(input);
            }
        }
        else
        {
            printf("Usage: ssu_backup <md5 | sha1>\n");
            return 1;
        }
    }

    return 0;
}