#include <atomic>
#include <memory>
#include <thread>
#include "Reader.hpp"

class Shell : public terminal::Reader {

public:
    pid_t pid_ = -1;
    int master_ = -1;

    pid_t GetPid() const;
    void ResetPid();
    bool SpawnChild();
    void CloseMaster();
    void ReaderWorker();
    void ProcessOutput(uint8_t *data, size_t size);

    std::atomic<bool> end_threads_{false};
    std::unique_ptr<std::thread> read_th_;
};
