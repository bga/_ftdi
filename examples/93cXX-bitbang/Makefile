# Copyright 2020 Bga <bga.email@gmail.com>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#   http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


TEMP ?= /tmp

space :=
space +=
pathToName = $(subst /,_,$(subst $(space),-,$1))

SELF_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
PROJECT = $(call pathToName,$(dir $(SELF_PATH)))

TARGET_EXEC ?= $(PROJECT)

ARCH ?= i386
PLATFORM ?= windows


BUILD_DIR ?= $(TEMP)/$(PROJECT)
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s' | grep -Fv '.bak.')
OBJS := $(subst \,/,$(SRCS:%=$(BUILD_DIR)/%.o))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

#CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
CPPFLAGS += -Wall -Wextra -Wcast-qual -Werror=sign-conversion
CPPFLAGS += -Wno-unused-variable -Wno-unused-parameter
CPPFLAGS += -I$(PLATFORM)/include -Iinclude -I../../$(PLATFORM)/include -I../../include -I../../../../!cpp/include
CPPFLAGS += -fPIC
# CPPFLAGS += -o .obj/$(@F)
CPPFLAGS += -fdollars-in-identifiers

ifeq "$(PLATFORM)" 'windows'
	TARGET_EXEC := $(TARGET_EXEC).exe
	CPPFLAGS += -D_WIN32
	LDFLAGS += -mwindows
endif

ifdef DEBUG
	CPPFLAGS += -ggdb -DDEBUG -Og
else
	CPPFLAGS += -DNDEBUG -O2
endif

#CPPFLAGS += -MMD -MP -MF $(BUILD_DIR)/$(@F).d
CPPFLAGS += -MMD -MP
-include $(DEPS)



# LDFLAGS += -L../../lib/$(PLATFORM)/$(ARCH) -L../../../../!cpp/lib/$(PLATFORM)/$(ARCH) 
CPPFLAGS += $(shell pkg-config --cflags libftdi1)
LDFLAGS += $(shell pkg-config --libs libftdi1)


# all: app

$(TARGET_EXEC)-w:
	$(MAKE) TARGET_EXEC=$(TARGET_EXEC)-w TEMP=$(TEMP)/w PLATFORM=$(PLATFORM) CPPFLAGS=-DMEM_ORG=1 clean
	$(MAKE) TARGET_EXEC=$(TARGET_EXEC)-w TEMP=$(TEMP)/w PLATFORM=$(PLATFORM) CPPFLAGS=-DMEM_ORG=1 $(TARGET_EXEC)-w

$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o "$@" $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@


test_chop ?= 93c66
test_range ?= 0 16
test-data.bin:
	dd if=/dev/urandom of=test-data.bin bs=1 count=256

test-verify: test-data.bin
	cat test-data.bin | ./$(TARGET_EXEC) $(test_chip) verify $(test_range)
test-read:
	./$(TARGET_EXEC) $(test_chip) read $(test_range) > test-data.out.bin 
test-write: test-data.bin
	cat test-data.bin | ./$(TARGET_EXEC) $(test_chip) write $(test_range)

test: test-write test-verify

.PHONY: clean list

clean:
	$(RM) -r $(BUILD_DIR)

MKDIR_P ?= mkdir -p

# [https://stackoverflow.com/a/26339924]
list:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$'

# debugging make
print-%:
	@echo "$*" = "$($*)"
