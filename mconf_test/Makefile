#Makefile

##
hide := @
export hide

INC_FILE=$(PWD)/include/generated/autoconf.h
OBJ_DIR=$(PWD)/output
SUBDIRS:=hello

export INC_FILE
export OBJ_DIR

##
CONF_FILE=./.config
CONF=./tools/conf
PARAM=--silentoldconfig
MCONF=./tools/mconf
RMDIR=rm -rf
DIRALL=./include ./include/config ./include/generated ./output
DIRRM=./include

##
ifeq ($(DIRALL), $(wildcard $(DIRALL)))
MKDIR=
else
DIRALL:=$(filter-out $(wildcard $(DIRALL)), $(DIRALL))
MKDIR=mkdir $(DIRALL)
endif

##
ifeq ($(CONF_FILE), $(wildcard $(CONF_FILE)))
CONFIG=$(CONF) $(PARAM)
Build:
	$(hide)$(MKDIR)
	$(hide) $(CONFIG) Kconfig
	$(hide) for dir in $(SUBDIRS);\
	do make -C $$dir all || exit 1;\
	done
	$(hide) gcc -include $(INC_FILE) -c main.c -o $(OBJ_DIR)/main.o
	gcc $(OBJ_DIR)/*.o -o main	
else
CONFIG=$(MCONF)
Build:
	$(hide) $(MKDIR)
	$(hide) $(CONFIG) Kconfig
endif

#All#
all:Build
#

##
menuconfig:
	$(MCONF) Kconfig

##
clean:
	$(hide) $(RMDIR) $(DIRRM) $(CONF_FILE) $(OBJ_DIR) main
