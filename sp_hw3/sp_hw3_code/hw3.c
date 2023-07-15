#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <errno.h>
#include "scheduler.h"
#include <string.h>
#include <signal.h>

#define SIGUSR3 SIGWINCH

int p, q, small_num, mutex, task, idx;
jmp_buf main_, SCHEDULER;
FCB func[5];
FCB_ptr Current, Head;
char arr[10000];
int bitmap_[8], now[5]; // record whether function in queue or not
sigset_t all, unblock1, unblock2, unblock3;

void initialize(){
	sigemptyset(&all);
	sigemptyset(&unblock1);
	sigemptyset(&unblock2);
	sigemptyset(&unblock3);
	sigaddset(&all, SIGUSR1);
	sigaddset(&all, SIGUSR2);
	sigaddset(&all, SIGUSR3);
	sigaddset(&unblock1, SIGUSR2);
	sigaddset(&unblock1, SIGUSR3);
	sigaddset(&unblock2, SIGUSR1);
	sigaddset(&unblock2, SIGUSR3);
	sigaddset(&unblock3, SIGUSR1);
	sigaddset(&unblock3, SIGUSR2);
	sigprocmask(SIG_BLOCK, &all, NULL);
	for(int i = 1; i <= 4; i++){
		func[i].Name = i;
		now[i] = 1;
	}
	func[1].Next = &func[2]; func[1].Previous = &func[4];
	func[2].Next = &func[3]; func[2].Previous = &func[1];
	func[3].Next = &func[4]; func[3].Previous = &func[2];
	func[4].Next = &func[1]; func[4].Previous = &func[3];
	Head = &func[1];
	Current = Head->Previous;
	return;
}

void handler(int signo){
	if(signo == SIGUSR1 || signo == SIGUSR2){
		// send  message to the parent
		char c = 's';
		write(1, &c, sizeof(c));
		sigprocmask(SIG_SETMASK, &all, NULL);
		longjmp(SCHEDULER, 1);
	}
	else{
		Current = Current->Previous;
		// print func_id in queue to stdout
		char queue[8] = "";
		for(int i = 1; i <= 4; i++){
			if(bitmap_[i] == 1){
				sprintf(queue+strlen(queue), "%d", i);
			}
		}
		write(1, &queue, sizeof(queue));
		sigprocmask(SIG_SETMASK, &all, NULL);
	}
	longjmp(SCHEDULER, 1);
	return;
}

void un_block(int signo){
	if(signo == 1)
		sigprocmask(SIG_SETMASK, &unblock1, NULL);
	if(signo == 2)
		sigprocmask(SIG_SETMASK, &unblock2, NULL);
	if(signo == 3)
		sigprocmask(SIG_SETMASK, &unblock3, NULL);
	return;
}

void funct_1(int name){
	//perror("funct_1");
	if(setjmp(func[1].Environment) == 0)
		funct_5(2);
	else{
		if(mutex == 0 || mutex == 1){
			mutex = 1;
			bitmap_[1] = 0;
			int cnt = 0;
			//perror("1 append");
			for(int i = now[1]; i <= p; i++){
				for(int j = 1; j <= q; j++){
					sleep(1);
					arr[idx++] = '1';
				}
				cnt++;
				//fprintf(stderr, "cnt1 = %d\n", cnt);
				if(task == 1) continue;
				if(task == 3){
					sigset_t check;
					if(sigpending(&check) != 0)
						perror("1:sigpending");
					if(sigismember(&check, SIGUSR1)){
						now[1] = i+1;
						//fprintf(stderr, "1:accept SIGUSR1\n");
						un_block(1);
					}
					else if(sigismember(&check, SIGUSR2)){
						mutex = 0;
						now[1] = i+1;
						//fprintf(stderr, "1:accept SIGUSR2\n");
						un_block(2);
					}
					else if(sigismember(&check, SIGUSR3)){
						now[1] = i+1;
						//fprintf(stderr, "1:accept SIGUSR3\n");
						un_block(3);
					}
					else{
						//fprintf(stderr, "1:Get nothing!\n");
						continue;
					}
				}
				else if(cnt == small_num){
					mutex = 0;
					now[1] += small_num;
					if(now[1] > p) longjmp(SCHEDULER, -2);
					else longjmp(SCHEDULER, 1);
				}
			}
			mutex = 0;
			longjmp(SCHEDULER, -2);
		}
		else {
			bitmap_[1] = 1;
			longjmp(SCHEDULER, 1);
		}
	}
	return;
}

