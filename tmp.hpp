#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
 public:
  static  ThreadPool& GetInstance() {
    static  ThreadPool tq;
    return tq;
  }
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;


  template <typename F, typename... Args>
  decltype(auto) add(F&& task, Args&&... args) {
    std::function<decltype(task(args...))()> func =
        std::bind(std::forward<F>(task), std::forward<Args>(args)...);

    auto task_ptr =
        std::make_shared<std::packaged_task<decltype(task(args...))()>>(
            func);  //(1)

    // std::function<void()> wrapper_func = [task_ptr] { (*task_ptr)(); }; //(2)
    {
      std::lock_guard<std::mutex> lg(_mtx);
      //_taskQueue.push(wrapper_func);
      _taskQueue.emplace([task_ptr] { (*task_ptr)(); });
    }
    _isEmpty.notify_one();
    return task_ptr->get_future();
  } /*
   template <typename F, typename... Args>
 void PushTask(F&& task, Args&&... args) {
   std::function<void()> func =
       std::bind(std::forward<F>(task), std::forward<Args>(args)...);

   auto task_ptr = std::make_shared<std::packaged_task<void()>>(func);

   {
     std::lock_guard<std::mutex> lg(_mtx);
     _taskQueue.emplace([task_ptr] { (*task_ptr)(); });
   }
   _isEmpty.notify_one();
 }


 */

 private:
  void LoopWork() {
    std::unique_lock<std::mutex> lck(_mtx);
    for (;;) {
      while (_taskQueue.empty() && !_stop) {
        _isEmpty.wait(lck);
      }
      if (_taskQueue.empty()) {
        lck.unlock();
        break;
      } else {
        auto task = std::move(_taskQueue.front());
        _taskQueue.pop();
        lck.unlock();
        task();
        lck.lock();
      }
    }
  }
  ThreadPool(size_t thread_nums = std::thread::hardware_concurrency())
      : _thread_nums(thread_nums), _stop(false) {
    for (size_t i = 0; i < _thread_nums; ++i) {
      _vt.emplace_back(std::thread(&ThreadPool::LoopWork, this));
    }
  }
  ~ThreadPool() {
    _stop = true;
    _isEmpty.notify_all();
    for (size_t i = 0; i < _thread_nums; ++i) {
      if (_vt[i].joinable()) {
        _vt[i].join();
      }
    }
  }

 private:
  size_t _thread_nums;
  std::vector<std::thread> _vt;
  std::queue<std::function<void()>> _taskQueue;
  std::mutex _mtx;
  std::condition_variable _isEmpty;
  std::atomic<bool> _stop;
};


//std::mutex gb_mtx;
/*int Add(int x, int y) {
  std::lock_guard<std::mutex> lg(gb_mtx);  // 为了保证输出结果不会打印错乱
  std::cout << x << " + " << y << " = " << x + y << std::endl;
}*/
/*
bool Add(int x, int y) {
  std::lock_guard<std::mutex> lg(gb_mtx);
  std::cout << x + y << std::endl;
}
int main() {
  //std::shared_ptr<ThreadPool> tp(new ThreadPool());
    ThreadPool &tp = ThreadPool::GetInstance();
  std::vector<std::future<bool>> res;
  for (int i = 0; i < 10; ++i) {
    res.emplace_back(tp.PushTask(Add, i, i + 1));
  }

  for (auto& e : res) {
    std::cout << e.get() << std::endl;
  }
  return 0;
}*/
