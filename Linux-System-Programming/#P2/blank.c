#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "blank.h"

char datatype[DATATYPE_SIZE][MINLEN] = {"int", "char", "double", "float", "long"
			, "short", "ushort", "FILE", "DIR","pid"
			,"key_t", "ssize_t", "mode_t", "ino_t", "dev_t"
			, "nlink_t", "uid_t", "gid_t", "time_t", "blksize_t"
			, "blkcnt_t", "pid_t", "pthread_mutex_t", "pthread_cond_t", "pthread_t"
			, "void", "size_t", "unsigned", "sigset_t", "sigjmp_buf"
			, "rlim_t", "jmp_buf", "sig_atomic_t", "clock_t", "struct"};


operator_precedence operators[OPERATOR_CNT] = {
	{"(", 0}, {")", 0}
	,{"->", 1}	
	,{"*", 4}	,{"/", 3}	,{"%", 2}	
	,{"+", 6}	,{"-", 5}	
	,{"<", 7}	,{"<=", 7}	,{">", 7}	,{">=", 7}
	,{"==", 8}	,{"!=", 8}
	,{"&", 9}
	,{"^", 10}
	,{"|", 11}
	,{"&&", 12}
	,{"||", 13}
	,{"=", 14}	,{"+=", 14}	,{"-=", 14}	,{"&=", 14}	,{"|=", 14}
};

