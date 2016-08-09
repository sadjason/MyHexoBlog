title: ReactiveCocoa实践
date: 2016-07-29 10:23:41
tags: ReactiveCocoa
categories: iOS

---

接着[RACSignal实践](/reactive-cocoa-part-2/)继续写，本文介绍ReactiveCocoa的简单应用。

## 使用ReactiveCocoa代替原生的消息传递机制

Cocoa框架为对象之间的通信提供了各种消息传递机制，包括：target-action、delegate、notification、KVO以及block。ReactiveCocoa可以代替它们所有，换句话说，ReactiveCocoa为这些通信方式提供了统一的消息传递机制。

P.S: Objc的[消息传递机制](https://www.objccn.io/issue-7-4/)介绍了各种消息传递机制之间的差异性。

使用ReactiveCocoa代替这些消息传递机制的本质是在它们的基础上进行了进一步包装，全部都包装成signal-subscriber模式。

在我看来，除了对delegation稍微需要点耐心，大部分都比较容易理解。

### 代替Delegation

Objective-C中一个典型的delegation应用套路如下：

1. 创建一个协议`P`；
2. 在类`A`中声明一个遵守`P`的delegate属性；
3. 在类`B`中创建一个`A`类型实例`a`，并设置`a.delegate = self`；
4. `B`遵守协议`P`；
5. 在`A`的实现逻辑中回调delegation；

伪代码如下：

```objc
/******************* P *******************/
@protocol P                                     // 1

- (void)protocolMethod;

@end

/******************* A *******************/

@interface ()

@property (nonatomic, weak) id<P> delegate      // 2

@end

@implementation A

- (void)someMethod {
    [self.delegate protocolMethod];             // 5
}

@end

/******************* B *******************/

@implementation B

- (void)createInstanceA {
    A *a = [A new];                             // 3
    a.delegate = self;
}

- (void)protocolMethod { /* ... */ }            // 4

@end
```

那么如何使用RAC代替delegation呢？RAC能够压缩上述的5个步骤吗，甚至一行代码解决？

RAC并不能让上述的传统delegation实现逻辑更简单，在RAC的基础上实现一个delegation的步骤大概也是上述的5步，只是步骤`4`略有不同。

**使用RAC不需要显式遵守协议**，即无需实现协议的方法，让RAC帮忙完成即可。简单来说，上述`B`的实现逻辑可改为：

```objc
/******************* B *******************/

@implementation B

- (void)createInstanceA {
    A *a = [A new];                               // 3
    a.delegate = self;
    [[self rac_signalForSelector:@selector(protocolMethod)
                    fromProtocol:@protocol(P)]
        subscribeNext:^(id x) {
        /* ... */
    }];
}

// 注释掉
// - (void)protocolMethod { /* ... */ }         // 4
// 哪怕不将protocolMethod注释掉，其中的代码也不会被调用。

@end
```

`rac_signalForSelector:fromProtocol:`在底层实现里为`B`自动添加了`protocolMethod`的实现逻辑，因此无需再显式编写`protocolMethod`方法的实现代码。原来需要放在`protocolMethod`的代码可以搬到`subscribeNext:`引导的block中。

>P.S: 在理解「RAC代替delegation」时，不要期待RAC会将「声明protocol」「设置delegate」「回调delegate方法」等步骤全部省掉。

单个方面来看，RAC把delegation的实现搞复杂了，也变得不那么容易理解了，这样有什么好处呢？

当`protocolMethod`方法独立完成某项工作，不会和其他逻辑耦合在一起时，使用RAC代替delegation纯属蛋疼行为，实在没有意义；但是当`protocolMethod`方法实现的逻辑和别的逻辑耦合在一起时（譬如它维护的某个属性在别的地方也会用到），或者用更专业的话来讲，当`protocolMethod`维护的状态和其他逻辑维护的状态有依赖关系时，RAC能够让这些杂乱的依赖关系显得更清晰可控。[Replacing the Objective-C “Delegate Pattern” with ReactiveCocoa](https://spin.atomicobject.com/2014/02/03/objective-c-delegate-pattern/)中的示例能够更生动地说明这一点！

RAC提供了`rac_signalForSelector:`和`rac_signalForSelector:fromProtocol:`这两个方法来帮助实现「代替传统的delegation」。这两个方法的实现逻辑差不多，只是后者会帮助做一下逻辑校验，以防将selector写错了。

最后，需要说明的是，`rac_signalForSelector:`和`rac_signalForSelector:fromProtocol:`返回的signal类型是`RACSubject`；`subscribeNext:^(id x) {}`传入的参数x的类型是`RACTuple`，有点类似于数组，delegation参数就被盛装于此。

P.S: 不太明白，为什么不使用`NSArray`，而另外弄一个`RACTuple`？

### 代替Target-Action

使用RAC的`rac_signalForControlEvents:`非常容易实现监听事件，如下：

```objc
UIButton *btn = [UIButton new];
[[btn rac_signalForControlEvents:UIControlEventTouchUpInside] subscribeNext:^(id x) {
    // 监听touch up inside事件
    // x是UIButton类型，即btn本身
}];
```

`rac_signalForControlEvents:`返回的信号类型为`RACDynamicSignal`。

### 代替KVO

`rac_valuesAndChangesForKeyPath:options:observer:`和`rac_valuesForKeyPath:observer:`可用来监听对象属性的变化，这两个方法差不多，只是后者默认将option值设置为`NSKeyValueObservingOptionInitial`。

这俩方法中都有`observer`参数，有什么用呢？

主要是为了解决KVO的remove observer痛点！众所周知，`addObserver:forKeyPath:options:context:`和`removeObserver:forKeyPath:`是对应关系，忘记了后者会造成内存泄漏问题（是内存泄漏问题吗？不太记得了）。

使用RAC代替KVO的好处之一就是不用再手动remove observer了。

`rac_valuesAndChangesForKeyPath:options:observer:`和`rac_valuesForKeyPath:observer:`的使用非常简单，此处略过。

### 代替Notification

`rac_addObserverForName:object:`可用来监听通知，其使用也非常简单，使用它后也无需再自己手动remove observer了。