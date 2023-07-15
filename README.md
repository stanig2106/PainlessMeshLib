# PainlessMeshLib 

This is a wrapper of [painlessmeshboost](https://gitlab.com/painlessMesh/painlessmeshboost).  
This wrapper is useful if you want to use painlessMeshBoost as a library in your own project.

to compile, `cmake .` and `make`, then use `libpainlessMeshBoost.a`

## Usage

To use the `PainlessMeshLib` library, you first need to include the header in your project with:

```cpp
#include <painlessMesh.h>
```

The library provides a class `PainlessMeshLib` with an easy-to-use API. Here are the basic steps:

1. **Initialize PainlessMeshLib:** This can be done using the `PainlessMeshLib` constructor. Provide the port number and the path to the OTA directory where firmware files are located.

```cpp
PainlessMeshLib server(5555, "/path/to/firmware/directory");
```

2. **Start the server:** You can start the server by calling the `start_server` method. It will begin the mesh network server on the port defined during initialization. It also sets up the OTA firmware distribution system, and starts handling incoming mesh network connections.

```cpp
server.start_server();
```

3. **Send messages:** To send messages to a specific node, use the `send_message` method. This method takes the target node ID and the message as parameters.

```cpp
server.send_message(targetNodeId, "Hello, Mesh Network!");
```

4. **Add callbacks:** `PainlessMeshLib` allows you to register callbacks for when messages are received or new connections are made. Use `add_message_callback` and `add_connection_callback` methods to do so.

```cpp
server.add_message_callback([](uint32_t nodeId, const std::string& msg) {
  // handle received message
});

server.add_connection_callback([](uint32_t nodeId) {
  // handle new connection
});
```

5. **Stop the server:** You can stop the server at any time by calling the `stop_server` method.

```cpp
server.stop_server();
```

Remember that all interaction with `PainlessMeshLib` should be done on the same thread, as it's not thread-safe. Keep all the interaction in the main thread to avoid any concurrency issues.

# PainlessMeshBoost README

The original README is [here](./PainlessMeshBoostREADME.md).