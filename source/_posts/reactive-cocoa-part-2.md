title: RACSignal实践
date: 2016-07-28 22:39:41
tags: ReactiveCocoa
categories: iOS

---

接着[初识ReactiveCocoa](/reactive-cocoa-part-1/)继续写，本文总结`RACSignal`的简单使用以及基本操作。

Signal传递的data是event，它所传递的event包括3种：**值事件**、**完成事件**和**错误事件**。其中在传递值事件时，可以携带数据。落实到代码层面，传递**值事件**、**完成事件**以及**错误事件**的本质就是向subscriber发送`sendNext:`、`sendComplete`以及`sendError:`消息。

为了更形象对各种操作进行表述，下文会大量使用图例，绿色圆圈代表值事件，红色叉代表错误事件，红色杠代表完成事件，灰色带箭头直线代表时间线，如下：

<div class="imagediv" style="width:580px; heiht:69px">{% asset_img signal-symbols@2x.png %}</div>

Signal在其生命周期内，可以传递任意多个值事件，但最多只能传递一个完成事件或错误事件；换句话说，一旦Signal的事件流中出现了错误事件或者完成事件，之后产生的任何事件都是无效的。

## 信号的简单使用

这一部分将从3个方面介绍信号的简单实用，包括：创建信号、订阅信号、订阅过程。

### 获取信号

获取信号的方式有很多种：

* 创建单元信号
* 创建动态信号
* 通过Cocoa桥接
* 从别的信号变换而来
* 由序列变换而来

**单元信号**

最简单的信号是单元信号，有4种：

```objc
// return信号：被订阅后，立马产生一个值事件，然后产生一个完成事件
RACSignal *signal1 = [RACSignal return:someObject];
// error信号：被订阅后，立马产生一个错误事件
RACSignal *signal2 = [RACSignal error:someError];
// empty信号：被订阅后，立马产生一个完成事件
RACSignal *signal3 = [RACSignal empty];
// never信号：永远不产生事件
RACSignal *signal4 = [RACSignal never];
```

可以用图例描述这4个信号：

<div class="imagediv" style="width: 472px; height: 76px;">{% asset_img return-error-empty-never@2x.png %}</div>

**动态信号**

```objc
RACSignal *signal5 = [RACSignal createSignal:^RACDisposable *(id<RACSubscriber> subscriber) {
    [subscriber sendNext:@"1"];
    [subscriber sendNext:@"2"];
    [subscriber sendCompleted];
    return [RACDisposable disposableWithBlock:^{
        
    }];
}];
```

**Cocoa桥接**

RAC为大量的Cocoa类型提供便捷的信号桥接工具，如下是一些常见的桥接方式：

```objc
RACSignal *signal6 = [object rac_signalForSelector:@selector(setFrame:)];
RACSignal *signal7 = [control rac_signalForControlEvents:UIControlEventTouchUpInside];
RACSignal *signal8 = [object rac_willDeallocSignal];
RACSignal *signal9 = RACObserve(object, keyPath);
// 还有更多
```

**信号变换**

```objc
RACSignal *signal10 = [signal1 map:^id(id value) {
    return someObject;
}];
```

**序列变换**

```objc
RACSignal *signal11 = sequence.signal;
```

### 订阅信号

订阅信号的方式有3种：

* 通过`subscribeNext:error:completed:`方法订阅
* RAC宏绑定
* Cocoa桥接

**通过subscribeNext:error:completed:方法订阅**

`subscribeNext:error:completed:`是最基础的信号订阅方法，相关的方法原型如下：

```objc
- (RACDisposable *)subscribeNext:(void (^)(id x))nextBlock
                           error:(void (^)(NSError *error))errorBlock
                       completed:(void (^)(void));
```

**RAC宏绑定**

可以使用`RAC()`宏（和上述的`RACObserve()`宏不一样）绑定：

```objc
RAC(view, backgroundColor) = signal10;
// 每当signal10产生一个值事件，就将view.backgroundColor设为相应的值
```

**Cocoa桥接**

```objc
[object rac_liftSelector:@selector(someSelector:) withSignals:signal1, signal2, nil];
[object rac_liftSelector:@selector(someSelector:) withSignalsFromArray:@[signal1, signal2]];
[object rac_liftSelector:@selector(someSelector:) withSignalOfArguments:signal1];
```

### 订阅过程

所谓订阅过程指的是信号被订阅的处理逻辑，如下是简单的例子：

