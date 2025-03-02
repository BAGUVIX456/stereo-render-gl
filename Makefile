CXX := g++
LINKER := -lglfw -lGL -lm -lX11 -lpthread -lXrandr -ldl -lassimp
SO_FILE_DIR := -L/usr/lib/x86_64-linux-gnu
DEBUG := -g
SRC := src
SHADER := src/shader/shader.cpp
CAMERA := src/camera
MODEL := src/model/model.cpp
MESH := src/model/mesh.cpp
OUT := gl
BUILD := build

run: $(OUT)
	__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia ./$(BUILD)/$(OUT)

$(OUT): $(SRC)/main.cpp $(SHADER) $(MODEL) $(SRC)/glad.c $(MESH)
	if [ ! -d "$(BUILD)" ]; then mkdir $(BUILD); fi
	$(CXX) $(DEBUG) $^ -o $(BUILD)/$(OUT) $(LINKER) 

clean:
	rm -rf $(BUILD)
