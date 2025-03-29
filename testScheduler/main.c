#include <stdio.h>

#define SCH_MAX_TASKS 5
#define ERROR_SCH_TOO_MANY_TASKS 1

typedef unsigned char tByte;
typedef unsigned int tWord;

typedef struct {
    void (*pTask)(void);
    tWord Delay;
    tWord Period;
    tByte RunMe;
} sTask;

sTask SCH_tasks_G[SCH_MAX_TASKS];
tByte Error_code_G = 0;

tByte SCH_Add_Task(void (*pFunction)(), const tWord DELAY, const tWord PERIOD) {
    tByte Index = 0;
    // First find a gap in the array (if there is one)
    while ((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS)) {
        Index++;
    }
    
    if (Index == SCH_MAX_TASKS) { // Have we reached the end of the list?
        // Task list is full, Set the global error variable
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
        return SCH_MAX_TASKS; // Also return an error code
    }

    // If we're here, there is a space in the task array
    SCH_tasks_G[Index].pTask = pFunction;
    SCH_tasks_G[Index].Delay = DELAY;
    SCH_tasks_G[Index].Period = PERIOD;
    SCH_tasks_G[Index].RunMe = 0;
    
    return Index; // return position of task (to allow later deletion)
}

void SCH_Update(void) {
    tByte Index;

    // NOTE: calculations are in *TICKS* (not milliseconds)
    for (Index = 0; Index < SCH_MAX_TASKS; Index++) {
        // Check if there is a task at this location
        if (SCH_tasks_G[Index].pTask) {
            if (SCH_tasks_G[Index].Delay == 0) {
                // The task is due to run
                SCH_tasks_G[Index].RunMe += 1; // Inc. the 'RunMe' flag
                if (SCH_tasks_G[Index].Period) {
                    // Schedule periodic tasks to run again
                    SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
                }
            } else {
                // Not yet ready to run: just decrement the delay
                SCH_tasks_G[Index].Delay -= 1;
            }
        }
    }
}

void SCH_Dispatch_Tasks(void) {
    tByte Index;

    // Dispatches (runs) the next task (if one is ready)
    for (Index = 0; Index < SCH_MAX_TASKS; Index++) {
        if (SCH_tasks_G[Index].RunMe > 0) {
            (*SCH_tasks_G[Index].pTask)(); // Run the task
            SCH_tasks_G[Index].RunMe -= 1; // Reset / reduce RunMe flag
            
            // Periodic tasks will automatically run again
            // - if this is a 'one shot' task, remove it from the array
            if (SCH_tasks_G[Index].Period == 0) SCH_Delete_Task(Index);
        }
    }

    SCH_Report_Status(); // Report system status
    SCH_Go_To_Sleep();   // The scheduler enters idle mode at this point
}

void SCH_Delete_Task(const tByte TASK_INDEX) {
    SCH_tasks_G[TASK_INDEX].pTask = 0;
    SCH_tasks_G[TASK_INDEX].Delay = 0;
    SCH_tasks_G[TASK_INDEX].Period = 0;
    SCH_tasks_G[TASK_INDEX].RunMe = 0;
}

void SCH_Report_Status(void) {
    // Placeholder for reporting status
    printf("Scheduler status report\n");
}

void SCH_Go_To_Sleep(void) {
    // Placeholder for going to sleep
    printf("Scheduler going to sleep\n");
}

void Task1(void);
void Task2(void);

void Task1(void);
void Task2(void);
void Task3(void);
void Task4(void);
void Task5(void);

int main(void) {
    // Initialize the scheduler
    for (int i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_Delete_Task(i);
    }

    // Add tasks to the scheduler
    SCH_Add_Task(Task1, 0, 1);   // Task1 runs every 1000 ticks
    SCH_Add_Task(Task2, 5, 2); // Task2 runs every 2000 ticks, starting after 500 ticks
    SCH_Add_Task(Task3, 1, 0);   // Task3 runs once after 1000 ticks
    SCH_Add_Task(Task4, 2, 5);  // Task4 runs every 500 ticks, starting after 200 ticks
    SCH_Add_Task(Task5, 0, 3);   // Task5 runs every 3000 ticks

    // Simulate the scheduler running
    for (int i = 0; i < 10; i++) {
        SCH_Update();
        SCH_Dispatch_Tasks();
    }

    return 0;
}

void Task1(void) {
    printf("Task1 is running\n");
}

void Task2(void) {
    printf("Task2 is running\n");
}

void Task3(void) {
    printf("Task3 is running (one-shot task)\n");
}

void Task4(void) {
    printf("Task4 is running\n");
}

void Task5(void) {
    printf("Task5 is running\n");
}