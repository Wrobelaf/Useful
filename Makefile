ifeq ($(CONFIG),Debug)
    CFLAGS = -g
else
    CFLAGS = 
endif

LFLAGS = 
CC = g++

# Suffixes
.SUFFIXES:
.SUFFIXES: .o .cpp

$(CONFIG)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(CONFIG)/%.d: %.cpp
	-mkdir -p ./$(CONFIG)
	$(CC) -MM $(CFLAGS) $< \
    | sed "s/\($*\)\.o[ :]*/$(CONFIG)\/\1.o $(CONFIG)\/$(@F): /g" > $@


SRC = Useful.cpp
OBJ = $(SRC:%.cpp=$(CONFIG)/%.o)

libUseful.so : $(OBJ)
	$(CC) $(LFLAGS) -shared $(OBJ) -o $(CONFIG)/$@;
	
clean:
	rm -fR $(CONFIG)/*

ifneq ($(MAKECMDGOALS),clean)
include $(SRC:%.cpp=$(CONFIG)/%.d)
endif