void compare_tree(node *root1,  node *root2, int *result) // root1 , root2 node 의 tree 를 비교하는 함수
{
	node *tmp; // 임시 저장할 node 저장 변수
	int cnt1, cnt2;

	if(root1 == NULL || root2 == NULL) // root1, root2 node 중에 NULL 이 있을 경우
	{
		*result = false; // result 값을 false 
		return;
	}

	if(!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")) // 루트 노드 이름이 부등호인 경우
	{ 
		if(strcmp(root1->name, root2->name) != 0){ // 두 개의 루트 노드 이름이 다를 경우

			if(!strncmp(root2->name, "<", 1)) // root2 name 이 "<" 인 경우
				strncpy(root2->name, ">", 1); // root2 name 이 ">" 덮어쓰기

			else if(!strncmp(root2->name, ">", 1))
				strncpy(root2->name, "<", 1);

			else if(!strncmp(root2->name, "<=", 2))
				strncpy(root2->name, ">=", 2);

			else if(!strncmp(root2->name, ">=", 2))
				strncpy(root2->name, "<=", 2);

			root2 = change_sibling(root2); // 루트 node 의 자식 노드를 옆에 자식 노드로 바꾸기
		}
	}

	if(strcmp(root1->name, root2->name) != 0) // 두 개의 루트 노드 이름이 다를 경우
	{ 
		*result = false;
		return;
	}

	if((root1->child_head != NULL && root2->child_head == NULL) 
		|| (root1->child_head == NULL && root2->child_head != NULL)) // 두 개의 루트 노드 중에 하나만 자식 노드가 없는 경우
	{ 
		*result = false;
		return;
	}

	else if(root1->child_head != NULL) // root1 node 의 child 가 존재할 경우 == root1 , root2 node 둘다 NULL 이 아닌 경우
	{ 
		if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){ // 두 개의 루트 노드의 자식 노드의 수가 다른 경우
			*result = false;
			return;
		}

		if(!strcmp(root1->name, "==") || !strcmp(root1->name, "!=")) // root1 node 이름이 "==" , "!=" 둘중에 하나인 경우
		{
			compare_tree(root1->child_head, root2->child_head, result); // 루트 노드의 서브 트리를 비교하기

			if(*result == false) // result 가 false 일 경우
			{
				*result = true;
				root2 = change_sibling(root2); // 루트 node 의 자식 노드를 옆에 자식 노드로 바꾸기
				compare_tree(root1->child_head, root2->child_head, result); // 재귀적으로 함수 호출해서 서브트리 검사하기
			}
		}
		else if(!strcmp(root1->name, "+") || !strcmp(root1->name, "*")
				|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
				|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&")) // root1 node name 이 "+", "*", "|", "||", "&", "&&" 일 경우
		{
			if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)) // root1 child node 의 sibling node 개수, root2 child node 의 sibling node 개수가 다를 경우 
			{
				*result = false;
				return;
			}

			tmp = root2->child_head; // root2 child node 를 tmp 에 저장

			while(tmp->prev != NULL) // tmp node 의 prev 가 있을 경우
				tmp = tmp->prev; // 루트 노드의 자식 노드의 prev로 끝까지 이동하기

			while(tmp != NULL) // 맨 prev 로 이동한 tmp 가 NULL 이 아닌 경우
			{
				compare_tree(root1->child_head, tmp, result); // 두 개의 루트 노드의 자식 서브 트리 비교하기 
			
				if(*result == true) // result 가 true 인 경우
					break;
				else
				{
					if(tmp->next != NULL) // tmp next 가 NULL 이 아닌 경우
						*result = true; // result 가 true 저장
					tmp = tmp->next; // tmp 를 next 로 이동
				}
			}
		}
		else
		{
			compare_tree(root1->child_head, root2->child_head, result); // 재귀적으로 함수 호출해서 서브트리 검사하기
		}
	}	

	if(root1->next != NULL) // root node next 가 있을 경우
	{

		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)) // root1 child node 의 sibling node 개수, root2 child node 의 sibling node 개수가 다를 경우 
		{
			*result = false;
			return;
		}

		if(*result == true) // result true 일 경우
		{
			tmp = get_operator(root1); // root1 node 의 prev 맨 끝까지 이동후 parent node 저장
	
			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*")
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&")) // tmp node 의 name 이 "+", "*", "|", "||", "&", "&&" 일 경우
			{	
				tmp = root2; // tmp 에 root2 저장
	
				while(tmp->prev != NULL) // root2 node 의 prev 맨 끝까지 이동후 
					tmp = tmp->prev;

				while(tmp != NULL) // tmp 가 NULL 이 아닐 경우
				{
					compare_tree(root1->next, tmp, result); // 재귀적으로 tree 비교하는 함수

					if(*result == true) // result 가 true 인 경우
						break;
					else // result 가 false 인 경우
					{
						if(tmp->next != NULL) // tmp next 끝까지 이동
							*result = true;
						tmp = tmp->next;
					}
				}
			}

			else // tmp node 의 name 이 위의 연산자 가 아닌 경우
				compare_tree(root1->next, root2->next, result); // root1 , root2 next node 로 재귀적으로 tree 배교
		}
	}
}

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN]) // 인자로 들어오는 str 을 문자열 분리해서 tokens 만드는 함수
{
	char *start, *end; // end : start 이후로 op 가 처음으로 존재하는 포인터 저장 변수
	char tmp[BUFLEN]; // 임시 저장 변수
	char str2[BUFLEN];
	char *op = "(),;><=!|&^/+-*\""; // op 저장 변수
	int row = 0; //
	int i;
 	int isPointer; // "*" pointer 연산자 여부 확인 저장 변수
	int lcount, rcount; // lcount : "(" 개수, rcount : ")" 개수 저장 변수
	int p_str; // ( ) 안에 있는 tokens index 저장 변수 
	
	clear_tokens(tokens); // tokens 내용 지우는 함수

	start = str; // 인자로 받은 s_answer 저장 
	
	if(is_typeStatement(str) == 0) // 인자로 받은 str 시작하는 값에 따라 return 하는 함수 
		return false;	
	
	while(1)
	{
		if((end = strpbrk(start, op)) == NULL) // s_answer 가 op 가 존재하지 않을 경우
			break;

		if(start == end) // start, end 같은 경우 == start 시작이 op 로 시작한 경우
		{ 
			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)) // start 에 "--" , "++" 로 시작하는 경우
			{ 
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4)) // start 에 "----" , "++++" 로 시작하는 경우
					return false;

				// ex) ++a
				if(is_character(*ltrim(start + 2))) // start + 2 문자가 숫자, 소문자, 대문자 인 경우
				{ 
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])) // 이전 tokens 마지막 값이 char 인 경우
						return false; //ex) ++a++

					end = strpbrk(start + 2, op); // start + 2 와 op 변수중에 같은게 존재할 경우 해당 위치 저장

					if(end == NULL) // start + 2 와 op 같은게 존재하지 않을 경우
						end = &str[strlen(str)]; // str 문자열 맨끝을 가리키도록 한다
					while(start < end) { // start 포인터가 가리키는 위치가 end 포인터가 가리키는 위치보다 작을 경우
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1])) // start 왼쪽에 공백에 존재 && 이전 tokens 마지막 값이 char 인 경우
							return false;
						else if(*start != ' ') // start 가 공백이 아닐 경우
							strncat(tokens[row], start, 1); // tokens 에 start 문자 하나씩 추가
						start++;	
					}
				}
				// ex) a++
				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ // start + 2 문자가 숫자, 소문자, 대문자 아닌 경우 && 이전 tokens 마지막 값이 char 인 경우
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL) // 이전 tokens 값이 "++" , "--" 인 경우 
						return false;

					memset(tmp, 0, sizeof(tmp)); 
					strncpy(tmp, start, 2); // start 에서 2개 char tmp 에 저장
					strcat(tokens[row - 1], tmp); // tokens[row - 1] 에 tmp 붙히기
					start += 2;
					row--;
				}
				else{
					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp, start, 2);
					strcat(tokens[row], tmp);
					start += 2;
				}
			}

			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2)
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){ // 위에 조건 중에 하나로 start 시작할 경우

				strncpy(tokens[row], start, 2); // tokens 에 start 에서 2개 복사
				start += 2;
			}
			else if(!strncmp(start, "->", 2)) // "->" 로 start 시작하는 경우
			{
				end = strpbrk(start + 2, op); // start + 2 와 op 변수중에 같은게 존재할 경우 해당 위치 저장

				if(end == NULL) // op 와 같은게 없을 경우 
					end = &str[strlen(str)]; // end 는 마지막 문자 위치 저장

				while(start < end){ // start 포인터가 가리키는 위치가 end 포인터가 가리키는 위치보다 작을 경우
					if(*start != ' ') // start 가 공백이 아닐 경우
						strncat(tokens[row - 1], start, 1); // tokens 에 start 문자 하나씩 추가
					start++;
				}
				row--;
			}
			else if(*end == '&') // "&" 연산자 처리
			{
				// ex) &a (address)
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){ // 이전 tokens 가 없는 경우 || 이전 tokens 가 op 인 경우
					end = strpbrk(start + 1, op); // start + 1 와 op 변수중에 같은게 존재할 경우 해당 위치 저장
					if(end == NULL) // 다른 op 가 존재하지 않을 경우 
						end = &str[strlen(str)]; // end 는 마지막 문자 위치 저장
					
					strncat(tokens[row], start, 1); // 다른 op 가 존재할 경우 
					start++;

					while(start < end){ // start 포인터가 가리키는 위치가 end 포인터가 가리키는 위치보다 작을 경우
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&') // start 왼쪽에 공백 있을 경우 && 현재 tokens 에 "&" 가 없을 경우
							return false;
						else if(*start != ' ') // start 공백이 아닐 경우
							strncat(tokens[row], start, 1); // start 에 있는 값을 tokens 에 저장
						start++;
					}
				}
				// ex) a & b (bit)
				else{ // 
					strncpy(tokens[row], start, 1); // start 에 있는 값을 tokens 에 저장
					start += 1;
				}
				
			}
		  	else if(*end == '*') // "*" 연산자 처리
			{
				isPointer = 0; // "*" 가 pointer 연산자인지 확인하는 저장 변수

				if(row > 0) // tokens 가 존재하는 경우
				{
					//ex) char** (pointer)
					for(i = 0; i < DATATYPE_SIZE; i++)
					{ 
						if(strstr(tokens[row - 1], datatype[i]) != NULL) // 이전 tokens 에 datatype 과 같은 것이 존재할 경우
						{ 
							strcat(tokens[row - 1], "*"); // 이전 tokens 에 "*" 연산자 저장
							start += 1;	
							isPointer = 1; // pointer 연산자 확인하는 변수 "1" 저장
							break;
						}
					}
					if(isPointer == 1) // pointer 연산자인 경우
						continue;
					if(*(start+1) != 0) // start + 1 에 NULL 이 아닌 경우
						end = start + 1; // end = start + 1 로 포인터 이동

					// ex) a * **b (multiply then pointer)
					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)) // row > 1 && tokens[row - 2] 가 "*" 일 경우 && tokens[row - 1] 이 모두 "*" 일 경우 
					{ 
						strncat(tokens[row - 1], start, end - start); // 이전 token 에 start 부터 end 까지 저장
						row--;
					}
					
					// ex) a*b(multiply)
					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1) // 이전 tokens 의 마지막 값이 char 일 경우
					{  
						strncat(tokens[row], start, end - start); // 현재 tokens 에 start 부터 end 까지 저장
					}

					// ex) ,*b (pointer)
					else if(strpbrk(tokens[row - 1], op) != NULL) // 이전 tokens 에 "*" 가 존재하는 경우
					{ 
						strncat(tokens[row] , start, end - start); // 현재 tokens 에 start 부터 end 까지 저장
					}
					else
						strncat(tokens[row], start, end - start); // 현재 tokens 에 start 부터 end 까지 저장

					start += (end - start);
				}

			 	else if(row == 0) // tokens 가 존재하지 않는 경우
				{
					if((end = strpbrk(start + 1, op)) == NULL) // start + 1 뒤에 "*" 가 존재하지 않을 경우
					{ 
						strncat(tokens[row], start, 1); // start 에 있는 값을 tokens 에 저장
						start += 1; 
					}
					else{
						while(start < end)
						{ 
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1])) // start 이전 값이 공백 && 현재 tokens 의 마지막 값이 char 인 경우
								return false;
							else if(*start != ' ') // start 가 공백 일 경우
								strncat(tokens[row], start, 1); // start 에 있는 값을 tokens 에 저장
							start++;	
						}
						if(all_star(tokens[row])) // 현재 tokens 모두 "*" 인 경우
							row--;
						
					}
				}
			}
			else if(*end == '(') // "(" 연산자 처리
			{
				lcount = 0; // "(" 개수 저장 변수
				rcount = 0; // ")" 개수 저장 변수
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)) // row > 0 && (이전 tokens 이 "&" || 이전 tokens 이 "*") 인 경우
				{ 
					while(*(end + lcount + 1) == '(') 
						lcount++; // "(" 개수 저장 변수 증가
					start += lcount;

					end = strpbrk(start + 1, ")"); // start + 1 이후로 ")" 존재하는 포인터로 end 이동

					if(end == NULL) // end 가 NULL 일 경우 == ")" 가 없을 경우
						return false;
					else{ // end 가 NULL 이 아닐 경우 == ")" 가 있을 경우
						while(*(end + rcount + 1) == ')') 
							rcount++; // ")" 개수 저장 변수 증가
						end += rcount; 

						if(lcount != rcount) // "(" , ")" 개수 다를 경우
							return false;

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1) // (row > 1 && tokens[row - 2] 마지막 값 char 인 경우) || row == 1 인 경우
						{ 
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1); // tokens 에 저장
							row--;
							start = end + 1;
						}
						else
						{
							strncat(tokens[row], start, 1); // start 에 있는 값을 tokens 에 저장
							start += 1;
						}
					}
						
				}
				else
				{
					strncat(tokens[row], start, 1); // start 에 있는 값을 tokens 에 저장
					start += 1;
				}

			}
			else if(*end == '\"') // "\" 연산자 처리
			{
				end = strpbrk(start + 1, "\""); // start + 1 이후로 "\" 존재하는 포인터로 end 이동
				
				if(end == NULL) // "\" 존재하지 않을 경우
					return false;

				else{ // "\" 존재할 경우
					strncat(tokens[row], start, end - start + 1);
					start = end + 1;
				}

			}
			else
			{
				// ex) a++ ++ +b
				if(row > 0 && !strcmp(tokens[row - 1], "++"))
					return false;

				// ex) a-- -- -b
				if(row > 0 && !strcmp(tokens[row - 1], "--"))
					return false;
	
				strncat(tokens[row], start, 1);
				start += 1;
				
				// ex) -a or a, -b
				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++"))
				{
					// ex) -a or -a+b
					if(row == 0)
						row--;

					// ex) a+b = -c
					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]))
					{
						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL)
							row--;
					}
				}
			}
		}
		else // start 와 end 가 다를 경우
		{ 
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) // 이전 tokens 이 모두 "*" 인 경우 && row > 1 && tokens[row - 2] 마지막 값이 char 아닌 경우 
				row--;				

			if(all_star(tokens[row - 1]) && row == 1) // 이전 tokens 이 모두 "*" 인 경우 && row == 1
				row--;	

			for(i = 0; i < end - start; i++)
			{
				if(i > 0 && *(start + i) == '.') // i > 0 && start + i 값이 "." 인 경우
				{
					strncat(tokens[row], start + i, 1); //

					while(*(start + i + 1) == ' ' && i < end - start)
						i++; 
				}
				else if(start[i] == ' ') // start + i 값이 공백인 경우
				{
					while(start[i] == ' ') // 공백 나올때까지 
						i++;
					break;
				}
				else
					strncat(tokens[row], start + i, 1); // 현재 tokens 에 start + i 값을 저장
			}

			if(start[0] == ' ') // start[0] 공백인 경우
			{
				start += i;
				continue;
			}
			start += i;
		}
			
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); // 현재 tokens 좌 우 공백 제거 후 현재 tokens 에 저장 

		if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) // row > 0 && 현재 tokens 의 마지막 값이 char 인 경우 &&
				&& (is_typeStatement(tokens[row - 1]) == 2 // ( 인자로 받은 이전 tokens 시작하는 값에 따라 return ||
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) // 이전 tokens 의 마지막 값이 char 인 경우 ||
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ) // 이전 tokens 의 마지막 값이 "." 인 경우 )
		{

			if(row > 1 && strcmp(tokens[row - 2],"(") == 0) // row > 1 && tokens[row - 2] 가 "(" 인 경우
			{
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0) // 이전 tokens 값이 "struct" 아닌 경우 && 이전 tokens 값이 "unsigned" 아닌 경우
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) // row == 1 && 현재 tokens 의 마지막 값이 char 인 경우
			{
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) // tokens[0] 값이 "extern" && "unsigned" 아닌 경우 && tokens[0] 값이 type 예외 인 경우
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2) // row > 1 && 이전 tokens 값의 type 예외인 경우
			{
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0) // tokens[row - 2] 값이 "unsigned" && "extern" 이 아닌 경우
					return false;
			}
			
		}

		if((row == 0 && !strcmp(tokens[row], "gcc"))) // row == 0 && 현재 tokens 값이 gcc 인 경우
		{
			clear_tokens(tokens); // tokens 비우는 함수
			strcpy(tokens[0], str);	// str 값을 tokens[0] 에 저장
			return 1;
		} 

		row++;
	}

	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) // 이전 tokens 모든 값이 "*" 인 경우 && row > 1 && tokens[row - 2] 마지막 값이 char 아닌 경우
		row--;				
	if(all_star(tokens[row - 1]) && row == 1) // 이전 tokens 모든 값이 "*" 인 경우
		row--;	

	for(i = 0; i < strlen(start); i++)   
	{
		if(start[i] == ' ') // start[i] 공백인 경우
		{
			while(start[i] == ' ')
				i++;
			if(start[0]==' ') // start 처음이 공백인 경우
			{ 
				start += i;
				i = 0;
			}
			else
				row++;
			
			i--;
		} 
		else // start[i] 공백이 아닌 경우
		{
			strncat(tokens[row], start + i, 1); // 현재 tokens 에 start + i 값 저장
			if(start[i] == '.' && i < strlen(start)) // start[i] 값이 "." 인 경우 && i < start 길이 인 경우
			{
				while(start[i + 1] == ' ' && i < strlen(start)) // start[i + 1] 값이 공백 && i < start 길이 인 경우
					i++;

			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); // 현재 tokens 좌 우 공백 제거 후 현재 tokens 에 저장 

		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){ // 현재 tokens "lpthread" 아닌 경우 && 이전 tokens 값이 "-" 아닌 경우
			strcat(tokens[row - 1], tokens[row]); // 이전 tokens 에 현재 tokens 붙혀서 작성
			memset(tokens[row], 0, sizeof(tokens[row])); // 현재 tokens 비우기
			row--;
		}
	 	else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) // row > 0 && 현재 tokens 마지막 값이 char 인 경우 &&
				&& (is_typeStatement(tokens[row - 1]) == 2 // (이전 tokens 값이 type ||
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) // 이전 tokens 마지막 값이 char 인 경우 ||
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.')) // 이전 tokens 마지막 값이 "."
		{
			if(row > 1 && strcmp(tokens[row-2],"(") == 0) // row > 1 && tokens[row-2] 값이 "(" 인 경우
			{
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1], "unsigned") != 0) // 이전 tokens 값이 "struct" && "unsigned" 아닌 경우
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) // row == 1 && 현재 tokens 값이 char 인 경우
			{
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) // tokens[0] 값이 "extern" && "unsigned" 아닌 경우 && tokens[0] type 
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2) // row > 1 && tokens[row-1] type 예외 처리
			{
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0) // tokens[row -2] 값이 "unsigned" && "extern" 아닌 경우
					return false;
			}
		} 
	}


	if(row > 0)
	{
		// ex) #include <sys/types.h>
		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0) // tokens[0] 값이 "#include" || "include" || "struct" 인 경우
		{ 
			clear_tokens(tokens); // tokens 내용 지우기
			strcpy(tokens[0], remove_extraspace(str)); // tokens[0] 에 "inclue< ~ >" 값 저장
		}
	}

	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL) // tokens[0] type 이 2 인 경우 || tokens[0] "extern" 포함하지 않은 경우
	{
		for(i = 1; i < TOKEN_CNT; i++) 
		{
			if(strcmp(tokens[i],"") == 0) // tokens[i] 값이  없을 경우
				break;		       

			if(i != TOKEN_CNT - 1) // TOKEN_CNT - 1 가 아닌 경우
				strcat(tokens[0], " "); // tokens[0] 에 공백 붙혀넣기

			strcat(tokens[0], tokens[i]); // tokens[0] 에 모든 토큰 순차적으로 붙혀넣기 
			memset(tokens[i], 0, sizeof(tokens[i])); // 붙혀넣은 tokens 비우기 
		}
	}
	
	//change ( ' char ' )' a  ->  (char)a
	while((p_str = find_typeSpecifier(tokens)) != -1) // ( tokens[i] ) 형태로 되어있고 문법 맞는 경우
	{ 
		if(!reset_tokens(p_str, tokens)) // tokens "( )" 정리해서 tokens 내용 이동
			return false;
	}

	//change sizeof ' ( ' record ' ) '-> sizeof(record)
	while((p_str = find_typeSpecifier2(tokens)) != -1) // 현재 tokens 이 "struct" 이고 다음 tokens 값이 char 인 경우 해당 tokens indext 번호 return
	{  
		if(!reset_tokens(p_str, tokens)) // tokens "( )" 정리해서 tokens 내용 이동
			return false;
	}
	
	return true;
}

