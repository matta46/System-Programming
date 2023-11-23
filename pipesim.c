#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
/*  CITS2002 Project 1 2020
    Name:                Aryan Matta
    Student number(s):   22780982
 */


//  MAXIMUM NUMBER OF PROCESSES OUR SYSTEM SUPPORTS (PID=1..20)
#define MAX_PROCESSES                       20

//  MAXIMUM NUMBER OF SYSTEM-CALLS EVER MADE BY ANY PROCESS
#define MAX_SYSCALLS_PER_PROCESS            50

//  MAXIMUM NUMBER OF PIPES THAT ANY SINGLE PROCESS CAN HAVE OPEN (0..9)
#define MAX_PIPE_DESCRIPTORS_PER_PROCESS    10

//  TIME TAKEN TO SWITCH ANY PROCESS FROM ONE STATE TO ANOTHER
#define USECS_TO_CHANGE_PROCESS_STATE       5

//  TIME TAKEN TO TRANSFER ONE BYTE TO/FROM A PIPE
#define USECS_PER_BYTE_TRANSFERED           1


//  ---------------------------------------------------------------------

//  YOUR DATA STRUCTURES, VARIABLES, AND FUNCTIONS SHOULD BE ADDED HERE:

int timetaken       = 0;

struct event
{
	int thisPID;
    char command[20];

    int third;
    int fourth;
};


// A structure to represent a queue
struct Queue {
    int front, rear, size;
    int capacity;
    struct event* array;
};

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(int capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (struct event*)malloc(queue->capacity * sizeof(struct event));
    return queue;
}

// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, struct event item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
//    printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue.
// It changes front and size
struct event dequeue(struct Queue* queue)
{
//    if (isEmpty(queue))
//        return INT_MIN;
    struct event item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}


//  ---------------------------------------------------------------------

int findNumberOfEvents(char *name)
{
    FILE *fp;
    fp = fopen(name, "r");

    if(fp == NULL) {
        printf("unable to open '%s'\n", name);
        exit(EXIT_FAILURE);
    }


    #define LINELEN                 100
    #define CHAR_COMMENT            '#'


     char    line[LINELEN];
     int count = 0;

    // Extract characters from file and store in character c

    while(fgets(line, sizeof line, fp) != NULL)
    {
        if(line[0] == CHAR_COMMENT) {
            continue;
        }
        count++;
    }

    // Close the file
    fclose(fp);
    return count;
}

//  FUNCTIONS TO VALIDATE FIELDS IN EACH eventfile - NO NEED TO MODIFY
int check_PID(char word[], int lc)
{
    int PID = atoi(word);

    if(PID <= 0 || PID > MAX_PROCESSES) {
        printf("invalid PID '%s', line %i\n", word, lc);
        exit(EXIT_FAILURE);
    }
    return PID;
}

int check_microseconds(char word[], int lc)
{
    int usecs = atoi(word);

    if(usecs <= 0) {
        printf("invalid microseconds '%s', line %i\n", word, lc);
        exit(EXIT_FAILURE);
    }
    return usecs;
}

int check_descriptor(char word[], int lc)
{
    int pd = atoi(word);

    if(pd < 0 || pd >= MAX_PIPE_DESCRIPTORS_PER_PROCESS) {
        printf("invalid pipe descriptor '%s', line %i\n", word, lc);
        exit(EXIT_FAILURE);
    }
    return pd;
}

int check_bytes(char word[], int lc)
{
    int nbytes = atoi(word);

    if(nbytes <= 0) {
        printf("invalid number of bytes '%s', line %i\n", word, lc);
        exit(EXIT_FAILURE);
    }
    return nbytes;
}

//  parse_eventfile() READS AND VALIDATES THE FILE'S CONTENTS
//  YOU NEED TO STORE ITS VALUES INTO YOUR OWN DATA-STRUCTURES AND VARIABLES
struct event* parse_eventfile(char program[], char eventfile[])
{
#define LINELEN                 100
#define WORDLEN                 20
#define CHAR_COMMENT            '#'

	//  ATTEMPT TO OPEN OUR EVENTFILE, REPORTING AN ERROR IF WE CAN'T
    FILE *fp    = fopen(eventfile, "r");

    if(fp == NULL) {
        printf("%s: unable to open '%s'\n", program, eventfile);
        exit(EXIT_FAILURE);
    }

    int numberOfEvents = findNumberOfEvents(eventfile);

    struct event *events = malloc(sizeof(struct event) * numberOfEvents);

    char    line[LINELEN], words[4][WORDLEN];
    int     lc = 0;
    int eventNum = 0;

