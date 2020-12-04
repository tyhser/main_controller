#Sources
MIDDLEWARES_SRC = Middlewares
MIDDLEWARES_FILES = $(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/croutine.c \
					$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/event_groups.c \
					$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/list.c \
					$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/queue.c \
					$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/stream_buffer.c \
					$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/tasks.c \
					$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/timers.c \
					$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c \
					$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
					$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c

FEATURE = $(PROJ_PATH)/$(MIDDLEWARES_SRC)/feature.mk
include $(FEATURE)

ifeq ($(FEATURE_XX), y)
	CFLAGS += -DFEATURE_XX
endif

CFILES += $(MIDDLEWARES_FILES)

# include path
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/include
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
