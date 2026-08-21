# cmake/FetchGoogleTest.cmake
include(FetchContent)

FetchContent_Declare(
googletest
GIT_REPOSITORY https://github.com/google/googletest.git
GIT_TAG        v1.15.2                       # tag fixa (não "main") = reprodutível
SYSTEM                                        # headers do GTest = system headers 
)

FetchContent_MakeAvailable(googletest)         # define os alvos GTest::gtest / GTest::gtest_main
include(GoogleTest)                            # fornece gtest_discover_tests()
