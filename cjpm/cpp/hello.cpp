#include <iostream>
#include <string>
#include <vector>
#include "include/uv.h"

std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> res;
    size_t start = 0;
    size_t pos;

    while ((pos = s.find(sep, start)) != std::string::npos) {
        if (pos > start) {
            res.push_back(s.substr(start, pos - start));
        }
        start = pos + 1;
    }
    if (start < s.size()) {
        res.push_back(s.substr(start));
    }

    return res;
}

typedef void (*Callback)(const char *);

Callback cb;

void run_command(uv_fs_event_t *handle, const char *filename, int events, int status) {
    char path[1024];
    size_t size = 1023;
    uv_fs_event_getpath(handle, path, &size);
    path[size] = '\0';

    std::string dir = path;
    std::string file = filename;

    if (file.ends_with(".cjo.flag") && (events & UV_RENAME)) {
        std::cout << "Change detected in " << dir << " renamed " << file << std::endl;
        cb((dir + "/" + file).c_str());
    }
}

extern "C" {
void hello() {
    std::cout << "hello world" << std::endl;
}

int CMonitor(const char *arg, Callback c) {
    std::string str = arg;
    std::vector<std::string> parts = split(str, ',');
    for (const auto& part : parts) {
        std::cout << "[" << part << "]" << std::endl;
    }
    std::cout << "mk1" << std::endl;
    auto loop = uv_default_loop();
    std::cout << "mk2" << std::endl;
    for (const auto& part : parts) {
        std::cout << "mk3" << std::endl;
        uv_fs_event_t *fs_event_req = static_cast<uv_fs_event_t *>(malloc(sizeof(uv_fs_event_t)));
        std::cout << "mk4" << std::endl;
        uv_fs_event_init(loop, fs_event_req);
        std::cout << "mk5" << std::endl;
        uv_fs_event_start(fs_event_req, run_command, part.c_str(), UV_FS_EVENT_WATCH_ENTRY);
        std::cout << "mk6" << std::endl;
    }
    cb = c;
    std::cout << "mk7" << std::endl;
    int res = uv_run(loop, UV_RUN_DEFAULT);
    std::cout << "mk8" << std::endl;
    return res;
}
}