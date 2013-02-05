#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void user_error() // fix entire
{
	printf("\nException: User error encountered\n\n");
	exit(1);
}

void archive_error() // fix entire
{
	printf("\nException: Archive error encountered\n\n");
	exit(1);
}

void file_error() // fix entire
{
	printf("\nException: File error encountered\n\n");
	exit(1);
}

void program_error() // fix entire
{
	printf("\nException: Program error encountered\n\n");
	exit(1);
}

int is_archive(char *afile) // fix entire
{
	printf("Archive being verified...\n");
	return 0;
}

void concise_table(char *afile) // fix entire
{
	printf("Printing concise table...\n");
	exit(0);
}

void verbose_table(char *afile) // fix entire
{
	printf("Printing verbose table...\n");
	exit(0);
}

char ** fetch_files_cd(int *file_count) // fix entire
{
	char **files = (char **) malloc(sizeof(char *) * 1);
	printf("Fetching files in cd...\n");
	files[0] = "All regular files in current directory appended";
	*file_count = 1;
	return files;
}

void append(char *afile, char **files, int file_count) // fix entire
{
	int c;
	for(c = 0; c < file_count; c++)
		printf("%s appended to %s\n%d files appended\n", files[c], afile, c + 1);
	exit(0);
}

void extract(char *afile, char **files, int file_count) // fix entire
{
	int c;
	for(c = 0; c < file_count; c++)
		printf("%s extracted from %s\n%d files extracted\n", files[c], afile, c + 1);
	exit(0);
}

void delete(char *afile, char **files, int file_count) // fix entire
{
	int c;
	for(c = 0; c < file_count; c++)
		printf("%s deleted from %s\n%d files deleted\n", files[c], afile, c + 1);
	exit(0);
}

char ** get_files(int file_count, char **argv)
{
	char **files = (char **) malloc(sizeof(char *) * file_count);
	int i;
	for(i = 0; i < file_count; i++)
		files[i] = argv[i + 3];
	return files;
}

int main(int argc, char **argv)
{
	char *key = argv[1];
	char *afile = argv[2];
	if(argc <= 2)
		user_error();
	if(strlen(key) != 2)
		user_error();
	if(key[0] != '-')
		user_error();
	if(strchr("qxdAtv", key[1]) == NULL) // If key is NOT contained in "qxdAtv"
		user_error();
	if(strchr("Atv", key[1]) != NULL) { // If is key is contained in "Atv"
		if(argc != 3)
			user_error();
		if(key[1] != 'A') { // If key is NOT 'A' -> implies 't' or 'v'
			if(!is_archive(afile)) // Verifies archive exists
				archive_error();
			if(key[1] == 't') // If key is 't'
				concise_table(afile);
			if(key[1] == 'v') // If key is 'v'
				verbose_table(afile);
		} else { // If key is 'A'
			int file_count = 0;
			char **files = fetch_files_cd(&file_count);
			if (file_count == 0) // If file not found
				file_error();
			append(afile, files, file_count);
		}
	} else { // If key is contained in "qxd"
		if(argc < 4)
			user_error();
		int file_count = argc - 3; // Number of files = argc - 3
		char **files = get_files(file_count, argv);
		if(key[1] == 'q') // If key is 'q'
			append(afile, files, file_count);
		if(!is_archive(afile)) // Verifies archive exists
			archive_error();
		if(key[1] == 'x') // If key is 'x'
			extract(afile, files, file_count);
		if(key[1] == 'd') // If key is 'd'
			delete(afile, files, file_count);
	}
	program_error();
	return 0;
}
