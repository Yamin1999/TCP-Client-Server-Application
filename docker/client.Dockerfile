FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY include/ /app/include/
COPY src/client/ /app/src/client/
COPY Makefile /app/

RUN mkdir -p build && make client

# Wait for server to be ready
CMD ["sh", "-c", "sleep 5 && ./build/client /app/data.csv /app/logs/client.log"]