#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define COMMAND_NAME "sort"
#define ENABLE_LOGS 0

static char *const *filenames;
static int nfiles = 0;        

/*
1. Criar struct FILE, contendo nome, valor, numero de linhas e quebras de linha
2. Aplicar o memcpy() em vez do strcat
3. Tentar ler mais que 1024 bytes
*/

typedef struct {
	int pointer;
	int mode;
	char* name;
	char** lines;
	int numberOfLines;
	int* linebreaks;
} File;


static void openFile(File* file, char* filename, int flag);
static ssize_t readFromFile(File* file, char* buf, ssize_t size);
static void getNumberOfLines(File* file);
static void extractLinesFromFile(File* file);
static void sortLines(File* file);
static void printLines(File* file);
static void sort(char *filename);
static void log(const char* format, ...);
static void freeFile(File* file);
static void resetFile(File* file);

static void log(const char* format, ...) { 
	if (ENABLE_LOGS) {
		va_list args;
	
		va_start(args, format);
		vfprintf(stdout, format, args);
		va_end(args);
	}
}

static void openFile(File* file, char* filename, int flag) {
	log("Opening %s...\n", filename);

	file->pointer = open(filename, flag);

	file->name = filename;
	file->mode = flag;

	if (file->pointer < 0)
	{
		fprintf(stderr, "%s: cannot open %s\n", COMMAND_NAME, filename);
		exit(errno);
	}

	log("Opened %s!\n", filename);
}

static ssize_t readFromFile(File* file, char* buf, ssize_t size) {
	log("Reading from %s...\n", file->name);

	ssize_t bytesRead;
	if ((bytesRead = read(file->pointer, buf, size)) < 0) {
		fprintf(stderr, "%s: cannot read %s\n", COMMAND_NAME, file->name);
	}

	log("Read %s!\n", file->name);

	return bytesRead;
}

static void getNumberOfLines(File* file)
{
	log("Getting number of lines...\n");

	resetFile(file);

	file->numberOfLines = 1;
	char buffer[BUFSIZ];
	ssize_t bytesRead; 

	int i = 0;

	do {
		bytesRead = readFromFile(file, buffer, BUFSIZ);

		log("bytes read NL: %d\n", bytesRead);
		
		if (bytesRead < 0) {
			file->numberOfLines = -1;
		}

		i = 0;

		while (i < bytesRead) {
			if (buffer[i] == '\n') {
				file->numberOfLines++;
			}
			i++;
		}

	} while (bytesRead == BUFSIZ);

	log("Got number of lines!\n");
}

static void getLineBreaks(File* file) {
	log("Getting line breaks...\n");

	resetFile(file);

	char buffer[BUFSIZ];
	ssize_t bytesRead;

	log("Number of lines: %d\n", file->numberOfLines);
	file->linebreaks = malloc((file->numberOfLines+1) * sizeof(int));

	file->linebreaks[0] = 0;
	int byteIndex = 0, linebreakIndex= 1;
	
	/*
	buffer de 10bytes
	eu tenho 15bytes
	n0: 10bytes
	n1: 5bytes
	i tá errado, dar uma olhada dps
	*/
	do {
		bytesRead = readFromFile(file, buffer, BUFSIZ);
		byteIndex = 0;

		while (byteIndex < bytesRead) {
			if (buffer[byteIndex] == '\n') {
				file->linebreaks[linebreakIndex] = byteIndex;
				linebreakIndex++;
			}
			byteIndex++;
		}
	} while (bytesRead == BUFSIZ);

	if (buffer[byteIndex] != '\n') {
		file->linebreaks[file->numberOfLines] = byteIndex;
	}

	log("Got line breaks!\n");
}

static void extractLinesFromFile(File* file) {
	log("Extracting lines from file...\n");

	resetFile(file);

	char buffer[BUFSIZ];
	ssize_t bytesRead;

	file->lines = malloc(file->numberOfLines * sizeof(char*));

	do {
		bytesRead = readFromFile(file, buffer, BUFSIZ);
				
		int currentLineIndex = 0;
		for(currentLineIndex=1; currentLineIndex <= file->numberOfLines; currentLineIndex++) {

			file->lines[currentLineIndex-1] = malloc(BUFSIZ*sizeof(char));

			int linebreakIndex = 0;
			for (linebreakIndex=file->linebreaks[currentLineIndex-1]; linebreakIndex <= file->linebreaks[currentLineIndex]; linebreakIndex++) {
				if (buffer[linebreakIndex] != '\n'){
					char cToStr[2] = {buffer[linebreakIndex], '\0'};
					strcat(file->lines[currentLineIndex-1], cToStr);
				}
			}
		}
		
	} while (bytesRead == BUFSIZ);

	log("Extracted lines from file!\n");
}

