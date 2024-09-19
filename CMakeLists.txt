cmake_minimum_required(VERSION 3.20)
project(http_server)

find_package(glog 0.8.0 REQUIRED)

#add_executable(server src/server.cpp)

# add_library(listener src/listener.cpp)
# target_include_directories(listener PUBLIC src)

add_executable(server src/main.cpp
                        src/listener.cpp
                        src/epoll.cpp    
                        src/server.cpp
                        src/http_server.cpp
                        src/file_manager.cpp
                        src/http_message/http_message.cpp
                        src/http_message/http_request.cpp
                        src/http_message/http_response.cpp)

target_include_directories(server 
                            PUBLIC src
                            )

target_link_libraries(server 
                        glog::glog
                    )         
                    
# target_compile_options(server PRIVATE -fsanitize=address)
# target_link_options(server PRIVATE -fsanitize=address)