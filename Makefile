.RECIPEPREFIX := >

SRC_DIR := src/compiler
BUILD_DIR := build
BUILD_DIR_STAMP := $(BUILD_DIR)/.dir

BISON_SRC := $(SRC_DIR)/parser.y
LEX_SRC := $(SRC_DIR)/lexer.l

BISON_C := $(BUILD_DIR)/parser.tab.c
BISON_H := $(BUILD_DIR)/parser.tab.h
BISON_REPORT := $(BUILD_DIR)/parser.output
LEX_C := $(BUILD_DIR)/lexer.yy.c

SRCS := \
  $(SRC_DIR)/main.c \
  $(SRC_DIR)/ast.c \
  $(SRC_DIR)/interpreter.c \
  $(SRC_DIR)/codegen.c \
  $(BISON_C) \
  $(LEX_C)

CFLAGS := -std=c11 -O2 -I$(SRC_DIR) -I$(BUILD_DIR)
LDFLAGS := -lm

ifeq ($(OS),Windows_NT)
TARGET := hindiscriptc.exe
else
TARGET := hindiscriptc
endif

.PHONY: all build clean rebuild test demo

all: build

build: $(TARGET)

$(BUILD_DIR_STAMP):
>powershell -NoProfile -Command "New-Item -ItemType Directory -Force '$(BUILD_DIR)' | Out-Null; Set-Content -Path '$(BUILD_DIR)/.dir' -Value ''"

$(BISON_C) $(BISON_H) $(BISON_REPORT): $(BISON_SRC) | $(BUILD_DIR_STAMP)
>bison -d -v -o $(BISON_C) $(BISON_SRC)
>powershell -NoProfile -Command "if (Test-Path parser.output) { Move-Item -Force parser.output '$(BISON_REPORT)' }"

$(LEX_C): $(LEX_SRC) | $(BUILD_DIR_STAMP)
>flex -o$(LEX_C) $(LEX_SRC)

$(TARGET): $(SRCS)
>gcc $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

test: build
>powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test.ps1

demo: build
>powershell -NoProfile -ExecutionPolicy Bypass -File scripts/demo.ps1

clean:
>powershell -NoProfile -Command "Remove-Item -Force -ErrorAction SilentlyContinue '$(TARGET)','.hs_tmp_gen.c','.hs_tmp_gen.exe','lex.yy.c','parser.output'; Remove-Item -Recurse -Force -ErrorAction SilentlyContinue '$(BUILD_DIR)','-p'"

rebuild: clean build
