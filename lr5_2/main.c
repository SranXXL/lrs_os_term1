#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define WIDTH 2560
#define HEIGHT 1600
#define N 100
#define OFFSET 17

int main(int argc, char *argv[]) {
	FILE *in, *out;
	//FILE *outX, *outY;
	char buf[N];
	int num[3];
	int sum;
	int i, j, k, l;
	int maskx[3][3] = {{-1,0,1}, {-2,0,2}, {-1,0,1}};
	int masky[3][3] = {{-1,-2,-1}, {0,0,0}, {1,2,1}};
	int pixels[3][3];
	int pixelX[3][3];
	int pixelY[3][3];
	int sumX, sumY;
	long int offset;
	
	in = fopen(argv[1], "r");
	out = fopen("bw", "w");
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
	printf("Bw created! %ld\n", clock() / CLOCKS_PER_SEC);
	
	in = fopen("bw", "r");
	out = fopen("Sobel_operator.pgm", "w");
	//outX = fopen("x.pgm", "w");
	//outY = fopen("y.pgm", "w");
	fprintf(out, "P2\n%d %d\n300\n", WIDTH-2, HEIGHT-2);
	//fprintf(outX, "P2\n%d %d\n255\n", WIDTH-2, HEIGHT-2);
	//fprintf(outY, "P2\n%d %d\n255\n", WIDTH-2, HEIGHT-2);

	fseek(in, OFFSET + 4*(WIDTH+1), SEEK_SET);
	for (i=1; i<HEIGHT-1; ++i) {
		for (j=1; j<WIDTH-1; ++j) {
			sumX = 0;
			sumY = 0;
			offset = ftell(in);
			for (k=-1; k<2; ++k) {
				fseek(in, -4 + k*4*WIDTH, SEEK_CUR);
				for (l=0; l<3; ++l) {
					fscanf(in, "%s", buf);
					pixels[k+1][l] = atoi(buf);
					pixelX[k+1][l] = pixels[k+1][l] * maskx[k+1][l];
					pixelY[k+1][l] = pixels[k+1][l] * masky[k+1][l];
					sumX += pixelX[k+1][l];
					sumY += pixelY[k+1][l];
				}
				fseek(in, offset, SEEK_SET);
			}
			sum = (int)round(sqrt(sumX*sumX + sumY*sumY));
			fprintf(out, "%d ", sum);
			//fprintf(outX, "%d ", abs(sumX));
			//fprintf(outY, "%d ", abs(sumY));
			fseek(in, 4, SEEK_CUR);
		}
		fseek(in, 8, SEEK_CUR);
		fprintf(out, "\n");
		//fprintf(outX, "\n");
		//fprintf(outY, "\n");
	}
	fclose(out);
	//fclose(outX);
	//fclose(outY);
	fclose(in);
	printf("Finished! %ld\n", clock() / CLOCKS_PER_SEC);
}

