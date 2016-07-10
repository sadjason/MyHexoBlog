title: Objective-C基础知识
date: 2015-04-12 22:27:32
tag: Objective-C
categories: iOS

---

## 写在前面

本文属于汇总文，对Objective-C语言层面的一些基础问题进行汇总，有的问题可直接在本文找到答案，有的知识提供其他博文的链接。


## 内联函数

「内联函数」是一个很老的概念，在其他语言譬如C++语言中也出现了。所谓「内联函数」指的是：有函数的结构，但不具备函数的性质，类似于宏替换功能的代码块。

在实际应用中，常常把规模较小、逻辑较简单的操作定义成内联函数，定义内联函数只要在函数前面加上`inline`关键字修饰即可；站在编译器的角度，处理内联函数就是在每个它的调用点上「内联地」展开。假设有定义如下函数：

```objc
inline NSString * getNavigationTitleWithCount(int cnt) {
    return [NSString stringWithFormat:@"已选中%d项", (int)cnt];
}
```

则编译器对如下代码

```objc
UILabel *label = [UILabel new];
label.text = getNavigationTitleWithCount(0);
```

的处理，如同对如下代码代码的处理：

```objc
UILabel *label = [UILabel new];
label.text = [NSString stringWithFormat:@"已选中%d项", (int)0];
```

一句话说明：**程序在调用内联函数时的开销和调用宏的开销是一样的，但内联函数显然比宏要强大一些，至少它保证了类型安全**；相较于普通函数，调用内联函数没有「保护现场、push栈、pop栈等等」之类的开销。

内联函数的好处显而易见，但并不是所有函数都可以形成真正的内联函数。如上所述，定义一个内联函数只要在定义时加上`inline`关键字即可，但能否形成真正的内联函数，还要看编译器对内联函数体内部定义的具体处理。

一般来说，内联函数定义的代码量逻辑简单、代码量小，并且属于那种频繁使用的代码块；**内联函数不能使用循环语句，不能使用递归调用**；

## const的使用

在Objective-C代码中，经常需要定义一些常量指针，譬如AFNetworking中有如下代码：

```objc
extern NSString * const AFNetworkingReachabilityDidChangeNotification;
extern NSString * const AFNetworkingReachabilityNotificationStatusItem;
```

和其他语言一样，定义常量的关键字是`const`，但问题是常常不知道`const`该往哪里放，`NSString * const XXOO`还是`NSString const * XXOO`？

其实，在Objective-C中还好一点，C/C++中`const`的各种使用姿势更难懂，如下：

```c
const char * label1       = "";
char const * label2       = "";
char * const label3       = "";
const char * const label4 = "";
```

经常傻傻分不清楚。不过还好，脑子恰好还残留着一些本科老师讲过的内容，记得本科C语言老师教过一种简单的记忆方法：const总是修饰其左边的东东，如果其左边没有东东，则修饰右边的东东。

如何理解呢？对于`char const * label2`而言，const的左边是char，所以const修饰的是char本身，即内容本身，所以表示“label1指向的内容是常量，不可变”；对于`const char * label1`而言，const左边没有东东，所以它修饰右边的东东，即char，所以表示“label2指向的内容是常量，不可变”；对于`char * const label3`，const左边的东东是*，所以修饰的是指针，即表示“label3指针本身是常量，不可变”，所以解释如下：

```c
const char * label1       = "";      // 内容是常量，不可变
char const * label2       = "";      // 内容是常量，不可变
char * const label3       = "";      // 指针是常量，不可变
const char * const label4 = "";      // 指针和内容都是常量，不可变
```

这个判断方法同样适用于Objective-C，所以如下代码是没问题的：

```objc
// const的左边是NSString，修饰的是NSString对象，表示“NSString对象本身不可修改”（但指针可以）
static NSString const * testString = @"I am an iOS developer";
    
- (void)viewDidLoad {
    [super viewDidLoad];
    testString = @"我是一个iOS开发者"; // 合法
    NSLog(@"%@", testString);   // print "我是一个iOS开发者"
}
```

但如下代码不合法：

```objc
// const的左边是*，修饰的是指针本身，表示testString地址值不可修改
static NSString * const testString = @"I am an iOS developer";
    
- (void)viewDidLoad {
    [super viewDidLoad];
    testString = @"我是一个iOS开发者"; // 非法
    NSLog(@"%@", testString);
}
```

## .mm文件

* .m文件是Objective-C文件
* .mm文件相当于C++或者C文件

## Extension和Category

参考：http://blog.csdn.net/leikezhu1981/article/details/19091049

## Compiler Directives

http://nshipster.com/at-compiler-directives/

http://blog.sunnyxx.com/2014/04/13/objc_dig_interface/

## #pragma

