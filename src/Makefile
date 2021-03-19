TARGET  = main

CC		=	g++
SRC		=	tpx3parser.cc splitter.cc cluster.cc time-walk.cc $(TARGET).cc
OBJ		=	$(patsubst %.cc,%.o,$(filter %.cc,$(SRC))) \
			$(patsubst %.f90,%.o,$(filter %.f90,$(SRC)))

EXE		=	$(TARGET)

CFLAGS	+=	-g -std=c++17 -Wall
LFLAGS	+=  -L.
FFLAGS	+=

%.o : %.f90
	$(FC) $< $(FFLAGS) -c -o $@

%.o : %.cc
	$(CC) $< $(CFLAGS) -c -o $@

all : $(EXE)

$(EXE) : $(OBJ)
	$(CC) $(OBJ) $(LFLAGS) -o $@

clean:
	$(RM) $(OBJ)
	$(RM) *~
