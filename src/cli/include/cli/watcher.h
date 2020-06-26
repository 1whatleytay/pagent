#pragma once

#include <string>
#include <functional>

using WatcherCallback = std::function<void(std::string path)>;

class Watcher {
    std::string path;
    WatcherCallback callback;

public:
    volatile bool stop = false;

    void step();
    void exec();

    Watcher(std::string path, WatcherCallback callback);
};
