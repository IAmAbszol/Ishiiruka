set(SRC_SOCKET InputComm.cpp)

add_definitions(-std=c++11)
add_library(SocketComm STATIC ${SRC_SOCKET})
target_link_libraries(SocketComm yaml-cpp
                                 sfml-network
                                 sfml-system)
