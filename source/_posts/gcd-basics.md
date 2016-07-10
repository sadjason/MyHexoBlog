title: GCD基础知识
date: 2015-01-26 20:05:56
tags: GCD
categories: iOS

---

## 并行和并发

在英文世界里，「并行」和「并发」的区别比较清晰，「并行」对应「parallelism」，「并发」对应「concurrency」；但在中文世界里二者仅一字之差，两个概念非常容易弄混淆；

各种资料对「并行」和「并发」有各种各样的解释和比喻。我比较喜欢的一种是播客节目[内核恐慌](http://ipn.li/kernelpanic/13)中的主播Rio的描述，大概意思是：
>「并发」和「并行」是一种计算模型，使得计算机能够在同一时间处理多个任务；「并发」表示逻辑概念上的「同时」，「并行」表示物理概念上的「同时」。

简单来说，若说两个任务A和B并发执行，则表示任务A和任务B在同一时间段里被执行（更多的可能是二者交替执行）；若说任务A和B并行执行，则表示任务A和任务B在同时被执行（这要求计算机有多个运算器）；

一句话：并行要求并发，但并发并不能保证并行。

>P.S: 关于并发和并行，《[Grand Central Dispatch In-Depth: Part 1/2](https://www.raywenderlich.com/60749/grand-central-dispatch-in-depth-part-1)》中有更详细的图文解释。

## Dispatch Queues介绍

Dispatch Queues是GCD处理异步任务和并发任务的关键载体，简而言之，在GCD中，将task放入某个Dispatch Queue中，然后等待系统去处理之。

Dispatch queue是object-like structure，也就是说Dispatch queue在Objective-C中不是类结构，而是类类结构。dispatch queue对task的管理都遵循FIFO。GCD提供了一些公共的dispatch queue，但是用户也可以自定义一些dispatch queue；iOS对dispatch queue做了归类，分为三类：
* Serial Dispatch Queue
* Concurrent Dispatch Queue
* Main Dispatch Queue

**Serial Dispatch Queue**

顾名思义，serial dispatch queue中的block按照先进先出（FIFO）的顺序去执行，实际上为单线程执行。即每次从queue中取出一个task进行处理；用户可以根据需要创建任意多的serial dispatch queue，serial dispatch queue彼此之间是并发的；

创建serial dispatch queue使用`dispatch_queue_create`方法，指定其第二个参数为`DISPATCH_QUEUE_SERIAL`（即`NULL`）即可：

```objc
dispatch_queue_t queue;
queue = dispatch_queue_create("com.example.MySerialQueue", DISPATCH_QUEUE_SERIAL);
```

注意：如果不算“Main Dispatch Queue”，系统中不存在所谓的global serial dispatch queue。

>P.S: main dispatch queue其实也算serial dispatch queue，后文有述。

**Concurrent Dispatch Queue**

相对于Serial Dispatch Queue，Concurrent Dispatch Queue一次性并发执行一个或者多个task；和Serial Dispatch Queue不同，系统提供了四个global concurrent queue，使用`dispatch_get_global_queue`函数就可以获取这些global concurrent queue；

和Serial Dispatch Queue一样，用户也可以根据需要自己定义concurrent queue；创建concurrent dispatch queue也使用`dispatch_queue_create`方法，所不同的是需要指定其第二个参数为DISPATCH_QUEUE_CONCURRENT即可：

```objc
dispatch_queue_t queue;
queue = dispatch_queue_create("com.example.MyConcurrentQueue", DISPATCH_QUEUE_CONCURRENT);
```

>P.S: 根据我的理解，对于concurrent queue，其管理的task可能在多个不同thread上执行，至于dispatch queue管理多少个thread是未知的，这要视系统资源而定，用户无需为此烦扰。
创建concurrent queue也是使用`dispatch_queue_create`方法，指定第二个参数为`DISPATCH_QUEUE_CONCURRENT`。

**Main Dispatch Queue**

关于Main Dispatch Queue，《Concurrency Programming Guide》（Apple官方文档）的描述如下：
>The main dispatch queue is a globally available serial queue that executes tasks on the application’s main thread.

根据我的理解，application的主要任务（譬如UI管理之类的）都在main dispatch queue中完成；根据文档的描述，main dispatch queue中的task都在一个thread中运行，即application’s main thread（thread 1）。

>P.S: 所以，如果想要更新UI，则必须在main dispatch queue中处理，获取main dispatch queue也很容易，调用dispatch_get_main_queue()函数即可。

**关于Dispatch Queues的一些误解**

在学习GCD过程中，我一路上有许多关于dispatch的错误理解，如下是总结：

* 不存在所谓的「同步队列」和「异步队列」

「同步」或「异步」描述的是task与其上下文之间的关系，所以，笔者觉得「同步队列」和「异步队列」对于Objective-C的GCD而言是不靠谱的概念；

* Serial Dispatch Queue上的tasks并非只在同一个thread上执行

吾尝以为serial queue上的tasks都是在同一个thread上运行，后来明白了不是这样的，对于那些同步请求的任务，譬如使用dispatch_sync函数添加到serial dispatch queue中的任务，其运行的task往往与所在的上下文是同一个thread；对于那些异步请求的任务，譬如使用dispatch_async函数添加到serial dispatch queue中的任务，其运行的task往往是另一个的thread。举例说明：

```objc
- (void)viewDidLoad {
    [super viewDidLoad];
    
    dispatch_queue_t aSerialQueue = dispatch_queue_create("haha", DISPATCH_QUEUE_SERIAL);
    
    dispatch_sync(aSerialQueue, ^{
        // block 1
        NSLog(@"current 1: %@", [NSThread currentThread]);
    });
    
    dispatch_async(aSerialQueue, ^{
        // block 2
        NSLog(@"current 2: %@", [NSThread currentThread]);
    });
}
    
/*
执行结果：
current 1: <NSThread: 0x7f8f397152f0>{number = 1, name = main}
current 2: <NSThread: 0x7f8f39464db0>{number = 2, name = (null)}
*/
```
block 1和block 2都由同一个serial dispatch queue管理，但它们的执行线程显然不同，前者的执行线程是thread 1，后者的执行线程是thread 2。

* dispatch queue和thread并不存在一对一或者一对多的关系

通过设置断点等测试手段可以知道可能多个dispatch queue共用一个thread，也可能一个dispatch queue中的tasks在多个不同threads上执行。

总之，根据我的理解，thread和dispatch queue之间没有从属关系。

## dispatch_sync和dispatch_async

**dispatch_sync和dispatch_async介绍**

在GCD中，dispatch_sync和dispatch_async是两个函数，前者用于派发同步任务，后者用于派发异步任务，二者使用格式如下：

``` Objective
// dispatch task synchronously
dispatch_sync(someQueue1, ^{
    // do something 1
});
// do something 2
```

```objc
// dispatch task asynchronously
dispatch_async(someQueue2, ^{
    // do something 3
});
// do something 4
```

对于派发同步任务，`do something 2`一定会在`do something 1`完成之后执行，即所谓的“同步”。当执行到dispatch_sync(...)时，其上下文被阻塞，直到dispatch_sync派发的block被执行完毕；并且，根据笔者的理解，**dispatch_sync派发的block的执行线程和dispatch_sync上下文线程是同一个线程**。

>P.S: “dispatch_sync派发的block的执行线程和dispatch_sync上下文线程是同一个线程”这个说法还没有找到权威的、直接明了的佐证。

对于派发异步任务，`do something 4`会立即执行，而不会等到`do something 3`执行完，即所谓“异步”。当执行到dispatch_async(...)时，其上下文不被阻塞，继续运行；
根据笔者的理解，`do something 3`和`do something 4`往往在不在同一个Thread中被执行，即**dispatch_async派发的block的执行线程和dispatch_async上下文线程不是同一个线程**；

来看一个示例，如下有一段代码：

```objc
// 1. create a serial dispatch queue
dispatch_queue_t serial_queue=
dispatch_queue_create("com.zhangbuhuai.test", DISPATCH_QUEUE_SERIAL);    // Thread 1
    
// 2. add tasks to serial dispatch queue
// 1) add a task synchronously
dispatch_sync(serial_queue, ^{
    sleep(3);               // 休眠3秒
    NSLog(@"task 1");       // Thread 1
});
// 2) add a task synchronously too
dispatch_sync(serial_queue, ^{
    NSLog(@"task 2");       // Thread 1
});
// 3) add a task asynchronously
dispatch_async(serial_queue, ^{
    NSLog(@"task 3");       // Thread x  (x != 1)
});
// 4) add a task asynchronously too
dispatch_async(serial_queue, ^{
    NSLog(@"task 4");       // Thread x  (x != 1)
});
    
NSLog(@"test end");         // Thread 1
```

假设建立`serial_queue`所在的上下文运行的thread为Thread 1，则测试结果很可能是`NSLog(@"task 1");`和`NSLog(@"task 2");`也都在Thread 1中执行，而`NSLog(@"task 3");`和`NSLog(@"task 4");`在别的Thread中执行。

执行结果：

```
task 1
task 2
test end
task 3
task 4
```

根据结果我们知道，对于serial dispatch queue中的tasks，无论是同步派发还是异步派发，其执行顺序都遵循FIFO；同样，这个示例也可以直观阐述dispatch_sync和dispatch_async的不同效果。

* dispatch_sync和dispatch_async的使用时机 *

在大多数时候，dispatch_sync和dispatch_async的使用时机非常清晰的：

* 如果派发的task耗时长，不想让上下文线程被阻塞就用dispatch_async；
* 如果要处理的代码比较短，想要实现代码保护（线程安全），选用dispatch_sync；

>P.S: 关于dispatch_sync与线程同步（代码保护）之间的关系，以后会专门阐述。

但有些时候，使用dispatch_sync或者dispatch_async都可以的情况（譬如实现setter）下，就不是那么好选择了。

在《Effective Objective-C 2.0》Item 41（中文版P169）中看到非常重要的一句话：
>...，因为在执行异步派发时，需要拷贝块。

我对这句话的理解是：

* 执行同步派发（dispatch_sync）时，是不需要拷贝block的，这是因为dispatch_sync中所派发的task往往和当前上下文所处同一个Thread；
* 执行异步派发（dispatch_async）时，需要拷贝block，这是因为dispatch_async中所派发的task往往和当前上下文不同于一个Thread；

所以，当选择dispatch_sync或者dispatch_async都可以的情况下，站在效率的角度，如果拷贝block的时间成本过高，则使用dispatch_sync；如果拷贝block的时间成本远低于执行block的时间成本，则使用dispatch_async；

如上所引用的「...，因为在执行异步派发时，需要拷贝块」这句话，在某种程度上佐证了上文提到的两个说法：

* dispatch_sync派发的block的执行线程和dispatch_sync上下文线程是同一个线程；
* dispatch_async派发的block的执行线程和dispatch_async上下文线程不是同一个线程；


## 本文参考

* 《Concurrency Programming Guide》（Apple官方文档）
* 《Effective Objective-C 2.0》