详细内容参考《[NSHipster -- #pragma](http://nshipster.cn/pragma/)》和《[预处理指令#pragma](/pragma/)》

## 0/nil/Nil/NULL/NSNull

这一部分内容摘自[nil/Nil/NULL/NSNull](http://nshipster.cn/nil/)，原文详见[这里](http://nshipster.com/nil/)。

理解「不存在」的概念不仅仅是一个哲学的问题，也是一个实际的问题。我们是有形宇宙的居民，而原因在于逻辑宇宙的存在不确定性。作为一个逻辑系统的物理体现，电脑面临一个棘手的问题，就是如何用存在表达「不存在」。

在Objective－C中，有几个不同种类的「不存在」。C语言用`0`来作为不存在的原始值，而`NULL`作为指针（这在指针环境中相当于`0`）。

Objective-C在C的基础上增加了`nil`。`nil`是一个指向不存在的对象指针，虽然它在语义上与`NULL`不同，但它们在技术上是相等的。

在框架层面，Foundation定义了`NSNull`，`NSNull`中有一个类方法`+null`，它返回一个单独的`NSNull`对象。NSNull与nil以及NULL不同，因为它是一个实际的对象，而不是一个零值。

另外，在[Foundation/NSObjCRuntime.h](https://gist.github.com/4469665)中，`Nil`被定义为指向零的类指针，可以把它看做是`nil`的表亲。虽然它鲜为人知，但至少值得注意一下。

总的来说，这里的四个表达没有的值是每个Objective-C程序员都应该知道的：

标志 | 值 | 含义
:------------: | :------------: | :------------:
NULL | (void *)0  | C指针的字面零值
nil | (id)0  | Objective-C对象的字面零值
Nil | (Class)0 | Objective-C类的字面零值
NSNull | [NSNull null] | 用来表示零值的单独的对象

## 关于nil的一些事儿

刚被分配的NSObject的内容被设置为0。也就是说那个对象所有的指向其他对象的指针都从`nil`开始，所以在`init`方法中设置`self.(association) = nil`之类的表达是没有必要的。

当然，也许nil最显著的行为是，它虽然为零，仍然可以有消息发送给它。在其他的语言中，比如C++，这样做会使你的程序崩溃，**但在Objective-C中，在`nil`上调用方法返回一个零值**。这大大的简化了表达，因为它避免了在使用`nil`之前对它的检查。


## isEqual:和==

参考http://nshipster.com/equality/。

## 基本数据类型长度

名字 | typedef（32bit/64bit） | 长度（32bit/64bit）（单位：bit）
:---: | :---: | :---:
int | - | 32/32
long | - | 32/64
long long | - | 64/64
NSInteger | int/long | 32/64
float | - | 32/32
double | - | 64/64
long double | - | 128/128
CGFloat | float/double | 32/64


对于`int`、`long`、`float`、`double`，在不同的平台下（32位和64位）下唯一有区别的是`long`，其余三个在不同硬件平台所占据数据长度是一致的；至于`NSInteger`和`NSFloat`，数据长度和平台完全对应，由此可见，基于Cocoa编程尽可能使用`NSInteger`和`NSFloat`，而不直接使用`int`、`float`等。

P.S：与`NSInteger`和`int`对应的是`NSUInteger`和`unsigned int`，但是不存在所谓的`unsigned float`和`CGUFloat`哦！

`int`和`long`的最大值和最小值比较容易计算，`float`和`double`的最大值和最小值计算则麻烦多了，IEEE-754协议对此专门做了定义，详见[百度百科：IEEE-754](http://baike.baidu.com/view/1698149.htm)和[wiki: IEEE floating point](https://en.wikipedia.org/wiki/IEEE_floating_point)

## #import

**#import v.s #include**

`#import`和`#include`的作用类似：都是先要求**预处理器**读取某个文件（一般是头文件），然后将读入的内容添加至输出到对应的位置；或者简单来说，二者都用作**导入文件**。前者确保**预处理器**对指定的文件只导入一次，后者则允许多次导入同一个文件。

**尖括号 v.s 双引号**

如果使用尖括号（`<>`），则编译器会先在预先设定好的标准目录下查找相应的文件（譬如系统头文件）；如果使用双引号（`""`），则编译器会先在项目目录下查找相应的头文件。

**#import v.s @import**

如下内容摘自《[在ios 7下，使用@import代替#import](http://www.cnblogs.com/sammyCoding/p/3654743.html)》：
>在xcode 5 下，为了更易于开发，增加了modules和 auto-linking 这两个新特性。用 @import 来增加框架 到项目中比用 #import会更有效. 我们来看看为什么：
>>Modules for system frameworks speed build time and provide an alternate means to import APIs from the SDK instead of using the C preprocessor. Modules provide many of the build-time improvements of precompiled headers with less maintenance or need for optimization. They are designed for easy adoption with little or no source changes. Beyond build-time improvements, modules provide a cleaner API model that enables many great features in the tools, such as Auto Linking.

>Modules and auto-linking 默认情况下是enabled的。 如果是旧的项目，你可以通过设置"Language - Modules." 来设置Enable Modules 和Link Frameworks Automatically 为Yes。
>另外一个使用moudules的好处是你再也不用去链接你的framework到你的项目了。
例如，在以前，如果你要使用MapKit这个框架，你要这样做
1) 使用语句 #import <MapKit/MapKit.h> 导入框架
2) 去到项目的build phases 设置项，找到MapKit.framework.并加入到Link Binary With Libraries里面。
如果使用modules的话，只需要加入语句 "@import MapKit;" 你就可以开始使用了,根本不需要链接到你的项目。

更多内容参考《[stackoverflow: @import vs #import - iOS 7](http://stackoverflow.com/questions/18947516/import-vs-import-ios-7)》。

## 弱引用的自动置零特性

当某个由弱引用指向的对象被释放时，相应的指针变量会被**归零**（zerod），即赋为`nil`。


## 多态、动态类型和动态绑定

**什么是多态**

一句话解释：不同对象对同一个消息的不同响应。

**什么是动态绑定**

「动态绑定」是指在执行期间（非编译期）判断所引用对象的实际类型，根据其实际的类型调用其相应的方法。

**动态类型id**

`id`数据类型是一种通用的对象类型。也就是说，它可以用来存储任何对象。在代码中可以向id类型发送任何消息，Objective-C系统（编译器）不会在编译期对之进行类型检验，只有运行到相关代码时才会判断所引用对象的实际类型，然后根据实际的类型调用其相应的方法。

举个例子说明：

```objc
int main(int argc, char * argv[]) {
    id number = @5;
    if ([number isEqualToString:@"whatTheFuck"]) {
        NSLog(@"见鬼了");
    }
    return 0;
}
```

上述代码中，首先创建一个NSNumber类型实例，并把它赋给`id`类型变量number；在if语句中，向number发送`isEqualToString:`消息，根据我们对NSNumber的了解，它是没有定义`isEqualToString:`方法的，因此不会对这个消息进行响应。然而，这段代码完全可以以「0 error、0 warning」通过编译，只是在运行时会抛出NSInvalidArgumentException异常：
reason: '-[__NSCFNumber isEqualToString:]: unrecognized selector sent to instance 0x7bf67200'

这个示例形象阐述「动态绑定」的含义，并且验证了「id类型可以用来存储任何对象」的说法。

**Objective-C如何实现多态的**

根据上述对「多态」、「动态绑定」以及「动态类型id」的解释，可以直接回答：Objective-C实现多态的方式是动态绑定。

P.S：也有人认为「继承体系中方法重载也是实现多态的一种方式」，笔者对此比较疑惑，因为我总觉得「动态绑定」已将此包括进去了。

## @synthesize v.s @dynamic

引用[stackoverflow: @synthesize vs @dynamic, what are the differences?](http://stackoverflow.com/questions/1160498/synthesize-vs-dynamic-what-are-the-differences)：
>@synthesize will generate getter and setter methods for your property. @dynamic just tells the compiler that the getter and setter methods are implemented not by the class itself but somewhere else (like the superclass or will be provided at runtime).

简单来说，`@synthesize`的作用是：指示编译器，让其在编译期间自动生成getter/setter方法；当有自定义的setter或getter实现时，自定义setter或getter会屏蔽自动生成的setter或getter。不晓得从什么时候开始，Xcode默认帮助合成属性，无需显式使用`@synthesize`指令。

P.S：根据我的理解，准确来说，`@synthesize`的作用还包括「自动合成property对应的实例变量」，当然，如果已然有对应的实例变量存在，则会略过。比如说property名为foo，存在一个名为_foo的实例变量，那么就不会再自动合成新变量了。总之，`@synthesize`是一个帮助省功夫的指令。

P.S：不晓得为什么，当对某个属性同时自定义setter和getter时，XCode要求必须显式书写`@synthesize`，如下：

```objc
@interface UserInfo : NSObject
    
@property (nonatomic, copy) NSString *name;
    
@end
    
@implementation UserInfo
    
@synthesize name = _name;
    
- (void)setName:(NSString *)name {
    _name = [name copy];
}
    
- (NSString *)name {
    return _name;
}
    
@end
```

而`@dynamic`告诉编译器，不在编译期自动生成getter/setter方法，setter和getter会在runtime提供。`@dynamic`的主要应用场景是Core Data，除此之外，在category使用关联属性时也会用到。