#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

/* ---------- Cấu hình ---------- */
#define SCH_MAX_TASKS   20      /* Số task tối đa - chỉnh theo từng project   */
#define NO_TASK_ID      0       /* ID = 0 nghĩa là "không có task / lỗi"       */
#define TICK_MS         10      /* Chu kỳ ngắt timer (ms). PHẢI khớp CubeMX    */
#define MS_TO_TICKS(ms) ((ms) / TICK_MS)

/* ---------- Mã trả về & mã lỗi ---------- */
#define RETURN_ERROR                 0
#define RETURN_NORMAL                1
#define ERROR_SCH_TOO_MANY_TASKS     1
#define ERROR_SCH_CANNOT_DELETE_TASK 2

extern uint8_t Error_code_G;

/* ---------- API bắt buộc của lab ---------- */
void     SCH_Init(void);
void     SCH_Update(void);               /* gọi trong ngắt timer (mỗi 10 ms)   */
void     SCH_Dispatch_Tasks(void);       /* gọi trong while(1) của main        */
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t  SCH_Delete_Task(uint32_t taskID);

/* ---------- Hàm phụ trợ cho phần demo ---------- */
uint32_t SCH_get_time(void);              /* timestamp (milliseconds) */
uint32_t get_time(void);                  /* legacy compatibility alias */                 /* thời gian hiện tại tính bằng ms    */

#endif /* INC_SCHEDULER_H_ */
