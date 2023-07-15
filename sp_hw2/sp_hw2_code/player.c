#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>


int main(int argc, char *argv[]){
	int id, money;
	int cnt = 0;
	char read_buf[512], write_buf[512];
	id = atoi(argv[1]); 
	money = id * 100;
	sprintf(write_buf, "%d %d\n", id, money);
	write(1, write_buf, strlen(write_buf));
	while(cnt < 9){
		read(0, read_buf, sizeof(read_buf));
		write(1, write_buf, strlen(write_buf));
		cnt++;
	}
	return 0;
}
