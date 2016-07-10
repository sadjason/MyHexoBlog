title: Objective-C copy那些事儿
date: 2015-01-28 09:14:48
tags: Objective-C
categories: iOS
---

## 写在前面

Objective-C中的copy相关内容比我想象中要丰富多了。

## NSCopying和NSMutableCopying协议

使用对象时经常需要拷贝它。在Objective-C中，此操作是通过`copy`和`mutableCopy`方法完成的，基类NSObject中与copy相关的API如下：

```objc
- (id)copy;
- (id)mutableCopy;
    
+ (id)copyWithZone:(struct _NSZone *)zone OBJC_ARC_UNAVAILABLE;
+ (id)mutableCopyWithZone:(struct _NSZone *)zone OBJC_ARC_UNAVAILABLE;
```

显然，NSObject已经实现了`copy`和`mutableCopy`方法。

如果想让自己的类（继承自NSObject，假设叫「CustomClass」）支持拷贝操作，该怎么弄呢？第一个想到的处理方式恐怕是重写`copy`方法（暂时不谈`mutableCopy`，稍后再叙）。**No！这种做法是错误的！**

正确的做法是让自定义类实现NSCopying协议（NSObject并未遵循该协议），该协议只有一个方法：

```objc
@protocol NSCopying
    
- (id)copyWithZone:(NSZone *)zone;
    
@end
```

简单来说，当对某个对象发送`copy`消息时，NSObject copy的实现会去自动调用`copyWithZone:`方法，有点回调的感觉；因此，若想支持拷贝操作，需要在自定义类中让其支持NSCopying协议并实现`copyWithZone:`方法，而不是重写`copy`方法。

参数zone是什么鬼？这是因为在以前开发程序时，会把内存分为不同的「区」（zone），而对象会创建在某个区里面。现在不用了，每个程序只有一个区：默认区（default zone）。所以说，**尽管必须实现这个方法，但是不必担心其中的zone参数**。

举个栗子，有个表示个人信息的类，可以在其接口定义中声明此类遵循NSCopying协议，如下：

```objc
@interface UserInfo : NSObject <NSCopying>
    
@property (nonatomic, copy) NSString *firstName;
@property (nonatomic, copy) NSString *lastName;
    
- (instancetype)initWithFirstName:(NSString *)firstName
                      andLastName:(NSString *)lastName;
    
@end
    
@implementation UserInfo
    
- (instancetype)initWithFirstName:(NSString *)firstName
                      andLastName:(NSString *)lastName {
    if (self = [super init]) {
        _firstName = [firstName copy];
        _lastName = [lastName copy];
    }
    return self;
}
    
#pragma mark - NSCopying
    
- (instancetype)copyWithZone:(NSZone *)zone {
    UserInfo *copy = [[[self class] allocWithZone:zone] initWithFirstName:_firstName
                                                              andLastName:_lastName];
    return copy;
}
    
@end
```

再来讲一下`mutableCopy`方法和`NSMutableCopying`协议；它们俩与`copy`方法和`NSCopying`协议相对应。当你的类还有mutable版本时，你还应该遵循`NSMutableCopying`协议，并实现`mutableCopyWithZone:`方法，这样，当向该类对象发送`mutableCopy`消息时，NSObject的`mutableCopy`方法实现代码中会回调你的`mutableCopyWithZone:`方法。

值得一提的是，虽然在自定义`copyWithZone:`和`mutableCopyWithZone:`中你可以弄各种花样，但是务必保证如下的逻辑：

```
[CustomClass copy] -> CustomClass
[MutableCustomClass copy] -> CustomClass
[CustomClass mutableCopy] -> MutableCustomClass
[MutableCustomClass mutableCopy] -> MutableCustomClass
```

呵呵，就这些，似乎也就那么回事儿嘛。

No！不止这些！

向immutable对象发送`copy`消息一定会得到一个新对象吗？

No！下面的测试栗子所做的事情是分别向不可变的NSString、NSArray、NSDictionary以及NSSet对象发送`copy`消息，得到几个「新」的对象，新对象显然是immutable的，问题是：这些「新」对象真的是新对象吗？如下栗子分别把新老对象的地址给打印出来：

