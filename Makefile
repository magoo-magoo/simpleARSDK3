EXEC_NAME = SimpleSumo

SDK_DIR =correct_path/ARSDKBuildUtils  # PUT CORRECT PATH
IDIR =./

RM = rm -f

CXX = clang++ 

CXXFLAGS += -W -Wall
CXXFLAGS += -std=c++11 
CXXFLAGS +=-I$(IDIR) -I $(SDK_DIR)/Targets/Unix/Install/include

LDIR =$(SDK_DIR)/Targets/Unix/Install/lib

LIBRARY_PATH +=$(SDK_DIR)/Targets/Unix/Install/lib

LDFLAGS +=-L$(LDIR) -larsal -larcommands -larnetwork -larnetworkal -lardiscovery
LDFLAGS_DBG += -L$(SDK_DIR)/Targets/Unix/Install/lib -larsal_dbg -larcommands_dbg -larnetwork_dbg -larnetworkal_dbg -lardiscovery_dbg



SRCS		= JumpingNetworkManager.cpp main.cpp

OBJS		= $(SRCS:.cpp=.o)


all: $(EXEC_NAME)

$(EXEC_NAME): $(OBJS)
	$(CXX) $(OBJS) -o $(EXEC_NAME) $(LDFLAGS) 

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(EXEC_NAME)

re: fclean all

run:
	LD_LIBRARY_PATH=$(LDIR) ./$(EXEC_NAME)
