FROM gcc:latest AS builder

WORKDIR /home/client
COPY . .

RUN apt-get update && apt-get install -y cmake
RUN cmake .
RUN make

FROM scratch
COPY --from=builder /home/client/Projeto1_Client /
