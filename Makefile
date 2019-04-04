CC=g++
CFLAGS=-Wall -I. -I/usr/local/include
LIBS=-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio
OBJ=main.o filter.o filters1.o filters2.o

all: vidfilter oclean

vidfilter: $(OBJ)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

oclean:
	rm -f *.o

clean: oclean
	rm -f vidfilter
