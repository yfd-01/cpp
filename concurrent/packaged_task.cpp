#include <iostream>
#include <future>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>
#include <functional>

struct Scheduler {

public:
    Scheduler() : m_loopThread(&Scheduler::ScheduleLoop, this) {}
    ~Scheduler() {
        {
            std::lock_guard<std::mutex> lg(m_mtx);
            m_shutdown = true;
        }

        m_cv.notify_all();
        if (m_loopThread.joinable()) m_loopThread.join();
    }

private:
    void ScheduleLoop() {
        std::function<void()> executedTask;

        while (!m_shutdown) {
            std::unique_lock<std::mutex> locker(m_mtx);
            m_cv.wait(locker, [this]() {
                return !m_tasks.empty() || m_shutdown;
            });

            if (m_shutdown) return;

            executedTask = std::move(m_tasks.front());
            m_tasks.pop_front();

            executedTask();
        }
    }

public:
    template <typename F, typename... Args>
    auto ScheduleTask(F&& f, Args&&... args) {
        using RetType = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

        auto task = std::make_shared<std::packaged_task<RetType()>>(
            [fn = std::forward<F>(f), ... as = std::forward<Args>(args)] {
                return std::invoke(fn, as...);
            }
        );

        {
            std::lock_guard<std::mutex> lg(m_mtx);
            m_tasks.emplace_back([task]() { (*task)(); });
        }

        m_cv.notify_one();
        return task->get_future();
    }

    template <typename F, typename... Args>
    auto ScheduleTaskAfter(std::chrono::seconds delay, F&& f, Args&&... args) {
        using RetType = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

        auto task = std::make_shared<std::packaged_task<RetType()>>(
            [fn = std::forward<F>(f), ... as = std::forward<Args>(args)] {
                return std::invoke(fn, as...);
            }
        );

        std::thread([this, task, delay]() {
            std::this_thread::sleep_for(delay);
            ScheduleTask([task]() { (*task)(); });
        }).detach();

        return task->get_future();
    }

private:
    std::thread m_loopThread;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::deque<std::function<void()>> m_tasks;
    bool m_shutdown = false;
};

template <typename T1, typename T2>
std::common_type_t<T1, T2> PrintSum(T1 a, T2 b) {
    std::cout << "std::common_type_t<T1, T2> PrintSum(T1 a, T2 b)" << std::endl;
    return a + b;
}

int PrintNum(int a) {
    std::cout << "int PrintNum(int a)" << std::endl;
    return a;
}

struct Foo {
    int a;

    auto PrintMem() {
        std::cout << "Foo::PrintMem()" << std::endl;
        return a;
    }
};

// int main() {
//     Scheduler scheduler;
//
//     auto fut1 = scheduler.ScheduleTask(PrintNum, 10);
//     auto fut2 = scheduler.ScheduleTask([] {
//         std::cout << "lambda\n";
//         return 20;
//     });
//     auto fut3 = scheduler.ScheduleTaskAfter(
//         std::chrono::seconds(3), PrintSum<int, double>, 10, 3.14);
//
//     Foo f;
//     f.a = 99;
//
//     auto fut4 = scheduler.ScheduleTaskAfter(
//         std::chrono::seconds(1), &Foo::PrintMem, &f
//     );
//
//     std::cout << fut1.get() << std::endl;
//     std::cout << fut2.get() << std::endl;
//     std::cout << fut3.get() << std::endl;
//     std::cout << fut4.get() << std::endl;
//
//     std::this_thread::sleep_for(std::chrono::seconds(5));
//
//     return 0;
// }