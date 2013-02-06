#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ar.h>
#include <fcntl.h>
#include <sys/stat.h>

#define NAME_LIMIT 16
#define DATE_LIMIT 11
#define UID_LIMIT 5
#define GID_LIMIT 5
#define MODE_LIMIT 7
#define SIZE_LIMIT 9
#define ARFMAG_LIMIT 1

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

int is_archive(char *afile)
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
				printf("Exception: Archive format invalid\n");
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
	struct stat buffer;
	int i;
	for(i = 0; i < file_count; i++) {
		if(access(files[i], F_OK) == -1) {
			success = 0;
			printf("Exception: %s not found\n", files[i]);
		}
		stat(files[i], &buffer);
		if(!S_ISREG(buffer.st_mode)) {
			success = 0;
			printf("Exception: %s is not a regular file\n", files[i]);
		}
	}
	return success;
}

char ** fetch_files_cd(int *file_count) // fix entire
{
	char **files = (char **) malloc(sizeof(char *) * 1);
	printf("Fetching files in cd...\n");
	files[0] = "All regular files in current directory appended\n";
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

char * get_permissions(struct ar_hdr *header)
{
	mode_t file_mode = strtoul(header->ar_mode, NULL, 8);
	int max_chars = sizeof("rwxrwxrwx");
	char *permissions = (char *) malloc(sizeof(char) * max_chars);
	snprintf(permissions, max_chars, "%c%c%c%c%c%c%c%c%c",
		 (file_mode & S_IRUSR) ? 'r' : '-', // User permissions
		 (file_mode & S_IWUSR) ? 'w' : '-',
		 (file_mode & S_IXUSR) ? 'x' : '-',
		 (file_mode & S_IRGRP) ? 'r' : '-', // Group permissions
		 (file_mode & S_IWGRP) ? 'w' : '-',
		 (file_mode & S_IXGRP) ? 'x' : '-',
		 (file_mode & S_IROTH) ? 'r' : '-', // Others permissions
		 (file_mode & S_IWOTH) ? 'w' : '-',
		 (file_mode & S_IXOTH) ? 'x' : '-');
	return permissions;
}

void print_header(struct ar_hdr *header, char key)
{
	if(strcmp(header->ar_name, "")) {
		if(key == 't')
			printf("%s\n", header->ar_name);
		if(key == 'v') {
			char *permissions = get_permissions(header);
			char buffer[1000];
			time_t sec = (time_t) strtol(header->ar_date, NULL, 0);
			struct tm *time = localtime(&sec);
			strftime(buffer, 1000, "%b %d %H:%M %Y", time);
			clean_string(buffer);
			printf("%s %s/%s\t   ",
			       permissions, header->ar_uid, header->ar_gid);
			int size = atoi(header->ar_size);
			int i;
			int j = 1;
			for(i = 10; i <= size; i *= 10) {
				printf("\b");
				j++;
				if(j > 6) {
					printf(" ");
				}
			}
			printf("%s", header->ar_size);
			for(i = 8 - j; i > 0; i--)
				printf("\b");
			if(buffer[4] == '0')
				buffer[4] = ' ';
			printf("%s %s\n", buffer, header->ar_name);
		}
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

int open_ar(char *afile)
{
	int ar_desc = open(afile, O_RDONLY);
	if(ar_desc == -1) {
		printf("Exception: Cannot read archive: %s\n", afile);
		archive_error();
	}
	return ar_desc;
}

int open_file(char *file)
{
	int file_desc = open(file, O_RDONLY);
	if(file_desc == -1) {
		printf("Exception: Cannot read file: %s\n", file);
		file_error();
	}
	return file_desc;
}

void close_ar(int ar_desc)
{
	if(close(ar_desc == -1)) {
		printf("Exception: Cannot close archive");
		archive_error();
	}
}

void close_file(int file_desc)
{
	if(close(file_desc == -1)) {
		printf("Exception: Cannot close file");
		file_error();
	}
}

void concise_table(char *afile)
{
	int ar_desc = open_ar(afile);
	print_table(ar_desc, 't');
	close_ar(ar_desc);
	exit(0);
}

void verbose_table(char *afile)
{
	int ar_desc = open_ar(afile);
	print_table(ar_desc, 'v');
	close_ar(ar_desc);
	exit(0);
}

void init_ar(int ar_desc)
{
	if(lseek(ar_desc, 0, SEEK_SET) == lseek(ar_desc, 0, SEEK_END)) {
		write(ar_desc, ARMAG, SARMAG);
	} else {
		printf("Unable to initialize archive - improper format\n");
		archive_error();
	}
}

int create_ar(char *afile, int flags, int permissions)
{
	int ar_desc = open(afile, flags, permissions);
	if(ar_desc == -1) {
		printf("Exception: Cannot create archive: %s\n", afile);
		archive_error();
	}
	printf("Creating archive: %s\n", afile);
	return ar_desc;
}

struct ar_hdr * init_header(int file_desc, char *file) // NOT WORKING
{
	struct stat buffer;
	fstat(file_desc, &buffer);
	struct ar_hdr *header = (struct ar_hdr *) malloc(sizeof(struct ar_hdr));
	char header_name[16];
	strcpy(header_name, file);
	strcat(header_name, "/");
	snprintf(header->ar_name, 60, "%-16s%-121d%-61d%-61d%-81o%-1011d",
		 header_name, buffer.st_mtime, (long) buffer.st_uid,
		 (long) buffer.st_gid, (unsigned long) buffer.st_mode,
		 (long long) buffer.st_size);
	strcpy(header->ar_fmag, ARFMAG);
	lseek(file_desc, 0, SEEK_SET);
	return header;
}

void write_header(int ar_desc, struct ar_hdr *header) // NOT WORKING
{
	write(ar_desc, header->ar_name, NAME_LIMIT);
	write(ar_desc, header->ar_date, DATE_LIMIT + 1);
	write(ar_desc, header->ar_uid, UID_LIMIT + 1);
	write(ar_desc, header->ar_gid, GID_LIMIT + 1);
	write(ar_desc, header->ar_mode, MODE_LIMIT + 1);
	write(ar_desc, header->ar_size, SIZE_LIMIT + 1);
	write(ar_desc, header->ar_fmag, ARFMAG_LIMIT + 1);
}

off_t get_size(int file_desc) // NOT WORKING
{
	off_t size;
	int pos = lseek(file_desc, 0L, SEEK_CUR);
	size = lseek(file_desc, 0, SEEK_END);
	lseek(file_desc, pos, SEEK_SET);
	return size;
}

void append_content(int ar_desc, int file_desc, int size) // NOT WORKING LIKELY CAUSE
{
	char buffer[1];
	off_t appended = 0;
	int char_written = 0;
	while(appended < size) {
		read(file_desc, buffer, 1);
		char_written = write(ar_desc, buffer, 1);
		appended += char_written;
		lseek(file_desc, 0, SEEK_CUR);
	}
}

void append_file(int ar_desc, char *file) // NOT WORKING
{
	int file_desc = open_file(file);
	struct ar_hdr *header = init_header(file_desc, file);
	write_header(ar_desc, header);
	append_content(ar_desc, file_desc, get_size(file_desc));
	close_file(file_desc);
}

void append(char *afile, char **files, int file_count) // NOT WORKING
{
	int ar_desc;
	if(!is_archive(afile)) {
		if(is_file(&afile, 1)) {
		   printf("Exception: Archive invalid - cannot append\n");
		   archive_error();
		} else {
			int flags = O_CREAT | O_WRONLY | O_APPEND;
			ar_desc = create_ar(afile, flags, 0666);
			init_ar(ar_desc);
		}
	} else {
		ar_desc = open_ar(afile);
	}
	int i;
	for(i = 0; i < file_count; i++)
		append_file(ar_desc, files[i]);
	close_ar(ar_desc);
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
