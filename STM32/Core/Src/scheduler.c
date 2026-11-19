
#include "scheduler.h"
#include "main.h"

typedef struct {
    void   (*pTask)(void);
    uint32_t Delay;
    uint32_t Period;
    uint32_t TaskID;
    int8_t   Next;
} sTask;

static sTask             SCH_tasks_G[SCH_MAX_TASKS];
static volatile int8_t   head = -1;
static volatile uint32_t tick_count = 0;
static uint32_t          next_id = 1;

uint8_t Error_code_G = 0;

static void free_node(int8_t idx)
{
    SCH_tasks_G[idx].pTask  = 0;
    SCH_tasks_G[idx].Delay  = 0;
    SCH_tasks_G[idx].Period = 0;
    SCH_tasks_G[idx].TaskID = NO_TASK_ID;
    SCH_tasks_G[idx].Next   = -1;
}

static void insert_node(int8_t idx, uint32_t delay)
{
    int8_t prev = -1;
    int8_t cur  = head;

    while (cur != -1 && delay >= SCH_tasks_G[cur].Delay) {
        delay -= SCH_tasks_G[cur].Delay;
        prev = cur;
        cur  = SCH_tasks_G[cur].Next;
    }

    SCH_tasks_G[idx].Delay = delay;
    SCH_tasks_G[idx].Next  = cur;
    if (cur != -1) {
        SCH_tasks_G[cur].Delay -= delay;
    }
    if (prev == -1) head = idx;
    else            SCH_tasks_G[prev].Next = idx;
}


void SCH_Init(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    for (int8_t i = 0; i < SCH_MAX_TASKS; i++) {
        free_node(i);
    }
    head       = -1;
    tick_count = 0;
    next_id    = 1;
    Error_code_G = 0;

    __set_PRIMASK(primask);
}


void SCH_Update(void)
{
    tick_count++;
    if (head != -1 && SCH_tasks_G[head].Delay > 0) {
        SCH_tasks_G[head].Delay--;
    }
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD)
{
    if (pFunction == 0) return NO_TASK_ID;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    int8_t idx = -1;
    for (int8_t i = 0; i < SCH_MAX_TASKS; i++) {
        if (SCH_tasks_G[i].pTask == 0) { idx = i; break; }
    }
    if (idx == -1) {
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
        __set_PRIMASK(primask);
        return NO_TASK_ID;
    }

    SCH_tasks_G[idx].pTask  = pFunction;
    SCH_tasks_G[idx].Period = PERIOD;
    SCH_tasks_G[idx].TaskID = next_id++;
    if (next_id == NO_TASK_ID) next_id = 1;

    insert_node(idx, DELAY);
    uint32_t id = SCH_tasks_G[idx].TaskID;

    __set_PRIMASK(primask);
    return id;
}

uint8_t SCH_Delete_Task(uint32_t taskID)
{
    uint8_t ret = RETURN_ERROR;

    if (taskID == NO_TASK_ID) {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        return RETURN_ERROR;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    int8_t prev = -1;
    int8_t cur  = head;
    while (cur != -1 && SCH_tasks_G[cur].TaskID != taskID) {
        prev = cur;
        cur  = SCH_tasks_G[cur].Next;
    }

    if (cur != -1) {
        int8_t nxt = SCH_tasks_G[cur].Next;
        if (nxt != -1) {
            SCH_tasks_G[nxt].Delay += SCH_tasks_G[cur].Delay;
        }
        if (prev == -1) head = nxt;
        else            SCH_tasks_G[prev].Next = nxt;

        free_node(cur);
        ret = RETURN_NORMAL;
    } else {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
    }

    __set_PRIMASK(primask);
    return ret;
}


void SCH_Dispatch_Tasks(void)
{
    void (*ready[SCH_MAX_TASKS])(void);
    uint8_t n = 0;


    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    while (head != -1 && SCH_tasks_G[head].Delay == 0 && n < SCH_MAX_TASKS) {
        int8_t idx = head;
        head = SCH_tasks_G[idx].Next;

        ready[n++] = SCH_tasks_G[idx].pTask;

        if (SCH_tasks_G[idx].Period > 0) {
            insert_node(idx, SCH_tasks_G[idx].Period);
        } else {
            free_node(idx);
        }
    }

    __set_PRIMASK(primask);


    for (uint8_t i = 0; i < n; i++) {
        ready[i]();
    }


}

uint32_t SCH_get_time(void)
{

    return tick_count * TICK_MS;
}

uint32_t get_time(void)
{
    return SCH_get_time();
}
