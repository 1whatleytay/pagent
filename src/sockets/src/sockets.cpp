#include <sockets/sockets.h>

#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace sockets {
    int32_t ClientSocket::getSocket() const { return socket; }
    Port ClientSocket::getPort() const { return port; }
    Address ClientSocket::getAddress() const { return address; }

    size_t ClientSocket::read(void *data, size_t size) {
        ssize_t bytesRead = ::recv(socket, data, size, 0);

        if (bytesRead < 0)
            throw std::runtime_error("Socket write error.");

        return bytesRead;
    }

    size_t ClientSocket::write(const void *data, size_t size) {
        ssize_t bytesSent = ::send(socket, data, size, 0);

        if (bytesSent < 0)
            throw std::runtime_error("Socket write error.");

        return bytesSent;
    }

    ClientSocket::ClientSocket(Port port, Address address) : port(port), address(address) {
        socket = ::socket(AF_INET, SOCK_STREAM, 0);

        if (socket == -1)
            throw std::runtime_error("Cannot create socket.");

        sockaddr_in addressIn = {};
        std::memset(&addressIn, 0, sizeof(addressIn));
        addressIn.sin_family = AF_INET;
        addressIn.sin_port = htons(port);
        std::memcpy(&addressIn.sin_addr.s_addr, address.data(), address.size());

        if (::connect(socket, reinterpret_cast<sockaddr *>(&addressIn), sizeof(addressIn)) == -1)
            throw std::runtime_error("Socket connection failed.");
    }

    ClientSocket::~ClientSocket() {
        close(socket);
    }

    int32_t ServerSocket::getSocket() const { return socket; }
    Port ServerSocket::getPort() const { return port; }

    bool ServerSocket::ready() const {
        pollfd fd;
        memset(&fd, 0, sizeof(fd));
        fd.fd = socket;
        fd.events = POLLIN;

        int32_t result = poll(&fd, 1, 0); // 100 ms

        return result > 0;
    }

    ServerSocket::ServerSocket(Port port) : port(port) {
        socket = ::socket(AF_INET, SOCK_STREAM, 0);

        if (socket == -1)
            throw std::runtime_error("Cannot create socket.");

        sockaddr_in addressIn = {};
        std::memset(&addressIn, 0, sizeof(addressIn));
        addressIn.sin_family = AF_INET;
        addressIn.sin_port = htons(port);
        addressIn.sin_addr.s_addr = INADDR_ANY;

        if (::bind(socket, reinterpret_cast<sockaddr *>(&addressIn), sizeof(addressIn)) == -1)
            throw std::runtime_error("Socket binding failed.");

        if (::listen(socket, 5) == -1)
            throw std::runtime_error("Socket listening failed.");
    }

    ServerSocket::~ServerSocket() {
        close(socket);
    }

    int32_t ConnectionSocket::getSocket() const { return socket; }
    Port ConnectionSocket::getPort() const { return port; }
    Address ConnectionSocket::getAddress() const { return address; }

    size_t ConnectionSocket::read(void *data, size_t size) {
        ssize_t bytesRead = ::recv(socket, data, size, 0);

        if (bytesRead < 0)
            throw std::runtime_error("Socket write error.");

        return bytesRead;
    }

    size_t ConnectionSocket::write(const void *data, size_t size) {
        ssize_t bytesSent = ::send(socket, data, size, 0);

        if (bytesSent < 0)
            throw std::runtime_error("Socket write error.");

        return bytesSent;
    }

    ConnectionSocket::ConnectionSocket(const ServerSocket &server) {
        sockaddr_in addressIn = {};
        uint32_t size;

        socket = ::accept(server.getSocket(), reinterpret_cast<sockaddr *>(&addressIn), &size);

        if (socket == -1)
            throw std::runtime_error("Socket connection failed.");

        port = ntohs(addressIn.sin_port);
        std::memcpy(address.data(), &addressIn.sin_addr.s_addr, address.size());
    }

    ConnectionSocket::~ConnectionSocket() {
        close(socket);
    }
}
