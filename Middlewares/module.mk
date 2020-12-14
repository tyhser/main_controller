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
					$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c\
					$(MIDDLEWARES_SRC)/Third_Party/modbus/port/portevent.c\
					$(MIDDLEWARES_SRC)/Third_Party/modbus/port/portother.c\
					$(MIDDLEWARES_SRC)/Third_Party/modbus/port/portserial.c\
					$(MIDDLEWARES_SRC)/Third_Party/modbus/port/porttimer.c\
					$(MIDDLEWARES_SRC)/Third_Party/modbus/mb.c\
					$(MIDDLEWARES_SRC)/Third_Party/modbus/user/mbuser.c\
					$(MIDDLEWARES_SRC)/Third_Party/modbus/functions/mbfuncuser.c\
					$(MIDDLEWARES_SRC)/Third_Party/modbus/functions/mbutils.c

FEATURE = $(PROJ_PATH)/$(MIDDLEWARES_SRC)/feature.mk
include $(FEATURE)

ifeq ($(FEATURE_XX), y)
	CFLAGS += -DFEATURE_XX
endif

ifeq ($(FEATURE_MB_USER_ENABLE), y)
	CFLAGS += -DFEATURE_MB_USER_ENABLE
endif

CFILES += $(MIDDLEWARES_FILES)

# include path
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/include
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/modbus/port
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/modbus/include
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARES_SRC)/Third_Party/modbus/user
