title: 初识ReactiveCocoa
date: 2016-07-28 22:31:28
tags: ReactiveCocoa
categories: iOS

---

## 关于ReactiveCocoa

ReactiveCocoa是iOS环境下的一个函数式响应式编程框架。**函数式响应式编程**（Functional Reactive Programming, FRP）这个概念由微软的牛X[团队](http://reactivex.io/)提出，ReactiveCocoa是受其[启发](https://msdn.microsoft.com/en-us/data/gg577609.aspx)而诞生的框架，应用范围非常广泛。

ReactiveCocoa至今已经发展出第4个版本了，而本文所针对的ReactiveCocoa版本是v2.5；照常例，下文将使用RAC代替ReactiveCocoa。

P.S: ReactiveCocoa 3.0已经放弃对iOS 7的支持，因此如果支持iOS 7，必须得使用更低的版本。因此，若想通过CocoaPods安装ReactiveCocoa v2.5，需要指明platform是低于v8.0的版本，譬如`platform :ios, '7.0'`。如果使用Swift语言，[RxSwift](RxSwift)似乎是更好的选择。

在本文中，理解Signal是重头戏，个人认为，结合Sequence理解信号（Signal）比较容易一些，因此会花一些篇幅阐述Sequence；除此之外，还有必要介绍高阶函数和函数式编程。

P.S: Sequence在RAC中的地位越来越低，在Swift环境中，干脆被干掉了。

## 高阶函数和函数式编程

从维基百科的[解释](https://en.wikipedia.org/wiki/Higher-order_function)来看，一个高阶函数需要满足如下两个条件：

* 一个或者多个函数作为输入；
* 有一个函数作为输出。

P.S: 维基百科代表不了权威，至少在高阶函数这个问题上，网友存在分歧，有的人认为这两个条件满足一个即可；另一部分人认为二者皆不可缺。为了配合我所参考的资料 -- [iOS的函数响应型编程](https://www.gitbook.com/book/kevinhm/functionalreactiveprogrammingonios/details) -- 的说法，我只好选择后者。

有必要在了解ReactiveCocoa之前认识一下函数式编程，可以参考[函数式编程初探](http://www.ruanyifeng.com/blog/2012/04/functional_programming.html)和[用RXCollections进行函数式编程](https://kevinhm.gitbooks.io/functionalreactiveprogrammingonios/content/chapter3/functional_programming_with_RXCollections.html)。

## 流和序列

RAC有一个核心概念叫**流**（Stream），它是data的序列化抽象。下文提到的[LimBoy](http://limboy.me/)把Signal比喻成水管，我认为完全可以把Stream也比喻成水管，data就好像流淌在水管中的玻璃球，玻璃球直径和水管内径相仿，只能依次流过。以我们当前的认知水平，data的序列化就好像是一个数组或者列表。Data在序列中依次被排好序，它们能像水管中的玻璃球一样流出来。

P.S: 「水管」和「玻璃球」比喻，我所知道的出处是[LimBoy](http://limboy.me/)，下文再解释Signal时会再次说明。

在RAC中，使用类`RACStream`抽象Stream，该类是个抽象类，本身不能被用来定义实例，用得更多的是它的两个子类：`RACSequence`和`RACSignal`。换句话说，在RAC中，有两种特定的流：序列（`RACSequence`）和信号（`RACSignal`）。

可以使用RAC为`NSArray`定义的category方法`rac_sequence`，将数组桥接为一个序列：

```objc
NSArray *array = @[@1, @2, @3, @4];
RACSequence *sequence1 = [array rac_sequence];
```

可以对生成的`sequence1`进行map、filter等处理，得到一个新的序列，前者对序列中的每个data进行处理，后者将序列中的data进行匹配过滤。下面例子先进行map处理，对序列中的data进行平方处理，得到一个新的序列`sequence2`，然后再对新序列进行filter处理，将偶数给剔除掉，得到`sequence3`，最后，再将`sequence3`还原为`NSArray`，并将所有元素给打印出来：

```objc
RACSequence *sequence2 = [sequence1 map:^id(NSNumber * value) {
    return [NSNumber numberWithInteger:value.integerValue*value.integerValue];
}];
RACSequence *sequence3 = [sequence2 filter:^BOOL(NSNumber * value) {
    return value.integerValue % 2 == 1;
}];
NSLog(@"%@", sequence3.array);
// print: (1, 9)
```

显然，上述的两个步骤其实可以链式串起来，这样至少可以将`sequence2`这个临时变量给省掉。

到了这里，应该对Sequence有了基本的理解了。

P.S: 文笔不佳，阅读《[iOS的函数响应型编程](https://www.gitbook.com/book/kevinhm/functionalreactiveprogrammingonios/details)》的前两章效果会更好。

P.S: 除了`NSArray`，还可以桥接为`RACSequence`的类型包括：`NSDictionary`、`NSEnumerator`、`NSIndexSet`、`NSSet`、`RACSequence`、`RACTuple`（RAC定义的一种类型，和其他语言譬如Swift中的`Tuple`类似）。

除了上面提到的map、filter，RAC的Stream还定义了其他很多基本操作：

```objc
- (instancetype)flattenMap:(RACStream * (^)(id value))block;
- (instancetype)flatten;
- (instancetype)map:(id (^)(id value))block;
- (instancetype)filter:(BOOL (^)(id value))block;
- (instancetype)ignore:(id)value;
- (instancetype)skip:(NSUInteger)skipCount;
- (instancetype)take:(NSUInteger)count;
+ (instancetype)zip:(id<NSFastEnumeration>)streams;
```

如上只是其中一部分，更过基本操作详见`RACStream.h`。需要说明的是，上述的基本操作都是建立在如下API基础之上，`RACStream`并未实现这些API，得由子类（即`RACSequence`和`RACSignal`）自己实现。

```objc
+ (instancetype)empty;
+ (instancetype)return:(id)value;
- (instancetype)bind:(RACStreamBindBlock (^)(void))block;
- (instancetype)concat:(RACStream *)stream;
- (instancetype)zipWith:(RACStream *)stream;
```

P.S: `RACStream`定义的如上5个抽象方法，sunnyxx在其博客[Reactive Cocoa Tutorial [2] = 百变RACStream](http://blog.sunnyxx.com/2014/03/06/rac_2_racstream/])里已有比较好的说明，可以参考一下。

如何看待上述的基本方法呢？这些方法（无论是类方法还是实例方法）都返回`RACStream`对象，这意味着可以在它们的基础上进行**链式调用**，事实上达成了函数式编程的目的。

想想数学分支中的代数，最基础的运算无非是加减，在加减的基础上引出了乘除，然后有了各种各样更复杂的数学运算，譬如求导、微分、积分、卷积等等。对于Stream也一样，在理解这些基本操作后，我们就可以基于链式调用实现各种复杂的逻辑。

至于这些操作的具体意义及用法，本文先略过，以后再说。

## 信号

在RAC中，Signal也是一种Stream，可以被绑定和传递。把Sequence想象成Stream并不是很难，但把Signal理解成Stream还是蛮有挑战的。

如何理解Signal呢？能力有限，我也没有更好的表述。如下是一段在ReactiveCocoa的中文世界里被广泛传播的解释：
>可以把信号想象成水龙头，只不过里面不是水，而是玻璃球(data)，直径跟水管的内径一样，这样就能保证玻璃球是依次排列，不会出现并排的情况(数据都是线性处理的，不会出现并发情况)。水龙头的开关默认是关的，除非有了接收方(subscriber)，才会打开。这样只要有新的玻璃球进来，就会自动传送给接收方。可以在水龙头上加一个过滤嘴(filter)，不符合的不让通过，也可以加一个改动装置，把球改变成符合自己的需求(map)。也可以把多个水龙头合并成一个新的水龙头(combineLatest:reduce:)，这样只要其中的一个水龙头有玻璃球出来，这个新合并的水龙头就会得到这个球。

P.S: 这段摘自LimBoy的博文[ReactiveCocoa与Functional Reactive Programming](http://limboy.me/tech/2013/06/19/frp-reactivecocoa.html)。

### Sequence v.s Signal

Sequence和Signal都是Stream，但它们是不同类型的流。前者是pull-driven，后者是push-driven。所谓pull-driven，可以类比获取网页的方式，发起一个正确的HTTP请求，我们总会得到一些数据，因为数据就在服务端的数据库中躺着；而push-driven，可以类比推送（Push），数据并不是随时都有的，客户端也不知道什么时候该去获取，只能与服务端保持长连接，当服务端有新数据时，就主动推送（Push）过来。

对于初学者，理解pull-driven和push-driven这两个名词不是很容易，但理解Sequence和Signal的区别还是不难：对于Sequence而言，在它被创立之初，其中的data（玻璃球）是被确定的，可以从流中把它们一个一个查询出来；但对于Signal而言，在它（水管）被创立的时候，其中是没有data（玻璃球）的，data是之后在某个时刻（譬如notification发生时、网络请求完成时）才被放入的。

除了data驱动方式不同，Sequence和Signal所传递的data类型还不同，Sequence传递的是对象，Signal传递的是事件，无论是对象还是事件，在本文中都以data概述。

P.S: `RACSequence`和`RACSignal`可以互相转化，详见[Signal](http://rcdp.io/Signal.html)。

### Signal和Subscriber

对于Sequence而言，在其创立之初，其中的data就是确定的，经过一系列的操作（譬如map、filter）便可将同步将结果给取出来。

但对于Signal，不晓得什么时候才有data被放进去，显然不能同步等待处理结果。因而需要有一种机制来解决这个问题。

Subscription就是来解决这个问题的，Subscriber是Subscription中的核心概念，RAC定义`RACSubscriber`来描述它。关于Subscription和Subscriber，官方文档已经有非常清晰的表述，详见[这里](https://github.com/ReactiveCocoa/ReactiveCocoa/blob/v2.5/Documentation/FrameworkOverview.md#subscription)和[这里](https://github.com/ReactiveCocoa/ReactiveCocoa/blob/v2.5/Documentation/BasicOperators.md#subscription)。

P.S: 当把**信号**理解成**流**时，「signal」这个名词怎么看都觉得别扭，但当它和subscriber搭配时，却又显得那么和谐。

在描述signal和subscriber的关系时，[Limboy](http://limboy.me/tech/2013/12/27/reactivecocoa-2.html)使用插座和插头分别来类比它们，插座（signal）负责取电，插头（subscriber）负责使用电，一个插座可以插任意数量的插头。当一个插座没有插头时，什么都不会干，处于冷（cold）状态，只有插了插头才会去获取电，此时处于热（hot）状态。

上文已经提到Signal传递的data是event，它所传递的event包括3种：**值事件**、**完成事件**和**错误事件**。其中在传递值事件时，可以携带数据。

落实到代码层面，传递**值事件**、**完成事件**以及**错误事件**的本质就是向subscriber发送`sendNext:`、`sendComplete`以及`sendError:`消息，如下代码可以简单描述它们的关系：

```objc
// 1. 创建信号（冷信号）
RACSignal *signal = [RACSignal createSignal:^RACDisposable *(id<RACSubscriber> subscriber) {
    // block在什么时候被调用: 当信号被订阅时调用；
  
    // 3. 模拟data（值事件）的产生
    [subscriber sendNext:@"Hello"];
    return nil;
}];

// 2. 订阅信号（冷信号变热信号）
[signal subscribeNext:^(id x) {
    // block什么时候被调用：当subscriber接收到sendNext:消息时；
    NSLog(@"%@", x);
}];
```

上述代码中，步骤`2`在内部实现里创建一个`RACSubscriber`对象，该对象会被传入到步骤`3`所对应的block中。

### 冷信号和热信号

没有被订阅的信号被称为**冷信号**，冷信号默认情况下什么都不干，换句话说，冷信号的subscription block永远都不会被执行，譬如：

```objc
RACSignal *coldSignal = [RACSignal createSignal:^RACDisposable *(id<RACSubscriber> subscriber) {
    NSLog(@"triggered");
    [subscriber sendNext:@"hello, next"];
    [subscriber sendCompleted];
    return nil;
}];
```

这段代码创建了一个signal，但因为没有被subscribed，所以什么也不会发生。代码中使用类方法`createSignal:`创建一个`RACDynamicSignal`（`RACSignal`的子类）对象，后者有一个名为`didSubscribe`的Block属性，调用`createSignal:`传入的实参block被赋予该属性，当`RACDynamicSignal`被订阅（subscribe）时，会回调该block。

P.S: RAC中的`RACSignal`以类簇的方式实现，有点类似于Foundation中`NSString`、`NSArray`等，它定义了很多`RACSignal`子类，暂时不用理会这些子类，以后的博客中再详细介绍。

### Side Effect

如果某个`RACSignal`（以`RACDynamicSignal`为例）被多个subscriber订阅，那么它的`didSubscribe`会被多次调用吗？默认情况下是的，如下：

```objc
RACSignal *signal = [RACSignal createSignal:^RACDisposable *(id<RACSubscriber> subscriber) {
    // didSubscribe block
    NSLog(@"triggered");
    [subscriber sendNext:@"test"];
    return nil;
}];

[signal subscribeNext:^(id x) {
    NSLog(@"subscriber No.1: %@", x);
}];

[signal subscribeNext:^(id x) {
  NSLog(@"subscriber No.2: %@", x);
}];

/* prints:
triggered
subscriber No.1: test
triggered
subscriber No.2: test
*/
```

显然，`didSubscribe`被调用了两次。或许这是你想要的结果，或许不是；更多的时候这不是我们想要的结果，即所谓的**副作用**（side effect）。如果想要避免这种情况的发生，可以使用`reply`方法，它的作用是保证signal只被触发一次，然后把`sendNext:`的value给缓存起来，下一次再有新的subscriber时。直接发送缓存的value。如下：

```objc
RACSignal *signal = [RACSignal createSignal:^RACDisposable *(id<RACSubscriber> subscriber) {
    // didSubscribe block
    NSLog(@"triggered");
    [subscriber sendNext:@"test"];
    return nil;
}];

signal = [signal replay];

[signal subscribeNext:^(id x) {
    NSLog(@"subscriber No.1: %@", x);
}];

[signal subscribeNext:^(id x) {
    NSLog(@"subscriber No.2: %@", x);
}];

/* prints:
triggered
subscriber No.1: test
subscriber No.2: test
*/
```

P.S: 我对side effects的理解有问题，把它单纯想象成应该避免的负面东西，这是不对的。需要有更深刻的理解，得重写一下！还得思考「side effects里一般放什么样的代码？」。

To be continue...

## 学习资料

时至今日，ReactiveCocoa已经和AFNetWorking一样，变得非常大众了，关于它的学习资料也不像前两年那样稀缺，如下是我认为质量比较高的学习资料：

* [ReactiveCocoa v2.5官方说明文档](https://github.com/ReactiveCocoa/ReactiveCocoa/tree/v2.5/Documentation)
* LeeBoy的几篇ReactiveCocoa博文
    * [ReactiveCocoa与Functional Reactive Programming](http://limboy.me/tech/2013/06/19/frp-reactivecocoa.html)
    * [说说ReactiveCocoa 2](http://limboy.me/tech/2013/12/27/reactivecocoa-2.html)
    * [ReactiveCocoa2实战](http://limboy.me/tech/2014/06/06/deep-into-reactivecocoa2.html)
* [iOS的函数响应型编程](https://www.gitbook.com/book/kevinhm/functionalreactiveprogrammingonios/details)，非常浅显易懂。
* [ReactiveCocoa Design Patterns](http://rcdp.io/)
* 美团点评技术团队分享的几篇关于冷信号和热信号的博客
    * [细说ReactiveCocoa的冷信号与热信号（一）](http://tech.meituan.com/talk-about-reactivecocoas-cold-signal-and-hot-signal-part-1.html)
    * [细说ReactiveCocoa的冷信号与热信号（二）](http://tech.meituan.com/talk-about-reactivecocoas-cold-signal-and-hot-signal-part-2.html)
    * [细说ReactiveCocoa的冷信号与热信号（三）](http://tech.meituan.com/talk-about-reactivecocoas-cold-signal-and-hot-signal-part-3.html)
* Sunnyxx系列博客
    * [Reactive Cocoa Tutorial [0] = Overview](http://blog.sunnyxx.com/2014/03/06/rac_0_overview/)
    * [Reactive Cocoa Tutorial [1] = 神奇的Macros](http://blog.sunnyxx.com/2014/03/06/rac_1_macros/)
    * [Reactive Cocoa Tutorial [2] = 百变RACStream](http://blog.sunnyxx.com/2014/03/06/rac_2_racstream/)
    * [Reactive Cocoa Tutorial [3] = RACSignal的巧克力工厂](http://blog.sunnyxx.com/2014/03/06/rac_3_racsignal/)
    * [Reactive Cocoa Tutorial [4] = 只取所需的Filters](http://blog.sunnyxx.com/2014/04/19/rac_4_filters/)