```objc
NSString *testString = @"1";
NSString *copyString = [testString copy];
NSLog(@"testString address = %x", testString);
NSLog(@"copyString address = %x", copyString);
// print:
// testString address = 79720cc0
// copyString address = 79720cc0
    
NSArray *testArray = @[@1, @2, @3];
NSArray *copyArray = [testArray copy];
NSLog(@"testArray address = %x", testArray);
NSLog(@"copyArray address = %x", copyArray);
// print:
// testArray address = 79722fb0
// copyArray address = 79722fb0
    
NSDictionary *testDictionary = @{@1:@2};
NSDictionary *copyDictionary = [testDictionary copy];
NSLog(@"testDictionary address = %x", testDictionary);
NSLog(@"copyDictionary address = %x", copyDictionary);
// print:
// testDictionary address = 79722fd0
// copyDictionary address = 79722fd0    
    
NSSet *testSet = [NSSet setWithObject:@1];
NSSet *copySet = [testSet copy];
NSLog(@"testSet address = %x", testSet);
NSLog(@"copySet address = %x", copySet);
// print:
// testSet address = 79722ff0
// copySet address = 79722ff0
```

答案很明了！NSString、NSArray、NSDictionary以及NSSet，这是我们最常用的四个含有mutable版本的对象；向这些immutable对象发送`copy`消息，这些对象会直接返回本身，而不是返回一个新创建的对象。

关于这一点，笔者返回使用各种姿势测试了很多次，均得到这样的结果；但是目前还没能找到比较权威的说法对这个现象进行说明。不过想想也很容易理解，对于一个immutable对象，真的没必要再复制一个，毕竟其中的内容不会发生改变，如果复制了，那么内存中将会存在两个一模一样的资源，岂不浪费？

总结这一段内容的要点如下：
* 若想令自己的类具备拷贝功能，则需要遵循NSCopying协议，实现其定义的`copyWithZone:`方法；
* 若自定义的类分为immutable和mutable版本，则需要同时遵循NSCopying和NSMutableCopying协议；
* 向immutable对象发送**copy**消息，并不一定会得到一个新对象；

## 深拷贝和浅拷贝

在很长时间里，我都认为**浅拷贝**（shallow copy）指的是「指针拷贝」，而**深拷贝**（deep copy）才是真正copy一个对象；显然，这种说法是不正确的。

一般来说，「深拷贝」和「浅拷贝」这两个概念是分析collection类型才会谈及的。深拷贝的意思是：再拷贝对象时，将其底层数据也一并复制过去。**Foundation框架中所有collection类在默认情况下都执行浅拷贝，也就是说，只拷贝容器对象本身，而不复制其中数据。这样做的原因在于，容器内的对象未必能拷贝，而且调用者也未必想在拷贝容器时一并拷贝其中每一个对象。**

深拷贝和浅拷贝对比图如下：

<div class="imagediv" style="width: 560px; height: 280px">{% asset_img 20150720-01.png %}</div>

## 理解@property中的copy修饰符

经常看到@property中有些对象类型属性被`strong`修饰，有些被`copy`修饰。`strong`修饰符的作用不消多说，该如何理解`copy`修饰符呢？

**如何使用copy修饰符**

关于@property中`copy`修饰符的使用，我曾经历了这么两个阶段：
1. 使用`copy`修饰mutable类型，使用`strong`修饰immutable类型；
2. 使用`copy`修饰immutable类型，使用`strong`修饰mutable类型；

关于第1个阶段，我忘记了当时是怎么想的，它显然是错的；

