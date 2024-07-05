# InstaX2-OpenCV-Linux
### A project integrating Insta X2 360 camera SDK with OpenCV for video streaming on Linux.

This project integrates the Insta X2 360 camera SDK with OpenCV for video streaming on Linux. It provides a framework to capture and process video streams from the Insta X2 360 camera using OpenCV libraries. The project focuses on leveraging the camera SDK capabilities for streaming and demonstrates real-time video processing using OpenCV.

## Requirements
- Insta X2 360 Camera SDK: Ensure you have the official SDK installed. You can obtain it from the manufacturer's website.
- OpenCV 4: Required for image and video processing tasks. Install using your system's package manager or build from source.
- C++ Compiler: This project is written in C++11 and requires a compatible compiler (e.g., g++).
- ffmpeg: ffmpeg is utilized for video decoding tasks in conjunction with the camera SDK. Ensure ffmpeg is installed and accessible in your system's PATH.

## Installation
### Cloning the Repository
Clone the repository to your local machine:
```
git clone https://github.com/username/InstaX2-OpenCV-Linux.git
cd InstaX2-OpenCV-Linux
```
### Setting Up the SDK and OpenCV
- SDK Installation: Follow the instructions provided with the Insta X2 360 camera SDK to install it on your system. Ensure that the SDK libraries (libCameraSDK.so) and headers are accessible.
- OpenCV Installation: Install OpenCV 4 on your system. You can typically install it using your package manager (e.g., apt-get, yum) or build it from source. Make sure the OpenCV headers and libraries are correctly configured.

## Building the Project
The project includes a Makefile for compilation. Ensure the paths in the Makefile (CXXFLAGS and LDFLAGS) point to the SDK and OpenCV locations on your system.

To build the project, run:

```
make install
// This will compile the project and generate an executable named SDKDemo.
```

## Running the Project
### Starting the Application
Run the compiled executable:
```
make run
// This will connect to the Insta X2 360 camera, start live streaming, and display the processed video feed using OpenCV.
```

## Project Structure
- main.cc: Contains the main application logic, including camera initialization, stream handling, and OpenCV integration.
- Makefile: Compiles the project using g++ with specified flags for SDK and OpenCV.
- README.md: This file, providing instructions and documentation for the project.
## Usage Notes 
- Camera Connection: Ensure the Insta X2 360 camera is connected and accessible before running the application.
- SDK Documentation: Refer to the official Insta X2 360 camera SDK documentation for detailed API usage and additional features.
- OpenCV Usage: Utilize OpenCV functions in process_video_data and display_frame methods for image processing and visualization.
## Contributing
Contributions to this project are welcome. Fork the repository, make improvements, and submit a pull request. Ensure any changes align with the project's goals and coding standards.
