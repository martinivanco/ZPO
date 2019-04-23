CC=g++
CFLAGS=-Wall -I. -I/usr/local/include
LIBS=-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio
OBJ=main.o stabilize.o exposure_correct.o

all: timelapse oclean

timelapse: $(OBJ)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

oclean:
	rm -f *.o

clean: oclean
	rm -f vidfilter