node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses) // tree 생성후 root node return 하는 함수
{
	node *cur = root;
	node *new;
	node *saved_operator;
	node *operator;
	int fstart; // make_tree 시작 여부 저장 변수
	int i;

	while(1)	
	{
		if(strcmp(tokens[*idx], "") == 0) // tokens 값 없을 경우
			break;
	
		if(!strcmp(tokens[*idx], ")")) // tokens 값이 ")" 인 경우
			return get_root(cur); // root node 찾기

		else if(!strcmp(tokens[*idx], ","))
			return get_root(cur); // root node 찾기

		else if(!strcmp(tokens[*idx], "(")) // tokens 값이 "(" 인 경우
		{
			// function()
			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0) // idx > 0 && tokens[idx - 1] 값이 operator 아닌 경우 && tokens[idx - 1] 값이 "," 아닌 경우
			{
				fstart = true; 

				while(1)
				{
					*idx += 1;

					if(!strcmp(tokens[*idx], ")")) // tokens[idx] 가 ")" 인 경우
						break; 
					
					new = make_tree(NULL, tokens, idx, parentheses + 1); // 새로운 node 만들기
					
					if(new != NULL) // new 가 NULL 아닌 경우
					{
						if(fstart == true)
						{
							cur->child_head = new; // cur 의 자식 노드로 new 추가
							new->parent = cur; // new 의 부모 노드로 cur 추가
	
							fstart = false;
						}
						else
						{
							cur->next = new; // cur 의 next 노드로 new 추가
							new->prev = cur; // new 의 prev 노드로 cur 추가
						}

						cur = new; // cur 을 new 로 바꾸기
					}

					if(!strcmp(tokens[*idx], ")")) // tokens[idx] 가 ")" 인 경우
						break;
				}
			}
			else // tokens[idx - 1] 값이 operator 인 경우 || tokens[idx - 1] 값이 "," 인 경우
			{
				*idx += 1;
	
				new = make_tree(NULL, tokens, idx, parentheses + 1); // 새로운 node 만들기

				if(cur == NULL) // cur 없을 경우
					cur = new; // 새로 만든 node 

				else if(!strcmp(new->name, cur->name)) // new node 이름 과 cur node 이름이 같을 경우
				{
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||") 
						|| !strcmp(new->name, "&") || !strcmp(new->name, "&&")) // new node 이름이 "|", "||", "&", "&&" 중에 하나인 경우
					{
						cur = get_last_child(cur); // cur 트리의 마지막 node 를 cur 로 바꾸기 

						if(new->child_head != NULL) // new->child_head NULL 아닌 경우
						{
							new = new->child_head; // new 를 new->child_head 로 바꾸기

							new->parent->child_head = NULL; // 원래 new 와 new->child_head 를 자식 node로 보지 않기
							new->parent = NULL;  
							new->prev = cur; // new node 마지막 node 연결하기
							cur->next = new; // 마지막 node cur 뒤에 new node 넣기
						}
					}
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*")) // new node 이름이 "+", "*" 중에 하나인 경우
					{
						i = 0;

						while(1)
						{
							if(!strcmp(tokens[*idx + i], "")) // tokens 이 NULL 인 경우
								break;

							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0) // tokens 값이 operator 인 경우 && tokens 값이 ")" 아닌 경우
								break;

							i++; // i 값 증가
						}
						
						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name)) // tokens 값의 precedence < new node 의 precedence 인 경우 == 연산자 우선순위 확인해서 node 삽입
						{
							cur = get_last_child(cur); // cur 의 마지막 node 구하기
							cur->next = new; // 마지막 node 의 다음에 new node 에 넣기
							new->prev = cur; // new node 마지막 node 연결하기
							cur = new; // cur 을 new 로 바꾸기
						}
						else // new node 의 precdenece < tokens 값의 precedence 인 경우
						{
							cur = get_last_child(cur); // cur 의 마지막 node 구하기

							if(new->child_head != NULL) // new->child_head NULL 아닌 경우
							{
								new = new->child_head; // new 를 new->child_head 로 바꾸기

								new->parent->child_head = NULL; // 원래 new 와 new->child_head 를 자식 node로 보지 않기
								new->parent = NULL;
								new->prev = cur; // new node 마지막 node 연결하기
								cur->next = new; // 마지막 node cur 뒤에 new node 넣기
							}
						}
					}
					else // new node 의 name 위에 나온 경우말고 전부다
					{
						cur = get_last_child(cur); // cur 의 마지막 node 구하기
						cur->next = new; // 마지막 node cur 뒤에 new node 넣기
						new->prev = cur; // new node 마지막 node 연결하기
						cur = new; // cur 을 new 로 바꾸기
					}
				}
	
				else // new node 이름과 cur node 이름이 다른 경우
				{
					cur = get_last_child(cur);

					cur->next = new; // 마지막 node cur 뒤에 new node 넣기
					new->prev = cur; // new node 마지막 node 연결하기
	
					cur = new; // cur 을 new 로 바꾸기
				}
			}
		}
		else if(is_operator(tokens[*idx])) // tokens 값이 operator 일 경우
		{
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&") 
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*")) // tokens 값이 "||", "&&", "|", "&", "+", "*" 연산자들 중에 하나인 경우
			{
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx])) // cur 이 현재 root 이므로 root name 이 연산자인 경우 &&  root name 과 tokens 값이 같은 경우 
					operator = cur; // operator 에 cur 넣기 == operator = cur = root
		
				else
				{
					new = create_node(tokens[*idx], parentheses); // 새로 생성한 node 를 new 에 저장 == new node 는 연산자 node
					operator = get_most_high_precedence_node(cur, new); // 우선 순위 높은 node 를 저장

					if(operator->parent == NULL && operator->prev == NULL) // operator 의 parent, prev node 가 없을 경우
					{
						if(get_precedence(operator->name) < get_precedence(new->name)) // opertor 의 우선순위 < new 의 우선순위 == operator 우선순위가 높은 경우
						{
							cur = insert_node(operator, new); // opertor node 에 new 를 넣고 new node 의 child node 에 operator node 넣고 new node 를 저장
						}

						else if(get_precedence(operator->name) > get_precedence(new->name)) // opertor 의 우선순위 > new 의 우선순위 == new 우선순위가 높은 경우
						{
							if(operator->child_head != NULL) // opertor node 의 child node 가 있을 경우
							{
								operator = get_last_child(operator); // opertor node 이후로 마지막 노드를 opertor 에 저장
								cur = insert_node(operator, new); // opertor node 에 new 를 넣고 new node 의 child node 에 operator node 넣고 new node 를 저장
							}
						}
						else // opertor 의 우선순위 == new 의 우선순위 == 우선순위가 같은 경우
						{
							operator = cur;
	
							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx])) // operator node 가 연산자인 경우 && operator node 와 tokens 의 연산자가 같을 경우
									break;
						
								if(operator->prev != NULL) // operator node 의 prev node 끝까지 이동
									operator = operator->prev;
								else
									break;
							}

							if(strcmp(operator->name, tokens[*idx]) != 0) // operator node 와 tokens 의 연산자가 다를 경우
								operator = operator->parent; // opertor 를 opertor parent 로 바꾸기

							if(operator != NULL) // operator 가 NULL 이 아닌 경우
							{
								if(!strcmp(operator->name, tokens[*idx])) // operator node 와 tokens 의 연산자가 같을 경우
									cur = operator;
							}
						}
					}

					else
						cur = insert_node(operator, new); // opertor node 에 new 를 넣고 new node 의 child node 에 operator node 넣고 new node 를 저장
				}

			}
			else // tokens[*idx] 값이 조건에 제시된 연산자 외에 다른 연산자 인 경우
			{
				new = create_node(tokens[*idx], parentheses); // 새로운 node 를 생성해서 new 에 저장

				if(cur == NULL) // cur 가 NULL 인 경우
					cur = new;

				else // cur 가 NULL 아닌 경우 
				{
					operator = get_most_high_precedence_node(cur, new); // 우선순위가 높은 node 를 저장

					if(operator->parentheses > new->parentheses) // operator 의 
						cur = insert_node(operator, new); // opertor node 에 new 를 넣고 new node 의 child node 에 operator node 넣고 new node 를 저장

					else if(operator->parent == NULL && operator->prev ==  NULL) // operator node 의 parent , prev 둘 다 없는 경우
					{
						if(get_precedence(operator->name) > get_precedence(new->name)) // operator 연산자 우선순위 > new 연산자 우선순위 인 경우
						{
							if(operator->child_head != NULL) // operatot node 의 childe 가 있을 경우
							{
								operator = get_last_child(operator); // operator node 기준으로 마지막 node 구해서 저장
								cur = insert_node(operator, new); // opertor node 에 new 를 넣고 new node 의 child node 에 operator node 넣고 new node 를 저장
							}
						}
					
						else // operator 연산자 우선순위 <= new 연산자 우선순위 인 경우 
							cur = insert_node(operator, new); // opertor node 에 new 를 넣고 new node 의 child node 에 operator node 넣고 new node 를 저장
					}
	
					else
						cur = insert_node(operator, new); // opertor node 에 new 를 넣고 new node 의 child node 에 operator node 넣고 new node 를 저장
				}
			}
		}
		else // tokens 값이 "(" , operator 둘다 아닐 경우
		{
			new = create_node(tokens[*idx], parentheses); // 새로운 node 를 생성해서 new 에 저장

			if(cur == NULL) // cur 가 NULL 인 경우
				cur = new;

			else if(cur->child_head == NULL) // cur node 의 child 가 없을 경우
			{
				cur->child_head = new; // cur node 의 child 에 new 저장
				new->parent = cur; // new node 의 parent 에 cur 저장

				cur = new; 
			}
			else // cur node 가 NULL 아닌 경우 && cur node 의 child node 가 있을 경우
			{
				cur = get_last_child(cur); // cur node 기준으로 마지막 node 구해서 저장

				cur->next = new; // 마지막 node next 에 new 저장
				new->prev = cur; // new node prev 에 cur 저장

				cur = new;
			}
		}

		*idx += 1; // idx 값을 증가후 저장
	}

	return get_root(cur); // cur node 기준으로 root node return
}

