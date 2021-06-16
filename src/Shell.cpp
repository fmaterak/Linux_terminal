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


static int read_int(const char* buf, const char* buf_end, int& result) {
    int num_read = 0;
    result = 0;

    while (buf < buf_end && '0' <= *buf && *buf <= '9') {
        result = result * 10 + (*buf - '0');
        buf++;
        num_read++;
    }

    return num_read;
}

static terminal::codepoint read_codepoint(const char *text, int &bytesProcessed) {
/*
    Adapted from raylib's GetNextCodepoint()
    UTF8 specs from https://www.ietf.org/rfc/rfc3629.txt

    Char. number range  |        UTF-8 octet sequence
       (hexadecimal)    |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/

    int code = '?';
    int octet = (unsigned char)(text[0]);
    bytesProcessed = 1;

    if (octet <= 0x7f) {
        // Only one octet (ASCII range x00-7F)
        code = text[0];
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        // Two octets
        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        unsigned char octet1 = text[1];

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { bytesProcessed = 2; return -1; } // Unexpected sequence

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            code = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            bytesProcessed = 2;
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        // Three octets
        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { bytesProcessed = 2; return -1; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { bytesProcessed = 3; return -1; } // Unexpected sequence

        /*
            [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
            [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
            [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
            [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        */

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) { bytesProcessed = 2; return -1; }

        if ((octet >= 0xe0) && (0 <= 0xef))
        {
            code = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            bytesProcessed = 3;
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        // Four octets
        if (octet > 0xf4) return -1;

        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';
        unsigned char octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { bytesProcessed = 2; return -1; }  // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { bytesProcessed = 3; return -1; }  // Unexpected sequence

        octet3 = text[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { bytesProcessed = 4; return -1; }  // Unexpected sequence

        /*
            [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
            [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
            [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail
        */

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) { bytesProcessed = 2; return -1; } // Unexpected sequence

        if (octet >= 0xf0)
        {
            code = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            bytesProcessed = 4;
        }
    }

    if (code > 0x10ffff) code = '?';     // Codepoints after U+10ffff are invalid

    return code;
}


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

void terminal::Shell::CloseMaster() {
    close(master_);
    master_ = -1;
}

bool terminal::Shell::read_overwrite() {
    buf_ptr = buf_end = buf;
    return read_save_tail();
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

int terminal::Shell::try_read_style_change(bool& style_changed, terminal::Style& new_style) {
    // returns EOF (-1) when there is not enough data
    // returns 0 when no style change detected
    // returns 1 when style change successfully parsed

#if (EOF != -1)
#error "EOF expected to be -1"
#endif

    char* tmp = buf_ptr;

    if (tmp == buf_end) {
        return EOF;
    }
    else if (*tmp++ != '\e') {
        return 0;
    }

    if (tmp == buf_end) {
        return EOF;
    }
    else if (*tmp++ != '[') {
        return 0;
    }

    if (*tmp == '?') {
        tmp++;
        while (tmp < buf_end) {
            char c = *tmp++;
            if (c == 'h' || c == 'l') {
                buf_ptr = tmp;
                return 1;
            }
        }
        return EOF;
    }

    int read_value, num_read;

    while ((num_read = read_int(tmp, buf_end, read_value))) {
        switch (read_value) {

        }

        if ((tmp += num_read) == buf_end) {
            return EOF;
        }

        if (*tmp == ';') {
            tmp++;
            continue;
        }
        else if (*tmp == 'm') {
            buf_ptr = tmp + 1;
            return 1;
        }
        else {
            // unknown thing
            return 0;
        }
    }

    return 0;
}

terminal::codepoint terminal::Shell::next_codepoint(bool& style_changed, terminal::Style& new_style) {
    if (master_ == -1 || pid_  == -1) {
        return -1;
    }

    read_preserve();

    int status;

    while ((status = try_read_style_change(style_changed, new_style)) == 1);

    if (status == EOF || buf_ptr == buf_end) {
        return EOF;
    }

    if (buf_end - buf_ptr >= 2 && buf_ptr[0] == '\r' && buf_ptr[1] == '\n') {
        buf_ptr += 2;
        return '\n';
    }

    int num_read_bytes = 0;
    codepoint c = read_codepoint(buf_ptr, num_read_bytes);

    if (c == -1 || num_read_bytes > buf_end - buf_ptr) {
        return EOF;
    } else {
        buf_ptr += num_read_bytes;
        return c;
    }
}


void terminal::Shell::write(const char* data, int nbytes) {
    ::write(master_, data, nbytes);
}
