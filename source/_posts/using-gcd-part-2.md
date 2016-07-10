title: GCD实践之二 -- 多用GCD，少用performSelector系列方法
date: 2015-01-28 13:53:22
tags: GCD
categories: iOS

---

## 写在前面

本文几乎全部参考《Effective Objective-C 2.0》 Item 42。

## performSelector介绍

Objective-C本质上是一门非常动态的语言，NSObject定义了几个方法，令开发者可以随意调用任何方法。这几个方法可以推迟执行方法调用，也可以指定运行方法所用的线程。这些功能在出现GCD之前非常有用。

这其中最简单的是`performSelector:(SEL)selector`。
该方法与直接调用选择子等效。所以下面两行代码的执行效果相同：

```objc
[object performSelector:@selector(selectorName)];
[object selectorName];
```

这种方法看上去似乎有些多余。但是如果某个方法只是这么来调用的话，那么此方式确实多余，然而，如果selector是在running time才决定的，那么就能体现出此方式的强大之处了。这就等于在动态绑定之上再次使用动态绑定，因而可以实现出下面这种功能：

```objc
SEL selector;
if ( /* some condition */ ) {
    selector = @selector(foo);
} else if ( /* some other condition */ ) {
    selector = @selector(bar);
} else {
    selector = @selector(baz);
}
    
[object performSelector:selector];
```

这种编程方式极为灵活，经常可用来简化复杂的代码。还有一种用法，即时先把选择子保存起来，等某个事件发生之后再调用。不管哪种用法，编译器都不知道要执行的选择子是什么，者必须到了运行期才能确定。然而，使用此特性的代价是，如果在ARC下编译代码，那么编译器会发出如下警示信息：

```
warningL performSelector may casue a leak because its selector
is unknown [-Warc-performSelector-leaks]
```

你可能没料到会出现这种警告。要是早就料到了，那么也许应该已经知道使用这些方法为何要小心了。这条消息看上去可能比较奇怪，而且令人纳闷：为什么其中会提到内存泄漏问题呢？只不过是用`performSelector:`调用了一个方法。原因在于，编译器并不知道将要调用的selector是什么，因此，也就不了解其方法签名及返回值，甚至连是否有返回值都不清楚。而且，由于编译器不知道方法名，所以就没办法用ARC的内存管理规则来判定返回值是不是该释放。鉴于此，ARC采用了比较谨慎的做法，就是不添加释放操作。然而，这么做可能导致内存泄漏，因为方法在返回对象时已经将其保留了。

这段话不是很容易懂，下面这段代码应该有助于理解：

```objc
SEL selector;
if ( /* some condition */ ) {
    selector = @selector(newObject);
    // newObject返回一个new object
} else if ( /* some other condition */ ) {
    selector = @selector(copy);
    // copy根据当前object copy出一个新的object
} else {
    selector = @selector(someProperty));
    // someProperty可以认为是对象的某个property
}
    
id ret = [object performSelector:selector];
```

此代码与刚才那个例子有所不同，以便展示问题所在，如果调用的是前两个选择子之一，那么ret对象应由这段代码来释放，而如果是第三个选择子，则无需释放。如果不使用ARC（此时编译器也不发出警告信息了），那么前两种情况下需要手动释放ret对象，而后一种不需要释放。如果使用ARC，则ARC应该帮忙处理这些事情，但是目前来说ARC是很难解决这个问题的，正如上文所述，其采取的是谨慎的做法：不添加释放操作，这就给程序带来了内存泄漏的可能。
显然，这已然是performSelector的一大缺点（或说这是performSelector系列函数的一个坑吧）了。这个问题很容易被忽视，而且就算用静态分析器，也很难侦测到随后的内存泄漏。
performSelector系列的方法之所以要谨慎使用，这就是其中一个原因。

performSelector的另一个局限在于：返回值只能是void或对象类型（id类型）。如果想返回整数或浮点数等scalar类型值，那么就需要执行一些复杂的转换操作，而这种转换操作很容易出错。由于id类型表示指向任意Objective—C对象的指针，所以从技术上来讲，只要返回的大小和指针所占大小相同就行，也就是说，在32位架构的计算机上，可以返回任意32位大小的类型；而在64位架构的计算机上，则可以返回任意64位大小的类型。除此之外，还可以返回NSNumber进行转换...若返回的类型为C语言结构体，则不可使用performSelector方法。

