INC_DIR:= include/ 
WEB_INC_DIR:= include/Web/
SRC_DIR:= src/
SRCS:=$(wildcard src/*.cc) $(wildcard src/net/*.cc) $(wildcard src/threadpool/*.cc)  $(wildcard src/Web/*.cc)
OBJS:= $(patsubst %.cc, %.o, $(SRCS))
LIBS:= -llog4cpp -lpthread -lhiredis

CXX:=g++

CXXFLAGS:= -w -g  $(addprefix -I , $(INC_DIR)) -I $(WEB_INC_DIR) $(LIBS) 

EXE:=bin/SearchEngine.exe

$(EXE):$(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(CXXFLAGS)

clean:
	rm -rf $(EXE)
	rm -rf $(OBJS)
