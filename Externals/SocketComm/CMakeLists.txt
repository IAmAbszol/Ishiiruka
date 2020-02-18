set(SRC_SOCKET src/InputComm.cpp
	       src/OutputComm.cpp)
add_definitions(-std=c++11)
add_library(SocketComm STATIC ${SRC_SOCKET})
target_include_directories(SocketComm PUBLIC include/)
target_link_libraries(SocketComm sfml-network
                                 sfml-system)