node *change_sibling(node *parent) // parent 로 들어온 노드의 자식 노드 next 노드를 parent 의 자식 노드로 바꿔주는 함수
{
	node *tmp;
	
	tmp = parent->child_head;

	parent->child_head = parent->child_head->next;
	parent->child_head->parent = parent;
	parent->child_head->prev = NULL;

	parent->child_head->next = tmp;
	parent->child_head->next->prev = parent->child_head;
	parent->child_head->next->next = NULL;
	parent->child_head->next->parent = NULL;		

	return parent;
}

node *create_node(char *name, int parentheses) // node 생성하는 함수
{
	node *new;

	new = (node *)malloc(sizeof(node)); // node 새로 생성
	new->name = (char *)malloc(sizeof(char) * (strlen(name) + 1)); // node 이름 저장할 변수
	strcpy(new->name, name); // 새로 생성한 node 이름 저장

	new->parentheses = parentheses; // 새로 생성한 node parentheses 저장
	new->parent = NULL;
	new->child_head = NULL;
	new->prev = NULL;
	new->next = NULL;

	return new;
}

int get_precedence(char *op) // operator 값으로 operator 구조체 에서 precedence 얻는 함수
{
	int i;

	for(i = 2; i < OPERATOR_CNT; i++)
	{
		if(!strcmp(operators[i].operator, op))
			return operators[i].precedence;
	}
	return false;
}

