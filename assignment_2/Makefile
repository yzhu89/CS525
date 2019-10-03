Compiler:= gcc
Source := dberror.c storage_mgr.c test_assign2_1.c test_assign2_2.c 
Object := dberror.o storage_mgr.o test_assign2_1.o test_assign2_2.o 

code_assignment2: $(Object)
	$(Compiler) -o test_assign2_1 $?
	$(Compiler) -o test_assign2_2 $?


%.o: %.c
	$(Compiler) -g -c $<

run: code_assignment1
	./test_assign2_1	
	./test_assign2_2
clean:
	rm -rf test_assign2_1 test_assign2_2 *.o
