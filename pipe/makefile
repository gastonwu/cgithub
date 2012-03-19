all: pipe  pipe_rw fifo
pipe:
	g++ pipe.c -o pipe
pipe_rw:
	g++ pipe_rw.c -o pipe_rw
fifo:
	g++ fifo.c -o fifo -levent -lpthread
clean:
	rm -rf pipe pipe_rw fifo