ncc: ncc.c

test: ncc
		./test.sh

clean:
		rm -f 9cc *.o *~ tmp*