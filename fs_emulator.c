#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
uint32_t inode;
uint32_t parentInode;
char type;
char name[32];
} Inode;

Inode inodeList[1024];
size_t inodeCount = 0;
uint32_t currentInode;
size_t loadedCount = 0;	//number of inodes initially loaded

//helper function to convert an uint32 number into a String
char *uint32_to_str(uint32_t i) {
	int length = snprintf(NULL, 0, "%u", i);
	if (length < 0) return NULL;
	char* str = (char*) malloc(sizeof(char) * (length + 1));
	if (str == NULL) return NULL;
	snprintf(str, length + 1, "%u", i);
	return str;
}


void saveInodeList() {
	//open file in write and append
	int fd = open("inodes_list", O_WRONLY | O_APPEND);
	if (fd == - 1) {
		perror("open");
		exit(1);
	}

	
	
	for (size_t i = loadedCount; i < inodeCount; i++) {
		if (write(fd, &inodeList[i], sizeof(Inode)) != sizeof(Inode)) {	//the last argument checks the size is complete
			perror("write");
			exit(1);
		}
		
		char inodeNum[50];		//store inode number
		char inodeParentNum[50];	//store inode parent number
		char *s = uint32_to_str(inodeList[i].inode);		//store converted inode number
		if (s == NULL) {	//check for null since use of malloc	
			perror("malloc");
			exit(1);
		}
		char *s2 = uint32_to_str(inodeList[i].parentInode);	//store converted parent inode number
		if (s2 == NULL) {	//check for null since use of malloc
                        perror("malloc");
                        exit(1);
                }

		//copy converted uint_32 number into str
		strcpy(inodeNum, s);
		strcpy(inodeParentNum,s2);
		//free the two strings becuase the helper function uses malloc and bcuz these variables are pointing to the heap
		free(s);
		free(s2);

		//create and write newly added inodes
		FILE *fptr = fopen(inodeNum, "w");
		if (fptr == NULL) {
			perror("open");
			exit(1);
		}

		//check inode type and write corresponding to type
		if (inodeList[i].type == 'd') {
			if(fprintf(fptr, "%s.\n%s..\n",inodeNum, inodeParentNum) < 0) {	//using fprintf to write onto newly created inode
				perror("fprintf");
				exit(1);
			}
		} else {
			if (fprintf(fptr, "%s\n", inodeList[i].name) < 0) {
				perror("fprintf");
				exit(1);
			}
		}
		fclose(fptr);

	}
	close(fd);
	loadedCount = inodeCount;
}


void loadInodeList(const char *path) {
	int i = 0; //counter
	
	int fd = open(path, O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit (1);
	}

	while(i < 1024 && read(fd, &inodeList[i], sizeof(Inode)) == sizeof(Inode)) {
		//printf("inode = %u, parent = %u, type = %c, name = %s\n", inodeList[i].inode, inodeList[i].parentInode, inodeList[i].type, inodeList[i].name);
		i++;
	}
	close(fd);
	loadedCount = i;
	inodeCount = i;
	//printf("%ld\n", inodeCount);
}

//ls()
void list(uint32_t parentInode) {
	for (int i = 0; i < inodeCount; i++) {
		if (inodeList[i].parentInode == parentInode) {
		printf("inode: %u, type: %c, name: %s\n", inodeList[i].inode, inodeList[i].type, inodeList[i].name);
		}
	}
}


//mkdir()
int makeDir(const char* name, uint32_t parentInode) {
	if (inodeCount >= 1024) {
		printf("exceeded amount of inodes");
		return -1;
	}

	char temp[32];
	if (strlen(name) >= 32) {
                for (int i = 0; i < 31; i++) {          //only store 31 characters 
                	temp[i] = name[i];

                }
                temp[31] = '\0'; //add \0 at character position 32
		
		//check if directory with the name already exists
        	for (int i = 0; i < inodeCount; i++) {
                	if(inodeList[i].parentInode == parentInode && strcmp(inodeList[i].name, temp) == 0 && inodeList[i].type == 'd') {
                		return -1;
                	}
        	}
                strcpy(inodeList[inodeCount].name, temp);
        } else {
		for (int i = 0; i < inodeCount; i++) {
                        if(inodeList[i].parentInode == parentInode && strcmp(inodeList[i].name, name) == 0 && inodeList[i].type == 'd') {
                                return -1;
                        }
                }
                strcpy(inodeList[inodeCount].name, name);
        }

	//add the rest of an inode directory properties
        inodeList[inodeCount].inode = inodeCount;
        inodeList[inodeCount].parentInode = parentInode;
        inodeList[inodeCount].type = 'd';	
	inodeCount++;

	return 0;
}


