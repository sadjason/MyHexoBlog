title: NSHashtable和NSMaptable
date: 2015-04-26 08:53:43
categories: iOS
---

**说明**

本文转自《[NSHashtable and NSMaptable](http://www.cocoachina.com/industry/20140605/8683.html)》，其原作者是[Mattt Thompson](http://nshipster.com/nshashtable-and-nsmaptable/)，该大神是AFNetworking的作者。作者简单介绍了一下iOS开发中相对于NSDictionary和NSSet来说，不常被人使用NSHashTable和NSMapTable的相关知识。

>P.S: 笔者关注《[NSHashtable and NSMaptable](http://www.cocoachina.com/industry/20140605/8683.html)》这篇博客的原因是因为以前遇到「如何实现NSNotificationCenter」这样的问题，我意识到实现NSNotificationCenter的关键在于找到一个能够支持弱指针的容器，而NSHashTable和NSMaptable恰好是这样的容器。

NSSet，NSDictionary，NSArray是Foundation框架关于集合操作的常用类，和[其他标准的集合操作库](http://en.wikipedia.org/wiki/Java_collections_framework)不同，他们的实现方法对开发者进行[隐藏](http://ridiculousfish.com/blog/posts/array.html)，只允许开发者写一些简单的代码，让他们相信这些代码有理由正常的工作。

然而这样的话最好的代码抽象风格就会被打破，苹果的本意也被曲解了。在这种情况下，开发者寻求更好的抽象方式来使用集合，或者说寻找一种更通用的方式。

对于NSSet和NSDictionary，打破代码抽象风格的是他们在内存中存取object的方式。在NSSet中，objects是被强引用的（strongly referenced），同样NSDictionary中的keys和values也会被NSDictionary复制。如果一个开发者想要存储一个weak类型的值或者使用一个没有实现NSCopying协议的object作为NSDictionary的key，他可能会很聪明的想到[NSValue+valueWithNonretainedObject](http://nshipster.com/nsvalue/)。 iOS6和OSX 10.5以后，可以分别使用和NSSet，NSDictionary地位相同的NSHashTable，NSMapTable。

这两个类在Foundation的collection中不常用到，为了避免你慌乱无措，下面将介绍这两个类的用法。

## NSHashTable

NSHashTable是更广泛意义的NSSet，区别于NSSet/NSMutableSet，NSHashTable有如下特性：

* NSHashTable是可变的；
* NSHashTable可以持有weak类型的成员变量；
* NSHashTable可以在添加成员变量的时候复制成员；
* NSHashTable可以随意的存储指针并且利用指针的唯一性来进行hash同一性检查（检查成员变量是否有重复）和对比操作（equal）；

用法如下：

```objc
NSHashTable *hashTable = [NSHashTable hashTableWithOptions:NSPointerFunctionsCopyIn];
[hashTable addObject:@"foo"];
[hashTable addObject:@"bar"]; 
[hashTable addObject:@42];
[hashTable removeObject:@"bar"];
NSLog(@"Members: %@", [hashTable allObjects]);
```

NSHashTable是根据一个option参数来进行初始化的，因为从OSX平台上移植到iOS平台上，原来OSX平台上使用的枚举类型被放弃了，从而用option来代替，命名也发生了一些变化：

* NSHashTableStrongMemory

等同于NSPointerFunctionsStrongMemory。对成员变量进行强引用，这是一个默认值，如果采用这个默认值，NSHashTable和NSSet就没什么区别了。

* NSHashTableWeakMemory

等同于NSPointerFunctionsWeakMemory。对成员变量进行弱引用，使用NSPointerFunctionsWeakMemory，object引用在最后释放的时候会被指向NULL。

* NSHashTableZeroingWeakMemory

已被抛弃，使用NSHashTableWeakMemory代替。

* NSHashTableCopyIn

在对象被加入集合之前进行复制（[NSPointerFunction-acquireFunction](https://developer.apple.com/library/ios/documentation/Cocoa/Reference/Foundation/Classes/NSPointerFunctions_Class/index.html#//apple_ref/occ/instp/NSPointerFunctions/acquireFunction)），等同于NSPointerFunctionsCopyIn。

* NSHashTableObjectPointerPersonality

用指针来等同代替实际的值，当打印这个指针的时候相当于调用description方法。和NSPointerFunctionsObjectPointerPersonality等同。

## NSMapTable

NSMapTable是对更广泛意义的NSDictionary。和NSDictionary/NSMutableDictionary相比具有如下特性：

* NSDictionary/NSMutableDictionary会复制keys并且通过强引用values来实现存储；
* NSMapTable是可变的；
* NSMapTable可以通过弱引用来持有keys和values，所以当key或者value被deallocated的时候，所存储的实体也会被移除；
* NSMapTable可以在添加value的时候对value进行复制；

和NSHashTable类似，NSMapTable可以随意的存储指针，并且利用指针的唯一性来进行对比和重复检查。

用法：假设用NSMapTable来存储不用被复制的keys和被若引用的value，这里的value就是某个delegate或者一种弱类型。

```objc
id delegate = ...;
NSMapTable *mapTable = [NSMapTable mapTableWithKeyOptions:NSMapTableStrongMemory
                                             valueOptions:NSMapTableWeakMemory];
[mapTable setObject:delegate forKey:@"foo"];
NSLog(@"Keys: %@", [[mapTable keyEnumerator] allObjects]);
```

看完上面几个小例子后，你可能会想 “为什么不使用object subscripting呢?”。一些激进的NSHipster估计已经开始动手写NSMapTable的subscripting category了。

**那么为什么NSMapTable不能继承subscripting？**

来看看下面的代码：

```objc
- (id)objectForKeyedSubscript:(id <NSCopying>)key;
- (void)setObject:(id)obj forKeyedSubscript:(id <NSCopying>)key;
```

**注意：参数key是类型的**。这对NSDictionary NSMutableDictionary来讲是非常有用的，但是我们不能臆断对NSMapTable也同样适用。我们陷入一个僵局：通过id，我们不能利用NSMapTable实现subscripting。如果object subscripting的代理方法放弃了约束，那么使用NSMutableDictionary -setObject:forKeyedSubscript:的时候编译将得不到想要的结果。

所以说实话，对比NSMapTable所处的位置，句法的方便和快捷并不是大数人所关注的。
 
通常，记住编程不是为了让人更聪明，而是最大化抽象一个问题的能力。NSSet和NSDictionary是非常伟大的类，他们能解决99%的问题，也无疑是用来工作的正确工具。如果，然而你的问题牵扯到上述的内存问题时候，NSHashTable和NSMapTable是值得一看的。