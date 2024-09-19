FROM ubuntu:latest

RUN apt-get update && apt-get install -y cmake build-essential git

RUN git clone https://github.com/google/glog.git
RUN cd glog && \
cmake -S . -B build -G "Unix Makefiles" && \
cmake --build build && \
cmake --build build --target install

WORKDIR /root

RUN mkdir -p /root/server/src /root/server/build

COPY src /root/server/src
COPY CMakeLists.txt /root/server/

RUN cd /root/server/build && \
cmake .. && \
cmake --build .

RUN mkdir -p /root/workdir

ENTRYPOINT [ "/root/server/build/server" ]


