CXX := g++
CXX_FLAGS := -Wall -Wextra -std=c++23 -ggdb3 -O3

DIRS = /app/abacos /app/abacos/app/abacos-dds

SOURCE = ./src
BIN = ./bin

MAIN_PUBLISHER = abacos-dds-publisher
MAIN_SUBSCRIBER = abacos-dds-subscriber

SOURCE_FILES = packet

INCLUDEDIRS = $(foreach dir, $(DIRS), $(addsuffix /include, $(dir)))

INCLUDES = $(foreach dir, $(INCLUDEDIRS), $(addprefix -I, $(dir)))

LIBS = ddscxx ddsc

LINKLIBS = $(foreach lib, $(LIBS), $(addprefix -l, $(lib)))

publisher: 
	$(CXX) $(CXX_FLAGS) $(SOURCE)/$(MAIN_PUBLISHER).cpp ./src/packet.cpp -o $(BIN)/$(MAIN_PUBLISHER).o $(INCLUDES) $(LINKLIBS)

subscriber:
	$(CXX) $(CXX_FLAGS) $(SOURCE)/$(MAIN_SUBSCRIBER).cpp ./src/packet.cpp -o $(BIN)/$(MAIN_SUBSCRIBER).o $(INCLUDES) $(LINKLIBS)