#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

u_int8_t x[60000][784], xt[784][60000], test_x[10000][784];
double w[784][10], y_hat[60000][10], wg[784][10], res[10000][10], lr = 0.0001;
int y[60000][10], num, cut[256][2];
pthread_t tid[256];

void init(){
	for(int i = 0; i < 60000; ++i){
		for(int j = 0; j < 10; ++j){
			y_hat[i][j] = 0.0;
			if(i < 10000) res[i][j] = 0.0;
		}
	}
	for(int i = 0; i < 784; ++i)
		for(int j = 0; j < 10; ++j)
			wg[i][j] = 0.0;
}

void *multiply(void *arg){
	int *b = (int *)arg;
	for(int i = b[0]; i <= b[1]; ++i){
		for(int j = 0; j < 784; ++j){
			double r = x[i][j]  * 1.0;
			for(int k = 0; k < 10; ++k)
				y_hat[i][k] += r * w[j][k];
		}
	}
	pthread_exit(NULL);
}

void *mul(void *arg){
	int *c = (int *)arg;
	for(int i = c[0]; i <= c[1]; ++i){
		for(int j = 0; j < 60000; ++j){
			double r = xt[i][j] * 1.0;
			for(int k = 0; k < 10; ++k)
				wg[i][k] += r * y_hat[j][k];
		}
	}
	pthread_exit(NULL);
}

void *mul_(void *arg){
	int *c = (int *)arg;
	for(int i = c[0]; i <= c[1]; ++i){
		for(int j = 0; j < 784; ++j){
			double r = test_x[i][j] * 1.0;
			for(int k = 0; k < 10; ++k)
				res[i][k] += r * w[j][k];
		}
	}
	pthread_exit(NULL);
}

void minus(){
	for(int i = 0; i < 60000; ++i)
		for(int j = 0; j < 10; ++j)
			y_hat[i][j] -= y[i][j];
	return;
}

void assign(){
	int siz = 60000 / num;
	for(int i = 0; i < num; ++i){
		cut[i][0] = i * siz;
		if(i != num-1) cut[i][1] = (i+1) * siz -1;
		else cut[i][1] = 59999;
		pthread_create(&tid[i], NULL, multiply, cut[i]);
	}
	for(int i = 0; i < num; ++i)
		pthread_join(tid[i], NULL);
}

void gen(){
	int siz = 784 / num;
	for(int i = 0; i < num; ++i){
		cut[i][0] = i * siz;
		if(i == num-1) cut[i][1] = 783;
		else cut[i][1] = (i+1) * siz - 1;
		pthread_create(&tid[i], NULL, mul, cut[i]);
	}
	for(int i = 0; i < num; ++i)
		pthread_join(tid[i], NULL);
}

void get_res(){
	int siz = 10000 / num;
	for(int i = 0; i < num; ++i){
		cut[i][0] = i * siz;
		if(i == num-1) cut[i][1] = 9999;
		else cut[i][1] = (i+1) * siz - 1;
		pthread_create(&tid[i], NULL, mul_, cut[i]);
	}
	for(int i = 0; i < num; ++i)
		pthread_join(tid[i], NULL);
}

void *t(void *arg){
	int *b = (int *)arg;
	for(int i = b[0]; i <= b[1]; ++i)
		for(int j = 0; j < 784; ++j)
			xt[j][i] = x[i][j];
	pthread_exit(NULL);
}

void trans(){
	int siz = 60000 / num;
	for(int i = 0; i < num; ++i){
		cut[i][0] = i * siz;
		if(i != num-1) cut[i][1] = (i+1) * siz - 1;
		else cut[i][1] = 59999;
		pthread_create(&tid[i], NULL, t, cut[i]);
	}
	for(int i = 0; i < num; ++i)
		pthread_join(tid[i], NULL);
}

int main(int argc, char *argv[]){
	int x_train, y_train, x_test;
	x_train = open(argv[1], O_RDWR);
	y_train = open(argv[2], O_RDWR);
	x_test  = open(argv[3], O_RDWR);
	num = atoi(argv[4]);
	for(int i = 0; i < 60000; ++i)
		read(x_train, &x[i], sizeof(x[i]));
	trans();	
	for(int i = 0; i < 60000; ++i){
		unsigned char c;
		read(y_train, &c, sizeof(c));
		y[i][c] = 1;
	}
	for(int i = 0; i < 10000; ++i)
		read(x_test, &test_x[i], sizeof(test_x[i]));

	int iteration;
	double rate;
	if(num == 1) iteration = 45, rate = 0.9925;
	else iteration = 170, lr = 0.000001, rate = 0.99;

	while(iteration--){
		init();
		assign();
		for(int r = 0; r < 60000; ++r){
			double max_ = -1000000;
			for(int i = 0; i < 10; ++i){
				if(max_ < y_hat[r][i]) 
					max_ = y_hat[r][i];
			}
			for(int i = 0; i < 10; ++i)
				y_hat[r][i] -= max_;
			double divide = 0;
			for(int i = 0; i < 10; ++i)
				divide += exp(y_hat[r][i]);
			for(int i = 0; i < 10; ++i)
				y_hat[r][i] = exp(y_hat[r][i]) / divide;
		}
		minus();
		gen();
		for(int i = 0; i < 784; ++i)
			for(int j = 0; j < 10; ++j)
				w[i][j] -= lr * wg[i][j];
		lr *= 0.98;
	}
	get_res();
	freopen("result.csv", "w", stdout);
	puts("id,label");
	for(int i = 0; i < 10000; ++i){
		int idx = 0;
		double mx = -10000000;
		for(int j = 0; j < 10; ++j){
			if(mx < res[i][j]){
				mx = res[i][j];
				idx = j;
			}
		}
		fprintf(stdout, "%d,%d\n", i, idx);
	}
	close(x_train);
	close(y_train);
	close(x_test);
	return 0;
}
