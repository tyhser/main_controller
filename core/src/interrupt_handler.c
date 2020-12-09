#include "cmsis_os.h"
#include "syslog.h"
#include "interrupt_handler.h"
#include "app_event.h"

volatile uint32_t interrupt_mask = 0;

osThreadId_t interruptTaskHandle = NULL;
osSemaphoreId_t interrupt_sem = NULL;

const osThreadAttr_t interruptTask_attributes = {
  .name = "interruptTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 8
};

void InterruptTask(void *argument)
{
    channel_id_t id = {0};
    for (;;) {
        if (osOK == osSemaphoreAcquire(interrupt_sem, osWaitForever)) {
            for (int i = 0; i < 8*sizeof(interrupt_mask); i++) {
                switch ((1<<i) & interrupt_mask)
                {
                    case INT_MASK_INPUT_1:
                    {
                        id.num = 1;
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_INPUT_2:
                    {
                        id.num = 2;
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_INPUT_3:
                    {
                        id.num = 3;
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_INPUT_4:
                    {
                        id.num = 4;
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_INPUT_5:
                    {
                        id.num = 5;
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_INPUT_6:
                    {
                        id.num = 6;
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_INPUT_7:
                    {
                        id.num = 7;
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_FAULT_1:
                    {
                        id.num = 1;
                        event_callback(EVENT_FAULT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_FAULT_2:
                    {
                        id.num = 2;
                        event_callback(EVENT_FAULT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_FAULT_3:
                    {
                        id.num = 3;
                        event_callback(EVENT_FAULT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_FAULT_4:
                    {
                        id.num = 4;
                        event_callback(EVENT_FAULT, (event_param_t *)&id);
                    }
                    break;
                    default:
                    break;
                }
            }
            interrupt_mask = 0;
        }
        osThreadYield();
    }
}

void interrupt_task_create(void)
{
    interrupt_sem = osSemaphoreNew(1U, 0U, NULL);
    if (interrupt_sem == NULL) {
        LOG_E("interrupt semaphore create fail");
    }
    interruptTaskHandle = osThreadNew(InterruptTask, NULL, &interruptTask_attributes);
    if (interruptTaskHandle == NULL) {
        LOG_E("interrupt task create fail");
    }
}
