CXX := g++
CXX_FLAGS := -Wall -Wextra -std=c++23 -ggdb3 -O0

DIRS = /app/abacos /app/abacos/app/abacos-vsomeip

SOURCE = ./src
BIN = ./bin

MAIN_PUBLISHER = abacos-vsomeip-publisher
MAIN_SUBSCRIBER = abacos-vsomeip-subscriber

INCLUDEDIRS = $(foreach dir, $(DIRS), $(addsuffix /include, $(dir)))

INCLUDES = $(foreach dir, $(INCLUDEDIRS), $(addprefix -I, $(dir)))

LIBS = vsomeip3

LINKLIBS = $(foreach lib, $(LIBS), $(addprefix -l, $(lib)))

publisher: 
	$(CXX) $(CXX_FLAGS) $(SOURCE)/$(MAIN_PUBLISHER).cpp -o $(BIN)/$(MAIN_PUBLISHER).o $(INCLUDES) $(LINKLIBS)

subscriber:
	$(CXX) $(CXX_FLAGS) $(SOURCE)/$(MAIN_SUBSCRIBER).cpp -o $(BIN)/$(MAIN_SUBSCRIBER).o $(INCLUDES) $(LINKLIBS)
