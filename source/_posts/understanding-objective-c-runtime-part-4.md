title: 理解Objective-C Runtime（四）Method Swizzling
date: 2015-04-27 21:10:53
tags: 
- Runtime
- Objective-C
categories: iOS

---

Objective-C对象收到消息之后，究竟会调用何种方法需要在运行期间才能解析出来。那你也许会问：与给定的选择子名称相应的方法是不是也可以在runtime改变呢？没错，就是这样。**若能善用此特性，则可发挥出巨大优势，因为我们既不需要源代码，也不需要通过继承子类来覆写方法就能改变这个类本身的功能**。这样一来，新功能将在本类的所有实例中生效，而不仅限于覆写了相关方法的那些子类实例。此方案就是大名鼎鼎的**method swizzling**，中文常称之为「方法调配」或「方法调和」或「方法混合」。


## Method Swizzling

**类的方法列表会把选择子的名称映射到相关的方法实现之上**，使得**动态消息派发系统**（dynamic message-dispatch system）能够据此找到应该调和的方法。这些方法均以函数指针的形式来表示，这种指针叫IMP（IMP在《[理解Objective-C Runtime（一）预备知识](/understanding-objective-c-runtime-part-1/)》已有说明）。

举个栗子，`NSString`类可以响应`lowercaseString`、`uppercaseString`、`capitalizedString`等选择子。这张映射表（selector table，也常称为**选择器表**）中的每个选择子都映射到不同的IMP之上，如下图所示：

<div class="imagediv" style="width: 500px; height: 200px">{% asset_img QQ20150428-1.png %}</div>

Objective-C runtime系统提供的几个方法都能够用来操作这张表。开发者可以向其中新增selector，也可以改变某个selector所对应的方法实现，还可以交换两个selector所映射到的指针。经过几次操作之后，类的方法就会变成如下图所示：

<div class="imagediv" style="width: 500px; height: 250px">{% asset_img QQ20150428-2.png %}</div>

在新的映射表中，多了一个名为newSelector的选择子，lowercaseString和uppercaseString的实现则互换了。上述修改均无需编写子类，只要修改**方法表**的布局即可，就会反映到程序中所有的NSString实例之上。

## 交换两个方法的实现

现在通过示例代码演绎「调换`NSString`的`lowercaseString`和`uppercaseString`的方法实现」，具体实现操作是这样的：

```objc
- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    NSString *aString = @"AbcDEfg";
    
    // lowercaseString和uppercaseString交换前：
    NSLog(@"lowercaseString和uppercaseString交换前：");
    NSLog(@"lowercase of the string : %@", [aString lowercaseString]);
    NSLog(@"uppercase of the string : %@", [aString uppercaseString]);
    
    // class_getInstanceMethod方法得到Method类型
    Method originalMethod = class_getInstanceMethod([NSString class], @selector(lowercaseString));
    Method swappedMethod = class_getInstanceMethod([NSString class], @selector(uppercaseString));
    
    // method_exchangeImplementations交换映射指针
    method_exchangeImplementations(originalMethod, swappedMethod);
    
    // lowercaseString和uppercaseString交换后：
    NSLog(@"lowercaseString和uppercaseString交换后：");
    NSLog(@"lowercase of the string : %@", [aString lowercaseString]);
    NSLog(@"uppercase of the string : %@", [aString uppercaseString]);
}
    
/* 输出结果：
lowercaseString和uppercaseString交换前：
lowercase of the string : abcdefg
uppercase of the string : ABCDEFG
lowercaseString和uppercaseString交换后：
lowercase of the string : ABCDEFG
uppercase of the string : abcdefg
*/
```

这演示了如何交换两个方法的实现，然而在实际应用中，像这样直接交换两个方法实现，其意义不大，除非闲得蛋疼。但是，可以通过这一手段来为既有的方法实现增添新功能。

## 修改既有方法的行为