int is_operator(char *op) // op 가 존재하는 operator 인지 확인하는 함수
{
	int i;

	for(i = 0; i < OPERATOR_CNT; i++)
	{
		if(operators[i].operator == NULL) // operators 구조체 집합에 operator 가 NULL 인 경우
			break;
		if(!strcmp(operators[i].operator, op)) // operators 구조체 집합에 operator 와 op 가 같은 경우
		{
			return true;
		}
	}

	return false;
}

void print(node *cur) // cur node 의 child node, next node 모두 출력
{
	if(cur->child_head != NULL){ // cur node 의 child node 가 있을 경우
		print(cur->child_head); 
		printf("\n");
	}

	if(cur->next != NULL){  // cur node 의 next node 가 있을 경우
		print(cur->next);
		printf("\t");
	}
	printf("%s", cur->name); // cur name 출력
}

node *get_operator(node *cur) // cur node 의 prev 끝까지 이동후 parent node return 하는 함수
{
	if(cur == NULL) // cur node 가 NULL 인 경우
		return cur;

	if(cur->prev != NULL) // cur node 의 prev 가 있을 경우
		while(cur->prev != NULL)
			cur = cur->prev; // cur node 를 prev 끝까지 이동하기

	return cur->parent; // cur 의 parent node 를 return
}

