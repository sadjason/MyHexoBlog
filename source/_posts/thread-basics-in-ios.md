title: iOS Thread基础知识
date: 2015-04-17 20:37:32
tags: GCD
categories: iOS

---

## 写在前面

无论是什么开发环境，多线程总是一个绕不开的概念，本文是对iOS开发过程中多线程基础知识的汇总。本文最主要的参考资料是《[Threading Programming Guide](https://developer.apple.com/library/ios/documentation/Cocoa/Conceptual/Multithreading/Introduction/Introduction.html)》，为了方便，下文所指的**文档**除非有特别说明，否则都是指《Threading Programming Guide》；本文还参考《UNIX环境高级编程》，该书在CS领域地位非常之高，大家习惯称之为APUE，显得逼格更高，本文也不避俗，下文中使用APUE代指《UNIX环境高级编程》。

## 多线程编程的几种方式

**Cocoa有哪几种方法多线程编程方式？**

iOS开发领域多线程编程方式有很多中，这里只谈论基于Cocoa的多线程编程，不包括比较底层的C语言级别的多线程编程（譬如基于POSIX API）。

主要有三种方式：
* 基于`NSThread`
* 基于GCD
* 基于`NSOperation`

## NSThread

相较于后两种多线程编程方式，基于`NSThread`的多线程编程麻烦多了，需要考虑很多问题，包括：

* 手动创建线程
* 配置线程属性
* 线程内存池管理
* 终止线程

本文只是为了做个简介，如下内容只涉及两方面：创建线程、运行线程。

**创建线程**

`NSThread`是一个类名，顾名思义，它是iOS对thread的一种封装。基于`NSThread`创建线程有两种方式：

* 使用`detachNewThreadSelector:toTarget:withObject:`类方法来生成一个新的线程
* 创建一个新的`NSThread`对象，并调用它的`start`方法（在iOS和Mac OS X 10.5+版本才支持）

`detachNewThreadSelector:toTarget:withObject:`类方声明如下：

```objc
+ (void)detachNewThreadSelector:(SEL _Nonnull)aSelector
                       toTarget:(id _Nonnull)aTarget
                     withObject:(id _Nullable)anArgument;
/* 参数说明：
 * aSelector
 *     The selector for the message to send to the target. 
 *     This selector must take only one argument and must not have a return value.
 * aTarget  
 *     The object that will receive the message aSelector on the new thread.
 * anArgument   
 *     The single argument passed to the target. May be nil.
 */
```

如果调用该接口，会立马创建并启动一个新线程；如果想创建一个线程但并不立马启动它，则使用如下方式：

```objc
NSThread *myThread = [[NSThread alloc] initWithTarget:self
                                             selector:@selector(doNothing)
                                               object:nil];
```

此方法和`detachNewThreadSelector:toTarget:withObject:`方法初始化一个新的`NSThread`实例需要相同的开销。然而它并没有启动一个线程，若需要启动一个线程，可以显式调用对象的`start`方法，即`[myThread start];`。

值得一提的是，如果你拥有一个`NSThread`对象，它的线程当前正在运行，你可以给该线程发送消息的唯一方法是在你应用程序里面的任何对象使用`performSelector:onThread:withObject:waitUntilDone:`方法。

**Using NSObject to Spawn a Thread**

除了以上两种方式之外，还可以使用`NSObject`派生出一个thread。iOS和OS X v10.5+版本都可以使用接口`performSelectorInBackground:withObject:`来派生出一个新thread，这个API和上文的`detachNewThreadSelector:toTarget:withObject:`有些类似，不同的是它没有target参数，因为target就是调用者本身，这里需要传入的第一个参数对应的是调用者的某个方法。

P.S: 不晓得这个接口有什么应用场景？

## GCD

本博客有多篇关于GCD的博客，详见[GCD](/tags/GCD/)。

## NSOperation

《[NSOperation v.s GCD](/nsoperation-vs-gcd/)》一文中对`NSOperation`的使用有详细说明。

## 线程同步

线程同步可是多线程领域的热门话题。线程是不存在独立内存空间的，同属一个进程的线程们共享所属进程的内存空间。当多个控制线程共享相同的内存时，需要确保每个线程看到一致的数据视图。如果每个线程使用的变量都是其他线程不会读取或修改的，那么就不存在一致性（同步）问题；同样地，如果变量是只读的，那么线程同时读取该变量也不会有一致性（同步）问题。但是，当某个线程可以修改变量，而其他线程也可以读取或者修改这个变量时，就需要对这些线程进行同步，以确保它们在访问变量的存储内容时不会访问到无效的数值。

《[Threading Programming Guide](https://developer.apple.com/library/ios/documentation/Cocoa/Conceptual/Multithreading/ThreadSafety/ThreadSafety.html#//apple_ref/doc/uid/10000057i-CH8-124887)》中介绍了5种同步工具：

* 原子操作
* 内存屏障和volatile变量
* 锁
* 条件变量
* Perform Selector Routines

根据我的理解，第二种同步工具「内存屏障和volatile变量」的本质是从控制编译的思路解决线程同步问题，这个太高深，本文就不涉及了，况且文档明确警告慎用：
>Because both memory barriers and volatile variables decrease the number of optimizations the compiler can perform, they should be used sparingly and only where needed to ensure correctness. For information about using memory barriers, see the OSMemoryBarrier man page.

至于最后一种同步工具在我看来把它划分到线程通信或许更合适。

因此本文只介绍其余3种同步工具。

### 原子操作

原子操作是同步的一个简单的形式，它处理简单的数据类型。对于简单的操作，比如递增一个计数器，原子操作比使用锁（下文会提到）具有更高的性能优势。

对原子操作的理解是非常基础的知识，本文就不多赘述了，我好奇的是Mac OS X/iOS的原子操作的实现机制。刚开始怀疑它是对下文要提到的互斥锁的一种封装，但看官方文档感觉不像，毕竟官方文档明确表明，对于简单的操作，原子操作比互斥锁具有更高的性能。如果是对互斥锁的封装，性能怎么可能会超过互斥锁呢？博文《[原子操作的实现原理](http://www.infoq.com/cn/articles/atomic-operation/)》对这个问题进行了比较详细的分析，虽然不确定详细实现，但能确定Apple的原子性实现是基于非常底层的处理。

在iOS开发中，我们如何使用**原子操作**这个同步工具呢？最常见的莫过于是对属性进行原子保护，只需要使用`atomic`修饰需要保护的属性即可；至于在其他场合使用原子操作工具，可以参考`/usr/include/libkern/OSAtomic.h`，该文件提供了大量的原子操作接口。

### 锁

**锁是最常用的同步工具**。你可以是使用锁来保护临界区（critical section），这些代码段在同一个时间只能允许被一个线程访问。比如，一个临界区可能会操作一个特定的数据结构，或使用了每次只能一个客户端访问的资源。根据应用场景不同，锁有许多划分：
* 互斥锁（mutex）
* 递归锁（recursive lock）
* 读写锁（read-write lock）
* 分布锁（distributed lock）
* 自旋锁（spin lock）
* 双重检查锁（double-checked lock）

不同开发语言中定义的锁的种类和应用场景可能不尽相同，如上这些锁是在文档中涉及过的；此外，和其他资源一样，下文中的锁资源只涉及Objective-C类型资源，更底层的譬如POSIX接口资源就不涉及了。

**互斥锁（Mutex）**

互斥锁在APUE中被称为**互斥量**，**互斥锁**的叫法更为广泛。根据我的理解，和其他锁一样，互斥锁的本质是基于信号量的封装，文档中明确写道：A mutex is a type of semaphore that grants access to only one thread at a time.

简单来说，**互斥锁是一种特殊的变量，用来保护同一时间只有一个线程访问数据**。顾名思义，可以把它看做一把锁，在访问共享资源前对之进行加锁（lock），在访问完后释放之（unlock）。对mutex加锁后，其他任何试图再次对mutex加锁的线程将会被阻塞直到当前线程释放mutex（unlock）。如果释放mutex时有多个线程阻塞，则所有在该mutex上的阻塞线程都会变成可运行状态，第一个变为可运行状态的线程可以对mutex加锁，进而访问共享资源，而其他线程将会看到互斥锁依然被锁住，只能回去再次等待它重新被释放。

P.S: 容易想到，加锁和释放锁的操作都是原子的！

值得注意的是，在开发时需要确保所有的线程必须遵守相同的数据访问规则：在使用共享资源时先获取锁，使用完了便释放锁。只有这样，互斥机制才能正常工作。操作系统并不会做数据访问的串行化，如果允许其中的某个线程在没有得到锁的情况下也可以访问共享资源，那么即使其他的线程在使用共享资源前都获取了锁，也还是会出现数据不一致的情况。

互斥锁的使用非常简单，基本套路就是：

```objc
[lock lock];            // 上锁
handle common source    // 处理公共资源
[lock unlock];          // 释放锁
```

如下是一个比较详细的示例：

```objc
@implementation ViewController {
    NSLock *testLock;
}
 
- (void)viewDidLoad {
    [super viewDidLoad];
    
    // 初始化「锁」
    testLock = [[NSLock alloc] init];
    
    // 1号线程
    NSThread *thread1 = [[NSThread alloc] initWithTarget:self
                                                selector:@selector(commonSource)
                                                  object:nil];
    thread1.name = @"1号线程";
    [thread1 start];

    // 2号线程
    NSThread *thread2 = [[NSThread alloc] initWithTarget:self
                                                selector:@selector(commonSource)
                                                  object:nil];
    thread2.name = @"2号线程";
    [thread2 start];
}
 
- (void)commonSource {
    [testLock lock];    // 上锁
    NSLog(@"%@ 已经锁住公共资源", [NSThread currentThread].name);
    sleep(5);
    NSLog(@"%@ 将要释放公共资源", [NSThread currentThread].name);
    [testLock unlock];  // 释放锁
}
 
@end
```

`NSLock`提供的接口除了`lock`和`unlock`之外，还有`-(BOOL)tryLock`和`-(BOOL)lockBeforeDate:`这两个API。

`tryLock`尝试给`NSLock`对象上锁，若上锁成功，则返回`YES`，否则，返回`False`，但它不会阻塞线程，`tryLock`的一般用法是：

```objc
if ([testLock tryLock]) {
    handle common source    // 处理公共资源
    [testLock unlock];
}
```

`lockBeforeDate:`，它给获取锁设定一个时间，在指定的date之前暂时阻塞线程（如果没有获取锁的话），如果到期还没有获取锁，则线程被唤醒，函数立即返回`NO`；

**使用@synchronized指令**

`@synchronized`指令而不是`@synthesize`哦！`@synchronized`是实现互斥锁的一种简洁版本。使用方式如下：

```objc
- (void)myMethod:(id)anObj {
    @synchronized(anObj) {
        // Everything between the braces is protected by the @synchronized directive.
    }
}
```

传给`@synchronized`指令的参数`self`是几个意思呢？文档说明如下：
>The object passed to the @synchronized directive is a unique identifier used to distinguish the protected block. If you execute the preceding method in two different threads, passing a different object for the *anObj* parameter on each thread, each would take its lock and continue processing without being blocked by the other. If you pass the same object in both cases, however, one of the threads would acquire the lock first and the other would block until the first thread completed the critical section.

**递归锁（recursive lock）**

使用**互斥锁**这个同步工具，哪怕各个线程在使用公共资源时都严格遵守「上锁-使用-释放」规则，仍然会有问题：当某个线程在持有锁的情况下再次获取锁时会造成死锁，如下：

```objc
- (void)commonSource {
    [testLock lock];    // 上锁
    [testLock lock];    // 上锁
    NSLog(@"%@ 已经锁住公共资源", [NSThread currentThread].name);
    sleep(5);
    NSLog(@"%@ 将要释放公共资源", [NSThread currentThread].name);
    [testLock unlock];  // 释放锁
}
```

**递归锁**和**互斥锁**差不多，只是不同的是它允许线程可以多次获取锁而不会造成死锁。

在Cocoa中，**递归锁**对应的类型是`NSRecursiveLock`，其接口和`NSLock`完全一致，就不再使用示例说明其用法了。

**读写锁（read-write lock）**

Cocoa没有提供**读写锁**相关的类型，引自文档：
>The system supports read-write locks using POSIX threads only. For more information on how to use these locks, see the pthread man page.

本文只对**读写锁**进行简要介绍。

**读写锁**与**互斥锁**类似，不过**读写锁**允许更高的并行性。**互斥锁**要么是锁住状态要么是不加锁状态，而且一次只能有一个线程对其加锁。**读写锁**可以有三种状态：读模式下加锁状态，写模式下加锁状态，不加锁状态。一次只有一个线程可以占有**写模式**的读写锁，但是多个线程可以同时占有**读模式**的读写锁。

当**读写锁**是**写加锁**状态时，在这个锁被解锁之前，所有试图对这个锁加锁的线程都会被阻塞。

当**读写锁**是**读加锁**状态时，所有试图以**读模式**对它进行加锁的线程都可以得到访问权，但是如果线程希望以**写模式**对此锁进行加锁，它必须阻塞直到所有线程释放读锁。虽然**读写锁**的实现各不相同，但当**读写锁**处于**读模式**锁住状态时，如果有另外的线程试图以**写模式**加锁，读写锁通常会阻塞随后的**读模式**锁请求，这样可以避免**读模式**长期占用，而等待的**写模式**请求一直得不到满足。

读写锁非常适合于对数据结构读的次数远大于写的情况。

当**读写锁**在**写模式**下时，它所保护的数据结构就可以被安全地修改，因为当前只有一个线程可以在**写模式**下拥有这个锁。

当**读写锁**在**读模式**下时，只要线程获取了读模式下的读写锁，该锁所保护的数据结构可以被多个获得读模式锁的线程所保护。

读写锁也叫做「共享-独占锁」，当**读写锁**以**读模式**锁住时，它是以共享模式锁住的，当它以**写模式**锁住时，它是以独占模式锁住的。

**分布锁（distributed lock）**

**分布锁**是进程级别的同步工具：
>A distributed lock provides mutually exclusive access at the process level. Unlike a true mutex, a distributed lock does not block a process or prevent it from running. It simply reports when the lock is busy and lets the process decide how to proceed.

本文略过！

**自旋锁（spin lock）**

引自百度百科：
>**自旋锁是专为防止多处理器并发而引入的一种锁**。何谓**自旋锁**？它是为实现保护共享资源而提出一种锁机制。其实，自旋锁与互斥锁比较类似，它们都是为了解决对某项资源的互斥使用。无论是互斥锁，还是自旋锁，在任何时刻，最多只能有一个保持者，也就说，在任何时刻最多只能有一个执行单元获得锁。但是两者在调度机制上略有不同。对于互斥锁，如果资源已经被占用，资源申请者只能进入睡眠状态。但是自旋锁不会引起调用者睡眠，如果自旋锁已经被别的执行单元保持，调用者就一直循环在那里看是否该自旋锁的保持者已经释放了锁，「自旋」一词就是因此而得名。

Mac OS X/iOS系统没有提供自旋锁的实现：
>The system does not provide any implementations of spin locks because of their polling nature, but you can easily implement them in specific situations. For information on implementing spin locks in the kernel, see *Kernel Programming Guide*.

本文略过！

**双重检查锁（double-checked lock）**

关于双重检查锁，能够找到的资料不多，引自文档：
>A double-checked lock is an attempt to reduce the overhead of taking a lock by testing the locking criteria prior to taking the lock. Because double-checked locks are potentially unsafe, the system does not provide explicit support for them and their use is discouraged.

## 关于锁的一些总结

**避免死锁**

锁是最常用的同步工具，同时，使用锁作为同步工具也存在一些问题，最经典的问题莫过于死锁。

上文在介绍递归锁时已经介绍了一种死锁情况：如果线程试图对一个**互斥锁**加锁两次，那么它自身就会陷入**死锁**状态。除此之外，还有一些更不明显的方式也可能会产生死锁。例如，程序中使用多个**互斥锁**时，如果允许一个线程一直占有第一个**互斥锁**，并且在试图锁住第二个互斥锁时处于阻塞状态，但是拥有第二个**互斥锁**的线程也在试图锁住第一个**互斥锁**，此时也会发生死锁，因为此时两个线程都在相互请求对方拥有的资源，所以这两个线程都无法向前运行，于是就产生**死锁**。

**原子操作和互斥锁的代价**

在很多场合下，既可以选择使用**原子操作**作为同步工具，也可以选择使用**互斥锁**；面对这种场景，应该选择哪个呢？站在性能的角度，当然应该选择性能更棒的，针对二者的性能对比，文档提供了一张表：

<div class="imagediv" style="width: 780px; height: 320px">{% asset_img atomic-vs-metux.png Atomic v.s Metux %}</div>

## 条件变量

关于条件变量，文档的描述如下：
>**A condition is another type of semaphore** that allows threads to signal each other when a certain condition is true. Conditions are typically used to indicate the availability of a resource or to ensure that tasks are performed in a specific order. When a thread tests a condition, it blocks unless that condition is already true. It remains blocked until some other thread explicitly changes and signals the condition.

**条件变量**为多个线程提供了一个会合的场所，和**锁**不同，**条件变量**允许线程以无竞争的方式等待特定的条件发生。

P.S: 所谓「无竞争」，根据我的理解，指的是线程之间不会以无序混乱的方式抢占公共资源，对于互斥锁，如果不考虑两个线程的时间先后顺序，它们抢占公共资源的概率均是50%，谁先获得是未知的。

基于**条件变量**的同步操作，主要包括两个动作：一个线程等待「条件变量的条件为真」；另一个线程使「条件成立」。前者**读**（或曰测试）条件变量，后者修改条件变量，当然需要保证其操作原子性，所以一般使用互斥锁保护条件变量本身。

文档介绍了一种需要条件变量作为同步工具的应用场景：
>One way you might use a condition is to manage a pool of pending events. The event queue would use a condition variable to signal waiting threads when there were events in the queue. If one event arrives, the queue would signal the condition appropriately. If a thread were already waiting, it would be woken up whereupon it would pull the event from the queue and process it. If two events came in to the queue at roughly the same time, the queue would signal the condition twice to wake up two threads.

OK，来介绍`NSCondition`（Cocoa中实现condition的类型）的使用方法。`NSCondition`的接口非常少，如下：

```objc
@interface NSCondition : NSObject <NSLocking>
 
- (void)wait;         // 等待「条件满足」，被唤醒
- (BOOL)waitUntilDate:(NSDate *)limit;
- (void)signal;       // 通知条件已满足，唤醒某个等待的线程
- (void)broadcast;    // 通知条件已满足，唤醒所有等待的线程
 
@end
```

`NSCondition`将用于保护condition原子性的互斥锁和condition打包封装到了一起，它遵循`NSLocking`协议，所以实现了`lock`和`unlock`这两个方法，用户不用额外创建锁。`NSCondition`的一般使用形式如下：

```objc
// myCondition: NSCondition object
// someCheckIsTrue: test condition
    
- (void)method1 {
    [myCondition lock];             // 设置锁，防止condition被别个线程修改
    while (!someCheckIsTrue) {      // 测试condition
        [myCondition wait];         // 睡眠，等待被唤醒
    }
    
    // Do something.
    
    [myCondition unlock];
}
 
- (void)method2 {
    [myCondition lock];             // 设置锁，防止别个线程读取condition
    
    // Do something.
    
    someCheckIsTrue = YES;          // 更新condition
    [myCondition signal];           // 通知condition已满足，唤醒某个正在等待的线程
    [myCondition unlock];
}
```

对代码进行说明：
1. 所谓test condition，并不是指test `NSCondition`对象，被test的condition可以是任何形式，譬如`[myArray count] == 0`等；
2. method1中`while`循环不能使用`if`代替，在`while`循环中检查条件，被唤醒后会再次test condition，若不满足会继续睡眠；
3. method2中的最后两行代码在我看来顺序无所谓，因为condition已经在上一行代码中更新完了，晚点儿解锁也没问题；

## 其他线程同步方式

对于iOS开发而言，如上所介绍的线程同步工具并非最好的选择，根据《Effective Objective-C 2.0》的说法，以上同步工具都不如使用GCD工具。


## 本文参考

* [UNIX环境高级编程](https://book.douban.com/subject/1788421/)
* [原子操作的实现原理](http://www.infoq.com/cn/articles/atomic-operation/)
* [Threading Programming Guide](https://developer.apple.com/library/ios/documentation/Cocoa/Conceptual/Multithreading/Introduction/Introduction.html)