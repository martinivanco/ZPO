CC=g++
CFLAGS=-Wall -I. -I/usr/local/include
LIBS=-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio

all: vidfilter oclean

vidfilter: main.o filter.o filters1.o filters2.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

main.o: main.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

filter.o: filter.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

filters1.o: filters1.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

filters2.o: filters2.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

oclean:
	rm -f *.o

clean: oclean
	rm -f vidfilter

# vidfilter: playground.o filters1.o filters2.o
# 	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# playground.o: playground.cpp
# 	$(CC) $(CFLAGS) -c $^ -o $@

# filters1.o: filters1.cpp
# 	$(CC) $(CFLAGS) -c $^ -o $@

# filters2.o: filters2.cpp
# 	$(CC) $(CFLAGS) -c $^ -o $@