## 多用GCD，少用performSelector系列方法

performSelector系列方法中有某些方法可以被GCD代替。

performSelector还有如下几个版本，可以在发消息时顺便传递参数：

```objc
- (id)performSelector:(SEL)aSelector withObject:(id)object;
- (id)performSelector:(SEL)aSelector withObject:(id)object1 withObject:(id)object2;
```

比方说，可以用下面这两个版本来设置对象中名为value的属性值：

```objc
id object = /* an object with a property called value */
id newValue = /* new value for the property */
[object performSelector:@selector(setValue:) withObject:newValue];
```

这些方法貌似有用，但是局限颇多！由于参数类型是id，所以传入的参数必须是对象才行。如果选择子所接受的参数是整数或浮点数，那就不能采用这些方法了。此外，选择子最多只能接受两个参数，也就是调用`performSelector:withObject:withObject:`这个版本。在参数不止两个的情况下，则没有对应的performSelector方法能够执行这种选择子。

performSelector系列方法还有两个功能，就是可以延后执行选择子，或将选择子放在另一个线程上执行。下面列出此方法中一些更为常用的版本：

```objc
- (void)performSelector:(SEL)aSelector withObject:(id)anArgument afterDelay:(NSTimeInterval)delay;
- (void)performSelector:(SEL)aSelector onThread:(NSThread *)thr withObject:(id)arg waitUntilDone:(BOOL)wait;
- (void)performSelectorOnMainThread:(SEL)aSelector withObject:(id)arg waitUntilDone:(BOOL)wait;
```

当然，这几个方法还有一两个别的变种，这里就略过了。然而，很快就会发现，这些方法太过局限了。例如，具备延后功能的那些方法无法处理带有两个参数的选择子。而能够指定执行线程的哪些方法，则与之类似，所以也不是特别通用。如果要用这些方法，就得把很多参数打包到字典中，然后在被调用的方法中将这些参数提取出来，这样会增加开销，同时也提高了产生bug的可能性。

如果改用替代方案GCD，那么就不受这些限制了。
performSelector系列方法所提供的线程功能，可以通过GCD机制中的块来实现；performSelector系列方法所提供的延后执行功能，也可以用dispatch_after来实现，在另一个线程上执行任务则可通过dispatch_async和dispatch_sync来实现。

例如，要延后执行某项任务，可以有下面两种方式实现，而我们应该优先考虑第二种：

```objc
// using performSelector:withObject:afterDelay:
[self performSelector:@selector(doSomething:) withObject:nil afterDelay:5.0]

// using dispatch_after
dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5.0 * NSEC_PER_SEC));
dispatch_after(time, dispatch_get_main_queue(), ^{
    [self doSomething];
});
```

想把任务放在主线程上执行，也可以有下面两种方式，而我们还应该优先选择后者：

```objc
// using performSelectorOnMainThread:withObject:waitUntilDone:
[self performSelectorOnMainThread:@selector(doSomething) withObject:nil waitUntilDone:NO];
    
// using dispatch_async
// (or if waitUntilDone is YES, then dispatch_sync)
dispatch_async(dispatch_get_main_queue(), ^{
    [self doSomething];
});
```

总结：

1. performSelector系列方法在内存管理上容易有缺失，它无法确定将要执行的选择子是什么，因而ARC编译器也无法插入适当的内存管理方法，这是一个大坑，使用GCD则不存在这个问题。
2. performSelector系列方法能处理的选择子太过局限了，选择子的返回值类型及发送给方法的参数个数都受到限制，不过GCD似乎也没有比较好的解决方法（或许只是笔者不知道）；
3. 如果想把任务放在另外一个线程上执行，或者想延时执行某个任务，最好应该把任务封装到block中，然后调用GCD的相关方法来实现。

## 本文参考

* 《Effective Objective-C 2.0》