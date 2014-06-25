PROJECT(c4jieba)

ADD_EXECUTABLE(1 aho-corasick-lite.cpp)
ADD_EXECUTABLE(HmmPos HmmPos_test.cpp)

ADD_DEFINITIONS(-std=c++0x -O3)
