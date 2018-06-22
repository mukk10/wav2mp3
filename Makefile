CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS := -lpthread -lmp3lame
CC_FLAGS := -Wall -g
INCLUDES := -Iinc -I/usr/include/lame
MAIN := bin/app_wave_to_mp3_multithreaded.exe

$(MAIN): $(OBJ_FILES)
	g++ -o $@ $^ $(LD_FLAGS)

obj/%.o: src/%.cpp
	g++ $(CC_FLAGS) $(INCLUDES) -c -o $@ $<

clean:
	$(RM) obj/*.o $(MAIN)
