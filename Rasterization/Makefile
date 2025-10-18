CXX = g++
NVCC = nvcc
CXXFLAGS = -std=c++23 -O2 # -Wall -Wextra
NVCCFLAGS = -std=c++20 -O2 -arch=sm_89 # -arch=sm_89 RTX 4090
# CXXFLAGS = -std=c++23 -g -O0 -Wall -Wextra
# NVCCFLAGS = -std=c++20 -g -O0 -arch=sm_89 -G -Xcompiler "-Wall -Wextra"
LDFLAGS = -Xcompiler -static-libgcc -Xcompiler -static-libstdc++

INCLUDE_DIR = include
SRC_DIR = src
CU_DIR = cu
BUILD_DIR = build
THIRD_PARTY_DIR = third_party

OBJ_DIR = build/obj

CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
CPP_OBJECTS = $(CPP_SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o) $(OBJ_DIR)/main.o

CU_SOURCES = $(wildcard $(CU_DIR)/*.cu)
CU_OBJECTS = $(CU_SOURCES:$(CU_DIR)/%.cu=$(OBJ_DIR)/%.cu.o)

HEADERS = $(wildcard $(INCLUDE_DIR)/*.h) $(wildcard $(INCLUDE_DIR)/*.hpp)
OBJECTS = $(CPP_OBJECTS) $(CU_OBJECTS)

TARGET = t

$(TARGET): $(OBJECTS)
	$(NVCC) $(NVCCFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -I$(THIRD_PARTY_DIR) -I$(CU_DIR) -c $< -o $@

$(OBJ_DIR)/%.cu.o: $(CU_DIR)/%.cu
	$(NVCC) $(NVCCFLAGS) -I$(INCLUDE_DIR) -I$(THIRD_PARTY_DIR) -I$(CU_DIR) -c $< -o $@

$(OBJ_DIR)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -I$(THIRD_PARTY_DIR) -I$(CU_DIR) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/obj/*.o $(BUILD_DIR)/obj/*.cu.o

.PHONY: all clean rebuild