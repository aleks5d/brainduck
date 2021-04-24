#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>	
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* C preambule */
char Cpreamb[] = 
"#include<stdio.h>\n"
"#include<stdlib.h>\n"
"unsigned char* mem;\n"
"int pos = 0;\n"
"int memSize = 30000;\n"
"\n";

/* C main */
char Cmain[] = 
"int main(int argc, char *argv[]) {\n"
"	mem = malloc(memSize);\n"
"	if (mem == NULL) {\n"
"		fprintf(stderr, \"Compilation error: Cannot create memory\\n\");\n"
"		exit(1);\n"
"	}\n";

/* C post*/
char Cpost[] = 
"	free(mem);\n"
"}\n";

/* C pre */
char CFuncPre[] = 
"void ";
/* C pre post */
char CFuncPrePost[] = 
"() {\n";

/* C post */
char CFuncPost[] = 
"}\n";

char Compile[] = 
"gcc";

const int MAXBAD = 5;

char genChar() {
	int x = rand() % (26 + 26 + 10);
	if (x < 26) return x + 'a';
	if (x < 26 + 26) return x - 26 + 'A';
	if (x < 26 + 26 + 10) return x - 26 - 26 + '0';
	return '0';
}

char* randname(char* name, int length) {
	for (int i = 0; i < length; i++) {
		name[i] = genChar();
	}
	name[length++] = '.';
	name[length++] = 'c';
	name[length++] = '\0';
	return name;
}

char isExist(char* name) {
	int isEx = access(name, F_OK);
	return !isEx;
}

char *getName() {
	int currLen = 5;
	int cnt = 0;
	const int maxLen = 10;
	char* path = getenv("XDG_RUNTIME_DIR");
	if (!path) path = getenv("TMPDIR");
	if (!path) path = "/tmp";

	int prefixLen = strlen(path) + 1;

	char *name = malloc((prefixLen + maxLen + 4) * sizeof(char));
	sprintf(name, "%s/", path);
	randname(name + prefixLen, currLen);
	++cnt;
	while (isExist(name)) {
		if (cnt == 100) {
			cnt = 0;
			++currLen;
			if (currLen > maxLen) {
				fprintf(stderr, "Can't create name for temp file\n");
				exit(1);
			}
		}
		randname(name + prefixLen, currLen);
	}
	return name;

}

char isFunLetter(char x) {
	if ('a' <= x && x <= 'z') return 1;
	if ('A' <= x && x <= 'Z') return 1;
	if ('0' <= x && x <= '9') return 1;
	return 0;
}

int tryWrite(int fd, char *buf, int size) {
	int cntBad = 0;
	int pos = 0;
	int k;
	while (size) {
		k = write(fd, buf, size);
		if (k == -1) return -1;
		if (k == 0) ++cntBad;
		if (cntBad == MAXBAD) return -1;
		size -= k;
		buf += k;
	}
	return 0;
}

char **operations;
int capacityOp = 0;
int sizeOp = 0;

int addOp(char *name, int nameSize, char *op, int opSize) {
	if (sizeOp == capacityOp) {
		capacityOp += capacityOp + 2;
		char **newMem = realloc(operations, capacityOp * sizeof(*operations));
		if (newMem == NULL) {
			fprintf(stderr, "Error: cannot realloc operations memory\n");
			return 1;
		}
		operations = newMem;
	}
	operations[sizeOp] = malloc(nameSize + 1);
	memcpy(operations[sizeOp], name, nameSize);
	operations[sizeOp][nameSize] = '\0';
	++sizeOp;
	operations[sizeOp] = malloc(opSize + 1);
	memcpy(operations[sizeOp], op, opSize);
	operations[sizeOp][opSize] = '\0';
	++sizeOp;
	return 0;
}

int addOp2(char* name, char* op) {
	return addOp(name, strlen(name), op, strlen(op));
}

int addFunction(char* name, int len) {
	if (sizeOp == capacityOp) {
		capacityOp += capacityOp + 2;
		char **newMem = realloc(operations, capacityOp * sizeof(*operations));
		if (newMem == NULL) {
			fprintf(stderr, "Error: cannot realloc operations memory\n");
			return 1;
		}
		operations = newMem;
	}
	operations[sizeOp] = malloc(len + 1);
	memcpy(operations[sizeOp], name, len);
	operations[sizeOp][len] = '\0';
	++sizeOp;
	operations[sizeOp] = malloc(len + 5);
	memcpy(operations[sizeOp], name, len);
	operations[sizeOp][len++] = '(';
	operations[sizeOp][len++] = ')';
	operations[sizeOp][len++] = ';';
	operations[sizeOp][len++] = '\n';
	operations[sizeOp][len++] = '\0';
	++sizeOp;
	return 0;
}

