title: GCD实践之三 -- 使用dispatch_once来执行只需运行一次的线程安全代码
date: 2015-02-01 13:53:40
tags: GCD
categories: iOS

---

单例模式（singleton）对Objective-C开发者而言并不陌生，常见的实现方式为：在类编写名为sharedInstance的方法，该方法只会返回全类共用的单例实例，而不会在每次调用时都创建新的实例。假设有一个类叫TestClass，那么这个共享实例一般会这么写：

```objc
+ (id)sharedInstance {
    static TestClass *sharedInstance = nil;
    @synchronized(self) {
        if (!sharedInstance) {
            sharedInstance = [[self alloc] init];
        }
    }
    return sharedInstance;
}
```

单例模式很容易引起激励讨论，Objective-C的单例尤其如此。线程安全是大家争论的主要问题。为了保证线程安全，上述代码将创建单例实例的代码包裹在**同步块**里。无论是好是坏，反正这种实现方式很常用，这样的代码也随处可见。

不过，GCD引入了一项特性，能使单例实现起来更加容易。所用的函数是：

```objc
void dispatch_once(dispatch_once_t *token, dispatch_block_t block);
```

此函数接受类型为dispatch_once_t的特殊参数，称其为“标记”（token），此外还接受block参数，对于给定token来说，该函数保证相关的block必定会执行，且仅执行一次。首次调用该函数时，必然会执行block的代码，最重要的一点在于，此操作完全是线程安全的。请注意，对于只需执行一次的块来说，每次调用函数时传入的token都必须是完全相同的。因此，开发者通常将标记变量声明在static或global作用域里。

上述实现单例模式所用的sharedInstance方法，可以用此函数来改写：

```objc
+ (id)sharedInstance {
    static TestClass *sharedInstance;
    static dispatch_once_t onceToken;   // typedef long dispatch_once_t;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });
    return sharedInstance;
}
```

使用disptch_once可以简化代码并且彻底保证线程安全，开发者根本无须担心加锁或同步。所有问题都由GCD在底层处理。由于每次调用时都必须使用完全相同的token，所以token要声明成static。把该变量定义在static作用域中，可以保证编译器在每次执行sharedInstance方法时都会复用这个变量，而不会创建新变量。

此外，dispatch_once更高效。它没有使用重量级的同步机制，若是那样做的话，每次运行代码前都要获取锁，相反，此函数采用“原子访问”（atomic access）来查询标记，以判断其所对应的代码原来是否已经执行过。《Effective Objective-C 2.0》的作者在装有64位的Mac OS X 10.8.2系统的电脑上简单测试了性能，分别采用@synchronized方式及dispatch_once方式来实现sharedInstance，结果显示，后者的速度几乎是前者的2倍。

总结：

1. 经常需要编写“只需执行一次的线程安全代码”（thread-safe single-code execution）。通过GCD提供的dispatch_once函数，很容易就能实现此功能；
2. token应该声明在static或global中，这样的话，在把只需一次执行的块传给dispatch_once函数时，传进去的标记也是相同的。

## 本文参考

* 《Effective Objective-C 2.0》