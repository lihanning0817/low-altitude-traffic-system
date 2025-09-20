# 使用官方 MySQL 8.0 镜像作为基础镜像
FROM mysql:8.0

# 设置环境变量
ENV MYSQL_ROOT_PASSWORD=rootpassword
ENV MYSQL_DATABASE=low_altitude_traffic
ENV MYSQL_USER=appuser
ENV MYSQL_PASSWORD=apppassword

# 复制初始化脚本
COPY ./database/schema.sql /docker-entrypoint-initdb.d/

# 暴露 MySQL 默认端口
EXPOSE 3306

# 启动 MySQL 服务
CMD ["mysqld"]