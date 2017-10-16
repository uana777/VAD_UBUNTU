INC=  
LIB= -lpthread  -lm 
  
CC=gcc  
CC_FLAG=-Wall  
  
PRG=vad_test  
OBJ=vad_test.o
  
$(PRG) : $(OBJ)  
	$(CC) $(INC)  -o $@ $(OBJ)  ./src/libvad.a $(LIB)
      
.SUFFIXES: .c .o .cpp  
.cpp.o:  
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o  
  
.PRONY:clean  
clean:  
	@echo "Removing linked and compiled files......"  
	rm -f $(OBJ) $(PRG)  
