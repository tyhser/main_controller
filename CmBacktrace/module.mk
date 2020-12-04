#Sources
CM_BACKTRACE_SRC = CmBacktrace
CM_BACKTRACE_FILES = \
				$(CM_BACKTRACE_SRC)/cm_backtrace/cm_backtrace.c


FEATURE = $(PROJ_PATH)/$(CM_BACKTRACE_SRC)/feature.mk
include $(FEATURE)

CFILES += $(CM_BACKTRACE_FILES)
SFILES += $(CM_BACKTRACE_SRC)/cm_backtrace/fault_handler/gcc/cmb_fault.S

# include path
CFLAGS += -I$(SOURCE_DIR)/CmBacktrace/cm_backtrace/
CFLAGS += -I$(SOURCE_DIR)/CmBacktrace/

