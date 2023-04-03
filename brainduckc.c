// "Copyright[2021] <aleks7dd@gmail.com>"
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
"#include<string.h>\n"
"unsigned char* mem;\n"
"int pos = 0;\n"
"int memSize = %s;\n"
"\n";

/* default memSize */
char DefualtMemSize[] = "30000";

/* C main */
char Cmain[] = 
"int main(int argc, char *argv[]) {\n"
"    mem = malloc(memSize);\n"
"    if (mem == NULL) {\n"
"        fprintf(stderr, \"Compilation error: Cannot create memory\\n\");\n"
"        exit(1);\n"
"    }\n"
"    memset(mem, 0, memSize);\n"
"    *(int*)(mem + 1) = argc;\n"
"    for (int i = 0, j = 6; i < argc; ++i) {"
"        int len = strlen(argv[i]);\n"
"        strcpy(mem + j, argv[i]);\n"
"        j += len + 1;\n"
"    }\n";

/* C post*/
char Cpost[] = 
"    free(mem);\n"
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

void printXsymbols(FILE* to, char* mem, int len) {
    while (len) {
        fprintf(to, "%c", *mem);
        --len;
        ++mem;
    }
}

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

char **operations = NULL;
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
    if(addOp2("{", "if(mem[pos]){\n")) return 1;
    if(addOp2("}", "}\n")) return 1;
    if(addOp2(",", "int _ = getchar(); mem[pos] = (_ == -1 ? 0 : _);\n")) return 1;
    if(addOp2(".", "putchar(mem[pos]);\n")) return 1;
    if(addOp2("n", "putchar(\'\\n\');\n")) return 1;
	if(addOp2("feof", "mem[pos] = feof(stdin);\n")) return 1;
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

int operation(char *name, int len, int fd) {
    if (!existsFunction(name, len)) {
        return 2;
    }
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

char **processedFiles = NULL;
int capacityPF = 0;
int sizePF = 0;
char **processingFiles = NULL;
int capacityPgF = 0;
int sizePgF = 0;

int prepare_file(char *filename, int fo) {
	for (int i = 0; i < sizePgF; ++i) {
		if (strcmp(filename, processingFiles[i]) == 0) {
			fprintf(stderr, "Error: Cycling include file %s\n", filename);
			return -1;
		}
	}
	for (int i = 0; i < sizePF; ++i) {
		if (strcmp(filename, processedFiles[i]) == 0) {
			// already processed
			return 0;
		}
	}

	if (sizePgF == capacityPgF) {
		capacityPgF *= 2;
		capacityPgF++;
		char **newProcessingFiles = realloc(processingFiles, capacityPgF * sizeof(char *));
		if (!newProcessingFiles) {
			fprintf(stderr, "Error: Out of memory while processing %s\n", filename);
			return -1;
		}
		processingFiles = newProcessingFiles;
	}
	processingFiles[sizePgF] = malloc(strlen(filename));
	if (processingFiles[sizePgF] == NULL) {
		fprintf(stderr, "Error: Out of memory while processing %s\n", filename);
		return -1;
	}
	strcpy(processingFiles[sizePgF], filename);
	sizePgF++;

	struct stat st;
	if (stat(filename, &st) < 0) {
		fprintf(stderr, "Error: Cannot open file %s\n", filename);
		return -1;
	}
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: source file %s is not regular\n", filename);
		return -1;
    }
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Error: Cannot open file %s\n", filename);
		return -1;
	}
    char *buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buf == MAP_FAILED) {
        fprintf(stderr, "Error: Cannot create mmap on source file %s\n", filename);
		close(fd);
		return -1;
    }

	for (int i = 0; i < st.st_size; ++i) {
		if (buf[i] == '#') {
			if (i + 1 == st.st_size || buf[i + 1] != 'i'
			 || i + 2 == st.st_size || buf[i + 2] != 'n'
			 || i + 3 == st.st_size || buf[i + 3] != 'c'
			 || i + 4 == st.st_size || buf[i + 4] != 'l'
			 || i + 5 == st.st_size || buf[i + 5] != 'u'
			 || i + 6 == st.st_size || buf[i + 6] != 'd'
			 || i + 7 == st.st_size || buf[i + 7] != 'e'
			 || i + 8 == st.st_size || buf[i + 8] != ' ') {
				fprintf(stderr, "Error: Invalid usage of # character in file %s\n", filename);
				return -1;
			 }
			 i += 9;
			 while (i < st.st_size && buf[i] == ' ') ++i;
			 if (i == st.st_size) {
				fprintf(stderr, "Error: No filename after include in file %s\n", filename);
				return -1;
			 }
			 int j = i;
			 while (j < st.st_size && buf[j] != ' ' && buf[j] != '\n') ++j;
			 char *newName = malloc(j - i);
			 if (newName == NULL) {
				fprintf(stderr, "Error: Out of memory while processing %s\n", filename);
				return -1;
			 }
			 memcpy(newName, buf + i, j - i);
			 if (prepare_file(newName, fo)) {
				return -1;
			 }
			 i = j - 1;
		} else {
			if (tryWrite(fo, buf + i, 1)) {
				fprintf(stderr, "Error: Error while writing to temp file in file %s\n", filename);
				return -1;
			}
		}
	}

	if (capacityPF == sizePF) {
		capacityPF *= 2;
		capacityPF++;
		char **newProcessedFiles = realloc(processedFiles, capacityPF * sizeof(char *));
		if (newProcessedFiles == NULL) {
			fprintf(stderr, "Error: Out of memory while processing %s\n", filename);
			return -1;
		}
		processedFiles = newProcessedFiles;
	}
	processedFiles[sizePF] = malloc(strlen(processingFiles[sizePgF - 1]));
	if (processedFiles[sizePF] == NULL) {
		fprintf(stderr, "Error: Out of memory while processing %s\n", filename);
		return -1;
	}
	strcpy(processedFiles[sizePF], processingFiles[sizePgF - 1]);
	free(processingFiles[sizePgF - 1]);
	--sizePgF;
	++sizePF;
	return 0;
}