node *get_root(node *cur) // root node 찾는 함수
{
	if(cur == NULL)
		return cur;

	while(cur->prev != NULL) // cur->prev 없을때까지 이동
		cur = cur->prev; 

	if(cur->parent != NULL) // cur->parent 있을 경우
		cur = get_root(cur->parent); // cur->parent 를 재귀로 parent 찾기

	return cur;
}

node *get_high_precedence_node(node *cur, node *new) // 연산자 우선순위가 작은 즉 우선순위가 높은 node 차는 함수
{// 이럴 경우에 무조건 지금 cur 만 return 되게 해놨는데 이게 맞아??
	if(is_operator(cur->name)) // cur node 의 name 이 operator 일 경우
		if(get_precedence(cur->name) < get_precedence(new->name)) // cur operator 의 우선순위 < new operator 의 우선순위
			return cur; // cur node return

	if(cur->prev != NULL) // cur의 prev node 가 존재할 경우
	{
		while(cur->prev != NULL) // cur 의 이전 node 없을때까지 이동하기
		{
			cur = cur->prev; 
			
			return get_high_precedence_node(cur, new); // 함수 재귀 호출
		}


		if(cur->parent != NULL) // cur 의 parent node 가 있을 경우
			return get_high_precedence_node(cur->parent, new); // cur 의 parent node 와 new 연산자 우선순위 확인
	}

	if(cur->parent == NULL) // cur 의 부모 node 가 없을 경우
		return cur;
}

node *get_most_high_precedence_node(node *cur, node *new) // 우선 순위가 높은 node 를 구하는 함수
{
	node *operator = get_high_precedence_node(cur, new); // cur, new 중에 연산자 우선순위가 높은 node 를 저장하기
	node *saved_operator = operator; // operator 변수를 재저장 변수

	while(1)
	{
		if(saved_operator->parent == NULL) // saved_operator node 의 parent node 가 없을 경우
			break;

		if(saved_operator->prev != NULL) // saved_operator node 의 prev node 가 있을 경우
			operator = get_high_precedence_node(saved_operator->prev, new); // 우선순위가 높은 node 를 저장

		else if(saved_operator->parent != NULL) // saved_operator node 의 parent node 가 있을 경우
			operator = get_high_precedence_node(saved_operator->parent, new); // 우선순위가 높은 node 를 저장

		saved_operator = operator;
	}
	
	return saved_operator;
}

node *insert_node(node *old, node *new) // old node 의 위치에 new node 를 넣고 new node 의 child node 로 old node 넣는 함수
{
	if(old->prev != NULL) // old 의 prev node 가 있을 경우
	{
		new->prev = old->prev; // old 위치에 new node 넣기
		old->prev->next = new;
		old->prev = NULL;
	}

	new->child_head = old; // old node 는 new node 의 child node
	old->parent = new; // new node 는 old node 의 parent node

	return new; 
}

node *get_last_child(node *cur) // 마지막 node 구하는 함수
{
	if(cur->child_head != NULL) // cur->child_head 가 있는 경우
		cur = cur->child_head; // cur 을 cur->child_head 로 바꾸기

	while(cur->next != NULL) // cur->next 가 NULL 아닌 경우
		cur = cur->next; // cur->next 없을때까지 이동하기

	return cur;
}

int get_sibling_cnt(node *cur) // cur node 의 sibling node 개수 구하는 함수
{
	int i = 0;

	while(cur->prev != NULL) // cur node 의 prev 있을 경우
		cur = cur->prev; // prev 없을때 까지 이동

	while(cur->next != NULL) // cur node 의 next 있을 경우 
	{
		cur = cur->next; // next 없을때 까지 이동
		i++; 
	}

	return i;
}

void free_node(node *cur) // cur 을 기준으로 트리를 모두 제거하는 함수
{
	if(cur->child_head != NULL) // cur node 의 child 가 있을 경우
		free_node(cur->child_head); // 재귀적으로 node 지우기

	if(cur->next != NULL) // cur node 의 next 가 있을 경우
		free_node(cur->next); // 재귀적으로 node 지우기

	if(cur != NULL){ // cur 이 NULL 아닐경우
		cur->prev = NULL;
		cur->next = NULL;
		cur->parent = NULL;
		cur->child_head = NULL; // cur node 와 연결된 node 끊기
		free(cur); // cur node 동적할당 free
	}
}


int is_character(char c) // 인자로 들어온 char 가 숫자, 소문자, 대문자인지 확인하는 함수
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_typeStatement(char *str) // 인자로 받은 str 시작하는 값에 따라 return 하는 함수
{ 
	char *start; // str 저장 변수
	char str2[BUFLEN] = {0}; // str 공백 제거 저장 변수
	char tmp[BUFLEN] = {0}; 
	char tmp2[BUFLEN] = {0}; // gcc 여부 확인 저장 변수
	int i;	 
	
	start = str; // start = str 저장
	strncpy(str2,str,strlen(str)); // str2 = str 저장
	remove_space(str2); // string 에서 공백 문자 제거 

	while(start[0] == ' ') // start 에서 좌측 공백 제거
		start += 1;

	if(strstr(str2, "gcc") != NULL) // "gcc" 가 존재하는 경우
	{
		strncpy(tmp2, start, strlen("gcc")); // tmp2 에 "gcc" 저장
		if(strcmp(tmp2,"gcc") != 0) // tmp2 가 "gcc" 가 맞는 경우
			return 0;
		else
			return 2;
	}
	
	for(i = 0; i < DATATYPE_SIZE; i++) // datatype 개수 만큼 반복문 실행
	{
		if(strstr(str2,datatype[i]) != NULL) // str2 에 datatype 포함했을 경우
		{	
			strncpy(tmp, str2, strlen(datatype[i])); // tmp 에 str2 에서 datatype 크기 만큼 저장
			strncpy(tmp2, start, strlen(datatype[i])); // tmp2 에 start 에서 datatype 크기 만큼 저장
			
			if(strcmp(tmp, datatype[i]) == 0) // tmp 가 datatype 으로 시작하는 경우
				if(strcmp(tmp, tmp2) != 0) // tmp, tmp2 다른 경우
					return 0;  
				else // tmp, tmp2 같은 경우
					return 2;
		}

	}
	return 1;

}

