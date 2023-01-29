#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <typeinfo>
#include <vector>

const int MIN = 0;
const int MAX = 1e8;
using LL = long long;

LL worker(int l, int r) {
    LL sum = 0;

    for(int i = l; i <= r; i++) {
        sum += (LL)i * i;
    }

    return sum;
}

void linearCalc() {
    auto start = std::chrono::steady_clock::now();
    LL res = worker(MIN, MAX);
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout<< res<< " The linear duration is: "<< duration<< "ms"<< std::endl;
}

static LL concurrentSum = 0;
static std::mutex mtx;
void concurrentWorker(int l, int r) {
    LL tmp = 0;

    for (int i = l; i <= r; i++) {
        tmp += (LL)i * i;
    }

    mtx.lock();
    concurrentSum += tmp;
    mtx.unlock();
}

void concurrentCalc() {
    auto start = std::chrono::steady_clock::now();
    short workersCount = std::thread::hardware_concurrency();

    std::vector<std::thread> threads;
    int stepRange = MAX / workersCount;
    for (int i = 0; i < workersCount; i++) {
        threads.push_back(std::thread(concurrentWorker, i * stepRange, (i + 1) * stepRange));
    }

    for (auto &t : threads) {
        t.join();
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout<< concurrentSum<< " The concurrent duration is: "<< duration<< "ms"<< std::endl;
}


int main() {
    std::cout<< worker(1, 3)<< std::endl;
    
    // 线性计算
    linearCalc();

    //并行计算
    concurrentCalc();

    return 0;
}
