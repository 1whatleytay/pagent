#pragma once

#include <array>
#include <cstdint>

namespace sockets {
    using Port = uint16_t;
    using Address = std::array<uint8_t, 4>;

    class ClientSocket {
        int32_t socket = -1;

        Port port = 0;
        Address address = {};

    public:
        int32_t getSocket() const;
        Port getPort() const;
        Address getAddress() const;

        size_t read(void *data, size_t size);
        size_t write(const void *data, size_t size);

        ClientSocket() = default;
        ClientSocket(Port port, Address address);
        ~ClientSocket();
    };

    class ServerSocket {
        int32_t socket = -1;

        Port port = 0;

    public:
        int32_t getSocket() const;
        Port getPort() const;

        bool ready() const;

        ServerSocket() = default;
        explicit ServerSocket(Port port);
        ~ServerSocket();
    };

    class ConnectionSocket {
        int32_t socket = -1;

        Port port = 0;
        Address address = {};

    public:
        int32_t getSocket() const;
        Port getPort() const;
        Address getAddress() const;

        size_t read(void *data, size_t size);
        size_t write(const void *data, size_t size);

        ConnectionSocket() = default;
        explicit ConnectionSocket(const ServerSocket &server);
        ~ConnectionSocket();
    };
}
