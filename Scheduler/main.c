#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_FILE_NAME "processes.in"
#define OUTPUT_FILE_NAME "processes.out"
#define BUFFER_MAX_SIZE 256

/** Datatypes **/
// Scheduler type enum declaration (and corresponding string array)
// Based on code from this source: https://goo.gl/eAU4bo
#define foreach_schedulerType(schedulerType) \
        schedulerType(FirstComeFirstServed)   \
        schedulerType(ShortestJobFirst)  \
        schedulerType(RoundRobin) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum {
    foreach_schedulerType(GENERATE_ENUM)
} schedulerTypeEnum;

static const char *schedulerTypeString[] = {
    foreach_schedulerType(GENERATE_STRING)
};

// Struct to hold process information.
typedef struct
{
   char name[10];
   int arrival;
   int burst;
} process;

/** Prototypes **/
void parseInputFile();
void printConfiguration();
void runFCFS();
void runSJF();
void runRR();

/** Globals **/
int processCount;
process* processes;
int processesIndex = 0;
int runtime;
schedulerTypeEnum schedulerType;
int quantum;

int main(int argc, char *argv[])
{
   parseInputFile();

   printConfiguration();

   // Based on the scheduling type, use the appropriate scheduling algorithm.
   switch (schedulerType)
   {
      case FirstComeFirstServed:
      {
         runFCFS();
         break;
      }

      case ShortestJobFirst:
      {
         runSJF();
         break;
      }

      case RoundRobin:
      {
         runRR();
         break;
      }
      default:
         break;
   }

   free(processes);
   return 0;
}

// Parse the input file.
// After this function is called, all globals have values that reflect
// the content of the input.
void parseInputFile()
{
   FILE* inputFile = fopen(INPUT_FILE_NAME, "r");
   FILE* outputFile = fopen(OUTPUT_FILE_NAME, "w");
   char buffer[BUFFER_MAX_SIZE];
   char* token;
   char* delims = " \t\n\r";

   // Read each line of the file.
   while (fgets(buffer, BUFFER_MAX_SIZE, inputFile))
   {
      token = strtok(buffer, delims);

      // Iterate through tokens of the line.
      while (NULL != token)
      {
         // If '#', the rest of the line is a comment.
         if ((strcmp(token, "#") == 0) || (strcmp(token, "end") == 0))
         {
            printf("");
            break;
         }
         else if (strcmp(token, "processcount") == 0)
         {
            token = strtok(NULL, delims);
            processCount = atoi(token);
            processes = calloc(processCount, sizeof(process));
         }
         else if (strcmp(token, "runfor") == 0)
         {
            token = strtok(NULL, delims);
            runtime = atoi(token);
         }
         else if (strcmp(token, "use") == 0)
         {
            token = strtok(NULL, delims);

            if (strcmp(token, "fcfs") == 0)
            {
               schedulerType = FirstComeFirstServed;
            }
            else if (strcmp(token, "sjf") == 0)
            {
               schedulerType = ShortestJobFirst;
            }
            else if (strcmp(token, "rr") == 0)
            {
               schedulerType = RoundRobin;
            }
            // Handle error
            else
            {
               printf("Invalid scheduling type");
            }
         }
         else if (strcmp(token, "quantum") == 0)
         {
            token = strtok(NULL, delims);
            quantum = atoi(token);
         }
         else if (strcmp(token, "process") == 0)
         {
            process* p = &processes[processesIndex++];
            token = strtok(NULL, delims);
            while (NULL != token)
            {
               if (strcmp(token, "name") == 0)
               {
                  token = strtok(NULL, delims);
                  strcpy(p->name, token);
               }
               else if (strcmp(token, "arrival") == 0)
               {
                  token = strtok(NULL, delims);
                  p->arrival = atoi(token);
               }
               else if (strcmp(token, "burst") == 0)
               {
                  token = strtok(NULL, delims);
                  p->burst = atoi(token);
               }
               // Handle error
               else
               {
                  printf("Invalid scheduling type");
               }
               token = strtok(NULL, delims);
            }
         }
         // Handle error.
         else
         {
            printf("Invalid token");
            break;
         }

         token = strtok(NULL, delims);
      }
   }

   fclose(inputFile);
}

// Print basic information about the data that is about to be processed.
void printConfiguration()
{
   printf("%d processes\n", processCount);
   printf("Using %s\n", schedulerTypeString[schedulerType]);
   printf("Quantum %d\n\n", quantum);
}

void runFCFS()
{

}

void runSJF()
{

}

void runRR()
{

}
