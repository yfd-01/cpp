#include <future>
#include <chrono>
#include <iostream>
#include <thread>

template <typename T>
void CalcSquare(std::promise<T> thePromise, T n) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    thePromise.set_value(n * n);
}

void ThrowFunc(std::promise<void> thePromise) {
    try {
        throw std::runtime_error("An exception");
    } catch (...) {
        thePromise.set_exception(std::current_exception());
    }
}

std::string GetPreData(char op) {
    if (op == 'a') return "opA";
    else if (op == 'b') return "opB";
    else return "none";
}

// int main() {
//     std::promise<int> intPromise;
//     std::future<int> fut1 = intPromise.get_future();
//
//     CalcSquare(std::move(intPromise), 90);
//     std::cout << fut1.get() << std::endl;
//
//     std::promise<void> voidPromise;
//     std::future<void> fut2 = voidPromise.get_future();
//
//     std::thread t(ThrowFunc, std::move(voidPromise));
//
//     try {
//         fut2.get();
//     } catch (std::exception& e) {
//         std::cout << e.what() << std::endl;
//     }
//     t.join();
//
//     std::future<std::string> fut3 = std::async(GetPreData, 'a');
//     std::shared_future<std::string> futShared = fut3.share();
//
//     std::thread t1([futShared]() {
//         std::this_thread::sleep_for(std::chrono::seconds(2));
//         std::cout << "thread-1 - "<< futShared.get() << std::endl;
//     });
//
//     std::thread t2([futShared]() {
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//         std::cout << "thread-2 - "<< futShared.get() << std::endl;
//     });
//
//     t1.join();
//     t2.join();
//
//     return 0;
// }
