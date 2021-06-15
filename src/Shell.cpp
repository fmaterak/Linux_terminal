#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <cstdint>
#include <sys/poll.h>
#include <sys/errno.h>
#include <sys/ioctl.h>

#include <iostream>

#include "Shell.hpp"


terminal::Shell::Shell(): buf_end(buf), buf_ptr(buf) { }

pid_t terminal::Shell::GetPid() const {
    return pid_;
}

void terminal::Shell::ResetPid() {
    pid_ = -1;
}

bool terminal::Shell::SpawnChild() {
#ifdef __APPLE__
    master_ = open("/dev/ptmx", O_RDWR | O_NOCTTY);
#else
    master_ = getpt();
#endif

    if (master_ == -1) {
        perror("getpt");
        return false;
    }

    char slave_path[256]{};
    if (ptsname_r(master_, slave_path, sizeof(slave_path)) != 0) {
        perror("ptsname_r");
        close(master_);
        return false;
    }

    if (grantpt(master_) != 0) {
        perror("grantpt");
        close(master_);
        return false;
    }

    if (unlockpt(master_) != 0) {
        perror("unlockpt");
        close(master_);
        return false;
    }

    int slave = open(slave_path, O_RDWR);
    if (slave < 0) {
        perror("open");
        close(master_);
        return false;
    }

    pid_ = fork();
    if (pid_ == -1) {
        perror("fork");
        close(master_);
        close(slave);
        return false;
    }

    if (pid_ == 0) {
        // Child process.
        close(0);  // stdin
        close(1);  // stdout
        close(2);  // stderr
        close(master_);
        master_ = -1;

        dup2(slave, 0);
        dup2(slave, 1);
        dup2(slave, 2);

        close(slave);

        if (execl("/bin/bash", "/bin/bash", nullptr) == -1) {
            abort();
        }
    }

    // Parent process.
    close(slave);
    return true;
}

void terminal::Shell::ReaderWorker() {
    // uint8_t buf[4096];

    // pollfd fds[] = {
    //         { master_, POLLIN | POLLPRI | POLLRDHUP | POLLERR, 0 /* ignored */ }
    // };

    // while (!end_threads_.load()) {

    //     int ret = poll(fds, 1, 50 /* ms */);
    //     if (ret == -1) {
    //         perror("poll");
    //         break;
    //     }

    //     if (ret == 0) {
    //         // Timeout.
    //         continue;
    //     }

    //     const int revents = fds[0].revents;

    //     if ((revents & POLLIN)) {
    //         ssize_t buf_read = read(master_, buf, sizeof(buf));
    //         if (buf_read == -1) {
    //             perror("ReaderWorker");
    //         }

    //         fwrite(buf, 1, buf_read, stdout);

    //     } else {
    //         fprintf(stderr, "poll revents == %.x\n", revents);
    //     }
    // }
}

void terminal::Shell::CloseMaster() {
    end_threads_.store(true);
    read_th_->join();
    close(master_);
    master_ = -1;
}

bool terminal::Shell::read_overwrite() {
    buf_ptr = buf_end = buf;
    return read_save_tail();
}


bool terminal::Shell::read_save_tail() {
    pollfd poll_files = { master_, POLLIN, 0 };
    int ret = poll(&poll_files, 1, 0);

    if (ret == -1) {
        perror("poll");
        throw std::runtime_error(strerror(errno));
    }

    if (ret == 0) {
        return false;
    }

    if (poll_files.revents == POLLIN) {
        int num_read = read(master_, buf_end, buf + BUF_SIZE - buf_end);
        if (num_read == -1) {
            throw std::runtime_error(strerror(errno));
        }
        buf_end += num_read;
        return true;
    } else {
        std::cout << "Unexpected revents mask: " << poll_files.revents << std::endl;
        throw std::runtime_error("");
    }
}


bool terminal::Shell::read_preserve() {
    if (buf_end == buf + BUF_SIZE && buf_ptr > buf) {
        char *tmp = buf;
        while (buf_ptr != buf_end) {
            *tmp++ = *buf_ptr++;
        }
        buf_ptr = buf;
        buf_end = tmp;
    }

    return read_save_tail();
}

bool terminal::Shell::require_chars_available(int num_chars) {
    if (buf_end - buf_ptr >= num_chars) {
        return true;
    }

    read_preserve();
    return buf_end - buf_ptr >= num_chars;
}

bool terminal::Shell::try_read_style_change(bool& style_changed, terminal::Style& new_style) {
    // returns false when there is not enough data, so that it will not be overwritten
    // returns true when style change successfully parsed or there is no style change

    if (!require_chars_available(3)) {
        return false;
    }

    if (buf_ptr[0] != '\e' || buf_ptr[1] != '[') {
        return true;
    }

    read_preserve();
    char* tmp = buf_ptr + 2;

    if (*tmp == '?') {
        tmp++;
        while (tmp < buf_end) {
            char c = *tmp++;
            if (c == 'h' || c == 'l') {
                buf_ptr = tmp;
                return true;
            }
        }
        return false;
    }

    return true;
}


terminal::codepoint terminal::Shell::next_codepoint(bool& style_changed, terminal::Style& new_style) {
    if (master_ == -1 || pid_  == -1) {
        return -1;
    }

    if (!try_read_style_change(style_changed, new_style)) {
        return EOF;
    }

    if (!require_chars_available(2)) {
        return EOF;
    }

    codepoint c = *buf_ptr++;

    if (c == '\r') {
        if (*buf_ptr == '\n') {
            return *buf_ptr++;
        }
    }

    return c;
}