```objc
RACSignal *signal = [RACSignal createSignal:^RACDisposable *(id<RACSubscriber> subscriber) {
    [subscriber sendNext:@"1"];
    [subscriber sendNext:@"2"];
    [subscriber sendCompleted];     
    [subscriber sendNext:@"3"];  // 无效
    return [RACDisposable disposableWithBlock:^{
        NSLog(@"dispose");       // 当错误事件或者完成事件产生时，该block被调用
    }];
}];

[signal subscribeNext:^(id x) {
    NSLog(@"next value is :  %@", x);
} error:^(NSError *error) {
    NSLog(@"error : %@", error);
} completed:^{
    NSLog(@"completed");
}];

/* prints:
next value is :  1
next value is :  2
completed
dispose
*/
```

## 信号的各类操作

这一部分将介绍信号的各类操作，内容比较多。参考大神臧成威的说法，将信号的操作分为两类：单个信号的变换、多个信号的组合。

### 单个信号的变换

单个信号的变换也可以分为几类：

* 值操作
* 数量操作
* 时间操作

下文结合图例对各种操作进行说明。

**值操作 -- Map**

<div class="imagediv" style="width:350px; heiht:130px">{% asset_img map@2x.png %}</div>

* 当`signalA`事件流出现完成事件时，`signalB`的事件流也会出现完成事件
* 当`signalA`事件流出现错误事件时，`signalB`也会将该错误原封不动地释放出来

`map:`操作还有一个简化版：`mapReplace:`，`[signalA mapReplace:@886];`等价于`[signalA map:^id(id value) { return @886; }];`。

**值操作 -- ReduceEach**

<div class="imagediv" style="width:400px; heiht:130px">{% asset_img reduce-each@2x.png %}</div>

`reduceEach:`这个操作的名字不太容易理解，但是操作本身还是非常简单，是`map:`的变体。当`signalA`的值事件包裹的数据是`RACTuple`类型时，才可以使用该操作；稍微读一下该操作的实现源码即可明白。

此外，`reduceEach:`的block中，可以传入的参数是任意数量。

P.S: 如何理解「reduce」？

**值操作 -- 其他的Map变体操作**

除了`reduceEach:`，`map:`还有其他的一些变体操作：

```objc
- (RACSignal *)not;
- (RACSignal *)and;
- (RACSignal *)or;
- (RACSignal *)reduceApply;
```

前面3个都比较容易理解，`reduceApply`也要求值事件包裹的数据类型是`RACTuple`，并且该`RACTuple`的第一个元素是一个block，后面的元素作为该block的参数传入，返回该block的执行结果。

**值操作 -- Materialize和Dematerialize**

对于`signalB = [signalA materialize];`，`signalA`产生的值事件包裹的数据都被转化为`RACEvent`对象，错误事件和完成事件亦然：

```
signalA -- sendNext:x
=>
signalB -- sendNext:[RACEvent eventWithValue:x]

signalA -- sendError:error
=>
signalB --  sendNext:[RACEvent eventWithError:error]
signalB --  sendCompleted

signalA -- sendCompleted
=>
signalB -- sendNext:RACEvent.completedEvent
signalB -- sendCompleted
```

`dematerialize`操作与`materialize`相反，写代码体会一下就明白了。这两个操作似乎很少被用到。

**数量操作 -- Filter**

<div class="imagediv" style="width:350px; heiht:130px">{% asset_img filter@2x.png %}</div>

和`map:`一样：

* 当`signalA`产生完成事件时，`signalB`也会产生完成事件
* 当`signalA`产生错误事件时，`signalB`也会将该错误原封不动地释放出来

**数量操作 -- Ignore**

`ignore:`是`filter:`的变体操作：

<div class="imagediv" style="width:349px; heiht:130px">{% asset_img ignore@2x.png %}</div>

还有其他的变体版本：

```objc
- (RACSignal *)ignoreValues;
```

**数量操作 -- Distinct**

`distinctUntilChanged`去掉连续相同的值事件。

<div class="imagediv" style="width:345px; heiht:127px">{% asset_img distinct-until-changed@2x.png %}</div>

**数量操作 -- Take和Skip**

`take:`操作只取前`n`（传入的参数）个事件。

<div class="imagediv" style="width:352px; heiht:129px">{% asset_img take@2x.png %}</div>

假设`signalA`第一次出现错误事件/完成事件的index（从1开始）为`k`：

* 当`k` > `n`时，`signalB`中就不会出现错误事件/完成事件
* 当`k` <= `n`时，错误事件/完成事件也会出现`signalB`中

`skip:`与`take:`相反，它会将前`n`（传入的参数）个事件给过过滤掉。