int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]) // "( tokens[i] )" 형태로 되어있고 이후 문법 맞는지 확인하는 함수
{
	int i, j;

	for(i = 0; i < TOKEN_CNT; i++) // TOKEN_CNT 만큼 반복문 실행
	{
		for(j = 0; j < DATATYPE_SIZE; j++) // DATATYPE_SIZE 만큼 이중 반복문 실행
		{
			if(strstr(tokens[i], datatype[j]) != NULL && 0 < i) // tokens[i] 에 datatype[j] string 존재하지 하는 경우 && i > 0
			{
				if(!strcmp(tokens[i - 1], "(") && !strcmp(tokens[i + 1], ")") // tokens[i - 1] 값이 "(" , tokens[i + 1] 값 ")" 인 경우 == tokens[i] 가 ( ) 로 둘러싸여져있는 경우
						&& (tokens[i + 2][0] == '&' || tokens[i + 2][0] == '*' // tokens[i + 2][0] 값이 "&", "*", "(", ")", "-", "+", char 이 중에 하나인 경우
							|| tokens[i + 2][0] == ')' || tokens[i + 2][0] == '(' 
							|| tokens[i + 2][0] == '-' || tokens[i + 2][0] == '+' 
							|| is_character(tokens[i + 2][0])))  
					return i; // ( ) 안에 있는 tokens[i] index return
			}
		}
	}
	return -1;
}

int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]) // 현재 tokens 값이 "struct" 이고 다음 tokens 이 char 인지 확인하는 함수
{
    int i, j;

    for(i = 0; i < TOKEN_CNT; i++)
    {
        for(j = 0; j < DATATYPE_SIZE; j++)
        {
            if(!strcmp(tokens[i], "struct") && (i+1) <= TOKEN_CNT && is_character(tokens[i + 1][strlen(tokens[i + 1]) - 1])) // tokens[i] 값이 "struct" && (i+1) <= TOKEN_CNT && tokens[i + 1] 마지막 값이 char 인 경우
                    return i;
        }
    }
    return -1;
}

int all_star(char *str) // 문자열 전체가 "*" 인지 확인하는 함수
{
	int i;
	int length= strlen(str);
	
 	if(length == 0)	
		return 0;
	
	for(i = 0; i < length; i++) 
		if(str[i] != '*') // 문자가 "*" 아닌 경우 
			return 0;
	return 1;

}

int all_character(char *str) // char 가 숫자, 소문자, 대문자일 경우 return 1 함수
{
	int i;

	for(i = 0; i < strlen(str); i++)
		if(is_character(str[i])) // 인자로 들어온 char 가 숫자, 소문자, 대문자일 경우
			return 1;
	return 0;
	
}

int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]) // tokens "( )" 정리해서 tokens 들 이동시키고 정리하는 함수
{
	int i;
	int j = start - 1;
	int lcount = 0, rcount = 0; // lcount : "(" 개수, rcount : ")" 개수 저장 변수
	int sub_lcount = 0, sub_rcount = 0; // sub_lcount : "(" 개수, sub_rcount : ")" 개수 저장 변수

	if(start > -1) // start : ( ) 안에 있는 tokens index > -1 일 경우 
	{
		if(!strcmp(tokens[start], "struct")) // tokens[start] 값이 "struct" 인 경우
		{
			strcat(tokens[start], " "); // tokens[start] 에 공백 붙혀넣기
			strcat(tokens[start], tokens[start + 1]); // tokens[start] 뒤에 tokens 붙혀 넣기 == tokens[start] 에 ")" 붙혀넣기 == tokens[start] = "struct )"

			for(i = start + 1; i < TOKEN_CNT - 1; i++)
			{
				strcpy(tokens[i], tokens[i + 1]); // tokens[i + 1] 값을 tokens[i] 복사 == tokens 모두 한칸 앞으로 이동 
				memset(tokens[i + 1], 0, sizeof(tokens[0])); // tokens[i + 1] 비우기
			}
		}

		else if(!strcmp(tokens[start], "unsigned") && strcmp(tokens[start + 1], ")") != 0) // tokens[start] 값이 "unsigned" && tokens[start + 1] 값이 ")" 아닌 경우
		{ // 여기 들어올때 start + 1 은 ")" 아닌가?
			strcat(tokens[start], " "); // tokens[start] 공백 붙혀 넣기
			strcat(tokens[start], tokens[start + 1]); // tokens[start] 에 tokens[start + 1] 붙혀넣기 
			strcat(tokens[start], tokens[start + 2]); // tokens[start] 에 tokens[start + 2] 붙혀넣기 
// start 에 start + 2 까지 붙혔는데 반복문을 start + 1 부터 한다고? start + 1 에 start + 2 넣으면 현재 start 에 start + 2 까지 있는데 또 한번 start + 2 가??
			for(i = start + 1; i < TOKEN_CNT - 1; i++)
			{
				strcpy(tokens[i], tokens[i + 1]); // tokens[i + 1] 값을 tokens[i] 복사 == tokens 모두 한칸 앞으로 이동 
				memset(tokens[i + 1], 0, sizeof(tokens[0])); // tokens[i + 1] 비우기
			}
		}

		j = start + 1; 
		while(!strcmp(tokens[j], ")")) // tokens[j] ")" 인 경우 == 연속으로 ")" 나오는지 확인 ?
		{
			rcount ++; // ")" 개수 변수 증가
			if(j == TOKEN_CNT) // j == TOKEN_CNT 인 경우 반복문 중단
					break;
			j++; 
		}
	
		j = start - 1; 
		while(!strcmp(tokens[j], "(")) // tokens[j] "(" 인 경우 == 연속으로 "(" 나오는지 확인 ?
		{
			lcount ++; // "(" 개수 변수 증가
			if(j == 0) // j == 0 인 경우 반복문 중단
					break;
			j--;
		}

		// j == 0 또는 "(" 들어있는 tokens 의 이전 tokens

		if((j != 0 && is_character(tokens[j][strlen(tokens[j])-1])) || j == 0) // {(j != 0) && tokens[j] 마지막 값이 char 인 경우} || j == 0 인 경우
			lcount = rcount; 

		if(lcount != rcount ) // "(" 개수, ")" 개수 다를 경우
			return false;

		if( (start - lcount) > 0 && !strcmp(tokens[start - lcount - 1], "sizeof")) // start - lcount > 0 && tokens[start - lcount - 1] 값이 "sizeof" 인 경우 == "sizeof()" 형태 인 경우
		{
			return true; 
		}
		
		else if((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start + 1], ")")) // (tokens[start] 값이 "unsigned" || "struct" 인 경우) && tokens[start + 1] ")" 인 경우
		{ // tokens[start] 에 뒤에 뭐 붙혔는데 여기 이게 가능?
			strcat(tokens[start - lcount], tokens[start]); // 
			strcat(tokens[start - lcount], tokens[start + 1]);
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]);
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount; i++) {
				strcpy(tokens[i], tokens[i + lcount + rcount]);
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0]));
			}


		}
 		else
		{
			if(tokens[start + 2][0] == '(') // tokens[start + 2][0] 값이 "(" 인 경우
			{
				j = start + 2; 
				while(!strcmp(tokens[j], "(")) // tokens[j] 값이 "(" 인 경우
				{
					sub_lcount++; // "(" 개수 변수 증가
					j++;
				} 	
				if(!strcmp(tokens[j + 1],")")) // tokens[j + 1] 값이 ")" 인 경우
				{
					j = j + 1;
					while(!strcmp(tokens[j], ")")) // tokens[j] 값이 ")" 인 경우
					{
						sub_rcount++; // ")" 개수 변수 증가
						j++;
					}
				}
				else 
					return false;

				if(sub_lcount != sub_rcount) // "(" , ")" 개수 다를 경우
					return false;
				
				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]); // tokens[start + 2] 에 복사 == "(" 처음 들어간 부분에 다음 괄호들 무시하고 저장
				for(int i = start + 3; i<TOKEN_CNT; i++)
					memset(tokens[i], 0, sizeof(tokens[0])); // tokens[i] 비우기

			}

			// ex) (((start))) a b-> (start)a b
 			strcat(tokens[start - lcount], tokens[start]); // "(" 연속 괄호 무시하고 "(" 하나 남기고 tokens 에 붙혀넣기 
			strcat(tokens[start - lcount], tokens[start + 1]); // ")" 하나 붙혀넣기
			strcat(tokens[start - lcount], tokens[start + rcount + 1]); // ")" 연속 괄호 무시하고 다음 tokens 붙혀넣기
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount -1; i++) {
				strcpy(tokens[i], tokens[i + lcount + rcount +1]); // tokens 복사해서 이동시키기 == tokens[star - lcount] 로 다음 tokens 까지 옮겨놓은 상태에서 뒤에 tokens 들을 tokens[star - lcount + 1] 뒤로 tokens 들 순서대로 저장하기
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0])); // tokens[i + lcount + rcount + 1] 비우기
			}
		}
	}
	return true;
}