int addDefaultOps() {
	if(addOp2("<", "--pos;\n")) return 1;
	if(addOp2(">", "++pos;\n")) return 1;
	if(addOp2("+", "++mem[pos];\n")) return 1;
	if(addOp2("-", "--mem[pos];\n")) return 1;
	if(addOp2("[", "while(mem[pos]){\n")) return 1;
	if(addOp2("]", "}\n")) return 1;
	if(addOp2(",", "mem[pos]=getchar();\n")) return 1;
	if(addOp2(".", "putchar(mem[pos]);\n")) return 1;
	if(addOp2("n", "putchar(\'\\n\');\n")) return 1;
	return 0;
}

int operation(char *name, int len, int fd) {
	for (int i = 0; i < sizeOp; i += 2) {
		char fl = 0;
		for (int j = 0; j < len; ++j) {
			if (operations[i][j] != name[j]) {
				fl = 1;
				break;
			}
		}
		if (!fl && operations[i][len] != '\0') {
			fl = 1;
		}
		if (!fl) {
			return tryWrite(fd, operations[i + 1], strlen(operations[i + 1]));
		}
	}
	return 0;
}

int existsFunction(char *name, int len) {
	for (int i = 0; i < sizeOp; i += 2) {
		char fl = 0;
		for (int j = 0; j < len; ++j) {
			if (operations[i][j] != name[j]) {
				fl = 1;
				break;
			}
		}
		if (!fl) return 1;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	if (argc < 2) {
		fprintf(stderr, "Error: No input files\n");
		exit(1);
	}
	struct stat st;
	if (stat(argv[1], &st) < 0) {
		fprintf(stderr, "Error: Cannot open source file %s\n", argv[1]);
		exit(1);
	}
	if (!S_ISREG(st.st_mode)) {
		fprintf(stderr, "Error: source file %s is not regular\n", argv[1]);
		exit(1);
	}
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Error: Cannot open source file %s\n", argv[1]);
		exit(1);
	}
	char *buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (buf == MAP_FAILED) {
		fprintf(stderr, "Error: Cannot create mmap on source file %s\n", argv[1]);
		close(fd);
		exit(0);
	}


	char* outName = getName();
	int od = open(outName, O_WRONLY | O_CREAT | O_TRUNC, 0700);
	printf("%s\n", outName);
	if (od < 0) {
		fprintf(stderr, "Error: Cannot create temp fild\n");
		close(fd);
		free(outName);
		exit(1);
	}
	if (tryWrite(od, Cpreamb, strlen(Cpreamb))) {
		fprintf(stderr, "Error: Error while writing\n");
		close(fd);
		close(od);
		remove(outName);
		free(outName);
		exit(1);
	}

	if (addDefaultOps()) {
		fprintf(stderr, "Error: Error while adding default operations\n");
		close(fd);
		close(od);
		for (int i = 0; i < sizeOp; ++i) free(operations[i]);
		free(operations);
		remove(outName);
		free(outName);
		exit(1);
	}

	int x;
	int cnt = 0;
	int cycles = 0;
	char opend = 0;
	for (int i = 0; i < st.st_size; ++i) {
		int j = i;
		while (j < st.st_size && isFunLetter(buf[j])) ++j;
		if (j == st.st_size) {
			fprintf(stderr, "Error: Bad operation: ");
			while (i < j) {
				fprintf(stderr, "%c", buf[i++]);
			}
			fprintf(stderr, "\n");
			close(fd);
			close(od);
			for (int i = 0; i < sizeOp; ++i) free(operations[i]);
			free(operations);
			remove(outName);
			free(outName);
			exit(1);
		}
		char RE = 0;
		if (buf[j] == ':') {
			if (i == j) {
				fprintf(stderr, "Error: empty function name\n");
				close(fd);
				close(od);
				for (int i = 0; i < sizeOp; ++i) free(operations[i]);
				free(operations);
				remove(outName);
				exit(1);
			}
			if (existsFunction(buf + i, j - i)) {
				fprintf(stderr, "Error: Compilation error, function ");
				while (i < j) {
					fprintf(stderr, "%c", buf[i]);
					++j;
				}
				fprintf(stderr, "created twice\n");
				close(fd);
				close(od);
				for(int i = 0; i < sizeOp; ++i) free(operations[i]);
				free(operations);
				remove(outName);
				exit(1);
			}
			if (opend) {
				char *top;
				if (opend == 1) top = CFuncPost;
				if (opend == 2) top = Cpost;
				if (tryWrite(od, top, strlen(top))) {
					fprintf(stderr, "Error: Error while writing\n");
					close(fd);
					close(od);
					remove(outName);
					free(outName);
					for (int i = 0; i < sizeOp; ++i) free(operations[i]);
					free(operations);
					exit(1);
				}
			}
			if (j-i==4 && buf[i]=='m' && buf[i+1]=='a' && buf[i+2]=='i' && buf[i+3]=='n') {
				opend = 2;
			} else {
				opend = 1;
			}
			char *top;
			if (opend == 1) top = CFuncPre;
			if (opend == 2) top = Cmain;
			if (tryWrite(od, top, strlen(top))) {
				fprintf(stderr, "Error: Error while writing\n");
				close(fd);
				close(od);
				remove(outName);
				free(outName);
				for (int i = 0; i < sizeOp; ++i) free(operations[i]);
				free(operations);
				exit(1);
			}
			if (opend == 1) {
				if (tryWrite(od, buf+i, j-i)) {
					fprintf(stderr, "Error: Error while writing\n");
					close(fd);
					close(od);
					remove(outName);
					free(outName);
					for (int i = 0; i < sizeOp; ++i) free(operations[i]);
					free(operations);
					exit(1);
				}
				if (tryWrite(od, CFuncPrePost, strlen(CFuncPrePost))) {
					fprintf(stderr, "Error: Error while writing\n");
					close(fd);
					close(od);
					remove(outName);
					free(outName);
					for (int i = 0; i < sizeOp; ++i) free(operations[i]);
					free(operations);
					exit(1);
				}
			}
			if (addFunction(buf + i, j - i)) {
				fprintf(stderr, "Error: cannot add function\n");
				close(fd);
				close(od);
				remove(outName);
				free(outName);
				for (int i = 0; i < sizeOp; ++i) free(operations[i]);
				free(operations);
				exit(1);
			}
			i = j;
			continue;
		}
		if (buf[j] != '|') {
			if (i != j) {
				fprintf(stderr, "Error: Bad operation: ");
				while (i < j) {
					fprintf(stderr, "%c", buf[i++]);
				}
				fprintf(stderr, "\n");
				close(fd);
				close(od);
				for (int i = 0; i < sizeOp; ++i) free(operations[i]);
				free(operations);
				remove(outName);			
				exit(1);
			}
		} 
		if (buf[j] == '/') {
			if (buf[j + 1] == '/') {
				while (j < st.st_size && buf[j] != '\n') ++j;
				i = j;
				continue;
			}
			if (buf[j + 1] == '*') {
				j += 2;
				while (j < st.st_size - 1 && (buf[j] != '*' || buf[j + 1] != '/')) ++j;
				i = j;
				continue;
			}
		}
		if(operation(buf + i, j - i + (buf[j] != '|'), od)) {
			fprintf(stderr, "Error: Error while writing\n");
			close(fd);
			close(od);
			for (int i = 0; i < sizeOp; ++i) free(operations[i]);
			free(operations);
			remove(outName);
			free(outName);
			exit(1);
		}
		i = j;
	}

	if (opend) {
		char *top;
		if (opend == 1) top = CFuncPost;
		if (opend == 2) top = Cpost;
		if (tryWrite(od, top, strlen(top))) {
			fprintf(stderr, "Error: Error while writing\n");
			close(fd);
			close(od);
			remove(outName);
			free(outName);
			for (int i = 0; i < sizeOp; ++i) free(operations[i]);
			free(operations);
			exit(1);
		}
	}

	close(od);
	close(fd);
	for (int i = 0; i < sizeOp; ++i) free(operations[i]);
	free(operations);
	
	fprintf(stderr, "Ok: File created successfully\n");
	fprintf(stderr, "Ok: Starting compiling\n");

	int pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Error: Cannot fork\n");
		remove(outName);
		free(outName);
		exit(1);
	}	
	if (pid == 0) {
		execlp(Compile, Compile, outName, NULL);
		exit(1);
	}
	int res = 0;
	while(waitpid(pid, &res, 0) > 0);
	if (res) {
		fprintf(stderr, "Error: Compile error\n");
	} else {
		fprintf(stderr, "Ok: Copile successfully\n");
	}
	remove(outName);
	free(outName);

}