    //  READ EACH LINE FROM THE EVENTFILE, UNTIL WE REACH THE END-OF-FILE
    while(fgets(line, sizeof line, fp) != NULL) {
        ++lc;

        //  COMMENT LINES ARE SIMPLY SKIPPED
        if(line[0] == CHAR_COMMENT) {
            continue;
        }

        memset(words[2], 0, WORDLEN);
        memset(words[3], 0, WORDLEN);

        //  ATTEMPT TO BREAK EACH LINE INTO A NUMBER OF WORDS, USING sscanf()
        int nwords = sscanf(line, "%19s %19s %19s %19s",
                                    words[0], words[1], words[2], words[3]);

        //  WE WILL SIMPLY IGNORE ANY LINE WITHOUT ANY WORDS
        if(nwords <= 0) {
            continue;
        }

        //  ENSURE THAT THIS LINE'S PID IS VALID
        int thisPID = check_PID(words[0], lc);

        //  OTHER VALUES ON (SOME) LINES
        int otherPID, nbytes, usecs, pipedesc;

        //  IDENTIFY LINES RECORDING SYSTEM-CALLS AND THEIR OTHER VALUES
        //  THIS FUNCTION ONLY CHECKS INPUT;  YOU WILL NEED TO STORE THE VALUES
        if(nwords == 3 && strcmp(words[1], "compute") == 0) {
            usecs   = check_microseconds(words[2], lc);
        }
        else if(nwords == 3 && strcmp(words[1], "sleep") == 0) {
            usecs   = check_microseconds(words[2], lc);
        }
        else if(nwords == 2 && strcmp(words[1], "exit") == 0) {
            ;
        }
        else if(nwords == 3 && strcmp(words[1], "fork") == 0) {
            otherPID = check_PID(words[2], lc);
        }
        else if(nwords == 3 && strcmp(words[1], "wait") == 0) {
            otherPID = check_PID(words[2], lc);
        }
        else if(nwords == 3 && strcmp(words[1], "pipe") == 0) {
            pipedesc = check_descriptor(words[2], lc);
        }
        else if(nwords == 4 && strcmp(words[1], "writepipe") == 0) {
            pipedesc = check_descriptor(words[2], lc);
            nbytes   = check_bytes(words[3], lc);
        }
        else if(nwords == 4 && strcmp(words[1], "readpipe") == 0) {
            pipedesc = check_descriptor(words[2], lc);
            nbytes   = check_bytes(words[3], lc);
        }
        //  UNRECOGNISED LINE
        else {
        	printf("exiting..");
            printf("%s: line %i of '%s' is unrecognized\n", program,lc,eventfile);
            exit(EXIT_FAILURE);
        }

        events[eventNum].thisPID = thisPID;
        strcpy(events[eventNum].command, words[1]);

        if (nwords>2) {
        	events[eventNum].third = atoi(words[2]);
        }
        if (nwords>3) {
        	events[eventNum].fourth = atoi(words[3]);
        }

        eventNum++;

    }

    fclose(fp);

#undef  LINELEN
#undef  WORDLEN
#undef  CHAR_COMMENT

    return events;

}

//  ---------------------------------------------------------------------

//  CHECK THE COMMAND-LINE ARGUMENTS, CALL parse_eventfile(), RUN SIMULATION
int main(int argc, char *argv[])
{
	if( argc < 4 ) {
	  printf("Wrong Number of arguments supplied ");
	  exit(0);
	}

	char *program;
	char *filename;
	int pipesize;
	int timequantum;

	program = argv[0];
	filename = argv[1];
    timequantum = atoi(argv[2]);
	pipesize = atoi(argv[3]);

	struct event* events = parse_eventfile(program, filename);

	int numberOfEvents = findNumberOfEvents(filename);

    struct Queue* readyQueue = createQueue(numberOfEvents);

    int i=0;
    struct event runningevent = events[i];

    while(1) {

    	if(strcmp(runningevent.command, "exit") == 0) {
    		timetaken += 5;
    	}

    	else if(strcmp(runningevent.command, "compute") == 0) {

    		int computetime = runningevent.third;

    		if (computetime <= timequantum) {
    			timetaken += computetime;
    		}

    		else {
    			timetaken += timequantum;
    			runningevent.third = runningevent.third - timequantum;
    			enqueue(readyQueue, runningevent);
    		}

    		// time taken to transition to READY
    		timetaken += 5;
    	}

    	else if(strcmp(runningevent.command, "fork") == 0) {
    		timetaken += 10;
    	}

    	else if(strcmp(runningevent.command, "wait") == 0) {
    		timetaken += 10;
    	}

    	else if(strcmp(runningevent.command, "sleep") == 0) {
    		timetaken += 10 + runningevent.third;
    	}

    	else if(strcmp(runningevent.command, "pipe") == 0) {
    	    timetaken += 20;
    	}
    	else if(strcmp(runningevent.command, "writepipe") == 0) {

    		int writetime = runningevent.fourth;

    		if (writetime <= pipesize) {
    			timetaken += writetime;
    			// time taken to transition from RUNNING to READY
    			timetaken += 5;
    		}

    		else {
    			timetaken = timetaken + pipesize;
    			runningevent.fourth = runningevent.fourth - pipesize;
    			enqueue(readyQueue, runningevent);
    			// time taken to transition from to RUNNING to WRITEBLOCKED + WRITEBLOCKED to READY
    			timetaken += 10;
    		}

    	}

    	else if(strcmp(runningevent.command, "readpipe") == 0) {

    		int readtime = runningevent.fourth;

    		if (readtime <= pipesize) {
    			timetaken += readtime;
    			// time taken to transition from RUNNING to READY
    			timetaken += 5;
    		}

    		else {
    			timetaken = timetaken + pipesize;
    			runningevent.fourth = runningevent.fourth - pipesize;
    			enqueue(readyQueue, runningevent);
    			// time taken to transition from RUNNING to READ BLOCKED + READ BLOCKED to READY
    			timetaken += 10;
    		}

    	}

    	if (i<numberOfEvents-1) {
    		enqueue(readyQueue, events[++i]);
    	}

    	if(isEmpty(readyQueue)) {
    		break;
    	}

    	runningevent = dequeue(readyQueue);
    	// time taken to transition from READY to RUNNING
    	timetaken += 5;
    };


    printf("timetaken %i\n", timetaken);
    return 0;
}











