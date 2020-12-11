#include "cmsis_os.h"
#include "syslog.h"
#include "interrupt_handler.h"
#include "app_event.h"

volatile uint32_t interrupt_mask = 0U;

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
                    case INT_MASK_ZERO_1_OFF:
                    case INT_MASK_ZERO_1_ON:
                    {
                        id.num = 1;
                        if (((1<<i) & interrupt_mask) == INT_MASK_ZERO_1_ON) {
                            id.on_off = 1;
                        } else {
                            id.on_off = 0;
                        }
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_ZERO_2_OFF:
                    case INT_MASK_ZERO_2_ON:
                    {
                        id.num = 2;
                        if (((1<<i) & interrupt_mask) == INT_MASK_ZERO_2_ON) {
                            id.on_off = 1;
                        } else {
                            id.on_off = 0;
                        }
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_ZERO_3_OFF:
                    case INT_MASK_ZERO_3_ON:
                    {
                        id.num = 3;
                        if (((1<<i) & interrupt_mask) == INT_MASK_ZERO_3_ON) {
                            id.on_off = 1;
                        } else {
                            id.on_off = 0;
                        }
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_ZERO_4_OFF:
                    case INT_MASK_ZERO_4_ON:
                    {
                        id.num = 4;
                        if (((1<<i) & interrupt_mask) == INT_MASK_ZERO_4_ON) {
                            id.on_off = 1;
                        } else {
                            id.on_off = 0;
                        }
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_ZERO_5_OFF:
                    case INT_MASK_ZERO_5_ON:
                    {
                        id.num = 5;
                        if (((1<<i) & interrupt_mask) == INT_MASK_ZERO_5_ON) {
                            id.on_off = 1;
                        } else {
                            id.on_off = 0;
                        }
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_ZERO_6_OFF:
                    case INT_MASK_ZERO_6_ON:
                    {
                        id.num = 6;
                        if (((1<<i) & interrupt_mask) == INT_MASK_ZERO_6_ON) {
                            id.on_off = 1;
                        } else {
                            id.on_off = 0;
                        }
                        event_callback(EVENT_INPUT, (event_param_t *)&id);
                    }
                    break;
                    case INT_MASK_ZERO_7_OFF:
                    case INT_MASK_ZERO_7_ON:
                    {
                        id.num = 7;
                        if (((1<<i) & interrupt_mask) == INT_MASK_ZERO_7_ON) {
                            id.on_off = 1;
                        } else {
                            id.on_off = 0;
                        }
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
            taskENTER_CRITICAL();
            interrupt_mask = 0;
            taskEXIT_CRITICAL();
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
