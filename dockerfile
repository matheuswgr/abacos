FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /app

# ------------------------------------------------------------
# 1. System dependencies
# ------------------------------------------------------------
RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y \
      git \
      cmake \
      build-essential \
      g++ \
      libboost-all-dev \
      ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# ------------------------------------------------------------
# 2. Build & install Cyclone DDS (core)
# ------------------------------------------------------------
RUN git clone https://github.com/eclipse-cyclonedds/cyclonedds.git && \
    cd cyclonedds && \
    mkdir build && cd build && \
    cmake .. \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DBUILD_TESTING=OFF && \
    make && \
    make install

# ------------------------------------------------------------
# 3. Build & install Cyclone DDS C++ bindings
# ------------------------------------------------------------
RUN git clone https://github.com/eclipse-cyclonedds/cyclonedds-cxx.git && \
    cd cyclonedds-cxx && \
    mkdir build && cd build && \
    cmake .. \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DBUILD_EXAMPLES=OFF && \
    make && \
    make install

# ------------------------------------------------------------
# 4. Dynamic loader configuration
# ------------------------------------------------------------
RUN echo "/usr/local/lib" > /etc/ld.so.conf.d/cyclonedds.conf && \
    ldconfig

# ------------------------------------------------------------
# 5. Build vsomeip
# ------------------------------------------------------------
RUN git clone https://github.com/COVESA/vsomeip.git && \
    cd vsomeip && \
    mkdir build && cd build && \
    cmake .. && \
    make

# ------------------------------------------------------------
# 6. Application workspace
# ------------------------------------------------------------
WORKDIR /app/abacos
COPY . .

