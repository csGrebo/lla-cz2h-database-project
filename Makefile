# Adapted from https://gist.github.com/natanaeljr/f0b0218dea81b0a23d5d7be205f4a252
-include config.mk
PROJECT ?= $(notdir $(CURDIR))
EXECUTABLE = $(BUILDDIR)/$(PROJECT)

override SRCDIRS += src
override INCDIRS += include $(SRCDIRS)
BUILDDIR ?= build
OBJDIR ?= $(BUILDDIR)/obj

SRCEXTS := c
HDREXTS := h

SOURCES := $(foreach dir, $(SRCDIRS), $(foreach ext, $(SRCEXTS), $(wildcard $(dir)/*.$(ext))))
INCLUDES := $(foreach dir, $(INCDIRS), $(foreach ext, $(HDREXTS), $(wildcard $(dir)/*.$(ext))))
OBJECTS := $(foreach ext, $(SRCEXTS), $(patsubst %.$(ext), $(OBJDIR)/%.o, $(filter %.$(ext), $(SOURCES))))

CC := gcc

override CFLAGS += -g -Wall -Werror
override LDFLAGS +=
INCFLAGS := $(INCDIRS:%=-I%)
DEPFLAGS := -MMD -MP

MAKEFILE := $(lastword $(MAKEFILE_LIST))

define compilecc
	@mkdir -p $(dir $1)
	@$(CC) -c $2 -o $1 $(CFLAGS) $(INCFLAGS) -MT $1 -MF $(OBJDIR)/$3.Td $(DEPFLAGS)
	@mv -f $(OBJDIR)/$3.Td $(OBJDIR)/$3.d && touch $1
	@echo CC: $1
endef

$(OBJDIR)/%.o: %.c $(OBJDIR)/%.d $(MAKEFILE) $@
	$(call compilecc,$@,$<,$*)

$(OBJDIR)/%.d: ;

.PHONY: all run clean force $(PROJECT)

all: $(EXECUTABLE)
	@echo Done.

dbview: all

.PRECIOUS: $(OBJDIR)/%.d

-include $(wildcard $(foreach ext, $(SRCEXTS), $(patsubst %.$(ext), $(OBJDIR)/%.d, $(filter %.$(ext), $(SOURCES)))))

$(EXECUTABLE): $(OBJECTS) $(MAKEFILE) $@
ifeq ($(filter-out %.c,$(SOURCES)),$(blank))
	@$(CC) -o $@ $(OBJECTS) $(CFLAGS) $(LDFLAGS)
	@echo "CC: $@ (executable)"
else
	@$(CXX) -o $@ $(OBJECTS) $(CXXFLAGS) $(LDFLAGS)
	@echo CXX: $@
endif

run: $(EXECUTABLE)
	./$(EXECUTABLE) -f ./build/mynewdb.db -n -a "Lil Timmy,225 W Cheshire St,38"
	./$(EXECUTABLE) -f ./build/mynewdb.db -l -u "Lil Timmy|225 W Cheshire St|50"
	./$(EXECUTABLE) -f ./build/mynewdb.db -l -d "Lil Timmy"

clean:
	@rm -rf $(EXECUTABLE)
	@rm -rf $(BUILDDIR)
	@echo Cleaned.

force: clean all

info:
	@echo "Project: $(PROJECT)"

define print
	@echo $1

endef

, = ,
blank =

space = $(blank) $(blank)
$(space) = $(space)