void clearExit(int of, int od, char *name, char *tempName) {
    if (of) close(of);
    if (od) close(od);
    if (name) remove(name);
    if (name) free(name);
	if (tempName) remove(tempName);
	if (tempName) free(tempName);
    if (operations) {
        for (int i = 0; i < sizeOp; ++i) free(operations[i]);
        free(operations);
    }
	for (int i = 0; i < sizePgF; ++i) free(processingFiles[i]);
	if (processingFiles) free(processingFiles);
	for (int i = 0; i < sizePF; ++i) free(processedFiles[i]);
	if (processedFiles) free(processedFiles);
}

void clearExit1(int of, int od, char *name, char *tempName) {
	clearExit(of, od, name, tempName);
    exit(1);
}

void clearExit0(int of, int od, char *name, char *tempName) {
	clearExit(of, od, name, tempName);
	exit(0);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc < 2) {
        fprintf(stderr, "Error: No input files\n");
        fprintf(stderr, "Usage: brainduckc sourcefile_path [-m memsize] [-o object]\n");
        clearExit1(0, 0, 0, 0);
    }

	char *tempName = getName();
	int tod = open(tempName, O_WRONLY | O_CREAT | O_TRUNC, 0700);
	if (tod < 0) {
		fprintf(stderr, "Error: Cannot create temp file\n");
		clearExit1(0, 0, 0, tempName);
	}
	if(prepare_file(argv[1], tod)) {
		fprintf(stderr, "Error: Error while preprocessing file\n");
		clearExit1(0, tod, 0, tempName);
	}
	close(tod);

	struct stat st;
	if (stat(tempName, &st) < 0) {
		fprintf(stderr, "Error: Error cannot open temp file\n");
		clearExit1(0, 0, 0, tempName);
	}
    int fd = open(tempName, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error: Cannot open temp file %s\n", argv[1]);
        clearExit1(0, 0, 0, tempName);
    }
    char *buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buf == MAP_FAILED) {
        fprintf(stderr, "Error: Cannot create mmap on source file %s\n", argv[1]);
        clearExit1(fd, 0, 0, tempName);
    }


    char* outName = getName();
    int od = open(outName, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (od < 0) {
        fprintf(stderr, "Error: Cannot create temp file\n");
        free(outName);
        clearExit1(fd, 0, 0, tempName);
    }
    {
        char *realPreamb;
        int id = 0;
        while (id < argc && strcmp(argv[id], "-m")) ++id;
        if (id == argc) {
            realPreamb = malloc(strlen(Cpreamb) + strlen(DefualtMemSize));
            sprintf(realPreamb, Cpreamb, DefualtMemSize);
        } else if (id == argc - 1) {
            fprintf(stderr, "Error: Not given argument -m\n");
        } else {
            realPreamb = malloc(strlen(Cpreamb) + strlen(argv[id + 1]));
            sprintf(realPreamb, Cpreamb, argv[id + 1]);
        }
        if (tryWrite(od, realPreamb, strlen(realPreamb))) {
            free(realPreamb);
            fprintf(stderr, "Error: Error while writing\n");
            clearExit1(fd, od, outName, tempName);
        }
        free(realPreamb);
    }

    if (addDefaultOps()) {
        fprintf(stderr, "Error: Error while adding default operations\n");
        clearExit1(fd, od, outName, tempName);
    }

    int x;
    int cnt = 0;
    int strNum = 1;
    char opend = 0;
    for (int i = 0; i < st.st_size; ++i) {
        int j = i;
        char RE = 0;
        while (j < st.st_size && isFunLetter(buf[j])) ++j;
        if (j == st.st_size) {
            fprintf(stderr, "Error: Bad operation on line %d: ", strNum);
            printXsymbols(stderr, buf+i, j-i);
            fprintf(stderr, "\n");
            clearExit1(fd, od, outName, tempName);
        }
        if (buf[j] == ':') {
            if (i == j) {
                fprintf(stderr, "Error: empty function name on line %d\n", strNum);
                clearExit1(fd, od, outName, tempName);
            }
            if (existsFunction(buf + i, j - i)) {
                fprintf(stderr, "Error: Compilation error on line %d\n function ", strNum);
                printXsymbols(stderr, buf+i, j-i);
                fprintf(stderr, " created twice\n");
                clearExit1(fd, od, outName, tempName);
            }
            if (opend) {
                char *top;
                if (opend == 1) top = CFuncPost;
                if (opend == 2) top = Cpost;
                if (tryWrite(od, top, strlen(top))) {
                    fprintf(stderr, "Error: Error while writing\n");
                    clearExit1(fd, od, outName, tempName);
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
                clearExit1(fd, od, outName, tempName);
            }
            if (opend == 1) {
                if (tryWrite(od, buf+i, j-i)) {
                    fprintf(stderr, "Error: Error while writing\n");
                    clearExit1(fd, od, outName, tempName);
                }
                if (tryWrite(od, CFuncPrePost, strlen(CFuncPrePost))) {
                    fprintf(stderr, "Error: Error while writing\n");
                    clearExit1(fd, od, outName, tempName);
                }
            }
            if (addFunction(buf + i, j - i)) {
                fprintf(stderr, "Error: cannot add function\n");
                clearExit1(fd, od, outName, tempName);
            }
            i = j;
            continue;
        }
        if (buf[j] != '|') {
			if (i + 1 == j && buf[i] == 'C' && buf[j] == '{') {
				int bal = 1;
				j++;
				while (bal > 0 && j < st.st_size) {
					if (buf[j] == '{') ++bal;
					else if (buf[j] == '}') --bal;
					else if (buf[j] == '\n') ++strNum;
					++j;
				}
				if (bal > 0 && j == st.st_size) {
					fprintf(stderr, "Error: Bad bracket sequence for inline C code\n");
					clearExit1(fd, od, outName, tempName);
				}
				if(tryWrite(od, buf + i + 1, j - i - 1)) {
					fprintf(stderr, "Error: Error while writing\n");
					clearExit1(fd, od, outName, tempName);
				}
				i = j;
				continue;
			}
            if (i != j) {
                fprintf(stderr, "Error: Bad operation on line %d: ", strNum);
                printXsymbols(stderr, buf+i, j-i);
                fprintf(stderr, "\nmaybe forget | after function name\n");
                clearExit1(fd, od, outName, tempName);
            }
        }
        if (buf[j] == '\n') {
            strNum++;
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
        if((RE = operation(buf + i, j - i + (buf[j] != '|'), od))) {
            if (RE == 1) {
                fprintf(stderr, "Error: Error while writing\n");
                clearExit1(fd, od, outName, tempName);
            } else if (RE == 2 && buf[j] == '|') {
                fprintf(stderr, "Warning: using undeclared function on line %d: ", strNum);
                printXsymbols(stderr, buf+i, j-i);
                fprintf(stderr, "\n");
                RE = 0;
            }
        }
        i = j;
    }

    if (opend) {
        char *top;
        if (opend == 1) top = CFuncPost;
        if (opend == 2) top = Cpost;
        if (tryWrite(od, top, strlen(top))) {
            fprintf(stderr, "Error: Error while writing\n");
            clearExit1(fd, od, outName, tempName);
        }
    }

    close(od);
    close(fd);
    
    fprintf(stderr, "Ok: File created successfully\n");
    fprintf(stderr, "Ok: Starting compiling\n");

    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error: Cannot fork\n");
		clearExit1(0, 0, outName, tempName);
    }    
    if (pid == 0) {
        int id = 0;
        while (id < argc && strcmp(argv[id], "-o")) id++;
        if (id == argc) {
            execlp(Compile, Compile, outName, NULL);
        } else if (id == argc - 1) {
            fprintf(stderr, "Error: not given argument -o\n");
            exit(1);
        } else {
            execlp(Compile, Compile, outName, argv[id], argv[id + 1], NULL);
        }
        exit(1);
    }
    int res = 0;
    while(waitpid(pid, &res, 0) > 0);
    if (res) {
        fprintf(stderr, "Error: Compile error\n");
    } else {
        fprintf(stderr, "Ok: Compile successfully\n");
    }

	clearExit0(0, 0, outName, tempName);

}

