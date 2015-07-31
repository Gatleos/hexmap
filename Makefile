# Dependencies:
#	libsfml-dev
#	libjsoncpp-dev
#	libpugixml-dev

# For debug build:
#	"make" or "make all"
# For release build:
#	"make RELEASE=1" or "make R=1"
# Create a distribution (in folder "dist"):
#	"make install"
# Clean up:
#	"make clean" or "make clean R=1"
# Clean out distribution:
#	"make cleandist"

SFMLVER := 2.3
OUTD := hexmap-d
OUTR := hexmap
CXXFLAGS += -std=c++11
ifneq ("$(or $(RELEASE),$(R))","")
	CXXFLAGS += -O3 -D NDEBUG
	OBJDIR := obj/release/
	OUT := $(OUTR)
	LIB := -lsfml-graphics -lsfml-window -lsfml-system
else
	CXXFLAGS += -D _DEBUG -g
	OBJDIR := obj/debug/
	OUT := $(OUTD)
	LIB := -lsfml-graphics-d -lsfml-window-d -lsfml-system-d
endif

SRC := $(wildcard src/*.cpp)
OBJ := $(subst src/,$(OBJDIR), $(subst .cpp,.o, $(SRC)))
INC := 
LIB += -ljsoncpp -lpugixml -lsfgui

.PHONY: all clean

all: $(OBJ)
	$(CXX) -o $(OUT) $^ $(LIB)

$(OBJDIR)%.o: src/%.cpp
	@mkdir -p -m 777 $(@D)
	$(CXX) -c $< -o $@ $(INC) $(CXXFLAGS)

clean:
	rm -f $(OUT)
	rm -f -rf $(OBJDIR)

cleandist:
	rm -f -R dist/

install:
ifeq ("$(wildcard hexmap)","")
	$(error File "$(OUTR)" could not be found! Run "make R=1" first)
endif
	mkdir -p -m 777 dist/
	cp -R -p data/. dist/data/
	cp -p $(OUTR) dist/
	mkdir -p -m 777 dist/License/SFML-$(SFMLVER)
	cp -p SFML-$(SFMLVER)/license.txt dist/License/SFML-$(SFMLVER)/
	cp -p SFML-$(SFMLVER)/license.txt dist/License/SFML-$(SFMLVER)/
	mkdir -p -m 777 dist/License/jsoncpp
	cp -p jsoncpp/LICENSE.txt dist/License/jsoncpp/
	mkdir -p -m 777 dist/License/pugixml
	cp -p pugixml/readme.txt dist/License/pugixml/
	mkdir -p -m 777 dist/License/cpp11-range
	cp -p cpp11-range/LICENSE.txt dist/License/cpp11-range/
