cmake_minimum_required(VERSION 2.6)

project(triangualte)

set(PACKAGE_VERSION "0.1.0")

########## set compiler flags ##########
if(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /D _CRT_SECURE_NO_WARNINGS")
endif()


#####################################################
#                COMMON Libraries                   #
#####################################################


#################################################################
# Unfolder GUI
#################################################################

set(APP_NAME triangulate)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

set(MY_SRC
    Basic.cpp       main.cpp        polygon.cpp     triangulation.cpp
)

add_executable(${APP_NAME} ${MY_SRC})
#add_dependencies(${APP_NAME})
#target_link_libraries(${APP_NAME} )
