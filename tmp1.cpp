#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include "time_wheel.hpp"
using namespace monitor;
int main() {
  // 获取线程池实例
  TimeWheel &tw = TimeWheel::GetInstance();
  Task_Result tt = tw.add_loop_timer("Loop Timer", 50, []() -> bool {std::cout<<1<<std::endl; return true; });
  while(1){
    
  }
  return 0;
}