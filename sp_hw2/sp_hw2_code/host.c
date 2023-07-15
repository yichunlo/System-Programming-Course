#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>

char *path[16];
int pfd1_p_to_c[2], pfd1_c_to_p[2], pfd2_p_to_c[2], pfd2_c_to_p[2];
FILE *fp_left_w, *fp_right_w, *fp_left_r, *fp_right_r;

void close_all(){
    close(pfd1_c_to_p[1]);
    close(pfd1_p_to_c[1]);
    close(pfd2_p_to_c[1]);
    close(pfd2_c_to_p[1]);
    close(pfd1_c_to_p[0]);
    close(pfd1_p_to_c[0]);
    close(pfd2_p_to_c[0]);
    close(pfd2_c_to_p[0]);
}

void initialize(char *id ,  char *key , int depth , char *p1 , char *p2){
    if(pipe(pfd1_p_to_c) < 0 || pipe(pfd1_c_to_p) < 0 || pipe(pfd2_p_to_c) < 0 || pipe(pfd2_c_to_p) < 0)
	perror("pipe");
    pid_t pid[2];
    pid[0] = fork();
    if(pid[0] < 0) perror("fork");
    if(pid[0] == 0){
	dup2(pfd1_p_to_c[0], 0);
	dup2(pfd1_c_to_p[1], 1);
	close_all();
	if(depth == 0) execlp("./host", "./host", id, key, "1", NULL);
	if(depth == 1) execlp("./host", "./host", id, key, "2", NULL);
	if(depth == 2) execlp("./player", "./player", p1, NULL);
    }
    pid[1] = fork();
    if(pid[1] < 0) perror("fork");
    if(pid[1] == 0){
	dup2(pfd2_p_to_c[0], 0);
	dup2(pfd2_c_to_p[1], 1);
	close_all();
	if(depth == 0) execlp("./host", "./host", id, key, "1", NULL);
	if(depth == 1) execlp("./host", "./host", id, key, "2", NULL);
	if(depth == 2) execlp("./player", "./player", p2, NULL);
    }


    if( (fp_left_w  = fdopen(pfd1_p_to_c[1], "w")) == NULL)
	perror("fdopen");
    if( (fp_right_w = fdopen(pfd2_p_to_c[1], "w")) == NULL)
	perror("fdopen");
    if( (fp_left_r  = fdopen(pfd1_c_to_p[0], "r")) == NULL)
	perror("fdopen");
    if( (fp_right_r = fdopen(pfd2_c_to_p[0], "r")) == NULL)
	perror("fdopen");
}

