title: 理解Objective-C Runtime（一）预备知识
date: 2015-04-26 20:07:37
tags:
- Runtime
- Objective-C
categories: iOS

---

## 写在前面

很早就知道了Objective-C Runtime这个概念，「Objective-C奇技淫巧」「iOS黑魔法」各种看起来很屌的主题中总会有它的身影；但一直没有深入去学习，一来觉得目前在实际项目中还没有必要了解，二来懒。但，若想成为一个合格的iOS开发者，这个东西是躲不过的，好吧，抡起胳膊开始吧，争取一点点把它整明白吧！

和了解其他技术一样，在了解一个东西之前，我总是问自己，这个有啥实际意义？为什么要了解她？多自问一些问题，找到实际意义，在理解上总会容易一些，学习也自然更有目的性一些。

好吧，看了十几篇博客和iOS文档，还是很容易针对Objective-C Runtime提出一些问题的，有了这些问题，理解Objective-C Runtime就自然不会过于枯燥了，譬如：

* 在iOS中调用方法为什么叫「发送消息」，为什么这个概念这么重要？
* KVO的实现原理是什么，是否涉及Objective-C Runtime？
* Category的原理是什么？为什么其他语言中没有这个东东？
* Category中以「关联对象」的方式添加动态属性的原理什么？

看了一些大牛的博客之后，对Objective-C Runtime的重要性在心里基本有谱了，但路要一步一步走，饭要一口一口吃，先将一些基础性的概念进行整理，「消息转发」「Method Swizzling」之类的高端东西放在之后的博客分析吧！


## 关于Objective-C Runtime

**Runtime的学习资源**

Runtime的学习资源非常丰富，下面是个人的一些整理：

