cmake_minimum_required(VERSION 3.20)
project(camko VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
		marco
		GIT_REPOSITORY https://github.com/MarcoLabs/MarcoLib.git
		GIT_TAG main
)

set(MARCO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MARCO_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(marco)

file(GLOB_RECURSE CAMKO_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE CAMKO_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

list(FILTER CAMKO_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

if(CAMKO_SOURCES)
	add_library(camko_core STATIC ${CAMKO_SOURCES} ${CAMKO_HEADERS})
	target_include_directories(camko_core
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:include>
		PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/src
	)
	target_compile_options(camko_core PRIVATE -Wall -Wextra -Wpedantic)
	target_link_libraries(camko_core PUBLIC marco)
else()
	add_library(camko_core INTERFACE)
	target_include_directories(camko_core INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	)
	target_link_libraries(camko_core INTERFACE marco)
endif()

add_executable(camko src/main.cpp)
target_link_libraries(camko PRIVATE camko_core)
target_compile_options(camko PRIVATE
	-Wall -Wextra -Wpedantic
)

include(GNUInstallDirs)
install(TARGETS camko
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

option(CAMKO_BUILD_EXAMPLES "Build camko example executables" OFF)
option(CAMKO_BUILD_TESTS "Build camko tests" ON)

if(CAMKO_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

if(CAMKO_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()