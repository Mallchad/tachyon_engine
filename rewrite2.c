#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define FILE_ARRAY_SIZE 100
#define FILE_ITEM_SIZE 40

char** lsdir(const char *path){
	char** files = calloc(FILE_ARRAY_SIZE, FILE_ITEM_SIZE);

	struct dirent *file;
	DIR *dir = opendir(path);

	if(dir == 0){
		fprintf(stderr, "lsdir(): could not open %s\n", path);
		exit(1);
	}
	int count = 0;
	int filename_length = 0;
	while((file = readdir(dir)) != 0){
		if(strncmp(file->d_name, "..", 3) && strncmp(file->d_name, ".", 2) != 0){
			filename_length = strlen(file->d_name);
			files[count] = calloc(filename_length, sizeof(char));
			strncpy(files[count], file->d_name, filename_length);
			++count;
		}
	}

	closedir(dir);
	return files;
}

int main(){
	const char path[] = "/home/mallchad/userdata/notes/";
	const char* test = "/home/mallchad/userdata/notes/";
	char **files = lsdir(path);

	for(int i = 1; i<FILE_ARRAY_SIZE; ++i){
		const char* item = files[i];
		if(item == 0) continue;

		char _path[100] = "INITSTRING";
		memset(&_path, 0, sizeof(_path));
		strncpy(_path, path, (sizeof(_path)-1));

		char file[500] = "INITSTRING";
		memset(&file, 0, sizeof(file));
		/* int file_name_size = sizeof(_path)+sizeof(item)-1; */
		/* char* file_name = calloc(file_name_size, sizeof(char)); */

		/* strncpy(file_name, _path, strlen(_path)); */
		strncpy(file, strncat(_path,
							  item, (sizeof(_path)-strlen(_path)-1)), (sizeof(file)-1));

		FILE *fp = fopen(file,"r");

		if(fp == 0){
			fprintf(stderr, "main(): could not open %s\n", file);
			continue;
		}

		fclose(fp);
	}

	free(files);
}
