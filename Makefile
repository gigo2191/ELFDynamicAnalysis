TARGET = Analyzer
CC = g++
FILES = Utils.cpp PcapParser.cpp Strace.cpp Descriptor.cpp Process.cpp
LIBS = -ljsoncpp -lpcap

$(TARGET) : 
	$(CC) -o $(TARGET) $(FILES) $(LIBS) -std=c++11

clean : 
	rm $(TARGET)

all : $(TARGET);

