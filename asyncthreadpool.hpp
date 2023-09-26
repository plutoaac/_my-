/*
 * @Author: Adam Xiao
 * @Date: 2020-12-10 11:34:09
 * @LastEditors: Adam Xiao
 * @LastEditTime: 2020-12-17 19:09:11
 * @FilePath: ./asyncthreadpool.hpp
 * @Description:
 * 时间轮类辅助异步线程池类，由于之前已有的taskqueque不能取消提交的定时任务，将taskqueque拆分，然后重新写
 *               编译时，CXXFLAGS里要加-DASIO_DISABLE_STD_FUTURE
 * -DASIO_DISABLE_STD_EXCEPTION_PTR -DASIO_DISABLE_STD_NESTED_EXCEPTION
 *
 *
 */
#pragma once
#include <array>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <thread>

#ifndef USE_BOOST_FUTURE
#include <future>
#else
// #define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_VERSION 4
#include "boost/thread/future.hpp"
#endif

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#define ASIO_NO_DEPRECATED
#endif

#include "boost/asio.hpp"
//#include "asio/io_context.hpp"  //io_context use
//#include "asio/post.hpp"
#include "time_holder.hpp"
#include <iostream>
//#include "asio"

namespace monitor {
#ifndef USE_BOOST_FUTURE
using namespace std;
#else
using namespace boost;
#endif

// 不可移动不可复制类型，异步线程池
class AsyncThreadPool {
 public:
  // 直接添加任务
  future<bool> add(std::function<bool()> f) {
    packaged_task<bool(void)> pt(f);
    future<bool> fu = pt.get_future();
    std::cout<<"fufu"<<std::endl;
    boost::asio::post(ioctx,pt);
    return fu;
  }
  // 只能单例模式生成
  static AsyncThreadPool& GetInstance() {
    static AsyncThreadPool tq;
    return tq;
  }
  // 禁止复制，赋值，移动，移动赋值构造
  AsyncThreadPool(const AsyncThreadPool&) = delete;
  AsyncThreadPool& operator=(const AsyncThreadPool&) = delete;
  AsyncThreadPool(AsyncThreadPool&&) = delete;
  AsyncThreadPool& operator=(AsyncThreadPool&&) = delete;

 private:
  boost::asio::io_context ioctx;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;
  std::vector<std::thread> processors;
  AsyncThreadPool() : work(ioctx.get_executor()) {
    int thread_num = 8;
    processors.reserve(thread_num);
    // std::vector<std::thread> processors;
    for (int i = 0; i < thread_num; i++) {
      processors.emplace_back(std::thread([this] { ioctx.run(); }));
    }
    for (auto& th : processors) {
      th.detach();
    }
  }
};
}  // namespace monitor
