# Makefile para compilar o projeto com OpenCV e SDK da câmera
CXX = g++
CXXFLAGS = -std=c++11 `pkg-config --cflags opencv4` -I../include
LDFLAGS = `pkg-config --libs opencv4` -L../lib -lCameraSDK -ludev -lavcodec -lavformat -lavutil -lswscale 

TARGET = SDKDemo
SOURCES = main.cc

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

install: $(TARGET)
	sudo chmod +x $(TARGET)
	@echo "SDKDemo installed."

run: $(TARGET)
	sudo LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):../lib ./$(TARGET)

clean:
	rm -f $(TARGET)
	@echo "Cleaned up."

