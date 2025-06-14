#include "epoll_manager.hpp"
#include "utils.hpp"
#include <iostream>
#include <stdexcept>

EpollManager::EpollManager() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor");
    }
}

int EpollManager::register_fd(int fd, uint32_t flags) {
    struct epoll_event ev;
    ev.events = flags;
    ev.data.fd = fd;
    return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

int EpollManager::mod_fd(int fd, uint32_t flags) {
    struct epoll_event ev;
    ev.events = flags;
    ev.data.fd = fd;
    return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

int EpollManager::delete_fd(int fd) {
    int result = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
    return result;
}

int EpollManager::poll(std::vector<struct epoll_event>& events) {
    return epoll_wait(epoll_fd, events.data(), Consts::MAX_EPOLL_EVENTS, -1);
}