//cd
int changeDir(const char* name) {		
	for (int i = 0; i < inodeCount; i++) {
		if (strcmp(inodeList[i].name, name) == 0 && inodeList[i].type == 'd') {
			currentInode = inodeList[i].inode;
			return 0;
		}
	}
	printf("directory does not exist\n");
	return -1;
}

//touch
int  makeFile(const char* name, uint32_t parentInode) {
       if (inodeCount >= 1024) {
                printf("exceeded amount of inodes");
                return -1;
       }

       char temp[32];
       if (strlen(name) >= 32) {
	       for (int i = 0; i < 31; i++) {          //only store 31 characters
                        temp[i] = name[i];

               }
               temp[31] = '\0'; //add \0 at character position 32

               //check if file with the same name already exists	
               for (int i = 0; i < inodeCount; i++) {
                        if(inodeList[i].parentInode == parentInode && strcmp(inodeList[i].name, temp) == 0 && inodeList[i].type == 'f') {
                                return -1; //there exists a dupe so do nothing
                        }
               }
               strcpy(inodeList[inodeCount].name, temp);
       } else {
	       //check if file with the same name already exists
               for (int i = 0; i < inodeCount; i++) {
                        if(inodeList[i].parentInode == parentInode && strcmp(inodeList[i].name, name) == 0 && inodeList[i].type == 'f') {
                                return -1;	//there is a dupe so do nothing
                        }
               }
               strcpy(inodeList[inodeCount].name, name);
       }

       //add the rest of the inode file properties
       inodeList[inodeCount].inode = inodeCount;
       inodeList[inodeCount].parentInode = parentInode;
       inodeList[inodeCount].type = 'f';
       inodeCount++;

       return 0;
}











//use chdir to change directory
int main(int argc, char *argv[]) {
	if (argc < 2) {
		perror("fs file not provided");
		return -1;
	}
	const char *path = argv[1];
	
	if (chdir(path) != 0) {
		perror("chdir() to /fs failed");
		return 1;
	}
	loadInodeList("inodes_list");
	
	//check inode 0
	if (inodeList[0].inode != 0) {
                perror("no inode 0 exists");
                return 1;
        } else if (inodeList[0].type != 'd') {
		 perror("inode 0 is not a directory");
                return 1;
	}

	//start of sim
	currentInode = inodeList[0].inode;
        size_t bufsize = 0;
        char *buffer = NULL;
        ssize_t read = 0;
	printf("> ");


        while ((read = getline(&buffer, &bufsize, stdin)) != -1) {
                //char line[100];         //this will hold the user input
                //strcpy(line, buffer);   //copy user input to line
		//int i = 0;
		int j = 0;
		char temp[50] = ""; 	//hold first word
		char temp2[50] = "";	//hold second word
		char command[50] = "";	//hold the command
		char path[50] = "";	//hold the path/name
		int numCommands = 0;	//counter to keep track number of words user has entered

		for (int i = 0; i < strlen(buffer); i++) {
			//break words up by either a space or \n
			if (buffer[i] == ' ' || buffer[i] == '\n') {
				temp[j] = '\0';
				j = 0;
				i++;		//move to next indice to avoid space
				numCommands++;
			}
			
			//store words into their respected char array
			if (numCommands == 1)  {
				strcpy(command, temp);
			} else if (numCommands == 2) {
				strcpy(path, temp2);
			} 

			if (i != strlen(buffer) - 1) {
				if (numCommands == 1) {
					temp2[j] = buffer[i];
					j++;
				} else {
					temp[j] = buffer[i];
					j++;
				}
			}
		}

		//printf("%s\n", path);		

			
		//match first user input to command then to call the corresponding function
		if (strcmp(command, "ls") == 0) {
			if (numCommands > 1) {
				printf("ls: cannot access '%s': No such file or directory\n", path);
			} else {
				list(currentInode);
			}
        	} else if (strcmp(command, "mkdir") == 0) {
			if (numCommands > 2) {
				printf("error: only provide one argument for mkdir\n");
			} else {
				makeDir(path, currentInode);
			}
		} else if (strcmp(command, "touch") == 0) {
			if (numCommands > 2) {
				printf("error: only provide one argument for touch\n");
			} else {
				makeFile(path, currentInode);
			}
		} else if (strcmp(command, "cd") == 0) {
			changeDir(path);
		} else if (strcmp(command, "exit") == 0) {
			break;
		} else {
			printf("command not found\n");
		}

		printf("> ");
	}
	saveInodeList();	//save the current state of the inode list
        free(buffer);
	printf("$\n");
	


}


