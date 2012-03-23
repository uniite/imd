# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/mman.h>
# include <fcntl.h>

int main(int argc, char *argv[]) {
	FILE *file;
	struct stat *file_info;
	
	if (argc > 1) {
        file = fopen(argv[1], "rb");      // Open up argument one of command line
		//stat(argv[1], file_info);
    } else {
        file = fopen("test.mp3", "rb");   // or test.mp3 if no file name supplied
		//stat("test.mp3", file_info);
    }
	fstat(file, file_info);
	printf("File size: %d\n", file_info->st_size);
	fclose(file);
	return 0;
}