static void sortLines(File* file)
{
	log("Sorting lines...\n");

	char temp[BUFSIZ];

	int i=0, j=0;

	for(i=0; i<file->numberOfLines; i++){
		for(j=0; j<file->numberOfLines-1-i; j++){
			if(strcmp(file->lines[j], file->lines[j+1]) > 0){
				strcpy(temp, file->lines[j]);
				strcpy(file->lines[j], file->lines[j+1]);
				strcpy(file->lines[j+1], temp);
			}
		}
	}

	log("Sorted lines!\n");
}

static void printLines(File* file) {
	log("Printing lines...\n");

	int k = 0;
	for(k=0; k < file->numberOfLines; k++) {
		printf("%s\n", file->lines[k]);
	}

	log("Printed lines!\n");
}

static void freeFile(File* file) {
	log("Freeing file...\n");

	close(file->pointer);

	int k = 0;
	for(k=0; k < file->numberOfLines; k++) {
		free(file->lines[k]);
	}

	free(file->lines);
	free(file->linebreaks);

	log("Freed file!\n");
}

static void resetFile(File* file) { 
	close(file->pointer);
	openFile(file, file->name, file->mode);
}

static void sort(char *filename)
{
	File file;
	
	log("--- OPEN FILE ---\n\n");
    openFile(&file, filename, O_RDONLY);
	log("\n");
	
	log("--- GET NUMBER OF LINES ---\n\n");
	getNumberOfLines(&file);
	log("\n");

	log("--- GET LINE BREAKS ---\n\n");
	getLineBreaks(&file);
	log("\n");

	log("--- EXTRACT LINES ---\n\n");
	extractLinesFromFile(&file);
	log("\n");

	log("--- SORT LINES ---\n\n");
	sortLines(&file);
	log("\n");

	log("--- OPEN FILE ---\n\n");
	printLines(&file);
	log("\n");

	log("--- FREE FILE ---\n\n");
	freeFile(&file);
	log("\n");
}

static void version(void)
{
	printf("sort (Nanvix Coreutils) %d.%d\n\n", VERSION_MAJOR, VERSION_MINOR);
	printf("Copyright(C) 2011-2014 Pedro H. Penna\n");
	printf("This is free software under the "); 
	printf("GNU General Public License Version 3.\n");
	printf("There is NO WARRANTY, to the extent permitted by law.\n\n");
	
	exit(EXIT_SUCCESS);
}

static void usage(void)
{
	printf("Usage: sort [options] [files]\n\n");
	printf("Brief: Print file content in alphabetical order.\n\n");
	printf("Options:\n");
	printf("  --help    Display this information and exit\n");
	printf("  --version Display program version and exit\n");
	
	exit(EXIT_SUCCESS);
}

static void getargs(int argc, char *const argv[])
{
	int i;     
	char *arg; 
	
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];
		
		if (!strcmp(arg, "--help"))
			usage();
		
		else if (!strcmp(arg, "--version"))
			version();
		
		else
		{
			if (nfiles++ == 0)
				filenames = &argv[i];
		}
	}
}

int main(int argc, char *const argv[])
{	
	int i;         
	char *filename;
	struct stat st;
	
	getargs(argc, argv);
	
	if (nfiles == 0)
		sort("/dev/tty");
	
	else
	{
		for (i = 0; i < nfiles; i++)
		{
			filename = filenames[i];
			
			if (!strcmp(filename, "-"))
				filename = "/dev/tty";
			
			else
			{
				if (stat(filename, &st) == -1)
				{
					fprintf(stderr, "%s: cannot stat %s\n", COMMAND_NAME, filename);
					continue;
				}
				
				/* File is directory. */
				if (S_ISDIR(st.st_mode))
				{
					fprintf(stderr, "%s: %s is a directory\n", COMMAND_NAME, filename);
					continue;
				}
			}
				
			sort(filename);
		}
	}
	
	return(EXIT_SUCCESS);
}