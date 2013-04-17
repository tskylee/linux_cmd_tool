/*
ls 工具的简单实现
使用格式  [ls  option  file]
特点：
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>

/* The official name of this program .  */
#define PROGRAM_NAME "kls"

#define BUFSIZE 4096
#define LEN 100

typedef enum {false = 0, true = 1} bool; /* 定义bool类型 */
struct cmdMsg {                          /* 存储命令行参数信息 */
	bool _list;
	bool _all;
	bool _help;

	char in_dir[LEN];
};

/* ls -l 列出所有文件信息 */
struct filSta {
	char *f_mode;
	char f_lim[LEN];
	int f_link;
	char *f_user;
	char *f_group;
	unsigned long f_size;
	char f_time[LEN];
	char *f_name;
};

/* 函数声明 */
void usage(int exit_code);
void getFsAttr(struct stat st, struct filSta *fs);
void printFile(struct stat st, struct filSta *fs, char *fname, struct cmdMsg cm);
void get_limit(char *s, unsigned int mask, mode_t mo);

int main(int argc, char *argv[])
{
	int c;
	DIR *dir;
	struct dirent *dp;
	struct stat st;
	char opfile[LEN];
	struct cmdMsg cm = {false, false, false};
	struct filSta fs;
	
	static struct option const long_options[] =
	{
		{"list", no_argument, NULL, 'l'},
		{"all", no_argument, NULL, 'a'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};
	const char *const short_options = "lah";
	while ((c = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(c) {
			case 'l':
				cm._list = true;
				break;
			case 'h':
				cm._help = true;
				break;
			case 'a':
				cm._all = true;
				break;
			case '?':
				usage(EXIT_FAILURE);
				break;
			default:
				usage(EXIT_FAILURE);
		}
	}
	/* 打印help信息 */
	if (cm._help == true) {   
		usage(EXIT_SUCCESS);
	}
	
	memset(opfile, '\0', sizeof(opfile)); /* 每次打开的目录名 */
	memset(cm.in_dir, '\0', sizeof(cm.in_dir));
	 /*
	  * optind < argc，正常情况
	  * optind > argc，错误退出
	  * optind = argc，只有选项，没有目录参数，则取默认值当前目录
	 */
	if (optind > argc) {      
		usage(EXIT_FAILURE);
	} else if (optind < argc) {
		strcpy(cm.in_dir, argv[optind]);
	} else {                      /* 若没有参数，默认是当前目录 */
		strcpy(cm.in_dir, ".");               
	}
	strcpy(opfile, cm.in_dir);
	
	/*----------------ls core process--------------------*/
	if ((dir = opendir(cm.in_dir)) == NULL) {
		fprintf(stderr, "%s: error opening directory: %s\n", PROGRAM_NAME, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	while ((dp = readdir(dir)) != NULL) {
		/* dp->d_name只是一个文件名，不是绝对路径 */
		strcat(opfile, "/");
		strcat(opfile, dp->d_name);
		if(access(opfile, F_OK) < 0) /* 测试文件是否存在 */
			continue;
		if (lstat(opfile, &st) == -1) {
			continue;
		}

		/* 更改fs的值，保证后面还可以使用 */
		printFile(st, &fs, dp->d_name, cm);
		
		memset(opfile, '\0', sizeof(opfile));
		strcpy(opfile, cm.in_dir);
	}

	if (!cm._list) {
		fprintf(stdout, "\n");
	}
	
	closedir(dir);
	return 0;
}

void printFile(struct stat st, struct filSta *fs, char *fname, struct cmdMsg cm)
{
	/* 文件名 */
	fs->f_name = fname;
	/* 文件类型 */
	if (S_ISREG(st.st_mode)) {
		if (!cm._all && strncmp(fname, ".", 1) == 0) {
			return;
		}
		if (cm._list) {
			fs->f_mode = "-";
			getFsAttr(st, fs);
			fprintf(stdout, "%s%s %d %s %s %6ld %s ", fs->f_mode, fs->f_lim, fs->f_link, \
				fs->f_user, fs->f_group, fs->f_size, fs->f_time);
		}
		fprintf(stdout, "\033[32m\033[1m");  //  普通文件 绿色
		fprintf(stdout, "%s ", fs->f_name);
		fprintf(stdout, "\033[0m");
		if (cm._list) {
			fprintf(stdout, "\n");
		}
	}
	else if (S_ISDIR(st.st_mode)) {
		if (!cm._all && strncmp(fname, ".", 1) == 0) {
			return;
		}
		if (cm._list) {
			fs->f_mode = "d";
			getFsAttr(st, fs);
			fprintf(stdout, "%s%s %d %s %s %6ld %s ", fs->f_mode, fs->f_lim, fs->f_link, \
				fs->f_user, fs->f_group, fs->f_size, fs->f_time);
		}
		fprintf(stdout, "\033[34m\033[1m");  // 目录文件 蓝色
		fprintf(stdout, "%s ", fs->f_name);
		fprintf(stdout, "\033[0m");
		if (cm._list) {
			fprintf(stdout, "\n");
		}
	}
	else if (S_ISCHR(st.st_mode)) {     
		if (!cm._all && strncmp(fname, ".", 1) == 0) {
			return;
		}
		if (cm._list) {
			fs->f_mode = "c";
			getFsAttr(st, fs);
			fprintf(stdout, "%s%s %d %s %s %6ld %s ", fs->f_mode, fs->f_lim, fs->f_link, \
				fs->f_user, fs->f_group, fs->f_size, fs->f_time);
		}
		fprintf(stdout, "\033[33m\033[1m");  // 黄色
		fprintf(stdout, "%s ", fs->f_name);
		fprintf(stdout, "\033[0m");
		if (cm._list) {
			fprintf(stdout, "\n");
		}
	}
	else if (S_ISBLK(st.st_mode)) {  
		if (!cm._all && strncmp(fname, ".", 1) == 0) {
			return;
		}
		if (cm._list) {
			fs->f_mode = "b";
			getFsAttr(st, fs);
			fprintf(stdout, "%s%s %d %s %s %6ld %s ", fs->f_mode, fs->f_lim, fs->f_link, \
				fs->f_user, fs->f_group, fs->f_size, fs->f_time);
		}
		fprintf(stdout, "\033[33m\033[1m");    // 黄色
		fprintf(stdout, "%s ", fs->f_name);
		fprintf(stdout, "\033[0m");
		if (cm._list) {
			fprintf(stdout, "\n");
		}
	}
	else if (S_ISFIFO(st.st_mode)) {      
		if (!cm._all && strncmp(fname, ".", 1) == 0) {
			return;
		}
		if (cm._list) {
			fs->f_mode = "f";
			getFsAttr(st, fs);
			fprintf(stdout, "%s%s %d %s %s %6ld %s ", fs->f_mode, fs->f_lim, fs->f_link, \
				fs->f_user, fs->f_group, fs->f_size, fs->f_time);
		}
		fprintf(stdout, "\033[37m\033[1m");   // 白色
		fprintf(stdout, "%s ", fs->f_name);
		fprintf(stdout, "\033[0m");
		if (cm._list) {
			fprintf(stdout, "\n");
		}
	}
	else if (S_ISLNK(st.st_mode)) {
		char liName[LEN];
		char _dirName[LEN] = {'\0'};
		strcpy(_dirName, cm.in_dir);
		strcat(_dirName, "/");
		strcat(_dirName, fname);
		if (!cm._all && strncmp(fname, ".", 1) == 0) {
			return;
		}
		if (cm._list) {
			fs->f_mode = "l";       
			getFsAttr(st, fs);
			fprintf(stdout, "%s%s %d %s %s %6ld %s ", fs->f_mode, fs->f_lim, fs->f_link, \
				fs->f_user, fs->f_group, fs->f_size, fs->f_time);
		}
		fprintf(stdout, "\033[36m\033[1m");   // 青色
		fprintf(stdout, "%s ", fs->f_name);
		fprintf(stdout, "\033[0m");
		if (cm._list) {
			if (readlink(_dirName, liName, LEN) != -1) {
				fprintf(stdout, "-> %s", liName);	
			}
			fprintf(stdout, "\n");
		}
	}
	else if (S_ISSOCK(st.st_mode)) {
		if (!cm._all && strncmp(fname, ".", 1) == 0) {
			return;
		}
		if (cm._list) {
			fs->f_mode = "s";            
			getFsAttr(st, fs);
			fprintf(stdout, "%s%s %d %s %s %6ld %s ", fs->f_mode, fs->f_lim, fs->f_link, \
				fs->f_user, fs->f_group, fs->f_size, fs->f_time);
		}
		fprintf(stdout, "\033[35m\033[1m");    // 品红
		fprintf(stdout, "%s ", fs->f_name);
		fprintf(stdout, "\033[0m");
		if (cm._list) {
			fprintf(stdout, "\n");
		}
	else
		fs->f_mode = "u";  /* unknown mode */
	}
}

void getFsAttr(struct stat st, struct filSta *fs)
{
	unsigned int mask = 0000777;
	/* 文件权限 */
	get_limit(fs->f_lim, mask, st.st_mode);
	/* 连接数 */
	fs->f_link = st.st_nlink;
	/* 所有者 */
	struct passwd *pa = getpwuid(st.st_uid);
	fs->f_user = pa->pw_name;
	/* 所属组 */
	struct group *gp = getgrgid (st.st_gid);
	fs->f_group = gp->gr_name;
	/* 文件大小 */
	fs->f_size = st.st_size;
	/* 修改时间 */
	struct tm *tm_time;
	tm_time = localtime(&(st.st_mtime));     /* 将时间戳转化成标准时间字符串 */
	strftime(fs->f_time, sizeof(fs->f_time), "%m  %d %H:%M", tm_time);
}

void get_limit(char *s, unsigned int mask, mode_t mo)
{
	char _lim[LEN];
	char *_uli, *_gli, *_oli;
	sprintf(_lim, "%o", mask & mo);
	
	if (strlen(_lim) != 3)
		return;
	
	if (_lim[0] == '4')
		_uli = "r--";
	else if (_lim[0] == '2')
		_uli = "-w-";
	else if (_lim[0] == '1')
		_uli = "--x";
	else if (_lim[0] == '5')
		_uli = "r-x";
	else if (_lim[0] == '6')
		_uli = "rw-";
	else if (_lim[0] == '3')
		_uli = "-wx";
	else if (_lim[0] == '7')
		_uli = "rwx";
	else if (_lim[0] == '0')
		_uli = "---";
	
	if (_lim[1] == '4')
		_gli = "r--";
	else if (_lim[1] =='2')
		_gli = "-w-";
	else if (_lim[1] == '1')
		_gli = "--x";
	else if (_lim[1] == '5')
		_gli = "r-x";
	else if (_lim[1] == '6')
		_gli = "rw-";
	else if (_lim[1] == '3')
		_gli = "-wx";
	else if (_lim[1] == '7')
		_gli = "rwx";
	else if (_lim[1] == '0')
		_gli = "---";
		
	if (_lim[2] == '4')
		_oli = "r--";
	else if (_lim[2] == '2')
		_oli = "-w-";
	else if (_lim[2] == '1')
		_oli = "--x";
	else if (_lim[2] == '5')
		_oli = "r-x";
	else if (_lim[2] == '6')
		_oli = "rw-";
	else if (_lim[2] == '3')
		_oli = "-wx";
	else if (_lim[2] == '7')
		_oli = "rwx";
	else if (_lim[2] == '0')
		_oli = "---";
	
	sprintf(s, "%s%s%s", _uli, _gli, _oli);
}

void usage(int exit_code)
{
  if (exit_code != EXIT_SUCCESS)
    fprintf(stderr, "用法: %s [选项]... PATTERN [FILE]...\n试用'kls --help'来获得更多信息。\n", PROGRAM_NAME);
  else {
	printf ("用法: %s [选项]... PATTERN [FILE]...\n试用'kls --help'来获得更多信息。\n", PROGRAM_NAME);
	fputs ("Concatenate FILE(s), or standard input, to standard output.\n\
	-l, --list             	    show all message\n\
	-h, --help               	help information\n\
", stdout);
    }
	exit (exit_code);
}

















