#$Author: behle $
#$Date: 2007-03-01 17:12:27 +0100 (Thu, 01 Mar 2007) $


# Compiler g++
CCC = g++
COMPILER_FLAGS = -Wall -O3 -Wno-deprecated

#the number of sons of the signature MDD nodes can be fixed, this is recommended!
# (this might lead to a shorter runtime and it definitely leads to less memory consumption but a worse hitrate)
COMPILER_FLAGS += -DFIX_NUMBER_OF_SIGMDDNODE_SONS

#in case you have problems with the timer code, comment the following line
COMPILER_FLAGS += -DUSE_TIMER


########## NO CHANGES SHOULD BE NECESSARY BELOW THIS LINE ##########

GMP_LIBS = -lgmpxx -lgmp

CCCFLAGS = $(COMPILER_FLAGS) -I.

LDFLAGS = $(COMPILER_FLAGS) $(GMP_LIBS)

SOURCES = $(wildcard *.cpp)

OBJECTS = $(addsuffix .o, $(basename $(SOURCES)))


azove2:  $(OBJECTS)
	$(CCC) -o $@ $^ $(LDFLAGS)
  
clean:
	rm -f *~ *.o

%.o: %.cpp
	$(CCC) $(CCCFLAGS) -c $< -o $@
