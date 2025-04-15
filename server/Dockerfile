FROM gcc:latest

RUN apt-get update && apt-get install -y cmake

COPY . /home/server
WORKDIR /home/server

RUN cmake -B/home/server/build -H.
RUN make -C /home/server/build

WORKDIR /home/server/build
ENTRYPOINT ["./Projeto1_Server"]