介绍一个技巧，最好的方式就是提出具体的需求，然后用它跟其他的解决方法做比较。

所以，先来看看我们的需求：对 App 的用户行为进行追踪和分析。简单说，就是当用户看到某个View或者点击某个Button的时候，就把这个事件记下来。

**手动添加**

```objc
@implementation MyViewController ()
    
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    // Custom code 
    
    // Logging
    [Logging logWithEventName:@"my view did appear"];
}
    
- (void)myButtonClicked:(id)sender
{
    // Custom code 
    
    // Logging
    [Logging logWithEventName:@"my button clicked"];
}
```

这种方式的缺点也很明显：它破坏了代码的干净整洁。因为Logging的代码本身并不属于View Controller里的主要逻辑。随着项目扩大、代码量增加，你的View Controller里会到处散布着Logging的代码。这时，要找到一段事件记录的代码会变得困难，也很容易忘记添加事件记录的代码。

你可能会想到用继承或类别，在重写的方法里添加事件记录的代码。代码可以是长的这个样子：

```objc
- (void)myViewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    // Custom code 
    
    // Logging
    [Logging logWithEventName:NSStringFromClass([self class])];
}
    
- (void)myButtonClicked:(id)sender
{
    // Custom code 
    
    // Logging
    NSString *name = [NSString stringWithFormat:@“my button in %@ is clicked”, NSStringFromClass([self class])];
    [Logging logWithEventName:name];
}
```
Logging 的代码都很相似，通过继承或类别重写相关方法是可以把它从主要逻辑中剥离出来。但同时也带来新的问题：
1. 你需要继承`UIViewController`，`UITableViewController`，`UICollectionViewController`所有这些View Controller，或者给他们添加类别；
2. 每个View Controller里的ButtonClick方法命名不可能都一样；
3. 你不能控制别人如何去实例化你的子类；
4. 对于类别，你没办法调用到原来的方法实现，大多时候，我们重写一个方法只是为了添加一些代码，而不是完全取代它；
5. 如果有两个类别都实现了相同的方法，运行时没法保证哪一个类别的方法会给调用。

**Method Swizzling的做法**

Method Swizzling的做法是新增一个方法`log_viewDidAppear:`，在这个方法体中调用`viewDidAppear:`的方法体；然后将`log_viewDidAppear:`和`viewDidAppear:`进行调换。呃，有些绕，看图吧：

<div class="imagediv" style="width: 505px; height: 154px">{% asset_img QQ20150428-3.png 交换log_viewDidAppear:和viewDidAppear:的实现 %}</div>

新增方法`log_viewDidAppear:`的实现代码可以这样写：

```objc
- (void)log_viewDidAppear:(BOOL)animated
{
    [self log_viewDidAppear:animated];
        
    // Logging
    [Logging logWithEventName:NSStringFromClass([self class])];
}
```

看起来，这段代码好像会陷入递归使用的死循环，不过要记住，此方法是准备和`viewDidAppear:`方法互换的。所以，在runtime，`log_viewDidAppear:`选择子对应的是原来`viewDidAppear:`方法的实现；同样，当向对象发送`viewDidAppear:`消息时，如上这段代码会被调用，而这段代码的第一句是`[self log_viewDidAppear:animated];`，这其实是调用原来`viewDidAppear:`方法的实现代码...

定义了`log_viewDidAppear:`的实现后，还得与`viewDidAppear:`进行交换：

```objc
// class_getInstanceMethod方法得到Method类型
Method originalMethod = class_getInstanceMethod([NSString class], @selector(viewDidAppear:));
Method swappedMethod = class_getInstanceMethod([NSString class], @selector(log_viewDidAppear:));
    
// method_exchangeImplementations交换映射指针
method_exchangeImplementations(originalMethod, swappedMethod);
```

如何安排method swizzling相关的代码？

