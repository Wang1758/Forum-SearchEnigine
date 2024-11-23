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