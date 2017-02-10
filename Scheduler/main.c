#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_FILE_NAME "processes.in"
#define OUTPUT_FILE_NAME "processes.out"
#define BUFFER_MAX_SIZE 256
#define BOOL int
#define TRUE 1
#define FALSE 0
#define MAX_INT 2147483647

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
   // If this is true, then the process has arrived and has not finished.
   BOOL isReady;
   int burst;
   int wait;
   int startTime;
   int endTime;
} process;

// Integer queue for round robin scheduling by process index
typedef struct
{
   int *array;
   int head;
   int tail;
   int capacity;
} integerQueue;

/** Prototypes **/
void parseInputFile();
void printConfiguration();
void runFCFS();
void runSJF();
void runRR();

BOOL isFull(integerQueue *q);
BOOL isEmpty(integerQueue *q);
BOOL contains(integerQueue *q, int key);
void createQueue(integerQueue *q, int capacity);
void destroyQueue(integerQueue *q);
BOOL enqueue(integerQueue *q, int val);
int dequeue(integerQueue *q);

/** Globals **/
int processCount;
process* processes;
int runtime;
schedulerTypeEnum schedulerType;
int quantum;
FILE* outputFile;

int main(int argc, char *argv[])
{
   // Open the output file for writing.
   // This should be done before anything else.
   outputFile = fopen(OUTPUT_FILE_NAME, "w");
   if (outputFile == NULL)
   {
     fprintf(stderr, "Can't open output file %s\n", OUTPUT_FILE_NAME);
     exit(-1);
   }

   // Parse the input file.
   parseInputFile();

   // Print relevant information about the set of processes to be scheduled.
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

   // Close the output file and free memory used for the processes.
   fclose(outputFile);
   free(processes);

   return 0;
}

// Parse the input file.
// After this function is called, all globals have values that reflect
// the content of the input.
void parseInputFile()
{
   FILE* inputFile = fopen(INPUT_FILE_NAME, "r");
   if (inputFile == NULL)
   {
      fprintf(stderr, "Can't open input file %s\n", INPUT_FILE_NAME);
      exit(-1);
   }

   char buffer[BUFFER_MAX_SIZE];
   char* token;
   char* delims = " \t\n\r";
   int processesIndex = 0;

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
   char quantumStatement[50];
   sprintf(quantumStatement, "Quantum %d\n\n", quantum);
   fprintf(outputFile, "%d processes\n", processCount);
   fprintf(outputFile, "Using %s\n%s", schedulerTypeString[schedulerType],
           (RoundRobin == schedulerType) ? (quantumStatement) : ("\n"));
}

/** Standard prints used in each algorithm **/
void setProcessArrived(int time, process* p)
{
   p->isReady = TRUE;
   p->startTime = time;
   fprintf(outputFile, "Time %d: %s arrived\n", time, p->name);
}

void printProcessSelected(int time, process* p)
{
   fprintf(outputFile, "Time %d: %s selected (burst %d)\n", time, p->name, p->burst);
}

void setProcessFinished(int time, process* p)
{
   p->isReady = FALSE;
   p->endTime = time;
   fprintf(outputFile, "Time %d: %s finished\n", time, p->name);
}

void printIdle(int time)
{
   fprintf(outputFile, "Time %d: IDLE\n", time);
}

void printSchedulerFinished(int time)
{
   fprintf(outputFile, "Finished at time %d\n\n", time);
}

void printProcessStats(process* processArray, int count)
{
   int i;
   for (i = 0; i < count; i++)
   {
      if(processArray[i].endTime > 0)
         fprintf(outputFile, "%s wait %d turnaround %d\n", processArray[i].name,
                                                           processArray[i].wait,
                                                           processArray[i].endTime - processArray[i].startTime);
      else
         fprintf(outputFile, "%s didn't finish\n", processArray[i].name);
   }

}

/** Scheduling algorithms **/
void runFCFS()
{
   int idxOfCurrent = -1;

   // Iterate through each time slot of the total runtime.
   int time;
   for (time = 0; time < runtime; time++)
   {
      // Determine if current process has finished.
      if ((-1 != idxOfCurrent) && (0 == processes[idxOfCurrent].burst))
      {
         setProcessFinished(time, &processes[idxOfCurrent]);
         idxOfCurrent = -1;
      }

      int idxOfSelected = -1;
      int minArrival = MAX_INT;

      // Iterate through all the processes.
      int i;
      for (i = 0; i < processCount; i++)
      {
         // Determine if a process arrives at this time.
         if (time == processes[i].arrival)
         {
            setProcessArrived(time, &processes[i]);
         }

         // Out of ready processes, select the any that arrive first.
         if (processes[i].isReady && (processes[i].arrival < minArrival))
         {
            idxOfSelected = i;
            minArrival = processes[i].arrival;
         }
      }

      // Update the wait times of ready processes that were not selected.
      for (i = 0; i < processCount; i++)
      {
         if (processes[i].isReady && (i != idxOfSelected))
         {
            processes[i].wait++;
         }
      }

      // Only log the selection if the process is not currently running.
      if (idxOfSelected != idxOfCurrent)
      {
         idxOfCurrent = idxOfSelected;
         printProcessSelected(time, &processes[idxOfCurrent]);
      }

      // Update the remaining burst time of current process.
      if (-1 != idxOfCurrent)
      {
         processes[idxOfCurrent].burst--;
      }
      else
      {
         printIdle(time);
      }
   }

    // Determine if current process has finished.
    // For when the process happens to finish at the last tick.
   if ((-1 != idxOfCurrent) && (0 == processes[idxOfCurrent].burst))
   {
      setProcessFinished(time, &processes[idxOfCurrent]);
      idxOfCurrent = -1;
   }

   printSchedulerFinished(time);
   printProcessStats(processes, processCount);
}