void clear_tokens(char tokens[TOKEN_CNT][MINLEN]) // tokens 내용 지우는 함수
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i]));
}

char *rtrim(char *_str) // 오른쪽 공백 제거하는 함수
{
	char tmp[BUFLEN];
	char *end;

	strcpy(tmp, _str);
	end = tmp + strlen(tmp) - 1;
	while(end != _str && isspace(*end)) // tmp 마지막 부분에 공백인 경우 반복문 실행
		--end;

	*(end + 1) = '\0'; // 반복문 실행후 tmp 공백 제거
	_str = tmp;
	return _str;
}

char *ltrim(char *_str) // 왼쪽 공백 제거하는 함수
{
	char *start = _str;

	while(*start != '\0' && isspace(*start)) // start 가 "\0" 가 아니고 start 에 공백인 경우 반복문 실행
		++start;
	_str = start;
	return _str;
}

char* remove_extraspace(char *str) // "#include<stdio.h>" 형태의 값을 공백 없이 return 하는 함수
{
	int i;
	char *str2 = (char*)malloc(sizeof(char) * BUFLEN);
	char *start, *end; // start : str 저장 변수 , end : str 에 "<" 값 있는 포인터 저장 변수 
	char temp[BUFLEN] = "";
	int position;
	char *blank = " ";

	if(strstr(str,"include<")!=NULL) // str 에 "include<" 포함 되어 있는 경우
	{
		start = str;
		end = strpbrk(str, "<"); // "<" 위치 포인터 return
		position = end - start; // "include" 길이 저장 
	
		strncat(temp, str, position); // temp 에 str 에서 "include" 붙혀넣기
		strncat(temp, blank, 1); // temp 에 공백 넣기
		strncat(temp, str + position, strlen(str) - position + 1); // "include " 뒤에 값 모두 넣기

		str = temp; // "#include <stdio.h>" 형태 만들기
	}
	
	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] ==' ') // str[i] 공백인 경우
		{
			if(i == 0 && str[0] ==' ') // i == 0 && str[0] 공백 인 경우 == 왼쪽 공백 인 경우
				while(str[i + 1] == ' ') // 공백 안 나올때까지 건너뛰기 
					i++;	
			else
			{ 
				if(i > 0 && str[i - 1] != ' ') // i > 0 && str[i - 1] 공백이 아닌 경우 == 문자열 중간에 공백인 경우
					str2[strlen(str2)] = str[i]; 
				while(str[i + 1] == ' ') // 공백 안 나올때까지 건너뛰기
					i++;
			} 
		}
		else
			str2[strlen(str2)] = str[i]; // 공백이 아닌 경우 str2[] 에 저장
	}

	return str2;
}



void remove_space(char *str) // 공백 제거 함수
{
	char* i = str;
	char* j = str;
	
	while(*j != 0)
	{
		*i = *j++;
		if(*i != ' ')
			i++;
	}
	*i = 0;
}

int check_brackets(char *str) // "(" , ")" 개수 같은지 여부 확인 함수
{
	char *start = str; // 학생이 작성한 답안 저장 변수
	int lcount = 0, rcount = 0; // lcount : "(" 개수, rcount : ")" 개수 저장 변수
	
	while(1){
		// strpbrk() 함수로 start 문자열에서 "(" , ")" 찾았을 경우
		if((start = strpbrk(start, "()")) != NULL){ // start 에 "(" , ")" 발견한 포인터 return
			if(*(start) == '(') // 해당 포인터에 "(" 인 경우
				lcount++;
			else // 해당 포인터에 ")" 인 경우
				rcount++;

			start += 1; 		
		}
		else
			break;
	}

	if(lcount != rcount) // "(" , ")" 개수가 다를 경우
		return 0;
	else 
		return 1;
}

int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]) // token 개수 구하는 함수
{
	int i;
	
	for(i = 0; i < TOKEN_CNT; i++)
		if(!strcmp(tokens[i], "")) // tokens 값이 NULL 인 경우
			break;

	return i;
}