一般来说，runtime相关的代码都会以category的形式组织，所以上述`log_viewDidAppear:`方法的实现会写在一个UIViewController category中，比如`UIViewController+log.h`。而**交换方法**相关的代码会写在category的load中。因为load方法是在runtime之前就被执行的，只要category所在的头文件被引用，load方法就会被调用，并且同一个class在不同category之间允许有多个load方法，这些load方法都会被调用（唯一的问题是谁先谁后）。

通过method swizzling方案，开发者可以为那些完全不知道具体实现的（completely opaque，完全不透明）黑盒方法增加日志记录功能，这非常有助于程序调试，然而，此做法只在调试程序时有用。很少有人在调试程序之外的场合用上述**方法调配技术**来永久改变某个类的功能，因为如果使用不慎，它造成的破坏太大了，并且很难Debug。不能仅仅因为Objective-C语言里有这个特性就一定要用它。若是滥用，反而会令代码变得不易读懂且难于维护。

总之，Method Swizzling只一个挺有争议的技术，对此有很多分析的文章，底部的参考资料中有链接。

**补充**

后来终于有机会在实际项目中使用到method swizzling。应用场景是这样的，接手了一个完整的项目，我的任务是在该项目基础上添加一些功能，顺便将项目整理一下，尽可能清理没有用的内容和过时的技术。项目页面非常多，各种文件的命名非常糟糕，我首先需要做的事情是将页面逻辑给整理出来（各种View Controller之间的逻辑关系），简单来说，我需要结合所看到的运行页面（譬如首页），将它的View Controller类给找出来。

比较蠢的做法当然是去查看代码了。好在我比较机灵，决定使用method swizzling技术，让每个页面将它的View Controller类名自己喊出来。

我的思路：定义一个UIViewController category，添加一个方法，该方法调用`viewDidAppear:`，并且将该类的名字给打印出来，然后将该方法的SEL和`viewDidAppear:`方法的SEL调换，这样系统在回调`viewDidAppear:`时会定义该方法代码，如下：

```objc
@implementation UIViewController (sayHello)
 
+ (void)load {
    SEL originalSelector = @selector(viewDidAppear:);
    SEL swizzledSelector = @selector(swizzled_viewDidAppear:);
    
    // class_getInstanceMethod方法得到Method类型
    Method originalMethod = class_getInstanceMethod([self class], originalSelector);
    Method swizzledMethod = class_getInstanceMethod([self class], swizzledSelector);
 
    // method_exchangeImplementations交换映射指针
    method_exchangeImplementations(originalMethod, swizzledMethod);
}
 
- (void)swizzled_viewDidAppear:(BOOL)animated {
    [self swizzled_viewDidAppear:animated];
    
    NSLog(@"hello, my name is %@", NSStringFromClass([self class]));
}
 
@end
```

## AOP(Aspect Oriented Programming)

在阅读博客《[Method Swizzling和AOP实践](http://tech.glowing.com/cn/method-swizzling-aop/)》时了解到了一个新概念 -- `AOP`。

简单来说，在Objective-C世界中，AOP就是利用Runtime特性给指定的方法添加自定义代码，Method Swizzling是其中一种实现AOP的方式之一。

Mark一下，暂不多讲。

## 本文参考

* 《Effective Objective-C 2.0》
* 《iOS开发进阶》
*  《[Method Swizzling和AOP实践](http://tech.glowing.com/cn/method-swizzling-aop/)》
*  大神Mattt Thompson（AFNetworking作者）的《[Method Swizzling](http://nshipster.com/method-swizzling/)》
*  《[Objective-C的hook方案（一）: Method Swizzling](http://blog.csdn.net/yiyaaixuexi/article/details/9374411)》
*  [What are the Dangers of Method Swizzling in Objective C?](http://stackoverflow.com/questions/5339276/what-are-the-dangers-of-method-swizzling-in-objective-c)；
*  《[Objective-C Runtime](http://yulingtianxia.com/blog/2014/11/05/objective-c-runtime/#Method_Swizzling)》