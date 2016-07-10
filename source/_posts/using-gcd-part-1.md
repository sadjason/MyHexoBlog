title: GCD实践之一 -- 使用GCD保护property
date: 2015-01-26 21:49:05
tags: GCD
categories: iOS

---

## 写在前面

作为一个iOS开发者，必须要熟练使用GCD，本文是站在实际应用的角度总结GCD的用法之一 -- 使用barrier保护property。

在多线程环境下，如果有多个线程要执行同一份代码，那么有时会出现问题，为了保证某些资源操作的可控性，需要一些手段来对这些「公共资源」进行保护，无论是什么语言，只要支持多线程，都会面对这个问题，即所谓的「线程同步」问题。本文围绕property讨论各种**同步工具**的保护效果，这同样可以延伸到其他需要保护的「公共资源」上。

## atomic保护property

维持property原子操作的最简单的保护措施是为其添加`atomic`修饰词，这样编译器在为其生成setter和getter时对其进行原子保护。问题来了，对于使用`atomic`修饰的property，编译器为其生成的getter和setter是什么样子的呢？在很长时间里，由于受到不靠谱网友的误导，以为是这样：

```objc
- (NSString *)name {
    {lock}        // 上锁
    _ret = _name; // get name
    {unlock}      // 释放锁
    return _ret;  // 返回
}
 
- (void)setName:(NSString *)newName {
    {lock}        // 上锁
    {set name}    // set name
    {unlock}      // 释放锁
}
```

看来认真看了Apple官方文档《Threading Programming Guide》，意识到这种说法是错误的。Apple的「原子操作」的底层实现并不是基于「锁」的，具体是什么样子呢？我也不知道，但肯定不是如上这样的（以后有时间深入了解一下，希望能够找到答案吧）。

那么修饰词`atomic`靠谱吗？它能保证相关属性getter和setter的原子性吗？能信赖它吗？

根据我的理解，不太靠谱。对于某个被`atomic`修饰的属性，当完全依赖于编译器自动合成getter和setter时，相信它们的原子性能够得到保证（不管它是如何合成实现的）；但是我们常常免不了自己动手写setter或者getter，此时会将系统默认合成的setter或getter给覆盖掉，我们自己的代码能够保证原子性吗？如果我们只是自己实现setter和getter中的其中一个，另外一个依赖于编译器合成，那么自定义的这个该如何实现呢？

总总问题外加总总不确定，导致了在iOS开发中几乎不使用`atomic`修饰符（至少没在著名第三方库中看到使用它它修饰属性的案例）。

>P.S: 这种说法纯属一家之言，以后补充更靠谱的佐证吧！

## NSLock保护property

上文的代码块恰好是互斥锁（NSLock）或者递归锁（NSRecursiveLock）保护property的基本套路，为了方便说明，再次罗列如下：

```objc
- (NSString *)name {
    {lock}        // 上锁
    _ret = _name; // get name
    {unlock}      // 释放锁
    return _ret;  // 返回
}
 
- (void)setName:(NSString *)newName {
    {lock}        // 上锁
    {set name}    // set name
    {unlock}      // 释放锁
}
```

根据我的理解，在一般情况下，NSLock是能够保证property原子性的。但根据《Effective Objective-C 2.0》的描述：一旦遇到死锁，就会非常麻烦。

什么情况下会出现死锁呢？我觉得至少有这么一种情况：保护name属性的锁在其他地方也被使用了，即当前线程正在持有该锁，此时正在访问别处的某个公共资源，保护该资源的锁正被另外一个线程持有，而那个线程正在获取当前线程持有的这把锁...

当然，对于属性而言，这种情况过于极端，但如果NSLock保护的不是属性而是别的更复杂的公共资源，那么这种极端情况就不是那么极端了；再者，如果保护name属性的这把锁也被用来保护其他的资源，那么问题就变得更复杂了。

总之，根据我的理解，如果确实需要使用「锁」保护property，要做到：

* 尽量使用NSRecursiveLock，避免多次持有该锁造成死锁；
* 每个属性有一个单独的「锁」为之服务，不可与别的资源共用，否则问题会变得更复杂；

>P.S: 《Effective Objective-C 2.0》中关于NSLock保护property的这部分内容讲得非常不到位，以上纯属个人理解！

## @synchronized块保护属性

所谓「@synchronized块」在中文世界里常被称为「同步块」，根据《Threading Programming Guide》的描述，**同步块**是使用**同步锁**的简写形式，本质仍然是使用「同步锁」保护「公共资源」。同步块保护属性的一般形式是：

```objc
- (void)setSomeString:(NSString *)someString {
    @synchronized(self) {
        _someString = someString;
    }
}
 
- (NSString *)someString {
    @synchronized(self) {
        return _someString;
    }
}
```

这种做法有什么问题呢？相对于NSLock，这种处理问题更大！主要问题是：`@synchronized(self)`效率极低。

根据《Effective Objective-C 2.0》的说法。因为`@synchronized(){}`的本质是根据给定的对象，自动创建一个锁，并等待块中的代码执行完毕。执行到这段代码结尾处，锁就被释放了。通常传给`@synchronized`块的对象是self。这意味着同步锁将self整个对象都保护起来了，如果滥用，其他属性也都用`@synchronized(self){}`这种方式确保原子性，这样在`self`对象上频繁加锁，那么程序可能要等待另一段与此无关的代码执行完毕，才能继续执行当前代码，这显然会降低代码效率。