// Implementation of the pre-emptive shortest job first scheduling algorithm.
// It is Inefficient because processes are not stored in a data structure
// that maintains ordering based on burst time.
void runSJF()
{
   int idxOfCurrent = -1;

   // Iterate through each time slot of the total runtime.
   int time;
   for (time = 0; time < runtime; time++)
   {
      // Determine if current process has finished.
      if ((-1 != idxOfCurrent) && (0 == processes[idxOfCurrent].burst))
      {
         setProcessFinished(time, &processes[idxOfCurrent]);
         idxOfCurrent = -1;
      }

      int idxOfSelected = -1;
      int minBurst = MAX_INT;

      // Iterate through all the processes.
      int i;
      for (i = 0; i < processCount; i++)
      {
         // Determine if a process arrives at this time.
         if (time == processes[i].arrival)
         {
            setProcessArrived(time, &processes[i]);
         }

         // Out of ready processes, select the one that has shortest current burst time.
         if (processes[i].isReady && (processes[i].burst < minBurst))
         {
            idxOfSelected = i;
            minBurst = processes[i].burst;
         }
      }

      // Update the wait times of ready processes that were not selected.
      for (i = 0; i < processCount; i++)
      {
         if (processes[i].isReady && (i != idxOfSelected))
         {
            processes[i].wait++;
         }
      }

      // Only log the selection if the process is not currently running.
      if (idxOfSelected != idxOfCurrent)
      {
         idxOfCurrent = idxOfSelected;
         printProcessSelected(time, &processes[idxOfCurrent]);
      }

      // Update the remaining burst time of current process.
      if (-1 != idxOfCurrent)
      {
         processes[idxOfCurrent].burst--;
      }
      else
      {
         printIdle(time);
      }
   }

  // Determine if current process has finished.
  // For when the process happens to finish at the last tick.
  if ((-1 != idxOfCurrent) && (0 == processes[idxOfCurrent].burst))
  {
     setProcessFinished(time, &processes[idxOfCurrent]);
     idxOfCurrent = -1;
  }

   printSchedulerFinished(time);
   printProcessStats(processes, processCount);
}

// Round Robin scheduling algorithm.
void runRR()
{
   int i, time;
   int idxOfCurrent = -1;
   integerQueue readyQueue;
   int quantumRemaining = 0;
   BOOL processFinished = TRUE;

   createQueue(&readyQueue, processCount);

   // Iterate through each time slot of the total runtime.
   for (time = 0; time < runtime; time++)
   {
      // Check if the current process has finished all of its work
      if ((idxOfCurrent != -1) && (processes[idxOfCurrent].burst == 0))
      {
         setProcessFinished(time, &processes[idxOfCurrent]);
         processFinished = TRUE;
         idxOfCurrent = -1;
      }

      // Enqueue process if it ran out of quantum but still has work to do
      if (!quantumRemaining && !processFinished)
      {
         if(!enqueue(&readyQueue, idxOfCurrent))
         {
            fprintf(stderr, "Queue is full. Cannot enqueue idx %d\n", idxOfCurrent);
         }
      }

      // Enqueue newly arrived processes
      for (i = 0; i < processCount; i++)
      {
         if (time == processes[i].arrival)
         {
            setProcessArrived(time, &processes[i]);
            
            if(!enqueue(&readyQueue, i))
            {
               fprintf(stderr, "Queue is full. Cannot enqueue idx %d\n", i);
            }
         }
      }

      // Dequeue next process if the current one is out of time or finished
      if (!quantumRemaining || processFinished)
      {
         idxOfCurrent = dequeue(&readyQueue);

         if (idxOfCurrent != -1)
         {
            printProcessSelected(time, &processes[idxOfCurrent]);
            processFinished = FALSE;
         }

         quantumRemaining = quantum;
      }

      // Update the burst time and quantum remaining of current process.
      if (idxOfCurrent != -1)
      {
         processes[idxOfCurrent].burst--;
         quantumRemaining--;
      }
      else // Enter idle
      {
         printIdle(time);
      }

      // Update the wait times of ready processes that were not selected.
      for (i = 0; i < processCount; i++)
      {
         if (processes[i].isReady && (i != idxOfCurrent))
         {
            processes[i].wait++;
         }
      }
   }

   // Check for a process that finished at end of runtime
   if ((idxOfCurrent != -1) && (processes[idxOfCurrent].burst == 0))
   {
      setProcessFinished(time, &processes[idxOfCurrent]);
   }

   printSchedulerFinished(time);
   printProcessStats(processes, processCount);

   destroyQueue(&readyQueue);
}

void createQueue(integerQueue *q, int capacity)
{
   q->array = calloc(capacity, sizeof(int));
   q->head = q->tail = -1;
   q->capacity = capacity;
}

void destroyQueue(integerQueue *q)
{
   free(q->array);
   q->head = q->tail = -1;
   q->capacity = 0;
}

BOOL isEmpty(integerQueue *q)
{
   return (q->head == q->tail);
}

BOOL isFull(integerQueue *q)
{
   return (q->head == (q->tail - q->capacity));
}

BOOL enqueue(integerQueue *q, int val)
{
   if (isFull(q))
   {
      return FALSE;
   }

   q->tail++;
   q->array[q->tail % q->capacity] = val;

   return TRUE;
}

int dequeue(integerQueue *q)
{
   if (isEmpty(q))
   {
      return -1;
   }

   q->head++;
   return (q->array[q->head % q->capacity]);
}