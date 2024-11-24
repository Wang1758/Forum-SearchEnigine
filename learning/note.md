### 1.mylogger
##### 1.1 两种单例模式的区别
1. 将单例对象设置为私有的静态成员变量，通过静态方法获取单例对象
优点： 初始化位置清晰，易于管理
缺点： 无法实现延迟加载, 线程不安全
```cpp
class Singleton {
private:
    static Singleton* instance;

    Singleton() {}  // 私有构造函数

public:
    static Singleton* getInstance() {
        if (instance == nullptr) {
            instance = new Singleton();
        }
        return instance;
    }
};

Singleton* Singleton::instance = nullptr;  // 静态成员变量初始化
```

2. 将单例对象设置为静态局部变量，通过静态方法获取单例对象
优点： 实现了延迟加载，线程安全
缺点：返回的是对象的引用，而不是指针
```cpp
class Singleton {
private:
    Singleton() {}  // 私有构造函数

public:
    static Singleton* getInstance() {
        static Singleton instance;
        return &instance;
    }
};
```

##### 1.2 pthread_once
pthread_once 是 POSIX 线程库 (pthread) 中用于实现线程安全的一次性初始化函数。确保指定的函数只被执行一次。
```cpp
pthread_once_t once = PTHREAD_ONCE_INIT;

void init() {
    // do something
}

void foo() {
    pthread_once(&once, init);
}

int main() {
    pthread_t thread1, thread2; 
    pthread_create(&thread1, nullptr, threadFunc, nullptr); 
    pthread_create(&thread2, nullptr, threadFunc, nullptr); 
    pthread_join(thread1, nullptr); 
    pthread_join(thread2, nullptr);
}
```
> 原理：pthread_once 函数内部使用了互斥锁和条件变量，当init函数被调用时，会先检查once的状态，如是初始态再加锁，并调用init函数，一旦 init 函数执行完毕，_once 进入完成状态，其他所有阻塞的线程都会被唤醒，然后解锁，此后不再执行init函数

##### 1.3 atexit
atexit 函数用于注册一个函数，当程序正常退出时，会调用这个函数。可以注册多个函数，按照注册的顺序调用。
```cpp
void exitFunc() {
    // do something
}

int main() {
    atexit(exitFunc);
}
```
> 通常使用::atexit，表示使用全局命名空间，这是为了避免和某些局部命名空间的函数冲突

### 2. Nocopyble
##### 2.1 delete
C++11 中可以使用 delete 关键字来禁用某些函数，通常用在拷贝构造函数、赋值运算符和析构函数上。
> 编译器会在编译期间检查是否调用了被禁止的函数，而不是在运行时出现问题
```cpp
class Nocopyable {
public:
    Nocopyable() {}
    Nocopyable(const Nocopyable&) = delete;  // 禁用拷贝构造函数
    Nocopyable& operator=(const Nocopyable&) = delete;  // 禁用赋值运算符
    ~Nocopyable() = default;
};
```
> 在单例模式中，可以使用 delete 关键字禁用拷贝构造函数和赋值运算符，确保单例对象只有一个实例

**其他关键字**：
1. 构造和析构关键字
   - explicit：放置构造函数的隐式转换
   - virtual：虚函数或虚基类
   - override：显示声明覆盖基类的虚函数
   - final：禁止类被继承或者虚函数被覆盖
2. 特殊成员函数控制
   - delete：禁用函数，放置其被调用
   - default：显式要求编译器生成默认实现
3. 其他控制
   - mutable：允许 const 成员函数修改成员变量
   - constexpr：声明编译时常量表达式的函数或变量
   - noexcept：声明函数不抛出异常
   - inline：内联函数

### 3. Condition、MutexLock
##### 3.1 前向声明
前向声明是指在使用某个类之前，只声明这个类的存在，而不定义这个类的具体内容。这样可以减少头文件的依赖，提高编译速度。
> 使用前向声明的条件：适用于只需要使用指向某个类型的指针或引用的情况，编译器只需要知道这个类型的存在，而不需要知道这个类型的具体内容
```cpp
class MutexLock;  // 前向声明

class Condition : Nocopyble {
    // 类定义如
    Condition(MutexLock &mutex);
};
```
在.cpp实现的文件中，当需要使用 Condition 类时，再包含 MutexLock.h 头文件即可。
```cpp
#include "MutexLock.h"
#include "Condition.h"
// 代码实现
```

##### 3.2 std::mutex与std::condition_variable
MutexLock、Condition使用POSIX线程库实现了简单的互斥锁和条件变量，但是可以使用c++11实现的这些更高级的类，以下是C++11标准库中的实现：
1. std::mutex：互斥锁，用于保护共享资源，确保同一时间只有一个线程访问共享资源
2. std::condition_variable：条件变量，用于线程间的同步，当共享资源的状态发生变化时，通知等待的线程
3. std::lock_guard：在构造时自动加锁，析构时自动解锁，不允许手动控制。
4. std::unique_lock: 提供了手动控制锁的功能，可以延迟加锁、试图加锁、采用现有锁、手动解锁和重新加锁。
```cpp
std::mutex mtx;
std::unique_lock<std::mutex> lock(mtx);  // 自动加锁

std::unique_lock<std::mutex> lock(mtx, std::defer_lock);  // 延迟加锁，需要手动调用 lock()
lock.lock();  // 手动加锁

std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);  // 尝试加锁
// 如果锁已经被其他线程占用，则构造函数立即返回，不会阻塞
if (lock.owns_lock()) {
    // 锁定成功
} else {
    // 锁定失败
}

mtx.lock();  // 手动加锁
std::unique_lock<std::mutex> lock(mtx, std::adopt_lock);  // 采用现有锁
```


