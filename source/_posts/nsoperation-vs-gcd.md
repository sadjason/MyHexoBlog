title: NSOperation v.s GCD
date: 2015-02-07 10:24:41
tags: GCD
categories: iOS

---

## NSOperation

~~NSOperation其实是对task的一种Objective-C封装。~~

`NSOperation`是个抽象类，官方文档是这么描述的：

>The NSOperation class itself is an abstract base class that must be subclassed in order to do any useful work.

值得一提的是，Objective-C中的abstract class和其余语言中的abstract class不太一样，其他语言（主要是静态语言）中的抽象类是不允许进行实例化的，如果对抽象类实例化了，则在编译阶段就可以检查出来；对于Objective-C，对抽象类的说明仅仅停留在文档层面，如果作者想让某个类称为抽象类，则需要在开发文档中进行说明（没有所谓的abstract关键字来修饰），告诉用户不要直接对此类进行实例化，而是去继承它然后自己实现相关方法的定义，但是如果用户实例化该类，则也不会有啥问题（至少在编译阶段是没啥问题）。

如果抽象类的定义只是依靠文档说明，则未免有些太弱了，通常的做法是：You can force a user to at least override certain methods by raising an exception in those methods implementation in your abstract class:

```objc
[NSException raise:NSInternalInconsistencyException 
            format:@"You must override %@ in a subclass", NSStringFromSelector(_cmd)];
```

