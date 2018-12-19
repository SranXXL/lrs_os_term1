#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <pthread.h>
#define WIDTH 2560
#define HEIGHT 1600
#define N 100
#define OFFSET 17

int **picture;
int **result;
void *sobel_operator(void *arg);

int main(int argc, char *argv[]) {
	FILE *in, *out;
	char buf[N];
	int num[3];
	int sum;
	int i, j, k;
	int **arg;
	int num_of_threads, lines_thread;
	pthread_t *mas_thread;
	void *thread_result;
	struct timespec start, stop;
	
	
	in = fopen(argv[1], "r");
	out = fopen("bw", "w");
	if (!in || !out) {
		printf("Error! Unable to open the file!\n");
		exit(EXIT_FAILURE);
	}
	fprintf(out, "P2\n%d %d\n255\n", WIDTH, HEIGHT);
	fseek(in, OFFSET, SEEK_SET);
	i = 0;
	k = 0;
	while (fscanf(in, "%s", buf) != EOF) {
		num[k] = atoi(buf);
		++k;
		if (k == 3) {
			k = 0;
			sum = (num[0] + num[1] + num[2]) / 3;
			++i;
			if (i == 20) {
				fprintf(out, "%3d\n", sum);
				i = 0;
			} else {
				fprintf(out, "%3d ", sum);
			}
		}
	}
	fclose(out);
	fclose(in);
	
	picture = calloc(HEIGHT, sizeof(int *));
	if (!picture) {
		printf("Error! Unable to malloc!\n");
		exit(EXIT_FAILURE);
	}
	result = calloc(HEIGHT, sizeof(int *));
	if (!result) {
		printf("Error! Unable to malloc!\n");
		exit(EXIT_FAILURE);
	}
	for (i=0; i<HEIGHT; ++i) {
		picture[i] = calloc(WIDTH, sizeof(int));
		result[i] = calloc(WIDTH, sizeof(int));
		if (!picture[i] || !result[i]) {
			printf("Error! Unable to malloc!\n");
			exit(EXIT_FAILURE);
		}
	}
	
	in = fopen("bw", "r");
	if (!in) {
		printf("Error! Unable to open the file!\n");
		exit(EXIT_FAILURE);
	}
	fseek(in, OFFSET, SEEK_SET);
	for (i=0; i<HEIGHT; ++i) {
		for (j=0; j<WIDTH; ++j) {
			fscanf(in, "%s", buf);
			picture[i][j] = atoi(buf);
		}
	}
	fclose(in);

	out = fopen("Sobel_operator.pgm", "w");
	if (!out) {
		printf("Error! Unable to open the file!\n");
		exit(EXIT_FAILURE);
	}
	fprintf(out, "P2\n%d %d\n300\n", WIDTH-2, HEIGHT-2);
	printf("Enter the number of threads: ");
	scanf("%d", &num_of_threads);
	if (num_of_threads <= 0) {
		printf("Error! Invalid number of threads!\n");
		exit( EXIT_FAILURE );
	}
	if (clock_gettime(CLOCK_REALTIME, &start) == -1 ) {
		perror( "clock gettime" );
		exit( EXIT_FAILURE );
	}
	lines_thread = HEIGHT / num_of_threads;
	mas_thread = calloc(num_of_threads, sizeof(pthread_t));
	arg = calloc(num_of_threads, sizeof(int *));
	if (!mas_thread || !arg) {
		printf("Error! Unable to malloc!\n");
		exit(EXIT_FAILURE);
	}
	
	for (i=0; i<num_of_threads; ++i) {
		arg[i] = calloc(2, sizeof(int));
		if (!arg[i]) {
			printf("Error! Unable to malloc!\n");
			exit(EXIT_FAILURE);
		}
		arg[i][0] = i*lines_thread;
		if (i == num_of_threads - 1)
			arg[i][1] = HEIGHT - 2;
		else 
			arg[i][1] = (i+1)*lines_thread;
		k = pthread_create(&mas_thread[i], NULL, sobel_operator, (void *)arg[i]);
		if (k != 0) {
			printf("Error! Thread creation failed!\n");
			exit(EXIT_FAILURE);
		}
	}
	//printf("Waiting for threads to finish...\n");
	for (i=0; i<num_of_threads; ++i) {
		k = pthread_join(mas_thread[i], &thread_result);
		if (k != 0) {
			perror("Thread join-failed");
			exit(EXIT_FAILURE);
		}
		//printf("Thread-joined, it returned %s\n", (char *)thread_result);
	}
	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
		perror( "clock gettime" );
		exit( EXIT_FAILURE );
	}
	for (i=1; i<HEIGHT-1; ++i) {
		for (j=1; j<WIDTH-1; ++j) {
			fprintf(out, "%d ", result[i][j]);
		}
		fprintf(out, "\n");
	}
	fclose(out);
	free(picture);
	free(result);
	free(mas_thread);
	free(arg);
	printf( "%lu sec %lu nsec\n", (unsigned long)(stop.tv_sec - start.tv_sec), (unsigned long)(stop.tv_nsec - start.tv_nsec));
	return( EXIT_SUCCESS );
}

void *sobel_operator(void *arg) {
	int i, j, k, l;
	int maskx[3][3] = {{-1,0,1}, {-2,0,2}, {-1,0,1}};
	int masky[3][3] = {{-1,-2,-1}, {0,0,0}, {1,2,1}};
	int pixelX[3][3], pixelY[3][3];
	int sumX, sumY, sum;
	int *argv = (int *)arg;
	
	//printf("Start: %d, finish: %d\n", argv[0]+1, argv[1]+1);
	for (i=argv[0]+1; i<argv[1]+1; ++i) {
		for (j=1; j<WIDTH-1; ++j) {
			sumX = 0;
			sumY = 0;
			for (k=0; k<3; ++k) {
				for (l=0; l<3; ++l) {
					pixelX[k][l] = picture[i+k-1][j+l-1] * maskx[k][l];
					pixelY[k][l] = picture[i+k-1][j+l-1] * masky[k][l];
					sumX += pixelX[k][l];
					sumY += pixelY[k][l];
				}
			}
			result[i][j] = (int)round(sqrt(sumX*sumX + sumY*sumY));
		}
	}
	pthread_exit("Thank you for the CPU time!");
}