## GCD串行队列保护property

将对property的读写方法都安排在同一个队列中，即可保证数据同步，如下：

```objc
@property (nonatomic, strong) NSString *name;
@property (nonatomic, strong) dispatch_queue_t serialQueue;
    
@synthesize name = _name;
    
// create a serial dispatch queue
_serialQueue = dispatch_queue_create("com.zhangbuhuai.test", nil);
    
// getter
- (NSString *)name {
    __block NSString *localName;
    dispatch_sync(_serialQueue, ^{
        localName = _name;
    });
    return localName;
}
    
// setter
- (void)setName:(NSString *)name {
    dispatch_sync(_serialQueue, ^{
        _name = name;
    });
}
```

此模式的思路是：把setter和getter都安排在序列化的队列里执行，这样的话，所有针对属性的访问就都同步了。为了使代码块能够设置局部变量，getter中用到了__block语句，若是抛开这一点，这种写法比之前的那些更为整洁。全部加锁任务都在GCD中处理，而GCD是在相当深的底层来实现的，于是能够做许多优化。因此，开发者无需担心那些事，只要专心把访问方法写好就行了。

然而，**还可以进一步优化，毕竟setter方法不一定非得是同步的。设置实例变量所用的block，并不需要向setter返回什么值**。

也就是说，setter代码可以改成下面这样：

```objc
// setter
- (void)setName:(NSString *)name {
    dispatch_async(_serialQueue, ^{
        _name = name;
    });
}
```

这次只是把同步派发改成了异步派发，从调用者的角度来看，这个小改动可以提升设置方法的执行速度（毕竟直接返回而不用等待block执行完成），而读取操作与写入操作依然会按顺序执行。但是这么改有一个坏处：如果测试一下程序的性能，那么可能发现这种写法比原来慢，因为执行异步派发时，需要拷贝block。若拷贝block所用的时间明显超过执行块所用时间，则这种做法将比原来更慢。
P.S：所以，setter的block设置为asynchronous或者synchronous全看setter的block的复杂度。

## GCD并行队列和barrier保护property

其实在更多的时候，调用getter可以并发执行，而getter和setter之前不能并发执行。利用这个特点，还能写一些更快一些的代码。此时正可以体现出GCD写法的好处。用同步块或锁对象，是无法轻易实现出如下这种方案的，这次不用serial dispatch queue，而改用并发队列：

```objc
@property (nonatomic, strong) NSString *name;
@property (nonatomic, strong) dispatch_queue_t concurrentQueue;
    
@synthesize name = _name;
    
// create a concurrent dispatch queue
_concurrentQueue = dispatch_queue_create("com.zhangbuhuai.test", 0);
    
// getter
- (NSString *)name {
    __block NSString *localName;
    dispatch_sync(_concurrentQueue, ^{
        localName = _name;
    });
    return localName;
}
    
// setter
- (void)setName:(NSString *)name {
    dispatch_async(_concurrentQueue, ^{
        _name = name;
    });
}
```

然而，如上这样的代码，还**无法正确实现同步**。所有读取操作与写入操作都会在同一个队列上执行，不过由于是并发队列，所以读取与写入操作可能随时执行。而我们恰恰不想让这些操作随意执行。此问题用一个简单的GCD功能即可解决，它就是栅栏（barrier）。下列函数可以向队列中派发块，将其作为栅栏使用：

```objc
void dispatch_barrier_sync(dispatch_queue_t queue, dispatch_block_t block);
void dispatch_barrier_async(dispatch_queue_t queue, dispatch_block_t block);
```

在队列中，栅栏块必须单独执行，不能与其他块并行。这只对并发队列有意义，因为串行队列中的块总是按顺序逐个来执行的。并发队列如果发现接下来的要处理的block是barrier block，那么就一直要等当前所有并发块都执行完毕，才会单独执行这个栅栏块。待栅栏块执行完成后，再按正常方式继续向下执行。

在本例中，可以用栅栏块来实现属性的setter方法。在设置方法中使用了栅栏块之后，对属性的读取操作依然可以并发执行，但是写入操作却必须单独执行了，如下图所示：

<div class="imagediv" style="width: 301px; height: 298px">{% asset_img barrier_block.png %}</div>

代码实现很简单：

```objc
@property (nonatomic, strong) NSString *name;
@property (nonatomic, strong) dispatch_queue_t concurrentQueue;
    
@synthesize name = _name;
    
// create a concurrent dispatch queue
_concurrentQueue = dispatch_queue_create("com.zhangbuhuai.test", 0);
    
// getter
- (NSString *)name {
    __block NSString *localName;
    dispatch_sync(_concurrentQueue, ^{
        localName = _name;
    });
    return localName;
}
    
// setter
- (void)setName:(NSString *)name {
    dispatch_barrier_async(_concurrentQueue, ^{
        _name = name;
    });
}
```

测试一下性能，就会发现，这种做法肯定比使用串行队列要快。当然，将上述代码中的`dispatch_barrier_async`改为`dispatch_barrier_sync`也是没问题的，也**可能**会更高效，至于原因上文已经讲到了。在实际使用时，最好还是测一测每种做法的性能，然后从中选出最适合当前场景的方案。

## 本文参考

* 《Effective Objective-C 2.0》