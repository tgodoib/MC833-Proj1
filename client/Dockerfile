FROM gcc:latest AS builder

WORKDIR /home/client
COPY . .

RUN apt-get update && apt-get install -y cmake
RUN cmake .
RUN make