关于第2个阶段，我之所以有这样的认识是因为曾在[stackoverflow](http://stackoverflow.com/questions/387959/nsstring-property-copy-or-retain)中看到了如下这么一段说明：
>For attributes whose type is an immutable value class that conforms to the NSCopying protocol, you almost always should specify `copy` in your `@property` declaration. Specifying retain is something you almost never want in such a situation.

这句话错了吗？当然没有，要不也不会得到这么多的votes。但为什么这么说呢？不晓得是当时没耐心还是咋地，反正怎么思考这个问题。

接着以上文提到的UserInfo为栗子，对之进行简化，只是定义两个`NSString`属性：firstName和lastName，作为对比前者使用`copy`修饰，后者使用`strong`修饰。如下：

```objc
@interface UserInfo : NSObject <NSCopying>
    
@property (nonatomic, copy) NSString *firstName;
@property (nonatomic, strong) NSString *lastName;
    
@end
```

基于UserInfo创建实例进行测试：

```objc
NSMutableString *mutableFirstName = [NSMutableString stringWithFormat:@"张"];
NSMutableString *mutableLastName = [NSMutableString stringWithFormat:@"不坏"];
    
UserInfo *u = [[UserInfo alloc] init];
u.firstName = mutableFirstName;
u.lastName  = mutableLastName;
NSLog(@"全名：%@%@", u.firstName, u.lastName);
// print: 全名：张不坏
    
// 改mutableFirstName「张」为「长孙」
[mutableFirstName deleteCharactersInRange:NSMakeRange(0, 1)];
[mutableFirstName appendString:@"长孙"];
// 改mutableLastName「不坏」为「不坏蛋」
[mutableLastName appendString:@"蛋"];
NSLog(@"全名：%@%@", u.firstName, u.lastName);
// print: 全名：张不坏蛋
```

简单来说，对于immutable对象类型属性，假设该类型存在mutable版本，若使用`strong`修饰该属性，则将会是不安全的。

在上述代码中，UserInfo对象u的lastName属性被`strong`修饰，对之赋值一个mutable类型mutableLastName，之后改变mutableLastName的值（由「不坏」变为「不坏蛋」），显然也影响到了`u.lastName`的值，这通常是我们所不希望发生的；作为对比，u.firstName被`copy`修饰，也为之赋值mutable类型mutableFirstName，之后也改变mutableFirstName的值（由「张」变为「长孙」），但是u.firstName不受影响。

再往深一点看：`@property`的`copy`的作用机制是什么？根据我的理解，`copy`修饰符的意义有两点：
1. 在系统自动合成属性的setter提供「指示」，使用类似于`_iVar = [var copy];`的方式进行赋值；
2. 告诉使用者，安心的使用吧！

因此，根据我的理解，系统合成UserInfo的firstName和lastName的setter代码如下：

``` Objective-C Objective-C
- (void)setFirstName:(NSString *)firstName {
    _firstName = [firstName copy];
}
    
- (void)setLastName:(NSString *)lastName {
    _lastName = lastName;
}
```

写到这里，可以回答一个常见的问题了：「如何重写带copy关键字的setter？」

换句话说，即便firstName属性被`copy`修饰，但是如果重写setter时采用错误的方式，`copy`带来的好处会荡然无存。譬如这样重写`setFirstName:`：

``` Objective-C
- (void)setFirstName:(NSString *)firstName {
    _firstName = firstName;
}
```

得到的结果如下（显然，firstName也是不安全的）：

```
全名：张不坏
全名：长孙不坏蛋
```

继续深挖：

1. 是不是所有遵循NSCopying类型属性都应该使用copy修饰呢？
2. mutable类型属性可以使用`copy`修饰吗？

对于第一个问题，答案是No！对于向NSString、NSDictionary等属性才需要使用`copy`修饰，因为它们存在mutable版本，在为属性赋值时，右值很可能是它们的mutable类型对象，若使用`strong`修饰则会带来不稳定因子；另外一个方面，如果属性类型不存在对应的mutable版本，则完全不用担心这点，反正你也无法在外部修改它，不稳定因子自然不存在了。

对于第二个问题，答案仍然是No！被修饰符`copy`修饰的属性，默认的setter赋值方式是`_iVar = [var copy];`而`copy`方法返回的是immutable类型，将immutable对象赋值给mutable类型指针显然是不对的。

P.S：如果存在`mutableCopy`修饰符，或许可以使用`mutableCopy`修饰mutable属性^_^。

## 本文参考

* 《Effective Objective-2.0》
* [NSString: copy还是retain](http://stackoverflow.com/questions/387959/nsstring-property-copy-or-retain)
* [Value Objects](http://www.objc.io/issue-7/value-objects.html)
* [值对象](http://objccn.io/issue-7-2/)