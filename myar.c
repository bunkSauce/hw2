#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ar.h>
#include <fcntl.h>

#define NAME_LIMIT 16
#define DATE_LIMIT 11
#define UID_LIMIT 5
#define GID_LIMIT 5
#define MODE_LIMIT 7
#define SIZE_LIMIT 9

void error(char *message)
{
	printf("\nExiting process: %s\n\n", message);
	exit(1);
}

void user_error()
{
	error("User error encountered");
}

void archive_error()
{
	error("Archive error encountered");
}

void file_error()
{
	error("File not found error encountered");
}

void program_error()
{
	error("Program error encountered");
}

char ** get_files(int file_count, char **argv)
{
	char **files = (char **) malloc(sizeof(char *) * file_count);
	int i;
	for(i = 0; i < file_count; i++)
		files[i] = argv[i + 3];
	return files;
}

int is_archive(char *afile) // fix entire
{
	int success = 1;
	if(!is_file(&afile, 1)) {
		printf("Exception: Archive file does not exist\n");
		success = 0;
	} else {
		int file_desc = open(afile, O_RDONLY);
		if(file_desc != -1) {
			char buffer[SARMAG + 1]; // Used to read archive identifier
			read(file_desc, buffer, SARMAG);
			buffer[SARMAG] = '\0'; // Appending a string termination
			if(strcmp(buffer, ARMAG) != 0) { // Archive identifier mismatch
				printf("Exception: Archive format invalid");
				archive_error();
			}
		} else {
			printf("Exception: Cannot open archive\n");
			archive_error();
		}
	}
	return success;
}

int is_file(char **files, int file_count)
{
	int success = 1;
	int i;
	for(i = 0; i < file_count; i++) {
		if(access(files[i], F_OK) == -1) {
			success = 0;
			printf("Exception: %s not found\n", files[i]);
		}
	}
	return success;
}

char ** fetch_files_cd(int *file_count) // fix entire
{
	char **files = (char **) malloc(sizeof(char *) * 1);
	printf("Fetching files in cd...\n");
	files[0] = "All regular files in current directory appended";
	*file_count = 1;
	return files;
}

void clean_string(char *string)
{
	int i;
	for(i = 0; string[i] == ' '; i++)
		string++;
	for(i = strlen(string) - 1; string[i] == ' '; i--)
		string[i] = '\0';
}

void print_header(struct ar_hdr *header, char key)
{
	if(strcmp(header->ar_name, "")) {
		if(key == 't')
			printf("%s\n", header->ar_name);
		if(key == 'v')
			printf("\n\nDID NOT IMPLEMENT\n\n\n");
	}
}

int is_header(int ar_desc, int pos)
{
	int orig_pos = lseek(ar_desc, 0L, SEEK_CUR);
	int end_file = lseek(ar_desc, 0, SEEK_END);
	lseek(ar_desc, orig_pos, SEEK_SET);
	return !(orig_pos + pos >= (end_file - 1));
}

struct ar_hdr * get_header(int ar_desc, int pos)
{
	lseek(ar_desc, pos, SEEK_CUR);
	struct ar_hdr *header = (struct ar_hdr *) malloc(sizeof(struct ar_hdr));
	read(ar_desc, header, sizeof(struct ar_hdr));
	return header;
}

void format_header(struct ar_hdr *header, char key)
{
	int i;
	for(i = 0; header->ar_name[i] != '/'; i++) {
		if(i == NAME_LIMIT)
			break;
	}
	header->ar_name[i] = '\0';
	if(key == 'v') {
		header->ar_date[DATE_LIMIT] = '\0';
		header->ar_uid[UID_LIMIT] = '\0';
		header->ar_gid[GID_LIMIT] = '\0';
		header->ar_mode[MODE_LIMIT] = '\0';
		header->ar_size[SIZE_LIMIT] = '\0';
		clean_string(header->ar_date);
		clean_string(header->ar_uid);
		clean_string(header->ar_gid);
		clean_string(header->ar_mode);
	}
}

void print_table(int ar_desc, char key)
{
	lseek(ar_desc, SARMAG, SEEK_SET); // Skip archive identifier
	int pos = 0; // Set position to 0
	struct ar_hdr *header;
	while(is_header(ar_desc, pos)) {
		header = get_header(ar_desc, pos);
		format_header(header, key);
		print_header(header, key);
		pos = atoi(header->ar_size); // Adjusts position
		pos += pos % 2;
	}
}

void concise_table(char *afile) // fix entire
{
	int ar_desc = open(afile, O_RDONLY);
	if(ar_desc == -1) {
		printf("Exception: Cannot read %s\n", afile);
		archive_error();
	}
	print_table(ar_desc, 't');
	close(ar_desc);
	exit(0);
}

void verbose_table(char *afile) // fix entire
{
	printf("Printing verbose table...\n");
	exit(0);
}

int create_archive(char *afile) // fix entire
{
	int created = 1;
	printf("Creating archive: %s\n", afile);
	return created;
}

void append(char *afile, char **files, int file_count) // fix entire
{
	if(!is_archive(afile))
		if(!create_archive(afile)) {
		   printf("Exception: Archive not created");
		   archive_error();
		}
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
			if(!is_file(files, file_count)) // DELETE ME
				file_error(); // DELETE ME
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
