# 使用 Node.js 16 作为基础镜像
FROM node:16

# 设置工作目录
WORKDIR /app

# 复制 package.json 和 package-lock.json
COPY package*.json ./

# 安装依赖
RUN npm install

# 复制源代码
COPY . .

# 构建生产版本
RUN npm run build

# 安装 serve 来运行静态文件
RUN npm install -g serve

# 暴露端口
EXPOSE 5173

# 启动应用
CMD ["serve", "-s", "dist", "-l", "5173"]