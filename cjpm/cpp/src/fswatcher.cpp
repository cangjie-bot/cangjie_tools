#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <filesystem>
#include "uv.h"

std::vector<uv_fs_event_t*> g_fs_events;
uv_async_t* g_async = nullptr;
bool running = false;
typedef void (*callback)(const char *);
callback cb = nullptr;

std::mutex cb_mutex;
std::mutex global_state_mutex;

void fs_event_close_cb(uv_handle_t* handle) {
    if (handle != nullptr) {
        free(handle);
    }
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

std::string join_path(const std::string& dir, const std::string& filename) {
    std::filesystem::path fullpath = std::filesystem::path(dir) / filename;
    return fullpath.string();
}

void async_stop_callback(uv_async_t* handle) {
    if (handle != nullptr) {
        uv_stop(uv_default_loop());
    }
}

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

    if (start < s.size() && !s.substr(start).empty()) {
        res.push_back(s.substr(start));
    }

    return res;
}

void run_command(uv_fs_event_t *handle, const char *filename, int events, int status) {
    if (handle == nullptr || filename == nullptr || status != 0) {
        return;
    }

    size_t path_size = 0;
    int ret = uv_fs_event_getpath(handle, nullptr, &path_size);
    if (ret != UV_ENOBUFS || path_size == 0) {
        return;
    }

    std::vector<char> path_buf(path_size + 1);
    ret = uv_fs_event_getpath(handle, path_buf.data(), &path_size);
    if (ret != 0) {
        return;
    }
    path_buf[path_size] = '\0';

    std::string dir = path_buf.data();
    std::string file = filename;


    if (ends_with(file, ".cjo.flag") && (events & UV_RENAME)) {
        std::lock_guard<std::mutex> lock(cb_mutex);
        if (cb != nullptr) {
            cb(join_path(dir, file).c_str());
        }
    }
}

void clean() {
    {
        std::lock_guard<std::mutex> state_lock(global_state_mutex);
        running = false;
        for (auto fs_event : g_fs_events) {
            if (fs_event != nullptr && !uv_is_closing((uv_handle_t*)fs_event)) {
                uv_fs_event_stop(fs_event);
                uv_close((uv_handle_t*)fs_event, fs_event_close_cb);
            }
        }
        g_fs_events.clear();
        if (g_async != nullptr && !uv_is_closing((uv_handle_t*)g_async)) {
            uv_close((uv_handle_t*)g_async, [](uv_handle_t* handle) {
                if (handle != nullptr) {
                    free(handle);
                }
                std::lock_guard<std::mutex> state_lock(global_state_mutex);
                g_async = nullptr;
            });
        }
    }
    while (uv_run(uv_default_loop(), UV_RUN_ONCE) != 0);
    std::lock_guard<std::mutex> cb_lock(cb_mutex);
    cb = nullptr;
}

extern "C" {
bool initFSWatcher(const char *arg, callback c) {
    std::unique_lock<std::mutex> state_lock(global_state_mutex);
    if (running || !g_fs_events.empty() || g_async != nullptr) {
        return false;
    }
    if (arg == nullptr || c == nullptr) {
        return false;
    }

    std::string input = arg;
    std::vector<std::string> files = split(input, ',');
    if (files.empty()) {
        return false;
    }

    g_async = (uv_async_t*)malloc(sizeof(uv_async_t));
    if (g_async == nullptr) {
        return false;
    }

    if (uv_async_init(uv_default_loop(), g_async, async_stop_callback) != 0) {
        free(g_async);
        g_async = nullptr;
        return false;
    }

    bool init_success = true;
    for (const auto& file : files) {
        if (file.empty()) {
            continue;
        }

        uv_fs_event_t *fs_event_req = static_cast<uv_fs_event_t *>(malloc(sizeof(uv_fs_event_t)));
        if (fs_event_req == nullptr) {
            init_success = false;
            break;
        }

        if (uv_fs_event_init(uv_default_loop(), fs_event_req) != 0) {
            free(fs_event_req);
            init_success = false;
            break;
        }

        if (uv_fs_event_start(fs_event_req, run_command, file.c_str(), UV_FS_EVENT_WATCH_ENTRY) != 0) {
            uv_close((uv_handle_t*)fs_event_req, fs_event_close_cb);
            init_success = false;
            break;
        }

        g_fs_events.push_back(fs_event_req);
    }

    if (!init_success) {
        state_lock.unlock();
        clean();
        return false;
    }

    std::lock_guard<std::mutex> lock(cb_mutex);
    cb = c;
    return true;
}

void startFSWatcher() {
    {
        std::lock_guard<std::mutex> state_lock(global_state_mutex);
        if (running || g_async == nullptr || g_fs_events.empty()) {
            return;
        }
        running = true;
    }
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    clean();
}

void stopFSWatcher() {
    std::lock_guard<std::mutex> state_lock(global_state_mutex);
    if (!running || g_async == nullptr) {
        return;
    }
    uv_async_send(g_async);
}
}