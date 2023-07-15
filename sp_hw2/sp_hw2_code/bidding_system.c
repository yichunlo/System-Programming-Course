#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

int host_num, player_num, num = 1, tmp[16];

struct p{
	int score;
	int rank;
};
typedef struct p pp;
pp p[16];

char buff[4096][64];
void dfs(int now, int ptr) {
	if (ptr == 8) {
		for(int i = 0; i < 8; i++){
			if(i != 7) {
				char t[16];
				sprintf(t, "%d ", tmp[i]);
				strcat(buff[num], t);
			}
			else {
				char t[16];
				sprintf(t, "%d\n", tmp[i]);
				strcat(buff[num], t);
			}
		}
		num++;
		return;
	}
	if (now == player_num + 1) return;
	tmp[ptr] = now;
	dfs(now + 1, ptr + 1);
	dfs(now + 1, ptr);
	return;
}

char read_buf[512];
char end_message[64] = "-1 -1 -1 -1 -1 -1 -1 -1\n";
int key[16], fd_write[16];

int find_host(int k){
	for(int i = 1; i <= host_num; i++)
		if(k == key[i]) return i;
	return -1;
}

int main(int argc, char *argv[]){
	char *fifo_path[16];
	host_num = atoi(argv[1]), player_num = atoi(argv[2]);
	// make fifo
	fifo_path[0]  = "./Host.FIFO";
	fifo_path[1]  = "./Host1.FIFO" ;
	fifo_path[2]  = "./Host2.FIFO" ;
	fifo_path[3]  = "./Host3.FIFO" ;
	fifo_path[4]  = "./Host4.FIFO" ;
	fifo_path[5]  = "./Host5.FIFO" ;
	fifo_path[6]  = "./Host6.FIFO" ;
	fifo_path[7]  = "./Host7.FIFO" ;
	fifo_path[8]  = "./Host8.FIFO" ;
	fifo_path[9]  = "./Host9.FIFO" ;
	fifo_path[10] = "./Host10.FIFO";
	umask(0);
	unlink(fifo_path[0]);
	mkfifo(fifo_path[0], 0666);
	for(int i = 1; i <= host_num; i++){
		unlink(fifo_path[i]);
		mkfifo(fifo_path[i], 0666);
	}

	pid_t pid[16];
	// message handling
	dfs(1, 0);
	for(int i = num; i < num + 32; i++)
		strcpy(buff[i], end_message);
	num--;

	for(int i = 1; i <= player_num; i++){
		p[i].score = 0;
		p[i].rank = 1;
	}
	// init root_host
	for(int i = 1; i <= host_num; i++){
		key[i] = i * 100 + 7;
		pid[i] = fork();
		if ( pid[i] < 0 ) {
			perror("fork");
			exit(127);
		} else if( pid[i] == 0 ){
			char buf1[32], buf2[32];
			sprintf(buf1, "%d", i);
			sprintf(buf2, "%d", key[i]);
			execl("./host","./host", buf1, buf2,"0", NULL );
		} else { 
			// parent
			fd_write[i] = open(fifo_path[i], O_WRONLY);
			if(fd_write[i] < 0) perror("open failed");
		}
	}

	FILE *fp; 
	int fd_read;
	if( (fd_read = open("Host.FIFO", O_RDONLY)) < 0)
		perror("open failed");
	if( (fp = fdopen(fd_read, "r") ) == NULL) 
		perror("fdopen failed");
	int id, rank, key_now;
	// initial assignment
	for(int i = 1; i <= host_num; i++){
		write(fd_write[i], buff[i], strlen(buff[i]));
		fsync(fd_write[i]);
	}
	int counter = 0, buf_idx = host_num + 1;

	while(counter < num){
		fscanf(fp, "%d", &key_now);
		int host_now = find_host(key_now);
		for(int i = 0; i < 8; i++){
			fscanf(fp, "%d %d", &id, &rank);
			p[id].score += (8 - rank);
		}
		write(fd_write[host_now], buff[buf_idx], strlen(buff[buf_idx]));
		fsync(fd_write[host_now]);
		counter++; buf_idx++;
	}
	// ranking
	for(int i = 1; i <= player_num; i++){
		for(int j = 1; j <= player_num; j++){
			if(p[i].score < p[j].score)
				p[i].rank++;
	    }
		fprintf(stdout, "%d %d\n", i, p[i].rank);
	}
	for(int i = 0; i <= 10; i++)
		unlink(fifo_path[i]);
	fclose(fp);
	for(int i = 0; i < host_num; i++)
		wait(NULL);
	return 0;
}
