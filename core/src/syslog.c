#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "usart.h"
#include "syslog.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"

void (*syslog_assert_hook)(const char* expr, const char* func, uint32_t line);

#define LOG_POOL_ITEMS (SYSLOG_QUEUE_LENGTH + 8)

static const char *print_level_table[] = { "D", "I", "W", "E" };

#define change_level_to_string(level) \
  ((level) - PRINT_LEVEL_DEBUG <= PRINT_LEVEL_ERROR) ? print_level_table[level] : "debug"

static xQueueHandle g_log_queue = NULL;

static syslog_buffer_t syslog_buffer_pool[LOG_POOL_ITEMS];

typedef struct {
    log_switch_t log_switch;
    uint32_t syslog_task_ready;
    uint32_t syslog_init_done;
    print_level_t print_level;
    xTaskHandle syslog_task_handle;
    SemaphoreHandle_t syslog_semaphore;
    uint32_t syslog_drop_count;
} log_context_t;

static log_context_t syslog_context = {
    .syslog_task_ready  = false,
    .syslog_init_done   = false,
    .print_level        = PRINT_LEVEL_DEBUG,
    .syslog_task_handle        = NULL,
    .syslog_semaphore  = NULL,
    .syslog_drop_count  = 0,
};

/*buffer pool management*/
static void buffer_pool_init()
{
    memset(syslog_buffer_pool, 0, sizeof(syslog_buffer_pool));
}

syslog_buffer_t *buffer_pool_alloc(void)
{
    UBaseType_t uxSavedInterruptStatus;
    uint32_t index;

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    for (index = 0; index < LOG_POOL_ITEMS; index++) {
        if (!syslog_buffer_pool[index].occupied) {
            syslog_buffer_pool[index].occupied = true;
            portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
            return &syslog_buffer_pool[index];
        }
    }
    syslog_context.syslog_drop_count++;
    portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

#if (DEBUG_SYSLOG_BLOCK_ISSUE == 1)
    configASSERT(syslog_drop_count < 3000);
#endif

    return NULL;
}

static void buffer_pool_release(syslog_buffer_t *syslog_buffer)
{
    UBaseType_t uxSavedInterruptStatus;
    //log_message_t *log_message = (log_message_t *)syslog_buffer->buffer;

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    syslog_buffer->occupied = false;
    portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
}

/* syslog queue management */
static void log_queue_create(void)
{
    g_log_queue = xQueueCreate(SYSLOG_QUEUE_LENGTH, sizeof(syslog_buffer_t *));
    if (NULL == g_log_queue) {
        printf("log_queue_create: create log queue failed.\r\n");
    }
}

static void log_queue_send(syslog_buffer_t **log_buffer)
{
    if (portNVIC_INT_CTRL_REG & 0xff) {
        BaseType_t xTaskWokenByPost = pdFALSE;
        if (xQueueSendFromISR(g_log_queue, log_buffer, &xTaskWokenByPost) ==  errQUEUE_FULL ) {
            buffer_pool_release(*log_buffer);
        } else {
            portYIELD_FROM_ISR(xTaskWokenByPost);
        }
    } else {
        xQueueSend(g_log_queue, log_buffer, LOG_QUEUE_WAITING_TIME);
    }
}

static int log_queue_receive(syslog_buffer_t **log_buffer)
{
    BaseType_t ret_val = xQueueReceive(g_log_queue, log_buffer, portMAX_DELAY);
    return (pdPASS == ret_val) ? 0 : -1;
}
/* syslog queue management end*/

static void print_normal_log(const log_message_t *normal_log_message)
{
    //if (normal_log_message->print_level != PRINT_LEVEL_INFO)
    {
        printf("[%s/ T: %d F: %s L: %d]:\t\t",
               change_level_to_string(normal_log_message->print_level),
               (unsigned int)normal_log_message->timestamp,
               normal_log_message->func_name,
               normal_log_message->line_number);
   }
    printf("%s\r\n", (char *)normal_log_message->message);
}

static void process_log_message(const log_message_t *log_message)
{
    if (syslog_context.syslog_drop_count) {
        UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();;
        printf("\t\t>>> log drop count = %u\r\n\n", (unsigned int)syslog_context.syslog_drop_count);
        syslog_context.syslog_drop_count = 0;
        portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
    }
    print_normal_log(log_message);
}