<div class="imagediv" style="width:340px; heiht:129px">{% asset_img skip@2x.png %}</div>

`signalA`的前`n`个事件中可能会出现错误事件/完成事件，这种情况下，`signalB`的第一个信号就是错误事件/完成事件，且不会有任何的值事件。

`take:`和`skip:`也有几个变体：

```objc
- (RACSignal *)takeLast:(NSUInteger)count;
- (RACSignal *)takeUntilBlock:(BOOL (^)(id x))predicate;
- (RACSignal *)takeWhileBlock:(BOOL (^)(id x))predicate;
- (RACSignal *)skipUntilBlock:(BOOL (^)(id x))predicate;
- (RACSignal *)skipWhileBlock:(BOOL (^)(id x))predicate;
```

P.S: 还有两个操作也以「take」作为前缀，即`takeUntil:`和`takeUntilReplacement:`，但属于组合操作，详见下文。

**数量操作 -- Start With**

`startWith:`操作的作用是在事件流的开始新增一个值事件。

<div class="imagediv" style="width:370px; heiht:129px">{% asset_img start-with@2x.png %}</div>

**数量操作 -- Repeat**

<div class="imagediv" style="width:430px; heiht:129px">{% asset_img repeat@2x.png %}</div>

从图中可以看到，`repeat`操作会忽略`signalA`的完成事件，但它不会忽略错误事件，换句话说，如果`signalA`的事件流中含有错误事件，那么`signalB`的事件流会和`signalA`完全一致。

**数量操作 -- Retry**

<div class="imagediv" style="width:439px; heiht:130px">{% asset_img retry@2x.png %}</div>

然而，当`signalA`事件流中含有完成事件时，那么`signalA`的事件流会和`signalA`完全一致。

`retry`操作还有带参数版本`retry:`，该版本可以指定次数。这种操作在处理网络任务时非常有用。

**数量操作 -- Collect**

<div class="imagediv" style="width:360px; heiht:130px">{% asset_img collect@2x.png %}</div>

**Aggregate和Scan**

Aggregate（译作「合计」）和Scan这两个操作有些类似，但明显有区别，前者改变了事件流的事件数量，后者没有，看如下图例就明白。

<div class="imagediv" style="width:380px; heiht:260px">{% asset_img aggregate@2x.png %}</div>

<div class="imagediv" style="width:450px; heiht:264px">{% asset_img scan@2x.png %}</div>


Aggregate和Scan还有一些其他变种方法：

```objc
- (RACSignal *)aggregateWithStartFactory:(id (^)(void))startFactory reduce:(id (^)(id running, id next))reduceBlock;
- (RACSignal *)aggregateWithStart:(id)start reduceWithIndex:(id (^)(id, id, NSUInteger))reduceBlock;
- (RACSignal *)scanWithStart:(id)startingValue reduceWithIndex:(id (^)(id, id, NSUInteger))reduceBlock;
```

**时间操作 -- 常会用到的时间信号**

经常会有这样的需求：定时产生一个事件。

```objc
+ (RACSignal *)interval:(NSTimeInterval)interval onScheduler:(RACScheduler *)scheduler;
+ (RACSignal *)interval:(NSTimeInterval)interval onScheduler:(RACScheduler *)scheduler withLeeway:(NSTimeInterval)leeway;
```

这两个类方法都会产生一个时间信号，时间信号会以一定频率产生一个值事件，值事件包裹的是`NSDate`对象。第二个方法中的参数`leeway`是「缓冲」、「余地」的意思，还不太明白其作用。

**时间操作 -- Delay**

<div class="imagediv" style="width:380px; heiht:130px">{% asset_img delay@2x.png %}</div>

**时间操作 -- Throttle**

Throttle信号理解起来相对比较繁琐些。在下图中，`signalA`事件流中一共有5个值事件和1个完成事件，其中1号和2号事件间隔2s，2、3、4号事件间隔1s，5号值事件比4号值事件晚3s，完成事件比5号事件晚1s。

<div class="imagediv" style="width:510px; heiht:220px">{% asset_img throttle@2x.png %}</div>

`signalB`由`[signalA throttle:1.5]`得到，throttle表示门限，其作用效果是：

