cmake_minimum_required(VERSION 3.12)

project(SYN_MV)


find_package(glfw3  REQUIRED)
find_package(OpenGL REQUIRED)


set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH} ${SYN_MV_SOURCE_DIR}/cmake)

if((NOT CMAKE_BUILD_TYPE) AND (NOT $ENV{CMAKE_BUILD_TYPE} STREQUAL ""))
	set(CMAKE_BUILD_TYPE $ENV{CMAKE_BUILD_TYPE})
endif()

include_directories(${SYN_MV_SOURCE_DIR}/include)

file(GLOB SRC ${SYN_MV_SOURCE_DIR}/src/*c)

file(GLOB EXECLIST ${SYN_MV_SOURCE_DIR}/bin/*c)


set(CMAKE_BUILD_TYPE Debug)

if(NOT SRC STREQUAL "")
	set(LIBNAME "syn_mv_core")
	add_library(${LIBNAME} ${SRC})
endif()

foreach(EXEC ${EXECLIST})
	get_filename_component(EXECNAME ${EXEC} NAME_WE)
	add_executable(${EXECNAME} ${EXEC})
	target_link_libraries(${EXECNAME} glfw OpenGL GLEW ${LIBNAME})

	#if(NOT SRC STREQUAL "")
	#	target_link_libraries(${EXECNAME} LINK_PUBLIC ${LIBNAME})
	#endif()

	message("-- Adding executable: ${EXECNAME}")
endforeach(EXEC)

