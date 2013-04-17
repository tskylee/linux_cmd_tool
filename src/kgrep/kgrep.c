/*
grep 工具的简单实现
使用格式  [kgrep  option  file]
特点：
1、匹配模式高亮显示
2、同一行多个模式匹配
3、显示行号
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/times.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>

/* The official name of this program .  */
#define PROGRAM_NAME "kgrep"

#define BUFSIZE 4096

int state = 0;

typedef enum {false = 0, true = 1} bool; /* 定义bool类型 */
struct cmdMsg {                          /* 存储命令行参数信息 */
	bool _number;
	bool _color;
	bool _help;
	
	char *pa;
	char *in_filename;
};

/* 函数声明 */ 
void kmatch(char str[], struct cmdMsg *cp);
void usage(int status);

int main(int argc, char *argv[])
{
	if (argc == 1) {
		usage(EXIT_FAILURE);
	}
	
	int c;
	int lineNum = 1;
	FILE *fp;
	char StrLine[BUFSIZE];
	struct cmdMsg cm = {false, false, false, NULL, NULL};
	
	static struct option const long_options[] =
	{
		{"number", no_argument, NULL, 'n'},
		{"color", no_argument, NULL, 'c'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};
	const char *const short_options = "nch";
	while ((c = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(c) {
			case 'n':
				cm._number = true;
				break;
			case 'c':
				cm._color = true;
				break;
			case 'h':
				cm._help = true;
				break;
			default:
				usage(EXIT_FAILURE);
		}
	}
	
	if (cm._help == true) {
		usage(EXIT_SUCCESS);
	}
	
	if (optind >= argc - 1) {
		usage(EXIT_FAILURE);
	} else {
		cm.pa = argv[optind];
		cm.in_filename = argv[optind+1];
	}
	
	/*----------------kgrep core process--------------------*/
	if ((fp = fopen(cm.in_filename, "r")) == NULL) {
		fprintf(stderr, "%s: error opening file: %s\n", PROGRAM_NAME, strerror(errno));
		//perror(PROGRAM_NAME);
		exit(3);
	}
	if (cm._number == true) {
		while (!feof(fp)) {	
			fgets(StrLine, BUFSIZE, fp);
			if (strstr(StrLine, cm.pa) != NULL) {
				fprintf(stdout, "\033[32m\033[1m");
				fprintf(stdout, "%d:", lineNum++);
				fprintf(stdout, "\033[0m");
				kmatch(StrLine, &cm);
			}
			memset(StrLine, 0, sizeof(StrLine));
		}	
	} else {
		while (!feof(fp)) {
			fgets(StrLine, BUFSIZE, fp);
			kmatch(StrLine, &cm);
			memset(StrLine, 0, sizeof(StrLine));
		}
	}
	fclose(fp);
	/*----------------end core process----------------------*/
	return 0;
}

void kmatch(char Str[], struct cmdMsg *cp)
{
	char *p1, *p2;
	int paLen;
	
	if ((p1 = strstr(Str, cp->pa)) == NULL) {
		if (!state)
			return;
		else {
			fprintf(stdout, "%s", Str);
			return;
		}
	}
	state = 1;

	for (p2 = Str; p2 != p1; p2++) {
		fprintf(stdout, "%c", *p2);
	}
	paLen = strlen(cp->pa);
	if (cp->_color == true) {
		fprintf(stdout, "\033[31m\033[1m");
		while (paLen-- > 0) {
			fprintf(stdout, "%c", *p1++);
		}
		fprintf(stdout, "\033[0m");
	} else {
		while (paLen-- > 0) {
			fprintf(stdout, "%c", *p1++);
		}
	}
	if (*p1 != '\0')
		kmatch(p1, cp);
}

void usage(int status)
{
  if (status != EXIT_SUCCESS)
	fprintf(stderr, "用法: %s [选项]... PATTERN [FILE]...\n试用'kgrep --help'来获得更多信息。\n", PROGRAM_NAME);
  else {
	printf ("用法: %s [选项]... PATTERN [FILE]...\n试用'grep --help'来获得更多信息。\n", PROGRAM_NAME);
	fputs ("Concatenate FILE(s), or standard input, to standard output.\n\
	-c, --color              	show all color\n\
	-n, --number     		   	show all line number\n\
	-h, --help               	help information\n\n\
", stdout);
    }
	exit (status);
}