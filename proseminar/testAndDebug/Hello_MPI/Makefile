CC=mpicc
CC_FLAGS= -std=c99 -Wall -Wextra -pedantic
OBJS = Hello_MPI
TARGET_PATH = ./build/default/

ifeq ($(BUILD_MODE),debug)
#	@echo "Build in debug mode"
	TARGET_PATH = ./build/debug/
	CC_FLAGS += -g
else
	CC_FLAGS += -O2
endif


.PHONEY: all
all: $(OBJS)

$(OBJS): $(OBJS).c
	@$(CC) $(CC_FLAGS) $< -o $(TARGET_PATH)$(OBJS)

.PHONEY: clean
clean:
	@rm $(TARGET_PATH)$(OBJS)

.PHONEY: run
run: all
		@echo "MPI parallel:"
	@mpiexec -n 4 $(TARGET_PATH)$(OBJS)
		@echo
