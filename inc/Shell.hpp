#include "Reader.hpp"

namespace terminal {

class Shell: public Reader {
    pid_t pid_ = -1;
    int master_ = -1;

    static constexpr int BUF_SIZE = 1024;
    char buf[BUF_SIZE], *buf_end, *buf_ptr;

    bool read_preserve();
    bool read_overwrite();
    bool read_save_tail();
    int try_read_style_change(bool& style_changed, Style& new_style);

public:
    Shell();

    inline bool is_connected() { return master_ != -1; }

    pid_t GetPid() const;
    void ResetPid();
    bool SpawnChild();
    void CloseMaster();
    void ProcessOutput(uint8_t *data, size_t size);

    void write(const char* data, int nbytes);
    codepoint next_codepoint(bool& style_changed, Style& new_style);
};

};
