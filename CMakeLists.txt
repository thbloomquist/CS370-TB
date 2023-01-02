cmake_minimum_required(VERSION 3.16)   # CMake version check
set(PROJECT_NAME "house")
set(SRC_FOLDER "CS370_Project")
project(${PROJECT_NAME})                 # Create project "basicGeometry"
set(CMAKE_CXX_STANDARD 11)             # Enable c++11 standard
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${SRC_FOLDER}/bin)

#OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

#add include and link directories
if(APPLE)
    find_library(cf_lib CoreFoundation)
    find_library(cg_lib CoreGraphics)
    find_library(cocoa_lib Cocoa)
    find_library(io_lib IOKit)
    link_directories(${CMAKE_SOURCE_DIR}/lib/MacOSX)
elseif(WIN32)
	file(COPY ${CMAKE_SOURCE_DIR}/lib/Win/glew32.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	file(COPY ${CMAKE_SOURCE_DIR}/lib/Win/glfw3.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    link_directories(${CMAKE_SOURCE_DIR}/lib/Win)
endif()
link_directories(${CMAKE_SOURCE_DIR}/common)

#Main
set(SOURCE_FILES ${PROJECT_NAME}.cpp)
set(COMMON_FILES ${CMAKE_SOURCE_DIR}/common/utils.cpp ${CMAKE_SOURCE_DIR}/common/objloader.cpp ${CMAKE_SOURCE_DIR}/common/tangentspace.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${COMMON_FILES})

if(APPLE)
    # Add Apple frameworks
    target_link_libraries(${PROJECT_NAME} ${cf_lib})
    target_link_libraries(${PROJECT_NAME} ${cg_lib})
    target_link_libraries(${PROJECT_NAME} ${cocoa_lib})
    target_link_libraries(${PROJECT_NAME} ${io_lib})

    # Add GLFW and GLEW libraries
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
    target_link_libraries(${PROJECT_NAME} glfw3)
    target_link_libraries(${PROJECT_NAME} glew)
elseif(WIN32)
    # Add GLFW and GLEW libraries
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
    target_link_libraries(${PROJECT_NAME} glfw3)
    target_link_libraries(${PROJECT_NAME} glew32)
else()
    target_link_libraries(${PROJECT_NAME} OpenGL::GL)
    target_link_libraries(${PROJECT_NAME} glfw)
    target_link_libraries(${PROJECT_NAME} GLEW)
endif()



