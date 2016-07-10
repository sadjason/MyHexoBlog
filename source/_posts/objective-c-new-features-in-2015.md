title: 2015 Objective-C新特性
date: 2015-06-26 14:39:28
categories: Objective-C

---

**转载说明**

本文完全转载自[@sunnyxx](http://blog.sunnyxx.com/)的博文《[2015 Objective-C 新特性](http://blog.sunnyxx.com/2015/06/12/objc-new-features-in-2015/)》。

## Overview

自WWDC 2015推出和开源Swift 2.0后，大家对Swift的热情又一次高涨起来，在羡慕创业公司的朋友们大谈Swift新特性的同时，也有很多像我一样工作上依然需要坚守着Objective-C语言的开发者们。今年的WWDC中介绍了几个Objective-C语言的新特性，还是在「与Swift协同工作」这种Topic里讲的，越发凸显这门语言的边缘化了，不过有新特性还是极好的，接下来，本文将介绍下面三个主要的新特性：

* Nullability
* Lightweight Generics
* __kindof

## Nullability

然而Nullability并不算新特性了，从上一个版本的llvm 6.1 (Xcode 6.3) 就已经支持。这个简版的`Optional`，没有Swift中`?`和`!`语法糖的支持，在Objective-C中就显得非常啰嗦了：

```objc
@property (nonatomic, strong, nonnull) Sark *sark;
@property (nonatomic, copy, readonly, nullable) NSArray *friends;
+ (nullable NSString *)friendWithName:(nonnull NSString *)name;
```

假如用来修饰一个变量，前面还要加双下划线，放到block里面就更加诡异，比如一个Request的start方法可以写成：

```objc
- (void)startWithCompletionBlock:(nullable void (^)(NSError * __nullable error))block;
```

除了这俩外，还有个`null_resettable`来表示setter nullable，但是getter nonnull，绕死了，最直观例子就是UIViewController中的view属性：

```objc
@property (null_resettable, nonatomic, strong) UIView *view;
```

它可以被设成`nil`，但是调用getter时会触发`-loadView`从而创建并返回一个非nil的view。
从iOS9 SDK中可以发现，头文件中所有API都已经增加了Nullability相关修饰符，想了解这个特性的用法，翻几个系统头文件就差不离了。接口中`nullable`的是少数，所以为了防止写一大堆`nonnull`，Foundation还提供了一对儿宏，包在里面的对象默认加`nonnull`修饰符，只需要把`nullable`的指出来就行，黑话叫Audited Regions：

```objc
NS_ASSUME_NONNULL_BEGIN
@interface Sark : NSObject
@property (nonatomic, copy, nullable) NSString *workingCompany;
@property (nonatomic, copy) NSArray *friends;
- (nullable NSString *)gayFriend;
@end
NS_ASSUME_NONNULL_END
```

Nullability在编译器层面提供了空值的类型检查，在类型不符时给出warning，方便开发者第一时间发现潜在问题。不过我想更大的意义在于能够更加清楚的描述接口，是主调者和被调者间的一个协议，比多少句文档描述都来得清晰，打个比方：

```objc
+ (nullable instancetype)URLWithString:(NSString *)URLString;
```

NSURL的这个API前面加了`nullable`后，更加显式的指出了这个接口可能因为URLString的格式错误而创建失败，使用时自然而然的就考虑到了判空处理。
不仅是属性和方法中的对象，对于局部的对象、甚至C指针都可以用带双下划线的修饰符，可以理解成能用const关键字的地方都能用Nullability。
所以Nullability总的来说就是，写着丑B，用着舒服 - -

## Lightweight Generics

**Lightweight Generics**轻量级泛型，轻量是因为这是个纯编译器的语法支持（llvm 7.0），和Nullability一样，没有借助任何objc runtime的升级，也就是说，这个新语法在Xcode 7上可以使用且完全向下兼容（更低的 iOS 版本）。

**带泛型的容器**

这无疑是本次最重大的改进，有了泛型后终于可以指定容器类中对象的类型了：

```objc
NSArray<NSString *> *strings = @[@"sun", @"yuan"];
NSDictionary<NSString *, NSNumber *> *mapping = @{@"a": @1, @"b": @2};
```

返回值的`id`被替换成具体的类型后，令人感动的代码提示也出来了：

<div class="imagediv" style="width: 348px; height: 43px">{% asset_img 20150704-02.jpg %}</div>

假如向泛型容器中加入错误的对象，编译器会不开心的：

<div class="imagediv" style="width: 444px; height: 53px">{% asset_img 20150704-03.jpg %}</div>

系统中常用的一系列容器类型都增加了泛型支持，甚至连NSEnumerator都支持了，这是非常Nice的改进。和Nullability一样，我认为最大的意义还是丰富了接口描述信息，对比下面两种写法：

```objc
@property (readonly) NSArray *imageURLs;
@property (readonly) NSArray<NSURL *> *imageURLs;
```

不用多想就清楚下面的数组中存的是什么，避免了 NSString 和 NSURL 的混乱。

**自定义泛型类**

比起使用系统的泛型容器，更好玩的是自定义一个泛型类，目前这里还没什么文档，但拦不住我们写测试代码，假设我们要自定义一个 Stack 容器类：

```objc
@interface Stack<ObjectType> : NSObject
- (void)pushObject:(ObjectType)object;
- (ObjectType)popObject;
@property (nonatomic, readonly) NSArray<ObjectType> *allObjects;
@end
```

这个`ObjectType`是传入类型的placeholder，它只能在@interface上定义（类声明、类扩展、Category），如果你喜欢用**T**表示也ok，这个类型在@interface和@end区间的作用域有效，可以把它作为入参、出参、甚至内部NSArray属性的泛型类型，应该说一切都是符合预期的。我们还可以给ObjectType增加类型限制，比如：

```objc
// 只接受 NSNumber * 的泛型
@interface Stack<ObjectType: NSNumber *> : NSObject
// 只接受满足 NSCopying 协议的泛型
@interface Stack<ObjectType: id<NSCopying>> : NSObject
```

若什么都不加，表示接受任意类型（`id`）；当类型不满足时编译器将产生error。
实例化一个Stack，一切工作正常：

<div class="imagediv" style="width: 417px; height: 72px">{% asset_img 20150704-04.jpg %}</div>

对于多参数的泛型，用逗号隔开，其他都一样，可以参考NSDictionary的头文件。

**协变性和逆变性**

当类支持泛型后，它们的Type发生了变化，比如下面三个对象看上去都是Stack，但实际上属于三个Type：

```objc
Stack *stack; // Stack *
Stack<NSString *> *stringStack; // Stack<NSString *>
Stack<NSMutableString *> *mutableStringStack; // Stack<NSMutableString *>
```

当其中两种类型做类型转化时，编译器需要知道哪些转化是允许的，哪些是禁止的，比如，默认情况下：

<div class="imagediv" style="width: 660px; height: 179px">{% asset_img 20150704-05.jpg %}</div>

我们可以看到，不指定泛型类型的Stack可以和任意泛型类型转化，但指定了泛型类型后，两个不同类型间是不可以强转的，假如你希望主动控制转化关系，就需要使用泛型的**协变性**和**逆变性**修饰符了：

* `__covariant` - 协变性，子类型可以强转到父类型（里氏替换原则）
* `__contravariant` - 逆变性，父类型可以强转到子类型（WTF?）

协变：

```objc
@interface Stack<__covariant ObjectType> : NSObject
```

效果：

<div class="imagediv" style="width: 697px; height: 103px">{% asset_img 20150704-06.jpg %}</div>

逆变：
```objc
@interface Stack<__contravariant ObjectType> : NSObject
```

效果：

<div class="imagediv" style="width: 695px; height: 97px">{% asset_img 20150704-07.jpg %}</div>

协变是非常好理解的，像NSArray的泛型就用了协变的修饰符，而逆变我还没有想到有什么实际的使用场景。

## __kindof

`__kindof`这修饰符还是很实用的，解决了一个长期以来的小痛点，拿原来的UITableView的这个方法来说：

```objc
- (id)dequeueReusableCellWithIdentifier:(NSString *)identifier;
```

使用时前面基本会使用UITableViewCell子类型的指针来接收返回值，所以这个API为了让开发者不必每次都蛋疼的写显式强转，把返回值定义成了`id`类型，而这个API实际上的意思是返回一个UITableViewCell或UITableViewCell子类的实例，于是新的`__kindof`关键字解决了这个问题：

```objc
- (__kindof UITableViewCell *)dequeueReusableCellWithIdentifier:(NSString *)identifier;
```

既明确表明了返回值，又让使用者不必写强转。再举个带泛型的例子，UIView的subviews属性被修改成了：

```objc
@property (nonatomic, readonly, copy) NSArray<__kindof UIView *> *subviews;
```

这样，写下面的代码时就没有任何警告了：

```objc
UIButton *button = view.subviews.lastObject;
```

## Where to go

有了上面介绍的这些新特性以及如**instancetype**这样的历史更新，Objective-C这门古老语言的类型检测和类型推断终于有所长进，现在不论是接口还是代码中的id类型都越来越少，更多潜在的类型错误可以被编译器的静态检查发现。

同时，个人感觉新版的Xcode对继承链构造器的检测也加强了，**NS_DESIGNATED_INITIALIZER**这个宏并不是新面孔，可以使用它标志出像Swift一样的指定构造器和便捷构造器。

最后，附上一段用上了所有新特性的代码，Swift是发展趋势，如果你暂时依然要写Objective-C代码，把所有新特性都用上，或许能让你到新语言的迁移更无痛一点。

<div class="imagediv" style="width: 637px; height: 210px">{% asset_img 20150704-08.jpg %}</div>

## References

https://msdn.microsoft.com/zh-cn/library/dd799517.aspx
https://gist.github.com/jtbandes/881f07a955ff2eadd1a0