* 《Objective-C Runtime Programming Guide》（官方文档）；
* `/usr/include/objc/`下的头文件，譬如`objc.h`，`runtime.h`，`NSObject.h`，`message.h`等；`/usr/include/objc/`下的头文件不多，都可以看一下；
* 《Effective Objective-C 2.0》；
* 《[理解 Objective-C Runtime](http://www.tuicool.com/articles/FRRVNv)》；
* 《[Objective-C对象模型及应用](http://blog.devtang.com/blog/2013/10/15/objective-c-object-model/)》；
* 《[深入理解Tagged Point](http://blog.devtang.com/blog/2014/05/30/understand-tagged-pointer/)》；
* 《[Objective-C Runtime](http://yulingtianxia.com/blog/2014/11/05/objective-c-runtime/)》；
* Objective-C Runtime系列博客：
    1. 《[Objective C Runtime](http://tech.glowing.com/cn/objective-c-runtime/)》；
    2. 《[Method Swizzling 和 AOP 实践](http://tech.glowing.com/cn/method-swizzling-aop/)》；
    3. 《[如何自己动手实现 KVO](http://tech.glowing.com/cn/implement-kvo/)》；
* 刨根问底Objective－C Runtime系列博客：
    1. 《[刨根问底Objective－C Runtime（1）－ Self & Super](http://t.cn/R7HYfhz)》；
    2. 《[刨根问底Objective－C Runtime（2）－ Object & Class & Meta Class](http://t.cn/R7QUSUE)》；
    3. 《[刨根问底Objective－C Runtime（3）－ 消息 和 Category](http://t.cn/R7meOzE)》；
    4. 《[刨根问底Objective－C Runtime（4）－ 成员变量与属性](http://t.cn/R7mdOq1)》；
* 《[继承自NSObject的不常用又很有用的函数（2）](http://www.cnblogs.com/biosli/p/NSObject_inherit_2.html)》；

>P.S: 值得一提的是，Objective-C Runtime是开源的，任何时候都能从http://opensource.apple.com中获取源代码。

**动态语言vs静态语言**

在网上阅读Objective-C Runtime相关的信息时都提到了「Objective-C是一门动态语言...」「作为一门动态语言...」之类的字眼，这让我有些诧异。因为根据我之前的理解：虽然动态和静态语言都是相对的，但Objective-C怎么也不算动态语言吧！动态语言是Python、Javascript这种弱类型语言吧？！

关于Objective-C是否是动态语言，笔者才疏学浅，不敢妄论，知乎中关于这个有讨论：[Objective-C是动态语言吗？为什么？](http://www.zhihu.com/question/19970471)

但无论如何，它比C++、C这类传统静态语言具备更多的动态特性，说它是动态语言或许不为过吧。暂时不纠结这个问题了，希望以后有更多的自信回答这个问题。跟随主流意见，暂时认为Objective-C是一门动态语言吧。

P.S: 后来又仔细阅读了一下《Effective Objective-C 2.0》，其中对「Objective-C是动态语言」有更好的佐证，后面阐述「Objective-C的消息传递机制」时再补充说明吧！

P.S: 关于OC是动态语言这个话题，总会少不了「动态特性」「动态类型」「动态绑定」这几个关键词的讨论分析，笔者在博文《[Objective-C基础知识](/2015/04/12/Basics-in-Objective-C/#多态、动态类型和动态绑定)》中进行了简单的概述。

**Runtime是什么**

Runtime是什么？为什么有Runtime这个东东？

《Objective-C Runtime Programming Guide》的introduction的第一段就回答了这个问题：
>The Objective-C language defers as many decisions as it can from compile time and link time to runtime. Whenever possible, it does things dynamically. This means that the language requires not just a compiler, but also a runtime system to execute the compiled code. The runtime system acts as a kind of operating system for the Objective-C language; it’s what makes the language work.

不过貌似不够清晰，[理解Objective-C Runtime](http://www.tuicool.com/articles/FRRVNv)中有更浅显的解释：
>Objective-C是面相运行时的语言（runtime oriented language），就是说它会尽可能的把编译和链接时要执行的逻辑延迟到运行时。这就给了你很大的灵活性，你可以按需要把消息重定向给合适的对象，你甚至可以交换方法的实现，等等；这就需要使用runtime，runtime可以做对象自省查看他们正在做的和不能做的（don't respond to）并且适当地分发消息。

>Objective-C的Runtime是一个运行时库（Runtime Library），它是一个主要使用C和汇编写的库，为C添加了面相对象的能力并创造了Objective-C。这就是说它在类信息（Class Information）中被加载，完成所有的方法分发，方法转发，等等。Objective-C runtime创建了所有需要的结构体，让Objective-C的面相对象编程变为可能。

## 关于Runtime的一些术语

各种Runtime资料中有很多术语，有些是以前认识但理解不深刻的，譬如id、Class，有些是很少接触的，譬如isa。

为了更好地理解Runtime，先了解Runtime的一些术语是非常必要的。

首先简单提一下`objc_class`，它是一个结构体，其中包含一些类信息；然后是objc_object，它也是一个结构体，但它只包括一个条目，isa，后者是一个objc_class指针；

其实，说到底，NSObject就是建立在objc_class这个结构体之上的，所以了解objc_class对于了解NSObject以及Cocoa的对象模型是非常重要的。

P.S: objc_class和isa相对而言信息量比较大，后文再详细阐述。

OK，接着来看建立在objc_class和objc_object基础之上的一些关键字。

**Class**

Class（不是class哦），它在`<objc/objc.h>`中定义，如下：

```c
/// An opaque type that represents an Objective-C class.
typedef struct objc_class *Class;
```

可以看到Class其实是一种指针类型，即用于指向objc_class结构体。NSObject中定义的方法`- (Class)class;`用于返回其对应的objc_class结构体指针；

除了`- (Class)class;`方法之外，NSObject类中还有一些常见方法包含Class类型参数或者返回Class类型返回值，如下：

```objc
- (Class)class;
- (BOOL)isKindOfClass:(Class)aClass;
- (BOOL)isMemberOfClass:(Class)aClass;
+ (BOOL)isSubclassOfClass:(Class)aClass;
+ (BOOL)isSubclassOfClass:(Class)aClass;
+ (Class)superclass;
+ (Class)class;
```

**id**

对于有过iOS开发经验的人而言，id使用太广泛了，不过似乎对它还没有过比较深入的理解；事实上，一直以为id和`NSObject *`等价呢！现在看来，这种理解太naive了，它也在`<objc/objc.h>`中定义：

```c
struct objc_object {
    Class isa  OBJC_ISA_AVAILABILITY;
};
// objc_object也是一个只包含一个条目isa（指向objc_object结构体的指针）的结构体。
    
/// A pointer to an instance of a class.
typedef struct objc_object *id;
```

**SEL**

```c
/// An opaque type that represents a method selector.
typedef struct objc_selector *SEL;
```

没找到objc_selector的定义，但根据网友的描述：其实它就是个映射到「方法」的C字符串，可以用Objc编译器命令`@selector()`或者Runtime系统的`sel_registerName`函数来获得一个SEL类型的「方法选择器」（通常简称「选择子」）。

考虑到Xcode对`@selector`的支持比对`sel_registerName`的支持更好，所以`@selector`貌似用得更多一些，但有时候`sel_registerName`或许更简洁一些。

譬如有一段代码：

```objc
if ([self.selectedViewController respondsToSelector:@selector(isReadyForEditing)]) {
    boolNumber = [self.selectedViewController performSelector:@selector(isReadyForEditing)];
}
```

但当前上下文中没有isReadyForEditing这个方法，所以编译器会有警告；当然，可以有各种方式来关闭警告，如下是其中一种：

```objc
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundeclared-selector"
if ([self.selectedViewController respondsToSelector:@selector(isReadyForEditing)]) {
    boolNumber = [self.selectedViewController performSelector:@selector(isReadyForEditing)];
    }
#pragma clang diagnostic pop
```

但此时，若改用`sel_registerName`，则这个警告就没了！

P.S: 不晓得Apple更青睐于哪一个，个人感觉是`@selector`，因为`sel_registerName`的使用几乎没碰到过。

**IMP**

IMP的定义如下：

```c
/// A pointer to the function of a method implementation. 
#if !OBJC_OLD_DISPATCH_PROTOTYPES
typedef void (*IMP)(void /* id, SEL, ... */ );
#else
typedef id (*IMP)(id, SEL, ...);
#endif
```

可以了解到，IMP是一个函数指针。IMP是Implementation的缩写，一个函数是由一个selector(SEL)，和一个implement(IML)组成的；Selector相当于门牌号，而Implement才是真正的住户（函数实现）。理解Selector和Implementation的关系蛮重要的！

**Method**

```c
/// An opaque type that represents a method in a class definition.
typedef struct objc_method *Method;
```

**Ivar**

```c
/// An opaque type that represents an instance variable.
typedef struct objc_ivar *Ivar;
```

**Category**

```c
/// An opaque type that represents a category.
typedef struct objc_category *Category;
```

**objc_property_t**

```c
/// An opaque type that represents an Objective-C declared property.
typedef struct objc_property *objc_property_t;
```

可以通过class_copyPropertyList和protocol_copyPropertyList方法来获取类（Class）和协议（Protocol）中的属性，获取属性之后，还可以使用property_getName获取属性的名字（C字串）：

```objc
objc_property_t *class_copyPropertyList(Class cls, unsigned int *outCount);
objc_property_t *protocol_copyPropertyList(Protocol *proto, unsigned int *outCount);
const char *property_getName(objc_property_t property);
```

举个例子：

```objc
@interface Student : NSObject
    
@property (nonatomic, strong) NSString *name;
@property (nonatomic, assign) NSUInteger age;
@property (nonatomic, assign) NSUInteger score;
    
@end
    
@implementation Student
    
@end
    
@interface ViewController ()
    
@end
    
@implementation ViewController
    
- (void)viewDidLoad {
    [super viewDidLoad];
    unsigned int outCount;
    objc_property_t *aProperty = class_copyPropertyList([Student class], &outCount);
    
    NSLog(@"一共有%d个属性，它们的名字分别是：", outCount);
    for (int i = 0; i < outCount; ++i) {
        objc_property_t aP = aProperty[i];
        const char * property_name = property_getName(aP);
        NSLog(@"%s", property_name);
    }
}
@end

/*输出：
Student类一共有3个属性，它们的名字分别是：
name
age
score
*/
```

这个示例的执行结果也从侧面反映出了，NSObject中没有定义属性（只有一个叫isa成员变量）。

P.S: 属性 vs 成员变量？这里留个尾巴，以后把这个问题也分析一下吧！

## objc_class和isa

把objc_class和isa单独拧出来的原因是它们的信息量比较大，稍微复杂一点！

Objective-C是一种面向对象的语言。按照面向对象语言的设计原则，所有事物都应该是对象（严格来说Objective-C并没有完全做到这一点，因为它有象int, double这样的简单变量类型）。所以一定要有这个认识：**Objective-C中，类也是对象**。

在Cocoa中，所有类都继承自NSObject，参考NSObject在`<objc/NSObject.h>`中的定义如下：

```objc
@interface NSObject <NSObject> {
    Class isa  OBJC_ISA_AVAILABILITY;
}
```

objc_class的定义如下：

``` C
struct objc_class {
    Class isa  OBJC_ISA_AVAILABILITY;
    
#if !__OBJC2__
    Class super_class                                        OBJC2_UNAVAILABLE;
    const char *name                                         OBJC2_UNAVAILABLE;
    long version                                             OBJC2_UNAVAILABLE;
    long info                                                OBJC2_UNAVAILABLE;
    long instance_size                                       OBJC2_UNAVAILABLE;
    struct objc_ivar_list *ivars                             OBJC2_UNAVAILABLE;
    struct objc_method_list **methodLists                    OBJC2_UNAVAILABLE;
    struct objc_cache *cache                                 OBJC2_UNAVAILABLE;
    struct objc_protocol_list *protocols                     OBJC2_UNAVAILABLE;
#endif
    
}
```

可以知道在运行时，所有类对象都有一个名为「isa」的指针，这个指针指向一个objc_class结构体，objc_class结构体中包含一些与类相关的信息。问题是这个isa指向的objc_class结构体的信息所对应的是自己的信息还是对应的类的信息呢？当然是其所对应的类的信息，存储了变量列表、方法列表、遵守的协议列表等等。

简单来说，「isa」指针被称为「is a」指针，顾名思义，它告诉了对象所属的类信息，描述「aX is a X」。

某个对象的isa指针指向的objc_class结构体存放的是其所对应的**类**的「元数据」（metadata），通过它我们可以知道该对象：所对应类的名字（name）、可以做哪些事情（objc_method_list是个二维方法列表）、包括哪些对成员变量（objc_ivar_list）、遵守哪些协议（protocols）。

回到objc_class，可以看到objc_class结构体首变量也是一个isa指针，这也印证了「类也是对象」这个说法。对象的isa指针指向的是该对象的本类，而类的isa指针指向的另外一个类被称为「元类」（metaclass），用来表述类本身所表具备的元数据，类方法就定义于此；也正因为类本身也是一个对象，所以类本身可以接收消息。譬如：考虑到NSObject类本身也是一个对象（是metaclass的一个对象，常称之为「类类型对象」），所以`[NSObject alloc]`其实可以看成是对NSObject这个类类型对象发送一个消息（调用器alloc实例方法）。

除了isa指针之外，objc_class结构体中还有一个变量super_class，它指向了是这个类的超类（super class），可以看到这个super_class不是一个一位数组，而是一个单独的指针，即一个类有且仅有一个super class，即所谓的「单继承」。

关于isa和super_class的更直观描述，还请看图：
![class-diagram.jpg](/img/201504/class-diagram.png)

到了这里，Objective-C的对象模型基本上解释清楚了；可能还有一个问题：最终的元类是啥？在Cocoa中，所有类都继承自NSObject，而NSObject的元类（不晓得叫什么名字，假设叫NSObjectMetaClass）（上图中右上角的方框）也继承自NSObject。有些绕，具体来说，NSObject和NSObjectMetaClass的isa指针和super_class指针指向情况是这样的：

* NSObject的isa指针NSObjectMetaClass（终极meta class），NSObjectMetaClass指针指向自己；
* NSObject的super_class指针指向nil，NSObjectMetaClass的super_class指向NSObject；

这样就完了？！No！Objective-C的对象模型还有信息可挖，回过头来看objc_class的ivars变量和methodLists变量：

```c
struct objc_class {
    struct objc_ivar_list *ivars;
    struct objc_method_list **methodLists;
}
```

在运行时，类（包括class和metaclass）的objc_class结构体是固定的，不可能往这个结构体中添加数据，只能修改！譬如可以修改isa指针，让它指向一个中间类；在我的理解里，应该也可以修改ivars和methodLists，让它们指向一个新的区域；若可以这样，那么就可以在运行时随意添加/修改/删除成员变量和方法了。

但是，貌似Objective-C Runtime没有提供修改ivars和methodLists指针值的接口。

也因此，ivars在运行时指向的是一个固定区域，当然可以修改这个区域的值了，但这其实只是修改成员变量值而已；「在这个内存区域后面续上一段空余区域用于存放新的成员变量」？呵呵，想多了吧！因此，我们没办法在运行时为对象添加成员变量，这解释了为什么category中不能定义property（dynamic property不算）；

P.S: 那为什么protocol中可以添加变量，在我的理解里，protocol是在编译器处理的。所以objc_class中有一个变量叫protocols；

和ivars不同，methodLists是一个二维数组。虽然我们没办法扩展methodLists指向的内存区域，但是我们可以改变这个内存区域（这个内存区域存储的都是指针）的值。因此，我们可以在运行时动态添加（以及做其他的处理，譬如交换等）方法！

P.S: objc_class结构体中还有一个变量cache，顾名思义，它是用于缓存的，缓存啥呢？缓存方法，下一篇博客阐述「消息传递机制」时会谈到这个。

到了这里，谁还敢说Objective-C不是动态语言？