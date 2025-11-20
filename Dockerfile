# -*- mode: dockerfile; -*-

# This should fully encapsulate building a widely compatible GNU/Linux binary

FROM ubuntu:18.04

RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
  apt-get install -y --no-install-recommends \
  build-essential \
  pkg-config \
  libgtk+-3-dev \
  libcurl4-openssl-dev \
  libsqlite3-dev \
  wget \
  git

RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
  apt-get install -y --no-install-recommends \
  ca-certificates

RUN mkdir /app

RUN echo Building for hotfix 20251120
# RUN git clone --depth=1 https://github.com/ahungry/bg3-linux-ae /app

WORKDIR /app
COPY bg3_linux_ae.cpp /app/
COPY Makefile /app/
COPY package.sh /app/
COPY bg3-linux-ae.sh /app/

RUN make -Bj
RUN ./package.sh

CMD ["echo"]
