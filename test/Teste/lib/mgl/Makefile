CXX := clang++

INCLUDES := \
	-I/usr/include

LIBS := \
	-L/usr/lib -lOpenGL -lglfw -lGLEW -lassimp

INC := *.hpp
SRC := *.cpp

OUT := libmgl.so

all : release

release : CXXFLAGS := -O2 -D NDEBUG
release : $(OUT)

debug : CXXFLAGS := -g -Wall -D DEBUG
debug : $(OUT)

$(OUT) : $(SRC) $(INC)
	$(CXX) $(INCLUDES) $(CXXFLAGS) -fPIC -shared $(LIBS) -o $(OUT) $(SRC)

clean:
	$(RM) $(OUT)
