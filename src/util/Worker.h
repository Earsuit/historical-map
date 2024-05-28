#ifndef SRC_UTIL_WORKER_H
#define SRC_UTIL_WORKER_H

#include "blockingconcurrentqueue.h"

#include <type_traits>
#include <thread>
#include <atomic>

namespace util {
template<typename T>
requires (std::is_invocable_v<T>)
class Worker {
public:
    Worker() {
        runWorkerThread = true;
        workerThread = std::thread(&Worker::worker, this);
    }

    ~Worker() {
        runWorkerThread = false;

        // enqueue an empty task to wake up the wait_dequeue if necessary
        taskQueue.enqueue([](){});

        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    bool enqueue(T task) {
        return taskQueue.enqueue(task);
    }

private:
    moodycamel::BlockingConcurrentQueue<T> taskQueue;
    std::atomic_bool runWorkerThread;
    std::thread workerThread;

    void worker() {
        while (runWorkerThread) {
            T task; 

            taskQueue.wait_dequeue(task);

            task();
        }
    }
};
}

#endif
