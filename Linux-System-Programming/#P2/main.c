#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "ssu_score.h"

#define SECOND_TO_MICRO 1000000

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t); // begin_t, end_t 시간 차이를 출력하는 함수

int main(int argc, char *argv[])
{
	struct timeval begin_t, end_t; // 시작 시간, 끝나는 시간 저장 변수
	gettimeofday(&begin_t, NULL); // 시간을 측정해서 begin_t 에 저장

	ssu_score(argc, argv); 

	gettimeofday(&end_t, NULL); // 시간을 측정해서 end_t 에 저장
	ssu_runtime(&begin_t, &end_t); // 시간 출력 

	exit(0);
}

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t) // begin_t, end_t 시간 차이를 출력하는 함수
{
	end_t->tv_sec -= begin_t->tv_sec; // end_t - begin_t 초 단위 시간 저장

	if(end_t->tv_usec < begin_t->tv_usec){ // end_t->tv_usec < begin_t->tv_usec 인 경우
		end_t->tv_sec--; // end_t 에서 1초 뺀 값 저장
		end_t->tv_usec += SECOND_TO_MICRO; // 1초 뺀 값 마이크로초로 더한 값 저장
	}

	end_t->tv_usec -= begin_t->tv_usec; //end_t - begin_t 마이크로초 시간 저장
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t->tv_sec, end_t->tv_usec);
}
