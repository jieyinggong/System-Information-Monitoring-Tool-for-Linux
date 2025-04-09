CC = gcc
CFLAGS = -Wall -Wextra -std=c99 
LDFLAGS = -lm

SRC = myMonitoringTool.c sysInfoProducer.c sysInfoConsumer.c CLA.c cpu.c core.c memory.c pipeTool.c signalHandler.c

OBJ = $(SRC:.c=.o)

TARGET = myMonitoringTool

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)