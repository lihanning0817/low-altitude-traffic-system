# 使用 Ubuntu 20.04 作为基础镜像
FROM ubuntu:20.04

# 设置非交互式安装
ENV DEBIAN_FRONTEND=noninteractive

# 安装依赖项
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libmysqlcppconn-dev \
    libjsoncpp-dev \
    libssl-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# 克隆 Drogon 框架
RUN git clone https://github.com/an-tao/drogon.git
WORKDIR /drogon
RUN git submodule update --init
RUN mkdir build
WORKDIR /drogon/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release
RUN make -j$(nproc)
RUN make install

# 创建应用目录
WORKDIR /app
COPY ./backend .

# 构建应用
RUN mkdir build
WORKDIR /app/build
RUN cmake ..
RUN make -j$(nproc)

# 暴露 API 端口
EXPOSE 8080

# 启动应用
CMD ["./lowaltitudetraffic_server"]