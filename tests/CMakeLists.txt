include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG v1.15.2
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB CAMKO_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp")

add_executable(camko_tests ${CAMKO_TEST_SOURCES})

set_target_properties(camko_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

target_link_libraries(camko_tests
	PRIVATE
	camko_core
	GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(camko_tests)