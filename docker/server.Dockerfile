FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    libmysqlclient-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY include/ /app/include/
COPY src/server/ /app/src/server/
COPY Makefile /app/

RUN mkdir -p build && make server

EXPOSE 8080

# Pass database connection parameters as environment variables
ENV DB_HOST=mysql \
    DB_USER=root \
    DB_PASS=new_password \
    DB_NAME=user_db

CMD ["./build/server"]