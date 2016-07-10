title: 深入理解dispatch_sync
date: 2015-04-11 21:09:53
tags: GCD
categories: iOS

---

## 写在前面

关于GCD的基础知识，之前写过一篇博客，详见[GCD基础知识](../gcd-basics/)。虽然之前已经梳理过了，但对很多知识点的理解仍然不够透彻...写这篇博客的原因是在阅读AFNetworking代码时遇到一些奇怪的代码。

如下：

```objc
- (NSURLSessionDataTask *)dataTaskWithRequest:(NSURLRequest *)request
                            completionHandler:(void (^)(NSURLResponse *response,
                                                        id responseObject,
                                                        NSError *error))completionHandler
{
    __block NSURLSessionDataTask *dataTask = nil;
    dispatch_sync(url_session_manager_creation_queue(), ^{
        dataTask = [self.session dataTaskWithRequest:request];
    });
    
    [self addDelegateForDataTask:dataTask completionHandler:completionHandler];
    
    return dataTask;
}
```

这段代码是在AFURLSessionManager中定义的，用于创建Data Task（NSURLSessionDataTask实例），这段代码令我比较奇怪的地方在于：使用dispatch_sync同步派发的方式提交任务。这短短几行（6~13行）代码的逻辑是：

1. 以同步派发的方式提交任务 -- 创建一个NSURLSessionDataTask对象；
2. 第11行代码会被阻塞，直到NSURLSessionDataTask对象创建成功；
3. “创建一个NSURLSessionDataTask对象”这个block执行完成，继续执行11~13行代码；

为什么这么处理呢？为什么以dispatch_sync的方式处理“创建Data Task”呢？基于之前的知识，我的猜测如下：

1. 为了效率，“创建Data Task”是一个相对比较耗时间的工作，dataTaskWithRequest:completionHandler:这个方法的上下文可能是main thread，所以不能让“创建Data Task”过长时间阻塞当前线程；
>P.S: 但根据我的理解，dispatch_sync总会阻塞当前线程，直到所提交的任务执行完成，难道是我哪里理解错了，不禁差生了疑惑“dispatch_sync(..., block)”中的block的执行线程是哪个？是dispatch_sync当前上下文的线程还是另外一个线程？好吧，得趁机把这个问题搞清楚。
2. 为了其他目的，我尚且不知道的原因。

无论如何，我得对dispatch_sync有进一步的理解了。

## dispatch_sync使用说明

关于dispatch_sync代码的执行逻辑，恐怕很多人都已经知道了。以如下代码为例：

```objc
NSLog(@"step1");
dispatch_sync(aDispatchQueue, ^{
    NSLog(@"step2");
    //block具体代码
});
NSLog(@"step3");
```

简而言之，dispatch_sync()中的block会被同步派发，其上下文会被阻塞，直到dispatch_block派发的block被执行完成，这段代码的执行结果一定是：
```
step1
step2
step3
```

但问题是，dispatch_sync()的block的执行线程和dispatch_sync上下文所对应的线程是一个线程吗？

关于这个问题曾在[GCD基础知识](../gcd-basics/)中阐述过，也一直以为：dispatch_sync所派发的block的执行线程和dispatch_sync上下文线程是同一个线程（无论上述代码中的aDispatchQueue是serial dispatch queue还是concurrent dispatch queue）。