int main(int argc, char *argv[]){
    assert(argc == 4);
    path[0]  = "./Host.FIFO"; 
    path[1]  = "./Host1.FIFO" ; path[2]  = "./Host2.FIFO" ;
    path[3]  = "./Host3.FIFO" ; path[4]  = "./Host4.FIFO" ;
    path[5]  = "./Host5.FIFO" ; path[6]  = "./Host6.FIFO" ;
    path[7]  = "./Host7.FIFO" ; path[8]  = "./Host8.FIFO" ;
    path[9]  = "./Host9.FIFO" ; path[10] = "./Host10.FIFO";

    int id = atoi(argv[1]);
    int key = atoi(argv[2]);
    int depth = atoi(argv[3]);
    int score[16];
    pid_t pid[2];

    if(depth == 0){
	FILE *read_fifo_fp, *write_fifo_fp;
	if((read_fifo_fp = fopen(path[id], "r+")) == NULL)
	    perror("open failed");
	if((write_fifo_fp = fopen(path[0], "w+")) == NULL)
	    perror("open failed");
	initialize(argv[1], argv[2], depth, NULL , NULL);

	while(1){
	    int play[16];
	    for(int i = 0; i < 8; i++){ // read from bidding  : 8 ids
		fflush(read_fifo_fp);
		fscanf(read_fifo_fp, "%d", &play[i]);
	    }
	    char message[64] = "", left[32] = "", right[32] = "";
	    sprintf(message, "%d %d %d %d %d %d %d %d\n", play[0], play[1], play[2], play[3], play[4], play[5], play[6], play[7]);
	    sprintf(left, "%d %d %d %d\n", play[0], play[1], play[2], play[3]);
	    sprintf(right, "%d %d %d %d\n", play[4], play[5], play[6], play[7]);

	    fprintf(fp_left_w,  "%s", left); // print to child : 4 id
	    fflush(fp_left_w);
	    fprintf(fp_right_w, "%s", right); // print to child : 4 ids
	    fflush(fp_right_w);
	    if(play[0] == -1) break; // break if end

	    int p1, m1, p2, m2, win;
	    for(int i = 0; i < 10; i++){
		fflush(fp_left_r);
		fscanf(fp_left_r,  "%d%d", &p1, &m1); //read from child : id money
		fflush(fp_right_r);
		fscanf(fp_right_r, "%d%d", &p2, &m2); //read from child : id money
		//fprintf(stderr , "read from child : %d %d %d %d\n" , p1 , m1 , p2 , m2);
		win = (m1 > m2)? p1 : p2; //compare
		char win_id[16];
		sprintf(win_id, "%d\n", win);
		// write down
		if(i != 9){
		    fprintf(fp_left_w,  "%s", win_id); // print to child winid
		    fflush(fp_left_w);
		    fprintf(fp_right_w, "%s", win_id); // print to child winid
		    fflush(fp_right_w);
		}
	    }
	    char buf[128];
	    sprintf(buf, "%d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n" , key, play[0], 2, play[1], 2, play[2], 2, play[3], 2, play[4], 2, play[5], 2, play[6], 2, play[7], 1);
	    fprintf(write_fifo_fp, "%s", buf); // write to bidding
	    fflush(write_fifo_fp);
	}
	wait(NULL);
	wait(NULL);
	close_all();
	exit(0);
    }
    else if(depth == 1){
	initialize(argv[1], argv[2], depth, NULL, NULL);
	while(1){
	    int play[16];
	    fflush(stdin);
	    for(int i = 0; i < 4; i++) // read from root
		fscanf(stdin, "%d", &play[i]);
	    char message[64] = "", left[32] = "", right[32] = "";
	    sprintf(message, "%d %d %d %d\n", play[0], play[1], play[2], play[3]);
	    sprintf(left,  "%d %d\n", play[0], play[1]);
	    sprintf(right, "%d %d\n", play[2], play[3]);
	    fprintf(fp_left_w,  "%s", left); // print to leaf : 2 ids
	    fflush(fp_left_w);
	    fprintf(fp_right_w, "%s", right); // print to leaf : 2 ids
	    fflush(fp_right_w);
	    if(play[0] == -1) break; // break if end

	    int p1, m1, p2, m2, win_p, win_m;
	    for(int i = 0; i < 10; i++){
		fflush(fp_left_r);
		fscanf(fp_left_r,  "%d%d", &p1, &m1); //read from leaf : id money
		fflush(fp_right_r);
		fscanf(fp_right_r, "%d%d", &p2, &m2); //read from leaf : id money
		char winner[16], win_id[16];
		if(m1 > m2){
		    win_p = p1;
		    win_m = m1;
		}
		else{
		    win_p = p2;
		    win_m = m2;
		}
		sprintf(winner, "%d %d\n", win_p, win_m);
		fprintf(stdout, "%s", winner);
		fflush(stdout);
		// read from above
		int id_from_root;
		if(i != 9){
		    fflush(stdin);
		    fscanf(stdin, "%d", &id_from_root);
		    sprintf(win_id, "%d\n", id_from_root);
		    fprintf(fp_left_w,  "%s", win_id); // print to leaf winid
		    fflush(fp_left_w);
		    fprintf(fp_right_w, "%s", win_id); // print to leaf winid
		    fflush(fp_right_w);
		}
	    }
	}
	wait(NULL);
	wait(NULL);
	close_all();
	exit(0);
    }

    else{ // depth = 2
	while(1){
	    int play[16];
	    char pid1[16], pid2[16];
	    fflush(stdin);
	    fscanf(stdin, "%d", &play[0]);
	    fflush(stdin);
	    fscanf(stdin, "%d", &play[1]);
	    if(play[0] == -1) break;
	    sprintf(pid1, "%d", play[0]);
	    sprintf(pid2, "%d", play[1]);
	    initialize(NULL, NULL, 2, pid1, pid2);
	    int m1 , p1 , m2 , p2 , win_p , win_m;
	    for(int i = 0; i < 10; i++){
		fflush(fp_left_r);
		fscanf(fp_left_r,  "%d%d", &p1, &m1); //read from player : id money
		fflush(fp_right_r);
		fscanf(fp_right_r, "%d%d", &p2, &m2); //read from player : id money
		char winner[16], win_id[16];
		if(m1 > m2){
		    win_p = p1;
		    win_m = m1;
		}
		else{
		    win_p = p2;
		    win_m = m2;
		}
		sprintf(winner, "%d %d\n", win_p, win_m);
		fprintf(stdout, "%s", winner);
		fflush(stdout);
		// read from above
		// write down
		if(i != 9){
		    int id_from_root;
		    fflush(stdin);
		    fscanf(stdin, "%d", &id_from_root);
		    sprintf(win_id, "%d\n", id_from_root);

		    fprintf(fp_left_w,  "%s", win_id); // print to player winid
		    fflush(fp_left_w);
		    fprintf(fp_right_w, "%s", win_id); // print to player winid
		    fflush(fp_right_w);
		}
	    }
	    wait(NULL); wait(NULL);
	    close_all();
	}
	exit(0);
    }
    return 0;
}
