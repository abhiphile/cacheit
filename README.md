# README.md

# CacheIt

CacheIt is a simple Redis-like in-memory data store implemented in C++. It supports basic operations such as setting, getting, and deleting key-value pairs, along with a few additional data structures like linked lists and hash maps.

## Features

- TCP server for handling client connections
- Command parsing and execution
- Support for multiple data structures:
  - Linked List
  - Hash Map
- Non-blocking I/O using epoll
- Logging functionality with different log levels

## Project Structure

```
cacheit
├── include
│   ├── command.hpp
│   ├── conn.hpp
│   ├── datastructures
│   │   ├── linked_list.hpp
│   │   └── hashmap.hpp
│   ├── epoll_manager.hpp
│   ├── logger.hpp
│   ├── server.hpp
│   └── utils.hpp
├── src
│   ├── client.cpp
│   ├── command.cpp
│   ├── conn.cpp
│   ├── datastructures
│   │   ├── linked_list.cpp
│   │   └── hashmap.cpp
│   ├── epoll_manager.cpp
│   ├── logger.cpp
│   ├── server.cpp
│   └── utils.cpp
├── CMakeLists.txt
└── README.md
```

## Setup Instructions

1. Clone the repository:
   ```
   git clone <repository-url>
   cd cacheit
   ```

2. Create a build directory and navigate into it:
   ```
   mkdir build
   cd build
   ```

3. Run CMake to configure the project:
   ```
   cmake ..
   ```

4. Build the project:
   ```
   make
   ```

## Usage

To run the server, execute the following command:
```
./cacheit/src/server
```

To connect to the server and send commands, you can use the client:
```
./cacheit/src/client
```

## Supported Commands

### String (Key-Value)
- `set <key> <value>`: Set a value for a key
- `get <key>`: Get the value for a key
- `del <key>`: Delete a key

### Linked List (Redis-style)
- `lpush <list> <value>`: Push value to the end of a list
- `lrange <list> <start> <end>`: Get values from a list (use `0 -1` for all)
- `lrem <list> <count> <value>`: Remove value from a list (count times)

### Set
- `sadd <set> <value1> [value2 ...]`: Add one or more values to a set
- `srem <set> <value1> [value2 ...]`: Remove one or more values from a set
- `smembers <set>`: List all values in a set

### Hash
- `hset <hash> <field> <value>`: Set a field in a hash
- `hget <hash> <field>`: Get a field from a hash
- `hdel <hash> <field>`: Delete a field from a hash
- `hgetall <hash>`: Get all fields and values from a hash

### Sorted Set
- `zadd <zset> <score> <value>`: Add a value with a score to a sorted set
- `zrange <zset> <start> <end>`: Get values by rank (use `0 -1` for all)
- `zrem <zset> <value>`: Remove a value from a sorted set

### Bitmap
- `setbit <bitmap> <offset> <0|1>`: Set a bit at offset
- `getbit <bitmap> <offset>`: Get a bit at offset
- `bitcount <bitmap>`: Count bits set to 1

## Example Session
```
> set hello world
REPLY: set OK
> get hello
REPLY: world
> lpush mylist 42
REPLY: pushed 42 to mylist
> lrange mylist 0 -1
REPLY: 42
> sadd myset a b c
REPLY: added 3
> smembers myset
REPLY: a b c
> hset myhash field1 value1
REPLY: OK
> hget myhash field1
REPLY: value1
> zadd myzset 1.0 foo
REPLY: OK
> zrange myzset 0 -1
REPLY: foo
> setbit mybits 5 1
REPLY: OK
> getbit mybits 5
REPLY: 1
> bitcount mybits
REPLY: 1
```

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any suggestions or improvements.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.