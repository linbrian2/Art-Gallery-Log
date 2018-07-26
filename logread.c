#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <ctype.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define ARRAY_SIZE 1024
#define STRING_SIZE 32
#define OFFSET 4

void throwError() {
	printf("invalid");
	exit(255);
}

int fileExists(char *logpath) {
  struct stat buffer;   
  return (stat (logpath, &buffer) == 0);
}

//This is not secure. Needs encrypt/decrypt.
int verifyKey(char *key, char *logpath) {
	FILE *file;
	char *line = NULL;
	size_t len = 0;
	char fileKey[2048] = {0};

	file = fopen(logpath, "r");
	if (!file) throwError();
	int currLine = 1;
	while (getline(&line, &len, file) != -1) {
		if (currLine == 2) {
			sscanf(line, "%*s %s", fileKey);
			fclose(file);
			if (line) free(line);
			if (strcmp(key, fileKey) == 0) return 1;
			else return 0;
		}
		currLine++;
	}
	return 0;
	return 1;
}

int determineLocation(char* personType, char *person, char *logpath) {
	//printf("Determining Location of Person\n");
	FILE *file;
	char *line = NULL;
	size_t len = 0;

    file = fopen(logpath, "r");
    if (!file) throwError();
    char currPerson[STRING_SIZE + 1];
    char currAction[STRING_SIZE + 1];
    char currType[STRING_SIZE + 1];
    int currLine = 1;
    int room = -1;
    int location = -2;
    while (getline(&line, &len, file) != -1) {
    	if (currLine >= OFFSET) {
    		//printf("%s", line);
    		sscanf(line, "%*s %*d %s %s %*s %s %*s %d", 
    			   currType, currPerson, currAction, &room);
    		//printf("Person is %s\nAction is %s\nRoom is %d\n", 
    		//		currPerson, currAction, room);
    		//printf("Person type: %s\n", currType);
    	}
    	if (strcmp(person, currPerson) == 0 && strcmp(personType, currType) == 0) {
    		if (room >= 0 && strcmp(currAction, "Arrive") == 0) 
    			location = room;
    		else if (room < 0 && strcmp(currAction, "Arrive") == 0)  
    			location = -1;
    		else if (room >= 0 && strcmp(currAction, "Leave") == 0)
    			location = -1;
    		else if (room < 0 && strcmp(currAction, "Leave") == 0) 
    			location = -2;
    		else
    			throwError();
    	}
    	currLine++;
    	//if (currLine >= 4) printf("%s's current location is: %d\n\n", person, location);
    	room = -1;
    }

    fclose(file);
    if (line) free(line);

	return location;
}

void printPeopleInRoom(int room, char *logpath, int n,
					   char employeeList[ARRAY_SIZE][STRING_SIZE + 1], int m,
					   char guestList[ARRAY_SIZE][STRING_SIZE + 1], int l) {
	char peopleInRoom[ARRAY_SIZE][STRING_SIZE + 1];
	int numPeople = 0;
	int i;
	for (i = 0; i < m; i++) {
		if ((determineLocation("Employee:", employeeList[i], logpath)) == room) {
			strcpy(peopleInRoom[numPeople], employeeList[i]);
			numPeople++;
		}
	}
	for (i = 0; i < l; i++) {
		if ((determineLocation("Guest:", guestList[i], logpath)) == room) {
			strcpy(peopleInRoom[numPeople], guestList[i]);
			numPeople++;
		}
	}
	if (numPeople > 0) {
		printf("%d: ", room);
		for (i = 0; i < numPeople; i++) {
			if (i == 0) printf("%s", peopleInRoom[i]);
			else printf(",%s", peopleInRoom[i]);
		}
		printf("\n");
	}
}

