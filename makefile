all: fifo_task

clean:
	rm -f fifo_task

fifo_task: main.cpp
	g++ main.cpp -lpthread -o fifo_task