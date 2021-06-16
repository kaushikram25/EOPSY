#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

void showHelpMessage()
{
	printf("USAGE:\n\tcopy [-m] <file_name> <new_file_name>\n\tcopy [-h]\n\n\nFLAGS: \n\t-m : use mmap to copy files\n\t-h : show help message\n");
}

int copy_mmap(int fd_from, int fd_to) {
	unsigned char *mapFrom, *mapTo;
	int pa_offset;
	struct stat fstatbuf;
	if(fstat(fd_from, &fstatbuf) == -1) {
		fprintf(
			stderr,
			"Error [%d]: fstat(): Cannot read file\n",
			errno
		);
		return errno;
	}
	mapFrom = mmap(
		NULL,
		fstatbuf.st_size,
		PROT_READ,
		MAP_SHARED,
		fd_from,
		0
	);
	if ((void *)mapFrom == MAP_FAILED) {
		fprintf(stderr, "Error [%d]: (from) Memory mapping unsuccessful\n", errno);
		return errno;
	}
	if(close(fd_from) != 0) {
		fprintf(stderr, "Error [%d]: close(): Input file cannot be closed\n", errno);
		return errno;
	}
	if(ftruncate(fd_to, fstatbuf.st_size) == -1) {
		fprintf(stderr, "Error [%d]: ftruncate(): File truncation unsuccessful\n", errno);
		return errno;
	}
	mapTo = mmap(
		NULL,
		fstatbuf.st_size,
		PROT_WRITE,
		MAP_SHARED,
		fd_to,
		0
	);
	if ((void *)mapTo == MAP_FAILED) {
		fprintf(stderr, "Error [%d]: (to) Memory mapping unsuccessful\n", errno);
		return errno;
	}
	if(close(fd_to) != 0) {
		fprintf(stderr, "Error [%d]: close(): Output file cannot be closed\n", errno);
		return errno;
	}
	memcpy(mapTo, mapFrom, fstatbuf.st_size);
	munmap(mapFrom, fstatbuf.st_size);
	munmap(mapTo, fstatbuf.st_size);
	return 0;
}

int copy_read_write(int fd_from, int fd_to) {
	struct stat fstatbuf;
	if(fstat(fd_from, &fstatbuf) == -1) {
		fprintf(
			stderr,
			"Error [%d]: fstat(): Cannot read file\n",
			errno
		);
		return errno;
	}
	int size = fstatbuf.st_size;
	char buf[size];
	int count;
	do {
		count = read(fd_from, buf, size);
		if(count == -1) {
			fprintf(
				stderr,
				"Error [%d]: read(): File read unsuccessful\n",
				errno
			);
			return errno;
		} else if(count == 0) {
			printf("File copied successfully!\n");
		} else if(write(fd_to, buf, size) == -1) {
			fprintf(
				stderr,
				"Error [%d]: write(): File write unsuccessful\n",
				errno
			);
			return errno;
		}
	} while(count > 0);
	return 0;
}

int main(int argc, char* argv[])
{
	int arg, ret, fd_from, fd_to, useMmap = 0;
	char* filename = NULL;
	char* new_filename = NULL;
	struct stat fstatbuf;
	if(argc == 1) {
		showHelpMessage();
		return 0;
	}
	for(arg = 1; arg < argc; arg++) {
		if(!strcmp("-h", argv[arg])) {
			showHelpMessage();
			return 0;
		} else if(!strcmp("-m", argv[arg])) {
			useMmap = 1;
		} else if(filename == NULL) {
			filename = argv[arg];
		} else if(filename != NULL && new_filename != NULL) {
			fprintf(stderr, "Error: Too many arguments\n");
			return -1;
		} else {
			new_filename = argv[arg];
		}
	}
	if(filename == NULL || new_filename == NULL) {
		fprintf(stderr, "Error: Not enough arguments\n");
		return -1;
	} else {
		fd_from = open(filename, O_RDONLY);
		if(fd_from == -1) {
			fprintf(
				stderr,
				"Error: [%d]: open(): Input file cannot be opened\n",
				errno
			);
			return errno;
		}
		if(fstat(fd_from, &fstatbuf) == -1) {
			fprintf(
				stderr,
				"Error [%d]: fstat(): Cannot read file\n",
				errno
			);
			return errno;
		}
		fd_to = open(new_filename, O_TRUNC | O_RDWR | O_CREAT, fstatbuf.st_mode);
		if(fd_to == -1){
			fprintf(
				stderr,
				"Error: [%d]: open(): Output file cannot be opened\n",
				errno
			);
			return errno;
		}
	}
	if(useMmap) {
		ret = copy_mmap(fd_from, fd_to);
	} else {
		ret = copy_read_write(fd_from, fd_to);
	}
	if(ret != 0) {
		return ret;
	}
}
