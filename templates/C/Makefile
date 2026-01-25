# GNU Makefile by yilisharcs! This is a little template I'm making to understand
# how to write such things. To that end, this file is littered with odd opinions
# and comments that explain the syntax, structure, and my thought process for my
# own sake. And for future me, who is also me.

# `:=` means "Set this variable NOW."
# `?=` means "Set this ONLY if the user didn't provide it."

# All compiler flags below assume `gcc` as the compiler
CC := gcc
# Default profile from list:
#     - debug
#     - release
#     - tiny
PROFILE ?= debug

OBJDIR  := build/$(PROFILE)
BINPATH := $(OBJDIR)/GENIT_PKG

# Auto-detect all C files in the src/ directory
SRCS := $(wildcard src/*.c)
# Transform the list of source files into a list of object files:
#     src/main.c -> build/debug/main.o
OBJS := $(SRCS:src/%.c=$(OBJDIR)/%.o)
# Generate a list of dependency files:
#     $(gcc) => build/debug/main.d
DEPS := $(OBJS:.o=.d)

# Shared flags for all profiles
# Primary learning target is C11
CFLAGS := -std=c11

# .d file generation
# Track the dependency list dynamically
CFLAGS += -MMD
# Prevent stale header bugs from `-MMD`
CFLAGS += -MP

# Errors and warnings
# Standard warnings
CFLAGS += -Wall
# Even more warnings
CFLAGS += -Wextra
# Warn when a variable shadows another
CFLAGS += -Wshadow
# Check printf/scanf for type mismatches and null arguments
CFLAGS += -Wformat=2
# Warn if an undefined macro is used in #if
CFLAGS += -Wundef
# Treat string literals as const `char*`
CFLAGS += -Wwrite-strings
# Warn if switch cases fall through without comment
CFLAGS += -Wimplicit-fallthrough
# Prevent pointer assumptions
CFLAGS += -fno-strict-aliasing
# Force `(void)` for empty parameter lists
CFLAGS += -Wstrict-prototypes
# Force global functions to be declared before use
CFLAGS += -Wmissing-prototypes

# Shared compiler flags for Release and Tiny profiles
# Disable assertions
RCFLAGS := -DNDEBUG
# Treat warnings as errors for production
RCFLAGS += -Werror
# Enable link-time optimization
RCFLAGS += -flto
# Put each function in its own bucket for the linker
RCFLAGS += -ffunction-sections
# Put each piece of data in its own bucket
RCFLAGS += -fdata-sections

# Shared linker flags for all profiles... there aren't any :'(
LDFLAGS :=

# Shared linker flags for Release and Tiny profiles
# Enable link-time optimization (must be in both Compiler and Linker steps)
RLDFLAGS := -flto
# Tell the linker to throw away dead code (unused buckets)
RLDFLAGS += -Wl,--gc-sections
# Only link libraries that are actually used
RLDFLAGS += -Wl,--as-needed

# Debug: Maximum info, no optimization, memory safety checks
ifeq ($(PROFILE),debug)
	CFLAGS += -O0 -ggdb3
	# Catch memory errors
	CFLAGS += -fsanitize=address
	# Catch undefined behavior
	CFLAGS += -fsanitize=undefined
	# Linker needs sanitizers too to link the libraries
	LDFLAGS += -fsanitize=address -fsanitize=undefined
# Release: Optimize for speed, disable debug asserts
else ifeq ($(PROFILE),release)
	CFLAGS  += $(RCFLAGS) -O2
	LDFLAGS += $(RLDFLAGS)
# Tiny: Optimize for size, strip everything
else ifeq ($(PROFILE),tiny)
	CFLAGS  += $(RCFLAGS) -Os
	LDFLAGS += $(RLDFLAGS)
	# `ld` packs code and data more tightly
	LDFLAGS += -Wl,-z,noseparate-code
	# Remove unique build hash data
	LDFLAGS += -Wl,--build-id=none
	# Omit all symbol info from output
	LDFLAGS += -Wl,--strip-all
endif

# RULES
build: $(BINPATH) tags

# Nushell script for generating ordered ctags
NU_CTAGS := scripts/ctags.nu
tags: $(SRCS) $(wildcard src/*.h)
	@if command -v nu >/dev/null 2>&1 && [ -f $(NU_CTAGS) ]; then \
		echo "Generating ctags..."; \
		nu $(NU_CTAGS); \
	fi

# Link the executable
$(BINPATH): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile source files to objects
# This very file is considered a dependency. Equivalent to `NOB_GO_REBUILD_URSELF`, I believe.
$(OBJDIR)/%.o: src/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Include generated dependency files and fail silently if missing
# Apparently you can just pass `-include` without a target. Cool.
-include $(DEPS)

.PHONY: build clean run debug release tiny
clean:
	rm -rf build/

run: build
	@./$(BINPATH)

# Convenience wrappers for profiles
debug:
	@$(MAKE) --no-print-directory PROFILE=debug build

release:
	@$(MAKE) --no-print-directory PROFILE=release build

tiny:
	@$(MAKE) --no-print-directory PROFILE=tiny build
