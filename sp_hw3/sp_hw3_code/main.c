#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <errno.h>
#include "scheduler.h"
#include <string.h>
#include <signal.h>

#define SIGUSR3 SIGWINCH

int pfd[2];
pid_t pid;

int main(int argc, char *argv[]){
	char pc[8], qc[8];
	pipe(pfd);
	int p, q;
	scanf("%d%d", &p, &q);
	sprintf(pc, "%d", p);
	sprintf(qc, "%d", q);
	int sig_num;
	scanf("%d", &sig_num);
	int sig_[16];
	for(int i = 0; i < sig_num; i++)
		scanf("%d", sig_+i);
	sigset_t newmask, oldmask;
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1); sigaddset(&newmask, SIGUSR2); sigaddset(&newmask, SIGUSR3);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);

	pid = fork();
	if(pid == 0){
		close(pfd[0]);
		dup2(pfd[1], 1);
		execlp("./hw3", "./hw3", pc, qc, "3", "0", NULL);
	}
	else{
		close(pfd[1]);
		for(int i = 0; i < sig_num; i++){
			sleep(5);
			if(sig_[i] == 1) 
				kill(pid, SIGUSR1);
			if(sig_[i] == 2) 
				kill(pid, SIGUSR2);
			if(sig_[i] == 3) 
				kill(pid, SIGUSR3);
			char queue[128] = "";
			read(pfd[0], &queue, sizeof(queue));
			if(sig_[i] == 3){
				int l = strlen(queue);
				for(int i = 0; i < l; i++){
					if(i == l-1) printf("%d\n", queue[i]-'0');
					else printf("%d ", queue[i]-'0');
				}
			}
		}
		char message[128] = "";
		read(pfd[0], &message, sizeof(message));
		fprintf(stdout, "%s\n", message);
		close(pfd[0]);
		wait(NULL);
	}
	return 0;
}
