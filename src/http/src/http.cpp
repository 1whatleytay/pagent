#include <http/http.h>

#include <array>
#include <sstream>

namespace http {
    class Reader {
        constexpr static size_t bufferCapacity = 1024;

        size_t bufferIndex = 0;
        size_t bufferSize = 0;
        std::array<uint8_t, bufferCapacity> buffer = { };

        const ReadCallback &readCallback;

        void refill() {
            size_t size = readCallback(buffer.data(), buffer.size());
            assert(size != 0);

            bufferIndex = 0;
            bufferSize = size;
        }

    public:
        std::vector<uint8_t> read(size_t size) {
            std::vector<uint8_t> result(size);
            size_t index = 0;

            while (index < size) {
                if (bufferIndex >= bufferSize)
                    refill();

                size_t sizeToRead = std::min(size - index, bufferSize - bufferIndex);
                std::memcpy(&result[index], &buffer[bufferIndex], sizeToRead);
                bufferIndex += sizeToRead;

                index += sizeToRead;
            }

            return result;
        }

        std::vector<uint8_t> until(const std::string &value) {
            std::vector<uint8_t> result;

            while (true) {
                if (bufferIndex >= bufferSize)
                    refill();

                auto a = buffer[bufferIndex++];

                size_t index = 0;
                while (index < value.size() && value[index] == a) {
                    index++;

                    if (index != value.size()) {
                        a = buffer[bufferIndex++];
                    }
                }

                if (index == value.size()) {
                    break;
                } else if (index > 0) {
                    result.insert(result.end(), value.begin(), value.begin() + index);
                }

                result.push_back(a);
            }

            return result;
        }

        std::string untilString(const std::string &value) {
            std::vector<uint8_t> binary = until(value);
            return std::string(binary.begin(), binary.end());
        }

        explicit Reader(const ReadCallback &readCallback) : readCallback(readCallback) { }
    };

    Headers loadHeader(Reader &reader) {
        Headers headers;

        while (true) {
            std::string line = reader.untilString("\r\n");

            // reached double \r\n
            if (line.empty())
                break;

            auto index = line.find(": ");
            headers[line.substr(0, index)] = line.substr(index + 2);
        }

        return headers;
    }

    std::vector<uint8_t> loadBody(Reader &reader, Headers &headers) {
        if (headers.find("Content-Length") != headers.end()) {
            return reader.read(std::stoi(headers["Content-Length"]));
        } else if (headers.find("Transfer-Encoding") != headers.end()) {
            std::vector<uint8_t> body;

            std::string encoding = headers["Transfer-Encoding"];
            if (encoding != "chunked")
                throw std::runtime_error("Unsupported encoding.");

            while (true) {
                std::string sizeString = reader.untilString("\r\n");
                size_t size = std::stoull(sizeString, nullptr, 16);
                if (size == 0)
                    break;

                std::vector<uint8_t> data = reader.read(size);
                body.insert(body.end(), data.begin(), data.end());

                auto empty = reader.untilString("\r\n");
                if (!empty.empty())
                    throw std::runtime_error("Encoding error.");
            }

            return body;
        }

        // no body
        return { };
    }

    bool Request::send(const WriteCallback &write) {
        std::stringstream stream;

        // path parsing, like %20 and stuff
        stream << method << " " << path << " " << "HTTP/" << major << "." << minor << "\r\n";

        for (const auto &pair : headers) {
            stream << pair.first << ": " << pair.second << "\r\n";
        }

        if (!body.empty() && headers.find("Content-Length") == headers.end()) {
            stream << "Content-Length: " << body.size() << "\r\n";
        }

        stream << "\r\n";

        std::string result = stream.str();

        if (write(result.c_str(), result.size()) == 0)
            return false;

        if (write(body.data(), body.size()) == 0)
            return false;

        return true;
    }

    Request Request::receive(const ReadCallback &read) {
        Reader reader(read);

        method = reader.untilString(" ");
        path = reader.untilString(" ");

        if (reader.untilString("/") != "HTTP")
            throw std::runtime_error("Not an http request.");

        major = std::stoi(reader.untilString("."));
        minor = std::stoi(reader.untilString("\r\n"));

        headers = loadHeader(reader);
        body = loadBody(reader, headers);

        return *this;
    }

    Request Request::header(const std::string &name, const std::string &value) const {
        Request temp = *this;

        temp.headers[name] = value;

        return temp;
    }

    Request Request::content(std::vector<uint8_t> value) {
        Request temp = *this;

        temp.body = move(value);

        return temp;
    }

    Request::Request(std::string path) : path(std::move(path)) { }
    Request::Request(std::string path, std::string method) : path(std::move(path)), method(std::move(method)) { }

    bool Response::send(const WriteCallback &write) {
        std::stringstream stream;

        stream << "HTTP/" << major << "." << minor << statusCode << " " << statusString << "\r\n";

        for (const auto &pair : headers) {
            stream << pair.first << ": " << pair.second << "\r\n";
        }

        if (!body.empty() && headers.find("Content-Length") == headers.end()) {
            stream << "Content-Length: " << body.size() << "\r\n";
        }

        stream << "\r\n";

        std::string result = stream.str();

        if (write(result.c_str(), result.size()) == 0)
            return false;

        if (write(body.data(), body.size()) == 0)
            return false;

        return true;
    }

    Response Response::receive(const ReadCallback &read) {
        Reader reader(read);

        if (reader.untilString("/") != "HTTP")
            throw std::runtime_error("Not an http response.");

        major = std::stoi(reader.untilString("."));
        minor = std::stoi(reader.untilString(" "));

        statusCode = std::stoi(reader.untilString(" "));
        statusString = reader.untilString("\r\n");

        headers = loadHeader(reader);
        body = loadBody(reader, headers);

        return *this;
    }

    Response Response::content(std::vector<uint8_t> value) {
        Response temp = *this;

        temp.body = value;

        return temp;
    }

    Response Response::header(const std::string &name, const std::string &content) const {
        Response value = *this;

        value.headers[name] = content;

        return value;
    }

    Response::Response(std::vector<uint8_t> body) : body(std::move(body)) { }
}
