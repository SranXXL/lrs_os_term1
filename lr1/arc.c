#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#define N 100
#define M 50

int b4_zip(char *dir, FILE *tmp_header, FILE *tmp_arc) {
	DIR *dp;
	FILE *temp;
	struct dirent *d;
	struct stat statbuf;
	char buff[N];
	int size;

	if ((dp = opendir(dir)) == NULL) {
		printf("Error! Unable to open directory: %s!\n", dir);
		return (1);
	}
	chdir(dir);
	while((d = readdir(dp)) != NULL) {
		lstat(d->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			if (strcmp(".", d->d_name) == 0 || strcmp("..", d->d_name) == 0)
				continue;
			fprintf(tmp_header, "%s | ", d->d_name);
			b4_zip(d->d_name, tmp_header, tmp_arc);	
		} else {
			temp = fopen(d->d_name,"r+");
			if (!temp) {
				printf("Error! Unable to open the file: %s!\n", d->d_name);
				return (1);
			}
			while (fgets(buff, N, temp) != NULL)
				if (fputs(buff, tmp_arc) == EOF) {
					printf("Error! Unable to create an archive!\n");
					fclose(temp);
					return (1);
				}
			size = ftell(temp);
			fprintf(tmp_header, "%s %d ", d->d_name, size);
			fclose(temp);
		}
	}
	chdir("..");
	fprintf(tmp_header, "|| ");
	closedir(dp);
	return (0);
}

int zip(char *path) {
	FILE *tmp_header, *tmp_arc, *out;
	char fout[N];
	char buf[N];

	tmp_header = tmpfile();
	if (!tmp_header) { 
		printf("Error! Unable to create a temp file for header!\n");
		return (1);
	}
	tmp_arc = tmpfile();
	if (!tmp_arc) { 
		printf("Error! Unable to create a temp file for archive!\n");
		fclose(tmp_header);
		return (1);
	}
	if (b4_zip(path, tmp_header, tmp_arc)) {
		printf("Error! Something goes wrong!");
		return (1);
	}
	fseek(tmp_header, -3, SEEK_CUR);
	fprintf(tmp_header, "0_0\n");
	strcpy(fout, path);
	strcat(fout, ".df");
	out = fopen(fout, "w");
	if (!out) {
		printf("Error! Unable to create a file!\n");
		return (1);
	}
	rewind(tmp_header);
	while (fgets(buf, N, tmp_header) != NULL)
		if (fputs(buf, out) == EOF) {
			printf("Error! Unable to rewrite data from tmp_header!\n");
			fclose(out);
			return (1);
		}
	rewind(tmp_arc);
	while (fgets(buf, N, tmp_arc) != NULL)
		if (fputs(buf, out) == EOF) {
			printf("Error! Unable to rewrite data from tmp_arc!\n");
			fclose(out);
			return (1);
		}
	fclose(out);
	return (0);
}

int unzip(char *dir, char *file) {
	FILE *in, *out;
	char buf[N];
	char name[M], size[M];
	int isize;
	int depth = 0;
	int start, header = 0;

	if (chdir(dir) == -1) {
		printf("Error! Unable to find the directory: %s!\n", dir);
		return (1);
	}	
	in = fopen(file, "r");
	if (!in) {
		printf("Error! Unable to open the archive: %s!\n", file);
		return (1);	
	}
	while (fscanf(in, "%s", buf) != EOF)
		if (!strcmp(buf, "0_0"))
			break;	
	if (feof(in)) {
		printf("Error! Archive is damaged!\n");
		fclose(in);
		return (1);
	}
	start = ftell(in) + 1;
	strncpy(name, strrchr(file, '/') + 1, strrchr(file, '.') - strrchr(file, '/') - 1);
	if (chdir(name) != -1) {
		printf("Error! %s/%s directory is already exist!\n", dir, name);
		printf("Please rename archive or existing directory.\n");
		fclose(in);
		return (1);
	}
	if (mkdir(name, S_IRUSR|S_IWUSR|S_IXUSR)) {
		printf("Error! Unable to create directory!\n");
		fclose(in);
		return (1);
	}
	chdir(name);
	++depth;
	fseek(in, header, SEEK_SET);
	while (fscanf(in, "%s", buf) != EOF) {
		if (!strcmp(buf,"0_0"))
			break;
		if (!strcmp(buf, "||")) {
			if (depth <= 1) {
				printf("Error! Archive is damaged! Trying to leave archive directory!\n");
				fclose(in);
				return (1);
			}
			--depth;
			chdir("..");
		} else {
			strcpy(name, buf);
			fscanf(in,"%s",buf);
			if (!strcmp(buf, "|")) {
				if (mkdir(name, S_IRUSR|S_IWUSR|S_IXUSR)) {
					printf("Error! Unable to create directory!\n");
					fclose(in);
					return (1);
				}
				chdir(name);
				++depth;	
			} else {
				out = fopen(name, "w");
				if (!out) {
					printf("Error! Unable to create the file: %s!\n", name);
					fclose(in);
					return (1);	
				}
				strcpy(size, buf);
				isize = atoi(size);
				if (!isize) {
					if (buf[0] != '0') {
						printf("Error! Archive is damaged! Expected '|' or size of file!\n");
						fclose(in);
						fclose(out);
						return (1);
					}
				}
				header = ftell(in);
				fseek(in, start, SEEK_SET);
				while (isize > 0) {
					buf[0] = fgetc(in);
					if (buf[0] == EOF) {
						printf("Error! Archive is damaged! Lack of data!\n");
						fclose(in);
						fclose(out);
						return (1);
					}
					if (fputc(buf[0], out) == EOF) {
						printf("Error! Unable to write in file: %s!\n", name);
						fclose(in);
						fclose(out);
						return (1);
					}
					isize--;
				}
				start = ftell(in);
				fseek(in, header, SEEK_SET);
				fclose(out);
			}
		}
	}
	fseek(in, 0, SEEK_END);
	if (ftell(in) != start) {
		printf("Error! Archive is damaged! Excess of data!\n");
		fclose(in);
		return (1);
	} else {
		fclose(in);
		return (0);
	}
}

int main(int argc, char *argv[]) {	
	if ((argc < 2) || (argc > 3)) {
		printf("Error! Too few or too many arguments!\n");
		exit(1);
	}
	if (argc == 2) {
		if (zip(argv[1])) {
			printf("Fatal error in zip function!\n");
			exit (1);
		} else {
			printf("Success! You made an archive!\n");
		}
	} else {
		if (unzip(argv[1], argv[2])) {
			printf("Fatal error in unzip function!\n");
			exit (1);
		} else {
			printf("Success! Check %s!\n", argv[1]);
		}
	}
	exit (0);
}
