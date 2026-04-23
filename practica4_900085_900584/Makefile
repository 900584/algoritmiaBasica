CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11

SRC_DIR = src
BIN_BNB = formarEquipos
BIN_BF = verificarBruteforce

ifeq ($(OS),Windows_NT)
BIN_BNB_EXE = $(BIN_BNB).exe
BIN_BF_EXE = $(BIN_BF).exe
RM_BIN = if exist "$(BIN_BNB_EXE)" del /Q "$(BIN_BNB_EXE)" & if exist "$(BIN_BF_EXE)" del /Q "$(BIN_BF_EXE)"
RM_RESULTS = if exist "resultados\\*.txt" del /Q "resultados\\*.txt"
else
BIN_BNB_EXE = $(BIN_BNB)
BIN_BF_EXE = $(BIN_BF)
RM_BIN = rm -f "$(BIN_BNB_EXE)" "$(BIN_BF_EXE)"
RM_RESULTS = rm -f resultados/*.txt
endif

all: $(BIN_BNB) $(BIN_BF)

$(BIN_BNB): $(SRC_DIR)/formarEquipos.c
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_BF): $(SRC_DIR)/verificar_bruteforce.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM_BIN)
	$(RM_RESULTS)

.PHONY: all clean
