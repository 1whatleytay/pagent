#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace http {
    using Headers = std::unordered_map<std::string, std::string>;
    using ReadCallback = std::function<size_t(void *data, size_t size)>;
    using WriteCallback = std::function<size_t(const void *data, size_t size)>;

    class Request {
    public:
        uint32_t major = 1;
        uint32_t minor = 1;

        std::string method = "GET";
        std::string path = "/";

        Headers headers;
        std::vector<uint8_t> body;

        bool send(const WriteCallback &write);
        Request receive(const ReadCallback &read);

        Request content(std::vector<uint8_t> value);
        Request header(const std::string &name, const std::string &value) const;

        Request() = default;
        explicit Request(std::string path);
        Request(std::string path, std::string method);
    };

    class Response {
    public:
        uint32_t major = 1;
        uint32_t minor = 1;

        uint32_t statusCode = 200;
        std::string statusString = "OK";

        Headers headers;
        std::vector<uint8_t> body;

        bool send(const WriteCallback &write);
        Response receive(const ReadCallback &read);

        Response content(std::vector<uint8_t> value);
        Response header(const std::string &name, const std::string &content) const;

        Response() = default;
        explicit Response(std::vector<uint8_t> body);
    };
}
