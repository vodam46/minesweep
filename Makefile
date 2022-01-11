



CC=cc
OUT=out

out: main.o
	$(CC) main.o -o $(OUT)

clean:
	rm -Rf *.o $(OUT)

run: $(OUT)
	./out
