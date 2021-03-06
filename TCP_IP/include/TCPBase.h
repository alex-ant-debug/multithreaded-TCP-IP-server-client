#pragma once

#define SD_BOTH 0
#include <sys/socket.h>

#include <cstdint>
#include <cstring>
#include <cinttypes>
#include <malloc.h>

#include <queue>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#define ERR     -1

namespace tcp {

typedef socklen_t SockLen_t;
typedef struct sockaddr_in SocketAddr_in;
typedef int Socket;
typedef int ka_prop_t;

enum SocketStatus : uint8_t {
  connected,
  err_socket_init,
  err_socket_bind,
  err_socket_connect,
  disconnected = 4
};

typedef std::vector<uint8_t> ReceivedData;

enum SocketType : uint8_t {
  client_socket = 0,
  server_socket = 1
};

/// Simple thread pool implementation
class ThreadPool {
  std::vector<std::thread> list_thread_pool;
  std::queue<std::function<void()>> job_queue;
  std::mutex queue_mtx;
  std::condition_variable condition;
  std::atomic<bool> pool_terminated = false;

  void setupThreadPool(uint thread_count) {
    list_thread_pool.clear();
    for(uint i = 0; i < thread_count; ++i) {
        list_thread_pool.emplace_back(&ThreadPool::workerLoop, this);
    }
  }


  void workerLoop() {
    std::function<void()> job;
    while (!pool_terminated) {
      {
        std::unique_lock lock(queue_mtx);
        condition.wait(lock, [this](){
            return !job_queue.empty() || pool_terminated;
        });

        if(pool_terminated) {
            return;
        }
        job = job_queue.front();
        job_queue.pop();
      }
      job();
    }
  }

public:
  ThreadPool(uint thread_count = std::thread::hardware_concurrency()) {
    setupThreadPool(thread_count);
  }

  ~ThreadPool() {
    pool_terminated = true;
    join();
  }

  template<typename F>
  void addJob(F job) {
    if(pool_terminated) return;
    {
      std::unique_lock lock(queue_mtx);
      job_queue.push(std::function<void()>(job));
    }
    condition.notify_one();
  }

  template<typename F, typename... Arg>
  void addJob(const F& job, const Arg&... args) {
    addJob([job, args...]{job(args...);});
  }

  void join() {
    for(auto& thread : list_thread_pool) {
        thread.join();
    }
  }

  uint getThreadCount() const {
    return list_thread_pool.size();
  }

  void dropUnstartedJobs() {
    pool_terminated = true;
    join();
    pool_terminated = false;
    // Clear jobs in queue
    std::queue<std::function<void()>> empty;
    std::swap(job_queue, empty);
    // reset thread pool
    setupThreadPool(list_thread_pool.size());
  }

  void stop() {
    pool_terminated = true;
    join();
  }

  void start(uint thread_count = std::thread::hardware_concurrency()) {
    if(!pool_terminated) {
        return;
    }
    pool_terminated = false;
    setupThreadPool(thread_count);
  }

};

class TcpBase {
public:
  typedef SocketStatus status;
  virtual ~TcpBase() {};
  virtual status disconnect() = 0;
  virtual status getStatus() const = 0;
  virtual bool sendData(const void* buffer, const size_t size) const = 0;
  virtual ReceivedData loadData() = 0;
  virtual uint32_t getHost() const = 0;
  virtual uint16_t getPort() const = 0;
  virtual SocketType getType() const = 0;
};

}