void printRoomsInfo(int arr[], char *logpath, int n,
					char employeeList[ARRAY_SIZE][STRING_SIZE + 1], int m,
					char guestList[ARRAY_SIZE][STRING_SIZE + 1], int l) {
	int temp;
	int i, j;
	for (i = 0; i < n - 1 ; i++) {
		for (j = i + 1; j < n; j++) {
			if (arr[i] > arr[j]) {
				temp = arr[i];
				arr[i] = arr[j];
				arr[j] = temp;
			}
		}
	}
	for (i = 0; i < n; i++) {
		printPeopleInRoom(arr[i], logpath, n, employeeList, m, guestList, l);
	}
}

int inIntArr(int arr[], int x, int n) {
	int i;
	for(i = 0; i < n; i++) {
		if(arr[i] == x) {
        	return 1;
		}
	}
	return 0;
}

void printSortedList(char arr[ARRAY_SIZE][STRING_SIZE + 1], int n) {
	char temp[STRING_SIZE + 1];
	int i, j;
	for (i = 0; i < n - 1 ; i++) {
		for (j = i + 1; j < n; j++) {
			if (strcmp(arr[i], arr[j]) > 0) {
				strcpy(temp, arr[i]);
				strcpy(arr[i], arr[j]);
				strcpy(arr[j], temp);
			}
		}
	}
	for (i = 0; i < n; i++) {
		if (i == 0) printf("%s", arr[i]);
		else printf(",%s", arr[i]);
	}
	printf("\n");
}

int inArr(char arr[ARRAY_SIZE][STRING_SIZE + 1], char *str, int n) {
	int i;
	for(i = 0; i < n; i++) {
		if(strcmp(arr[i], str) == 0) {
        	return 1;
		}
	}
	return 0;
}

void printToStdout(char *logpath) {
	//printf("Print to Stdout\n");
	FILE *file;
	char *line = NULL;
	size_t len = 0;

    file = fopen(logpath, "r");
    if (!file) throwError();

    char currPerson[STRING_SIZE + 1] = {0};
    char currType[STRING_SIZE + 1] = {0} ;
    char employeeList[ARRAY_SIZE][STRING_SIZE + 1] = {0} ;
    char guestList[ARRAY_SIZE][STRING_SIZE + 1] = {0} ;
    int roomsList[ARRAY_SIZE] = {0};
    int currLine = 1;
    int room = -1;
    int i = 0, j = 0, k = 0;
    while (getline(&line, &len, file) != -1) {
    	if (currLine >= OFFSET) {
    		//printf("%s", line);
    		sscanf(line, "%*s %*d %s %s %*s %*s %*s %d", 
    			   currType, currPerson, &room);
    		//printf("Person is %s\nRoom is %d\n", 
    		//		currPerson, room);
    		//printf("Person type: %s\n", currType);
    	}
    	if (strcmp("Employee:", currType) == 0 && strcmp("", currPerson) != 0 &&
    		!inArr(employeeList, currPerson, i)) {
    		if (determineLocation("Employee:", currPerson, logpath) >= -1) {
    			strcpy(employeeList[i], currPerson);
    			i++;
    		}
    	}
    	if (strcmp("Guest:", currType) == 0 && strcmp("", currPerson) != 0 &&
    		!inArr(guestList, currPerson, j)) {
    		if (determineLocation("Guest:", currPerson, logpath) >= -1) {
    			strcpy(guestList[j], currPerson);
    			j++;
    		}
    	}
    	if (room >= 0 && !inIntArr(roomsList, room, k)) {
    		roomsList[k] = room;
    		k++;
    	}
    	currLine++;
    	room = -1;
    }

	//printf("Employee List:\n");
    printSortedList(employeeList, i);
    //printf("Guest List:\n");
    printSortedList(guestList, j);
    //printf("Rooms");
    printRoomsInfo(roomsList, logpath, k, employeeList, i, guestList, j);

    fclose(file);
    if (line) free(line);
}

