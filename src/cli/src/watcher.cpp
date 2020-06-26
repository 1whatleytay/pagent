#include <cli/watcher.h>

#include <sys/stat.h>

#include <filesystem>

namespace fs = std::filesystem;

void Watcher::exec() {
    if (fs::is_directory(path))
        throw std::runtime_error("Watcher can only watch individual files.");

    struct stat pathInfo;
    stat(path.c_str(), &pathInfo);

    timespec lastTime = pathInfo.st_mtimespec;

    while (!stop) {
        stat(path.c_str(), &pathInfo);

        if (lastTime.tv_nsec != pathInfo.st_mtimespec.tv_nsec
            || lastTime.tv_sec != pathInfo.st_mtimespec.tv_sec) {
            callback(path);
        }

        lastTime = pathInfo.st_mtimespec;
    }
}

Watcher::Watcher(std::string path, WatcherCallback callback) : path(move(path)), callback(move(callback)) { }