* 当`signalA`事件流产生一个值事件时，若1.5s内没有其他的值事件产生，则`signalB`事件流中也会产生该值事件；比如上图中的1号值事件，下一个值事件（2号）在其2s后产生，满足要求，故而在1.5s后，`signalB`的事件流也产生该信号；
* 当`signalA`事件流产生一个值事件时，若1.5s内有其他的值事件产生，则`signalB`会过滤掉该值事件；比如上图中的2号和3号值事件，在1s后分别有3号和4号信号产生，不满足要求，故而都不会出现在`signalB`的事件流中；
* 对于`signalA`中的最后一个值事件，`signalB`事件流中总会也包含它。

这种信号有什么用呢？有一个常用的应用场景：在App内经常需要搜索，为了确保实时性，简单的做法是每输入一个字符就检索一下，但是若用户输入字符比较快，这种检索策略会比较浪费流量，因此比较好的做法是，用户输入某个字符后，1s（参考值）内没有再输入别的字符，就检索一次搜索结果。此时throttle就有了用武之地。

`throttle:`还有一个变体`throttle:valuesPassingTest:`；及类似操作：`bufferWithTime:onScheduler:`。

**副作用操作**

再补充一些RAC提供的副作用操作：

```objc
- (RACSignal *)doNext:(void (^)(id x))block;
- (RACSignal *)doError:(void (^)(NSError *error))block;
- (RACSignal *)doCompleted:(void (^)(void))block;
- (RACSignal *)initially:(void (^)(void))block;
- (RACSignal *)finally:(void (^)(void))block;
```

这5个副作用操作，稍微查看一下源码立马能知道它们的作用；其中`initially:`的作用是让信号在第一次被订阅是调用其block；`finally:`的作用是在信号的事件流结束（出现完成事件或错误事件）时调用传入的block。

### 多个信号的组合

上面介绍的都是单信号操作，这部分介绍多信号组合操作，在熟悉这些组合操作时，以`signalA + signalB => signalC`为例，需要留意几个问题：

* 组合得到`signalC`的信号受哪个信号终止而终止，`signalA` or `signalB`？
* 当`signalA`或`signalB`事件流中出现错误信号，会如何？
* 各个信号何时开始被订阅？

**组合操作 -- Concat**

<div class="imagediv" style="width:460px; heiht:349px">{% asset_img concat@2x.png %}</div>

在上图中，当`signalA`的事件流中出现完成事件时，立马订阅`signalB`，`signalC`事件流的完成事件与`signalB`的完成事件对应。同时，注意到当`signalA`中出现错误事件时，`signalC`中的事件流与`signalA`的事件流完全保持一致，就没`signalB`什么事儿了。

**组合操作 -- Merge**

Merge操作比较简单，看图就明白。

<div class="imagediv" style="width:460px; heiht:349px">{% asset_img merge@2x.png %}</div>

除了`signalC = [signalA merge:signalB]`这种用法外，还可以这样：

```objc
signalC = [RACSignal merge:@[signalA, signalB]];
signalC = [RACSignal merge:RACTuplePack(signalA, signalB)];
```

**组合操作 -- Zip**

<div class="imagediv" style="width:344px; heiht:344px">{% asset_img zip@2x.png %}</div>

除了`signalC = [signalA zip:signalB]`这种用法外，还可以这样：

```objc
signalC = [RACSignal zip:@[signalA, signalB]];
signalC = [RACSignal zip:RACTuplePack(signalA, signalB)];
```

**组合操作 -- CombineLatest**

<div class="imagediv" style="width:370px; heiht:339px">{% asset_img combine-latest@2x.png %}</div>

除了`signalC = [signalA combineLatest:signalB]`这种用法外，还可以这样：

```objc
signalC = [RACSignal combineLatest:@[signalA, signalB]];
signalC = [RACSignal combineLatest:RACTuplePack(signalA, signalB)];
```

**组合操作 -- Sample**

<div class="imagediv" style="width:370px; heiht:170px">{% asset_img sample@2x.png %}</div>

在`signalC = [signalA sample:signalB]`中，`signalB`充当`signalA`的采样信号，一旦`signalA`或`signalB`先产生完成事件或者错误事件，`signalC`的事件流就被终止。

**组合操作 -- Take Until**

<div class="imagediv" style="width:370px; heiht:170px">{% asset_img take-until@2x.png %}</div>

`signalC`事件流中的事件和`signalA`一一对应，直到`signalB`事件流中出现了信号，事件流就终结。

**组合操作 -- Take Until Replacement**

<div class="imagediv" style="width:370px; heiht:170px">{% asset_img take-until-replacement@2x.png %}</div>

`signalC`事件流中的事件和`signalA`一一对应，一旦`signalB`事件流中出现了信号，`signalC`的事件就和`signalB`形成呼应，当然前提是`signalC`的事件流还没有终结。