#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <ctype.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define STRING_SIZE 64
#define OFFSET 4

void throwError() {
	printf("invalid");
	exit(255);
}

int fileExists(char *logpath) {
  struct stat buffer;   
  return (stat (logpath, &buffer) == 0);
}

int validTime(int time, char *logpath) {
	FILE *file;
	char *line = NULL;
	size_t len = 0;

    file = fopen(logpath, "r");
    if (!file) throwError();
    int currTime = -1, currLine = 1;
    while (getline(&line, &len, file) != -1) {
    	if (currLine >= OFFSET) {
    		sscanf(line, "%*s %d", &currTime);
    		//printf("Current time: %d\n", currTime);
    		//printf("%s", line);
    	}
    	currLine++;
    }

    if (!(currTime < time)) throwError();

    fclose(file);
    if (line) free(line);

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

int validArrive(char *employee, char *guest, char *logpath, int room) {
	int location;

	if (employee) location = determineLocation("Employee:", employee, logpath);
	else location = determineLocation("Guest:", guest, logpath);

	//printf("Desired location is: %d\n", room);

	//Case 1: Person not in gallery. Can enter gallery. Cannot enter any rooms.
	if (location == -2) {
		if (room >= 0) throwError();
	}
	//Case 2: Person in gallery. Cannot enter gallery. Can enter a room.
	else if (location == -1) {
		if (!(room >= 0)) throwError();
	}
	//Case 3: Person in room. Cannot enter gallery. Cannot enter any rooms.
	else {
		throwError();
	}
	return 1;
}

int validLeave(char *employee, char *guest, char *logpath, int room) {
	int location;

	if (employee) location = determineLocation("Employee:", employee, logpath);
	else location = determineLocation("Guest:", guest, logpath);

	//printf("Desired location to leave from: %d\n", room);

	//Case 1: Person not in gallery. Cannot leave gallery. Cannot leave any rooms.
	if (location == -2) {
		throwError();
	}
	//Case 2: Person in gallery. Can leave gallery. Cannot leave any rooms.
	else if (location == -1) {
		if (room >= 0) throwError();
	}
	//Case 3: Person in room. Cannot leave gallery. Can only leave current room.
	else if (location == room) {
		return 1;
	}
	//Case 4: Person in different room than one they want to leave from.
	else {
		throwError();
	}
	return 1;
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
}

int writeToFile(int time, char *key, char *employee, char *guest, 
	int aflag, int lflag, int room, char *logpath) {
	
	FILE *file;
	
	//creates file if DNE
	if (!(fileExists(logpath))) {
		if (room >= 0 || lflag == 1) {
			remove(logpath);
			throwError();
		}
    	//printf("File created\n");
		file = fopen(logpath, "a");
		fprintf(file, "Art Gallery Log\n"
					  "Key: %s\n\n", key);
	} 
	//verifies key if it does
	else  {
		//printf("Correct Invocation: Attempting to write to file\n");
		if (verifyKey(key, logpath)) {
			file = fopen(logpath, "a");
			//condition verifications
			if (!validTime(time, logpath)) throwError();
			if (aflag) 
				if (!validArrive(employee, guest, logpath, room)) throwError();
			if (lflag) 
				if (!validLeave(employee, guest, logpath, room)) throwError();
		}
		else {
			throwError();
		}
	}
	//write to file
	fprintf(file, "Time: %d", time);
	if (employee) fprintf(file, "   Employee: %s", employee);
	if (guest) fprintf(file, "   Guest: %s", guest);
	if (aflag) fprintf(file, "   Action: Arrive");
	if (lflag) fprintf(file, "   Action: Leave");
	if (room != -1) fprintf(file, "   Room: %d ", room);
	fprintf(file, "\n");
	fclose(file);

	//printf("Successful write to file\n");

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
	int is_good = -1;
	char *logpath = NULL;
	int time = 0, room = -1;
	char *key  = NULL, *employee  = NULL, *guest  = NULL;
	int eflag = 0, gflag = 0, aflag = 0, lflag = 0;

  //pick up the switches
	while ((opt = getopt(argc, argv, "T:K:E:G:ALR:B:")) != -1) {
		switch(opt) {
    		//timestamp
			case 'T':
				//printf ("Option T with %s\n", optarg);
				if (!(atoi(optarg) >= 1 && atoi(optarg) <= 1073741823)) throwError();
				time = atoi(optarg);
				break;
	      	//secret token
			case 'K':
				//printf ("Option K with %s\n", optarg);
				if (!stringIsAlnum(optarg)) throwError();
				key = optarg;
				break;
    	  	//employee name
			case 'E':
				//printf ("Option E with %s\n", optarg);
				if (gflag == 1 || !stringIsAlpha(optarg)) throwError();
				eflag = 1;
				employee = optarg;
				break;
    		//guest name
			case 'G':
				//printf ("Option G with %s\n", optarg);
				if (eflag == 1 || !stringIsAlpha(optarg)) throwError();
				gflag = 1;
				guest = optarg;
				break;
      		//arrival
			case 'A':
				//printf ("Option A\n");
				if (lflag == 1) throwError();
				aflag = 1;
				break;
      		//departure
			case 'L':
				//printf ("Option L\n");
				if (aflag == 1) throwError();
				lflag = 1;
				break;
    	  	//room ID
			case 'R':
				//printf ("Option R with %s\n", optarg);
				if (!(atoi(optarg) >= 0 && atoi(optarg) <= 1073741823)) throwError();
				room = atoi(optarg);
				break;
      		//batch file (OPTIONAL)
			case 'B':
				printf("unimplemented\n");
				exit(255);
				break;
      		//unknown option, leave
			default:
				break;
		}
	}

  //pick up the positional argument for log path
	if(optind < argc) {
		logpath = argv[optind];
	}

	//printf("time is %d\n", time);
	//printf("key is %s\n", key);
	//printf("employee is %s\n", employee);
	//printf("guest is %s\n", guest);
	//printf("arrival is %d\n", aflag);
	//printf("leave is %d\n", lflag);
	//printf("room is %d\n", room);

	//printf("logpath is %s\n\n", logpath);

	if (time && key && (employee || guest) && (aflag || lflag) && logpath)
		is_good = writeToFile(time, key, employee, guest, aflag, lflag, room, logpath);
	else
		throwError();

	return is_good;
}