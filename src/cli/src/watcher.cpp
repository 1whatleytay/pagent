#include <cli/watcher.h>

#include <sys/stat.h>

#include <thread>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

void Watcher::exec() {
    std::unordered_map<std::string, timespec> paths;

    if (fs::is_directory(path)) {
        for (auto &p : fs::recursive_directory_iterator(fs::path(path))) {
            paths[p.path().string()] = timespec();
        }
    } else {
        paths[path] = timespec();
    }

    struct stat pathInfo;

    for (auto &pair : paths) {
        stat(pair.first.c_str(), &pathInfo);
        paths[pair.first] = pathInfo.st_mtimespec;
    }

    while (!stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for (auto &pair : paths) {
            stat(pair.first.c_str(), &pathInfo);

            if (pair.second.tv_nsec != pathInfo.st_mtimespec.tv_nsec
                || pair.second.tv_sec != pathInfo.st_mtimespec.tv_sec) {
                callback(pair.first);
            }

            pair.second = pathInfo.st_mtimespec;
        }
    }
}

Watcher::Watcher(std::string path, WatcherCallback callback) : path(move(path)), callback(move(callback)) { }
