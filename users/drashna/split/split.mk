CUSTOM_SPLIT_TRANSPORT_SYNC ?= yes
ifeq ($(strip $(CUSTOM_SPLIT_TRANSPORT_SYNC)), yes)
    ifeq ($(strip $(SPLIT_KEYBOARD)), yes)
        SRC += $(USER_PATH)/split/transport_sync.c
        OPT_DEFS += -DCUSTOM_SPLIT_TRANSPORT_SYNC
        CONFIG_H += $(USER_PATH)/split/config.h
    endif
endif
