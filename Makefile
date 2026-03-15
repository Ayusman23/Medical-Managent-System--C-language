# ============================================================
# Hospital Management System — Makefile
# Usage:
#   make          -> build executable (hms or hms.exe)
#   make clean    -> remove build artefacts
# ============================================================

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -O2
TARGET  = hms

# Detect OS
ifeq ($(OS),Windows_NT)
    TARGET  := hms.exe
    RM      := del /Q
    RMDIR   := rmdir /S /Q
    MKDIR   := if not exist $(OBJDIR) mkdir
    EXT     := .exe
else
    RM      := rm -f
    RMDIR   := rm -rf
    MKDIR   := mkdir -p
    EXT     :=
endif

OBJDIR  = build

# ---- Source files -----------------------------------------------
SRCS =  main.c                          \
        backend/utils.c                  \
        backend/patient.c                \
        backend/doctor.c                 \
        backend/appointment.c            \
        backend/billing.c                \
        backend/ward.c                   \
        backend/staff.c                  \
        frontend/input_handler.c         \
        frontend/display.c               \
        frontend/auth_ui.c               \
        frontend/menu.c

OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

# ---- Rules -------------------------------------------------------
.PHONY: all clean run

all: $(TARGET)
	@echo "Build complete -> $(TARGET)"

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Generic rule: compile any .c -> build/<path>/<name>.o
$(OBJDIR)/%.o: %.c
	@$(MKDIR) $(dir $@) 2>$(NUL) || true
	$(CC) $(CFLAGS) -c $< -o $@

# Windows: 'dir' needs backslashes, '2>NUL' not '2>/dev/null'
ifeq ($(OS),Windows_NT)
NUL = NUL
$(OBJDIR)/%.o: %.c
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CC) $(CFLAGS) -c $< -o $@
else
NUL = /dev/null
endif

clean:
ifeq ($(OS),Windows_NT)
	-$(RM) $(TARGET)
	-$(RMDIR) $(OBJDIR)
else
	$(RM) $(TARGET)
	$(RMDIR) $(OBJDIR)
endif
	@echo "Cleaned."

run: all
	./$(TARGET)