void funct_2(int name){
	//perror("funct_2");
	if(setjmp(func[2].Environment) == 0)
		funct_5(3);
	else{
		if(mutex == 0 || mutex == 2){
			//perror("2 append");
			mutex = 2;
			bitmap_[2] = 0;
			int cnt = 0;
			for(int i = now[2]; i <= p; i++){
				for(int j = 1; j <= q; j++){
					sleep(1);
					arr[idx++] = '2';
				}
				cnt++;
				if(task == 1) continue;
				if(task == 3){
					sigset_t check;
					if(sigpending(&check) != 0)
						perror("2:sigpending");
					if(sigismember(&check, SIGUSR1)){
						now[2] = i+1;
						//fprintf(stderr, "2:accept SIGUSR1\n");
						un_block(1);
					}
					else if(sigismember(&check, SIGUSR2)){
						//fprintf(stderr, "2:accept SIGUSR2\n");
						mutex = 0;
						now[2] = i+1;
						un_block(2);
					}
					else if(sigismember(&check, SIGUSR3)){
						now[2] = i+1;
						//fprintf(stderr, "2:accept SIGUSR3\n");
						un_block(3);
					}
					else{
						//fprintf(stderr, "2:Get nothing!\n");
						continue;
					}
				}
				else if(cnt == small_num){
					mutex = 0;
					now[2] += small_num;
					if(now[2] > p) longjmp(SCHEDULER, -2);
					else longjmp(SCHEDULER, 1);
				}
			}
			mutex = 0;
			longjmp(SCHEDULER, -2);
		}
		else{
			//perror("2:locked!");
			bitmap_[2] = 1;
			longjmp(SCHEDULER, 1);
		}
	}
	return;
}

void funct_3(int name){
	//perror("funct_3");
	if(setjmp(func[3].Environment) == 0)
		funct_5(4);
	else{
		if(mutex == 0 || mutex == 3){
			//perror("3 append");
			bitmap_[3] = 0;
			mutex = 3;
			int cnt = 0;
			for(int i = now[3]; i <= p; i++){
				for(int j = 1; j <= q; j++){
					sleep(1);
					arr[idx++] = '3';
				}
				cnt++;
				if(task == 1) continue;
				if(task == 3){
					sigset_t check;
					if(sigpending(&check) != 0)
						perror("3:sigpending");
					if(sigismember(&check, SIGUSR1)){
						now[3] = i+1;
						//fprintf(stderr, "3:accept SIGUSR1\n");
						un_block(1);
					}
					else if(sigismember(&check, SIGUSR2)){
						now[3] = i+1;
						//fprintf(stderr, "3:accept SIGUSR2\n");
						mutex = 0;
						un_block(2);
					}
					else if(sigismember(&check, SIGUSR3)){
						now[3] = i+1;
						//fprintf(stderr, "3:accept SIGUSR3\n");
						un_block(3);
					}
					else{
						//fprintf(stderr, "3:Get nothing!\n");
						continue;
					}
				}
				else if(cnt == small_num){
					mutex = 0;
					now[3] += small_num;
					if(now[3] > p) longjmp(SCHEDULER, -2);
					else longjmp(SCHEDULER, 1);
				}
			}
			mutex = 0;
			longjmp(SCHEDULER, -2);
		}
		else{
			bitmap_[3] = 1;
			longjmp(SCHEDULER, 1);
		}
	}
	return;
}

void funct_4(int name){
	//perror("funct_4");
	if(setjmp(func[4].Environment) == 0)
		longjmp(main_, 1);
	else{
		if(mutex == 0 || mutex == 4){
			mutex = 4;
			bitmap_[4] = 0;
			int cnt = 0;
			//perror("4 append");
			for(int i = now[4]; i <= p; i++){
				for(int j = 1; j <= q; j++){
					sleep(1);
					arr[idx++] = '4';
				}
				cnt++;
				if(task == 1) continue;
				if(task == 3){
					sigset_t check;
					if(sigpending(&check) != 0)
						perror("4:sigpending");
					if(sigismember(&check, SIGUSR1)){
						now[4] = i+1;
						//fprintf(stderr, "4:accept SIGUSR1\n");
						un_block(1);
					}
					else if(sigismember(&check, SIGUSR2)){
						now[4] = i+1;
						//fprintf(stderr, "4:accept SIGUSR2\n");
						mutex = 0;
						un_block(2);
					}
					else if(sigismember(&check, SIGUSR3)){
						now[4] = i+1;
						//fprintf(stderr, "4:accept SIGUSR3\n");
						un_block(3);
					}
					else{
						//fprintf(stderr, "4:Get nothing!\n");
						continue;
					}
				}
				else if(cnt == small_num){
					mutex = 0;
					now[4] += small_num;
					if(now[4] > p) longjmp(SCHEDULER, -2);
					else longjmp(SCHEDULER, 1);
				}
			}
			mutex = 0;
			longjmp(SCHEDULER, -2);
		}
		else{
			bitmap_[4] = 1;
			longjmp(SCHEDULER, 1);
		}
	}
	return;
}

void funct_5(int name){ // dummy
	int a[10000];
	if(name == 1) funct_1(1);
	else if(name == 2) funct_2(2);
	else if(name == 3) funct_3(3);
	else funct_4(4);
	return;
}


int main(int argc, char *argv[]){
	p = atoi(argv[1]);
	q = atoi(argv[2]);
	task = atoi(argv[3]);
	small_num = atoi(argv[4]);
	initialize();
	if(task == 3){
		struct sigaction act;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		act.sa_handler = handler;
		sigaction(SIGUSR1, &act, NULL);
		sigaction(SIGUSR2, &act, NULL);
		sigaction(SIGUSR3, &act, NULL);
	}
	if(setjmp(main_) == 0)
		funct_5(1);
	else
		Scheduler();	
	return 0;
}