当然，也可以使用`assert`或`NSAssert`处理。这和Python很多场合对抽象类的处理有些类似，更多讯息参考来自stackoverflow的[Creating an abstract class in Objective-C](http://stackoverflow.com/questions/1034373/creating-an-abstract-class-in-objective-c)。

Foundation Framework提供了两个比较好用的`NSOperation`子类，`NSInvocationOperation`和`NSBlockOperation`。

就我目前的使用经验来看，还没有遇到直接使用`NSOperation`的机会，更多的时候使用AFNetworking框架提供的`AFHTTPRequestOperation`。

**执行operation**

`NSOperation`对象往往关联一段代码，执行这段代码有两种方式：

1. 直接执行其`start`方法；
2. 将`NSOperation`对象添加到`NSOperationQueue`中；

P.S: 关于`NSOperation`的内容还有很多，譬如优先级、completion block之类的，更多信息参考Apple文档[Concurrency Programming Guide](https://developer.apple.com/library/ios/documentation/General/Conceptual/ConcurrencyProgrammingGuide/OperationObjects/OperationObjects.html)，写得很详细。

## NSOperationQueue

在阅读[Concurrency Programming Guide](https://developer.apple.com/library/ios/documentation/General/Conceptual/ConcurrencyProgrammingGuide/Introduction/Introduction.html)的过程中，不免和GCD进行对比，非常疑惑的问题是，`NSOperation`的执行大概是什么样子的？并行？FIFO？在main thread还是other threads？下面就围绕这些问题进行阐述。

### 创建NSOperationQueue对象

创建`NSOperationQueue`有两种方式：

```objc
// 1. NSOperationQueue
NSOperationQueue *operationQueue1 = [[NSOperationQueue alloc] init];
    
// 2. NSOperationQueue
NSOperationQueue *operationQueue2 = [NSOperationQueue mainQueue];
```

先说`mainQueue`，it returns the operation queue associated with the main thread，简而言之，`mainQueue`上的operation（或曰task）都是在main thread上完成的，进一步来说，`mainQueue`中的operation都有控制UI的能力；对于第一种方式`[NSOperationQueue alloc]`，则不然，其关联的thread与main thread或许无关。

所以也可以看出来：**operation的执行thread由其所在的NSOperationQueue决定**。

值得一提的是，同一个`NSOperationQueue`的不同operation的执行线程并非同一个，因为`NSOperationQueue`可能同时涉及多个线程。

并且这和GCD不同，GCD中，dispatch_queue中的block的执行线程不单由queue决定，还与block的派发方式有关。

`NSOperationQueue`对象不存在所谓的`start`之类的启动方法，根据我的理解，自它被创建了就开始工作了。

P.S: 目前还没有到有用到`[NSOperationQueue mainQueue]`的应用，很想知道在哪些场合会用到...

### NSOperationQueue的工作方式

根据[Concurrency Programming Guide](https://developer.apple.com/library/ios/documentation/General/Conceptual/ConcurrencyProgrammingGuide/Introduction/Introduction.html)上介绍，获悉：

1. `NSOperationQueue`是不支持FIFO的，因为`NSOperation`存在priority的概念，所以`NSOperationQueue`是不支持FIFO的；
2. `NSOperationQueue`上的operation是并发执行的，但是用户可以配置同一时刻最多可以并发的operation数量，对应属性是`maxConcurrentOperationCount`，也就是说，如果`maxConcurrentOperationCount`的值设置为1，则`NSOperationQueue`中的operation是串行执行的（串行顺序依然受priority和就绪状态决定，不遵循FIFO）；

### NSOperation的completion block

考虑到接触的频率非常之高，有必要对`NSOperation`的completion block做更多的了解。

>In OS X v10.6 and later, an operation can execute a completion block when its main task finishes executing. You can use a completion block to perform any work that you do not consider part of the main task. For example, you might use this block to notify interested clients that the operation itself has completed. A concurrent operation object might use this block to generate its final KVO notifications.
>To set a completion block, use the setCompletionBlock: method of NSOperation. The block you pass to this method should have no arguments and no return value.

考虑这么一个应用场景，使用`NSOperation`服务下载一张图片，在completion block中更新相关`UIImageView`，图片下载图片后，就会调用completion block。

结合这个应用场景，刚开始，对`NSOperation`的completion block比较疑惑的地方在于，completion block哪一个线程执行，在main thread，还是和`NSOperation`所在线程是同一个？前者的可能性不高；如果是后者，考虑到只有main thread上代码才有控制UI的资格，那么`NSOperation`必须得在main thread上执行，即其所在operation queue的创建方式是`oq = [NSOperationQueue mainQueue];`，但是download image代码难免会产生阻塞，必然会影响UI的交互，显然是不可取。

将`NSOperation`和其completion block割裂来看的做法显然是愚蠢的，也就是说，执行`NSOperation`的completion block的线程和执行`NSOperation`的线程是一样的，也都是由其所在的`NSOperationQueue`决定的。

带着这个问题，我查看了AFNetworking的处理代码，它的做法是这样的，执行网络任务的`NSOperation`所在的`NSOperation`所关联的thread都不是main thread，为了在下载图片（也可能是其他数据）后更新UI，其调用completion block的方式并不是直接调用，而是使用`dispatch_group_async`函数将用户指定的completion block添加到main thread。
简单来说，在用户的completion block基础上加了一层，确保用户的completion block在main thread上执行，认识到这一点很重要！

P.S: AFNetworking的网络任务的CompletionBlock都是在main thread中完成的，这从`AFHTTPRequestOperation`的`setCompletionBlockWithSuccess:fuilure:`方法定义可以看出来。

## NSOperation v.s GCD

关于`NSOperation`（以及`NSOperationQueue`）和GCD的关系，引用《Effective-C 2.0》的描述：

>The similarity to GCD's dispatch queues is not a coincidence. Operation queues came before GCD, but there is no doublt that GCD is based on the principles made popular by operation queues. In fact, from iOS 4 and Mac OS X 10.6 onward, operation queues use GCD under hood.

GCD的同步机制无与伦比，但有些时候使用`NSOperation`或许是更好的选择，相较于GCD，`NSOperation`有如下这些优势：

1. 取消操作（任务）
在GCD中，是没办法对添加到queue的task进行cancel操作的，但是`NSOperation`是可以的；值得一提的是，`NSOperation`取消操作只适用于哪些还没被执行的operation，如果operation正在running，则cancel操作是无效的。

2. Operation dependencies
An operation can have dependencies on as many other operations as it wishes. This enables you to create a hierarchy of operations dictating that certain operations can execute only after another operation has completed successfully. For example, you may have operations to download and process files from a server that requires a manifest file to be downloaded first before others can be processed. The operation to download the manifest file first could be a dependency of the subsequent download operations. If the operation queue were set to allow concurrent execution, the subsequent downloads could execute in parallel but only after the dependent operation had completed.

3. KVO
对`NSOperation`，还可以对其某些属性进行KVO处理，这对于基于C-API的GCD是没办法做到的，至于`NSOperation`哪些属性支持KVO，还需要参考[Concurrency Programming Guide](https://developer.apple.com/library/ios/documentation/General/Conceptual/ConcurrencyProgrammingGuide/Introduction/Introduction.html)。

4. 优先级控制
GCD的dispatch queue中的任务的执行顺序全部遵循FIFO，但是`NSOperationQueue`中对operation的调用更复杂一些，因为它每个operation都有优先级，所以用户可以通过优先级控制实现对任务更复杂的管理。

5. Reuse of operations
因为`NSOperation`是完全支持面向对象的类，所以其对reuse的支持也是相对于GCD的一大优势。

## 本文参考

* [How To Use NSOperations and NSOperationQueues](http://www.raywenderlich.com/19788/how-to-use-nsoperations-and-nsoperationqueues)；
* [Concurrency Programming Guide](https://developer.apple.com/library/ios/documentation/General/Conceptual/ConcurrencyProgrammingGuide/Introduction/Introduction.html)
* [Effective Objective-C 2.0](https://book.douban.com/subject/21370593/)