void printRooms(char *person, char *personType, char *logpath) {
	//printf("Print rooms\n");
	FILE *file;
	char *line = NULL;
	size_t len = 0;

    file = fopen(logpath, "r");
    if (!file) throwError();
    char currPerson[STRING_SIZE + 1];
    char currType[STRING_SIZE + 1];
    char currAction[STRING_SIZE + 1];
    int currLine = 1;
    int room = -1;
    int i = 0;
    while (getline(&line, &len, file) != -1) {
    	if (currLine >= OFFSET) {
    		//printf("%s", line);
    		sscanf(line, "%*s %*d %s %s %*s %s %*s %d", 
    			   currType, currPerson, currAction, &room);
    		//printf("Person is %s\nRoom is %d\n", 
    		//		currPerson, room);
    		//printf("Person type: %s\n", currType);
    	}
    	if (strcmp(person, currPerson) == 0 && strcmp(personType, currType) == 0 && 
    		strcmp("Arrive", currAction) == 0 && room >= 0) {
    		if (i == 0) printf("%d", room);
    		else printf(",%d", room);
    		i++;
    	}
    	currLine++;
    	room = -1;
    }
    printf("\n");
    fclose(file);
    if (line) free(line);
}

int readFile(char *key, char *employee, char *guest, 
	int sflag, int rflag, char *logpath) {
	
	//error if file DNE
	if (!fileExists(logpath))
		throwError();
	//verifies key if it does
	else  {
		//printf("Correct Invocation: Attempting to read file\n");
		if (verifyKey(key, logpath)) {
			//Read from file
			if (sflag) printToStdout(logpath);
			else {
				if (employee) printRooms(employee, "Employee:", logpath);
				else printRooms(guest, "Guest:", logpath);
			}
		}
		else throwError();
	}


	return 1;
}

int stringIsAlpha(char *c) {
	while (*c) {
        //printf("%c\n", *c);
		if (!(isalpha(*c++))) {
			return 0;
		}
	}
	return 1;
}

int stringIsAlnum(char *c) {
	while (*c) {
        //printf("%c\n", *c);
		if (!(isalnum(*c++))) {
			return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[]) {
	int opt = -1; 
  //int len = -1;
	char *logpath = NULL;
	char *key  = NULL, *employee  = NULL, *guest  = NULL;
	int eflag = 0, gflag = 0, sflag = 0, rflag = 0;

	while ((opt = getopt(argc, argv, "K:PSRE:G:VT")) != -1) {
		switch(opt) {

      		//secret token
			case 'K':
				if (!stringIsAlnum(optarg)) throwError();
				key = optarg;
				break;
      		//prints log to stdout
			case 'S':
				if (rflag == 1) throwError();
				sflag = 1;
				break;
     		 //lists of rooms entered by one person
			case 'R':
				if (sflag == 1) throwError();
				rflag = 1;
				break;
    	  	//employee name
			case 'E':
				if (gflag == 1 || !stringIsAlpha(optarg)) throwError();
				eflag = 1;
				employee = optarg;
				break;
      		//guest name
			case 'G':
				if (eflag == 1 || !stringIsAlpha(optarg)) throwError();
				gflag = 1;
				guest = optarg;
				break;
      		
      		//rooms visited by all people specified over log history (OPTIONAL)
			case 'I':
				printf("unimplemented\n");
				exit(255);
				break;
      		//total time by person over log history (OPTIONAL)
			case 'T':
				printf("unimplemented\n");
				exit(255);
				break;
      		//unknown option, leave
			default:	
				break;
		}
	}
	if(optind < argc) {
		logpath = argv[optind];
	}

	//printf("key is %s\n", key);
	//printf("employee is %s\n", employee);
	//printf("guest is %s\n", guest);
	//printf("stdout is %d\n", sflag);
	//printf("roomOp is %d\n", rflag);

	//printf("logpath is %s\n", logpath);

	if ((key && sflag && logpath) || (key && rflag && (employee || guest) && logpath))
		readFile(key, employee, guest, sflag, rflag, logpath);
	else
		throwError();

	return 0;

}
