#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <chrono>

using namespace std;

// Функція для піднесення до квадрату 
void square(int x) {
    int result = x * x;
    cout << "[Square] " << x << "^2 = " << result << endl;
}

//  Глобальна змінна counter + м'ютекс для захисту 
int counter = 0;
mutex counter_mutex;

void increment(int thread_id, bool protect) {
    if (protect) {
        lock_guard<mutex> lock(counter_mutex);
        counter += thread_id;
        cout << "[Increment PROTECTED] Thread " << thread_id << ", counter = " << counter << endl;
    }
    else {
        counter += thread_id;
        cout << "[Increment UNPROTECTED] Thread " << thread_id << ", counter = " << counter << endl;
    }
}

// . М'ютекси для демонстрації Deadlock 
mutex mtx1, mtx2;

void task1() {
    lock_guard<mutex> lock1(mtx1);
    this_thread::sleep_for(chrono::milliseconds(100));
    lock_guard<mutex> lock2(mtx2);
    cout << "[Task 1] Acquired both locks" << endl;
}

void task2() {
    lock_guard<mutex> lock2(mtx2);
    this_thread::sleep_for(chrono::milliseconds(100));
    lock_guard<mutex> lock1(mtx1);
    cout << "[Task 2] Acquired both locks" << endl;
}

//  Безпечна черга Producer-Consumer 
class SafeQueue {
private:
    queue<int> q;
    mutex mtx;
    condition_variable cv;
    const size_t max_size = 5;

public:
    void enqueue(int value) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]() { return q.size() < max_size; });
        q.push(value);
        cout << "[Produced] " << value << endl;
        cv.notify_all();
    }

    int dequeue() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]() { return !q.empty(); });
        int value = q.front();
        q.pop();
        cout << "[Consumed] " << value << endl;
        cv.notify_all();
        return value;
    }
};

SafeQueue safeQueue;

void producer() {
    for (int i = 0; i < 10; ++i) {
        safeQueue.enqueue(i);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void consumer() {
    for (int i = 0; i < 10; ++i) {
        safeQueue.dequeue();
        this_thread::sleep_for(chrono::milliseconds(150));
    }
}

//  MAIN 
int main() {
    cout << "\n--- Part 1: Squaring numbers ---\n";
    thread t1(square, 2);
    thread t2(square, 3);
    thread t3(square, 4);
    t1.join();
    t2.join();
    t3.join();

    cout << "\n--- Part 2: Increment counter with Data Race ---\n";
    counter = 0;
    thread t4(increment, 1, false);
    thread t5(increment, 2, false);
    thread t6(increment, 3, false);
    t4.join();
    t5.join();
    t6.join();

    cout << "\n--- Part 3: Increment counter safely ---\n";
    counter = 0;
    thread t7(increment, 1, true);
    thread t8(increment, 2, true);
    thread t9(increment, 3, true);
    t7.join();
    t8.join();
    t9.join();

    cout << "\n--- Part 4: Deadlock demonstration (Warning: Might freeze!) ---\n";
    thread t10(task1);
    thread t11(task2);
    t10.join();
    t11.join();

    cout << "\n--- Part 5: Producer-Consumer example ---\n";
    thread producer_thread(producer);
    thread consumer_thread(consumer);
    producer_thread.join();
    consumer_thread.join();

    return 0;
}