但在阅读[网络资源](http://www.cnblogs.com/sunfrog/p/3305614.html)的时候发现有这么一种说法：
>编译器会根据实际情况优化代码，所以有时候你会发现block其实还在当前线程上执行，并没用产生新线程。

显然，网友的这个说法的言外之意是“dispatch_sync中的block的执行线程可能就是dispatch_sync上下文所在的线程，也可能不是”。

不禁对自己之前的理解产生了怀疑。孰是孰非？查了很多的资料，但都没能得到一个确切的答案。后来询问了一些不是相对权威的前辈，他们都支持“dispatch_sync所派发的block的执行线程和dispatch_sync上下文线程是同一个线程”这种说法。

我也通过示例代码做了一些验证：

```objc
- (void)viewDidLoad {
    [super viewDidLoad];
    
    NSLog(@"current thread 1: %@", [NSThread currentThread]);
    dispatch_sync(dispatch_queue_create("I.am.a.serial.dispatch.queue", DISPATCH_QUEUE_SERIAL), ^{
        NSLog(@"current thread 2: %@", [NSThread currentThread]);
    });
    
    dispatch_sync(dispatch_queue_create("I.am.a.concurrent.dispatch.queue", DISPATCH_QUEUE_CONCURRENT), ^{
        NSLog(@"current thread 3: %@", [NSThread currentThread]);
    });
}
    
/* 执行结果
current thread 1: <NSThread: 0x7fd96b42ca50>{number = 1, name = main}
current thread 2: <NSThread: 0x7fd96b42ca50>{number = 1, name = main}
current thread 3: <NSThread: 0x7fd96b42ca50>{number = 1, name = main}
*/
```

示例代码的执行结果显然说明dispatch_sync中的block的执行线程总是和dispatch_sync所在的上下文是同一个线程。但我依然怀疑“是不是dispatch_sync中处理的事情太简单了？弄复杂一点看看”，于是我做了如下修改：

```objc
- (void)viewDidLoad {
    [super viewDidLoad];
    
    NSLog(@"current thread 1: %@", [NSThread currentThread]);
    dispatch_sync(dispatch_queue_create("I.am.a.serial.dispatch.queue", DISPATCH_QUEUE_SERIAL), ^{
        dispatch_queue_t aConcurrentDispatchQueue = dispatch_queue_create("he", DISPATCH_QUEUE_CONCURRENT);
        __block long sum = 0;
        for (int i = 0; i < 10000; ++i) {
            dispatch_async(aConcurrentDispatchQueue, ^{
                sum += i;
            });
        }
        NSLog(@"current thread 2: %@", [NSThread currentThread]);
    });
    
    dispatch_sync(dispatch_queue_create("I.am.a.concurrent.dispatch.queue", DISPATCH_QUEUE_CONCURRENT), ^{
        dispatch_queue_t aSerialDispatchQueue = dispatch_queue_create("ha", DISPATCH_QUEUE_SERIAL);
        __block long sum = 0;
        for (int i = 0; i < 10000; ++i) {
            dispatch_sync(aSerialDispatchQueue, ^{
                sum += i;
            });
        }
        NSLog(@"current thread 3: %@", [NSThread currentThread]);
    });
}
```

无论如何增加了dispatch_sync中的block的复杂程度。得到的结果并无区别。

如此这般，我想我可以认定之前的说法：**dispatch_sync派发的block的执行线程总是和dispatch_sync所在的上下文是同一个线程**。

相信以后能发现更加权威的文档资料来佐证这个观点。

## 使用串行同步队列保护代码

回到本文开头部分的代码：

```objc
__block NSURLSessionDataTask *dataTask = nil;
dispatch_sync(url_session_manager_creation_queue(), ^{
    dataTask = [self.session dataTaskWithRequest:request];
});
    
[self addDelegateForDataTask:dataTask completionHandler:completionHandler];
    
return dataTask;
```

上述url_session_manager_creation_queue()函数返回的其实是一个serial dispatch queue，这种组合即所谓的「串行同步队列」。

经过上文的分析，这段代码中使用「串行同步队列」创建NSURLSessionDataTask实例显然不是为了提高效率，那么它的意义是什么呢？

好吧，既然搞不懂，就查资料吧！

翻开《Effective Objective-C 2.0》，看到item41 -- 《多用派发队列，少用同步锁》，我想，这段代码中使用「串行同步队列」的唯一解释是：为了保证`[self.session dataTaskWithRequest:request]`的线程安全。

至于为什么要保证`[self.session dataTaskWithRequest:request]`这个操作的线程安全，考虑到dataTaskWithRequest:这个方法是在iOS中定义的（只提供了接口，看不到源码），此处就不作详细分析了，我猜测这个方法不是「线程安全」方法吧。

## dispatch_sync使用注意事项

学习GCD当然要读Apple的官方文档《Concurrency Programming Guide》，其中包括关于使用dispatch_sync的提示：

>Important: You should never call the dispatch_sync or dispatch_sync_f function from a task that is executing in the same queue that you are planning to pass to the function. This is particularly important for serial queues, which are guaranteed to deadlock, but should also be avoided for concurrent queues.

简单来说，在dispatch_sync嵌套使用时要注意：不能在一个嵌套中使用同一个serial dispatch queue，因为会发生死锁；

假设有如下这么一段代码要执行：

```objc
- (void)test {
    dispatch_queue_t aSerialDispatchQueue =
    dispatch_queue_create("I.am.an.iOS.developer", DISPATCH_QUEUE_SERIAL);
       
    dispatch_sync(aSerialDispatchQueue, ^{
        // block 1
        NSLog(@"Good Night, Benjamin.");
        dispatch_sync(aSerialDispatchQueue, ^{
            // block 2
            NSLog(@"Good Night, Daisy.");
        });
    });
}
```

自己试着执行以下就会发现：`Good Night, Daisy.`这一句永远都无法被打印出来，原因很简单，程序产生了死锁。为什么会产生死锁呢？

可以想象aSerialDispatchQueue在底层实现中有一把“锁”，这把锁确保serial dispatch queue中只有一个block被执行，当执行到block 1代码时，这把锁为block 1所持有，当block 1执行完了，会释放之；然而block 1同步派发了一个任务block 2，同步派发意味着block 1会被阻塞，直到block 2被执行完成；但是这里产生了矛盾，block 2顺利执行的前提是aSerialDispatchQueue的这把“锁”被block 1释放，但是block 1释放这把“锁”的前提是block 1执行完成，而block 1执行完的前提是block 2执行完成；所以造成的局面是“block 2等待block 1执行完成置放‘锁’”，同时“block 1等待block 2执行完成”，这就是典型的deadlock。

这一段代码还好，比较容易避免，但是如果对GCD理解不深，更多的时候容会写出如下代码：

```objc
- (void)viewDidLoad {
    [super viewDidLoad];
    
    // 巴拉巴拉，做了很多事情
    
    NSLog(@"Good Night, Benjamin.");
    dispatch_sync(dispatch_get_main_queue(), ^{
        // refresh UI
    
        NSLog(@"Good Night, Daisy.");
    });
}
```

这段代码的问题其实和上一段代码类似，只不过这里的serial dispatch queue恰好是main queue。

上述的deadlock问题主要针对「同步串行队列」，对于「同步并行队列」，根据我的理解应该不存在这个deadlock问题，但是《Concurrency Programming Guide》明确说了：
>...This is particularly important for serial queues, which are guaranteed to deadlock, but should also be avoided for concurrent queues.

P.S: 目前还不理解《Concurrency Programming Guide》中的这个说辞。
P.S: 根据我的理解，「串行队列」在底层实现中应该有一把“锁”用来保证「串行队列」中的block有且仅有一个block被执行；但是「并行队列」的实现机制是啥呢？希望以后能回答这个问题。也许只有回答了这个问题，才能理解“... but should also be avoided for concurrent queues”。

## 本文参考

* 《Effective Objective-C 2.0》
* 博文《[iOS多线程的初步研究（八）-- dispatch队列](http://www.cnblogs.com/sunfrog/p/3305614.html)》
* 《Concurrency Programming Guide》