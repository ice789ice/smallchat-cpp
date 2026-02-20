# SmallChat (Day7)

一个基于 TCP 的小型终端聊天室。支持昵称、多人聊天、`/quit` 退出。

## 架构模式
- **事件驱动/反应器 (Reactor) + 单线程 I/O 多路复用**  
  服务端使用 `select` 监听监听套接字与已连接客户端，事件就绪后分发处理。
- **分层结构**  
  `main -> Server -> Connection`，`Server` 负责连接管理和广播逻辑，`Connection` 负责单个连接的读写细节。

## 设计模式（轻量使用）
- **Reactor 模式**：`select` 驱动事件循环，按可读事件处理连接。
- **Facade/封装**：`Connection` 封装 socket 细节，对 `Server` 提供简化接口。

## 功能列表
- 多客户端连接
- 昵称登记（首次输入作为昵称）
- 群聊广播（只发给其他客户端）
- `/quit` 退出并通知其他人
- 服务端打印连接和消息日志

## 编译
```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp server.cpp connection.cpp -o server
g++ -std=c++17 -Wall -Wextra -pedantic client.cpp -o client
```

## 运行
启动服务端：
```bash
./server
```

另开多个终端启动客户端：
```bash
./client
```

## 使用说明
1. 客户端启动后先输入昵称并回车。
2. 随后输入消息即可聊天。
3. 输入 `/quit` 退出聊天室。

## 文件结构
- `main.cpp`：程序入口
- `server.h/.cpp`：服务端核心逻辑（监听、连接管理、广播）
- `connection.h/.cpp`：单连接读写封装
- `client.cpp`：客户端（终端聊天交互）