static void syslog_task_entry(void *args)
{
    syslog_context.syslog_task_ready = true;
    vTaskPrioritySet(NULL, TASK_PRIORITY_SYSLOG);
    while (1) {
         syslog_buffer_t *log_buffer = NULL;
         if (0 == log_queue_receive(&log_buffer)) {
            process_log_message((log_message_t *)(log_buffer->buffer));
            buffer_pool_release(log_buffer);
         }
    }
}

void syslog_init(void)
{
    if (syslog_context.syslog_init_done) {
        return;
    }
    buffer_pool_init();
    log_queue_create();
    if (pdPASS != xTaskCreate(syslog_task_entry,
                              SYSLOG_TASK_NAME,
                              SYSLOG_TASK_STACKSIZE,
                              NULL,
                              TASK_PRIORITY_HARD_REALTIME,
                              &syslog_context.syslog_task_handle)) {
        printf("log_init: create syslog task failed.\r\n");
    }
    syslog_context.syslog_init_done = true;
}

static void vprint_module_log(const char *func,
                       int line,
                       print_level_t level,
                       const char *message,
                       va_list list)
{
    if ((syslog_context.log_switch == DEBUG_LOG_ON)
            && (syslog_context.print_level <= level)) {

        if (!syslog_context.syslog_task_ready
                || (xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED
                    && (portNVIC_INT_CTRL_REG & 0xff) == 0)) {
            if (syslog_context.syslog_init_done > 0) {
                UBaseType_t uxSavedInterruptStatus;
                uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
                if (syslog_context.syslog_init_done) {
                    /* gpt init done, time can be printed  */
                    printf("[%s/ T: %d F: %s L: %d]: ",
                           change_level_to_string(level),
                           osKernelGetTickCount(),
                           func,
                           line);
                }
                vprintf(message, list);
                printf("\r\n");
                portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
            }
        } else {
            /* syslog task is ready, send to queue to print */
            syslog_buffer_t *syslog_buffer = buffer_pool_alloc();
            log_message_t *log_message;

            if (NULL == syslog_buffer) {
                return;
            }
            log_message = (log_message_t *)syslog_buffer->buffer;
            log_message->print_level = level;
            log_message->func_name = func;
            log_message->line_number = line;
            log_message->timestamp = xTaskGetTickCount();

            /* This step might fail when log message is too long,
               but syslog will do it's best to print the log */
            (void)vsnprintf(log_message->message, MAX_LOG_SIZE, message, list);
            log_queue_send(&syslog_buffer);
        }
    }
}

void print_module_log(const char *func,
                      int line,
                      print_level_t level,
                      const char *message, ...)
{
    va_list ap;
    va_start(ap, message);
    vprint_module_log(func, line, level, message, ap);
    va_end(ap);
}

void log_error(const char *func, int line, const char *message, ...)
{
    va_list ap;
    va_start(ap, message);
    vprint_module_log(func, line, PRINT_LEVEL_ERROR, message, ap);
    va_end(ap);
}

void log_warning(const char *func, int line, const char *message, ...)
{
    va_list ap;
    va_start(ap, message);
    vprint_module_log(func, line, PRINT_LEVEL_WARNING, message, ap);
    va_end(ap);
}

void log_info(const char *func, int line, const char *message, ...)
{
    va_list ap;
    va_start(ap, message);
    vprint_module_log(func, line, PRINT_LEVEL_INFO, message, ap);
    va_end(ap);
}

void log_debug(const char *func, int line, const char *message, ...)
{
    va_list ap;
    va_start(ap, message);
    vprint_module_log(func, line, PRINT_LEVEL_DEBUG, message, ap);
    va_end(ap);
}

void syslog_assert_set_hook(void (*hook)(const char* expr, const char* func, uint32_t line)) {
    syslog_assert_hook = hook;
}

void hex_dump(const char *name, const char *data, int length)
{
    int index = 0;
    printf("%s: ", name);
    for (index = 0; index < length; index++) {
        printf("%02X", (int)(data[index]));
        if ((index + 1) % 16 == 0) {
            printf("\n");
            continue;
        }
        if (index + 1 != length) {
            printf(" ");
        }
    }
    if (0 != index && 0 != index % 16) {
        printf("\n");//add one more blank line
    }
}
