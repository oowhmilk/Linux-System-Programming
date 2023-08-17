#ifndef BLANK_H_
#define BLANK_H_

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef BUFLEN
	#define BUFLEN 1024
#endif

#define OPERATOR_CNT 24
#define DATATYPE_SIZE 35
#define MINLEN 64
#define TOKEN_CNT 50

typedef struct node{
	int parentheses;
	char *name;
	struct node *parent;
	struct node *child_head;
	struct node *prev;
	struct node *next;
}node;

typedef struct operator_precedence{
	char *operator; // 연산자
	int precedence; // 연산자 우선순위
}operator_precedence;

void compare_tree(node *root1,  node *root2, int *result); // root1 , root2 node 의 tree 를 비교하는 함수
node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses); // tree 생성후 root node return 하는 함수
node *change_sibling(node *parent); // parent 로 들어온 노드의 자식 노드 next 노드를 parent 의 자식 노드로 바꿔주는 함수
node *create_node(char *name, int parentheses); // node 생성하는 함수
int get_precedence(char *op); // operator 값으로 operator 구조체 에서 precedence 얻는 함수
int is_operator(char *op); // op 가 존재하는 operator 인지 확인하는 함수
void print(node *cur); // cur node 의 child node, next node 모두 출력
node *get_operator(node *cur); // cur node 의 prev 끝까지 이동후 parent node return 하는 함수
node *get_root(node *cur); // root node 찾는 함수
node *get_high_precedence_node(node *cur, node *new); // 연산자 우선순위가 작은 즉 우선순위가 높은 node 차는 함수
node *get_most_high_precedence_node(node *cur, node *new); // 우선 순위가 높은 node 를 구하는 함수
node *insert_node(node *old, node *new); // old node 의 위치에 new node 를 넣고 new node 의 child node 로 old node 넣는 함수
node *get_last_child(node *cur); // 마지막 node 구하는 함수
void free_node(node *cur); // cur 을 기준으로 트리를 모두 제거하는 함수
int get_sibling_cnt(node *cur); // cur node 의 sibling node 개수 구하는 함수

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN]);  // 인자로 들어오는 str 을 문자열 분리해서 tokens 만드는 함수
int is_typeStatement(char *str); // 인자로 받은 str 시작하는 값에 따라 return 하는 함수
int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]); // "( tokens[i] )" 형태로 되어있고 이후 문법 맞는지 확인하는 함수
int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]); // 현재 tokens 값이 "struct" 이고 다음 tokens 이 char 인지 확인하는 함수
int is_character(char c); // 인자로 들어온 char 가 숫자, 소문자, 대문자인지 확인하는 함수
int all_star(char *str); // 문자열 전체가 "*" 인지 확인하는 함수
int all_character(char *str); // char 가 숫자, 소문자, 대문자일 경우 return 1 함수
int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]); // tokens "( )" 정리해서 tokens 들 이동시키고 정리하는 함수
void clear_tokens(char tokens[TOKEN_CNT][MINLEN]); // tokens 내용 지우는 함수
int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]); // token 개수 구하는 함수
char *rtrim(char *_str); // 오른쪽 공백 제거하는 함수
char *ltrim(char *_str); // 왼쪽 공백 제거하는 함수
void remove_space(char *str); // 공백 제거 함수
int check_brackets(char *str); // "(" , ")" 개수 같은지 여부 확인 함수
char* remove_extraspace(char *str); // "#include<stdio.h>" 형태의 값을 공백 없이 return 하는 함수

#endif
