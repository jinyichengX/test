# TOP Makefile

BUILD_INFO := =============== Build by JinYiCheng  ===============

target = ipgui.a

# Makefile directory
makefile_dir = $(CURDIR)

# debug build?
build_for_debug = 1

# build for ARM?
build_for_arm = 0

# designate gcc path when no env
# gcc_path ?= F:/GNUtools/GNUtools/gcc-arm-none-eabi-10.3-2021.10

# cross compiler prefix name
ifneq ($(build_for_arm),0)
cross_compile ?= arm-none-eabi-
endif

# Build path
build_dir = build

# lib path
lib_dir = lib

# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
ifdef gcc_path
CC = $(gcc_path)/$(cross_compile)gcc
CXX = $(gcc_path)/$(cross_compile)g++
LD = $(gcc_path)/$(cross_compile)ld
AR = $(gcc_path)/$(cross_compile)ar
else
CC = $(cross_compile)gcc
CXX = $(cross_compile)g++
LD = $(cross_compile)ld
AR = $(cross_compile)ar
endif

obj_files = $(patsubst %.c,$(build_dir)/%.o,$(notdir $(c_srcfiles)))

# ASM files directory
asm_dir := 

# C source files directory
csrc_dir := $(makefile_dir)/src\
			$(makefile_dir)
include_dir := $(makefile_dir)/inc\
			$(makefile_dir)/src\
			$(makefile_dir)/../Include

ifneq ($(build_for_debug),0)
c_flags += -g
endif

c_srcfiles := $(foreach dir, $(csrc_dir), $(wildcard $(dir)/*.c))
#search C files in these dirs
vpath %.c $(sort $(dir $(c_srcfiles)))

asm_srcfiles := $(foreach dir, $(asm_dir), $(wildcard $(asm_dir)/*.s))
asm_srcfiles += $(foreach dir, $(asm_dir), $(wildcard $(asm_dir)/*.S))
#search ASM files in these dirs
vpath %.s $(sort $(dir $(asm_srcfiles)))

c_flags += -O3

# add c file header path
c_flags += $(addprefix -I,$(include_dir))

all : $(build_dir) $(lib_dir) $(lib_dir)/$(target) debug_info

$(lib_dir)/$(target) : $(obj_files)
	$(AR) -rcs $@ $^

$(build_dir)/%.o : %.c
	$(CC) -Wall $(c_flags) -c $< -o $@
	
$(build_dir) :
	mkdir $@

$(lib_dir) :
	mkdir $@

debug_info:
	@echo $(BUILD_INFO)

.PHONY: clean echo

echo:
	@echo $(obj_files)
	@echo $(build_dir)
	@echo $(c_srcfiles)
	@echo $(include_dir)
	@echo $(c_flags)

clean:
	-rm -fr $(build_dir)

