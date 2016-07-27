title: 理解Objective-C Runtime（三）消息转发机制
date: 2015-04-27 15:41:47
tags: 
- Runtime
- Objective-C
categories: iOS

---

## 消息转发机制概述

上一篇博客《[消息传递机制](/understanding-objective-c-runtime-part-2/)》中讲解了Objective-C中对象的消息传递机制。本文需要讲解另外一个重要问题：**当对象收到无法处理的消息会发生什么**？

显然，若想让类能理解某条消息，我们必须以程序代码实现出对应的方法才行。但是，在编译期向类发送了其无法理解解读的消息并不会报错，因为在运行期间允许继续向类中添加方法，所以，编译器在编译期间还无法确知类中到底会不会有某个方法的实现。当对象接收到无法理解的消息后，就会启动**消息转发**（message forwarding）机制，用户（程序员）可经此过程告诉对象应该如何处理未知消息。

向对象发送它无法理解的后果在实际开发中我们会经常遇到，如下：

```objc
- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    NSObject *aObject = [[NSObject alloc] init];
    NSLog(@"%@", [(NSString *)aObject lowercaseString]);
}
```

显然，这几行代码可以通过编译，但是在运行时会出现如下错误，并导致崩溃：

```
-[NSObject lowercaseString]: unrecognized selector sent to instance 0x7a8acba0
*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '-[NSObject lowercaseString]: unrecognized selector sent to instance 0x7a8acba0'
*** First throw call stack:
(
    0   CoreFoundation                      0x008f4746 __exceptionPreprocess + 182
    1   libobjc.A.dylib                     0x0057da97 objc_exception_throw + 44
    2   CoreFoundation                      0x008fc705 -[NSObject(NSObject) doesNotRecognizeSelector:] + 277
    3   CoreFoundation                      0x00843287 ___forwarding___ + 1047
    4   CoreFoundation                      0x00842e4e _CF_forwarding_prep_0 + 14
...
...
```

上面这段异常信息是由`NSObject`的`doesNotRecognizeSelector:`方法抛出的，此异常表明：消息的接收者的类型是`NSObject`（即receiver是`NSObject`类型对象），而该接收者无法理解名为`lowercaseString`的选择子。

在本例中，消息转发过程以应用程序崩溃告终，不过，开发者在编写自己的类时，可于转发过程中设置挂钩，用于执行预定的逻辑，而不使得应用程序崩溃。

当对象接收到未知的选择子（unknown selector）时，开启消息转发，这分为两大阶段：

* 第一阶段先与接收者所属的类打交道，看其是否能动态添加方法，以处理当前这个未知的选择子，这叫**动态方法解析**（dynamic method resolution）；
* 第二阶段涉及**完整的消息转发机制**（full forwarding mechanism）。如果runtime系统已经把第一阶段执行完了，那receiver自己就无法再以动态新增方法的手段来响应包含该选择子的消息了。此时，运行期系统会请求receiver用其他手段来处理这条消息相关的方法调用了。这又细分为为两小步：
    * 请receiver看看有没有其他对象能处理这条消息，若有，则runtime系统会把消息转发给那个对象，消息转发结束；
    * 若有没有**备援的接收者**（replacement receiver），则启动**完整的消息转发机制**，runtime系统会把与消息有关的全部细节封装到NSInvocation对象中，再给接收者最后一次机会，令其设法解决当前还未处理的这条消息。

## 动态方法解析

Receiver在收到unknown selector后，首先将调用其本类的`resolveInstanceMethod:`方法，该方法定义如下：

```objc
+ (BOOL)resolveInstanceMethod:(SEL)sel;
```

该方法的参数就是那个unknown selector，其返回值为`Boolean`类型，表示这个类是否能新增一个实例方法用以处理该unknown selector。在继续往下执行转发机制之前，本类有机会新增一个处理此selector的方法。所以`resolveInstanceMethod:`的一般使用套路是：

```objc
+ (BOOL)resolveInstanceMethod:(SEL)aSelector {
    if (/* aSelector满足某个条件  */) {
        /*
         调用class_addMethod为该类添加一个处理aSelector的方法，譬如：
         class_addMethod(self, aSelector, aImp, @"v@:@");
         */
        return YES;
    }
    return [super resolveInstanceMethod:aSelector];
}
```

假如尚未实现的方法不是实例方法而是类方法，那么runtime系统会调用另外一个与`resolveInstanceMethod:`类似的方法`resolveClassMethod:`，其原型如下：

```objc
+ (BOOL)resolveClassMethod:(SEL)sel;
```
举个栗子：

```objc
@interface Student : NSObject
    
@property (nonatomic, strong, getter=name, setter=setName:) NSString *name;
    
@end
    
@implementation Student
    
@dynamic name;
    
// 注意，这是C语言函数（不是Objective-C方法）
id name(id self, SEL _cmd) {
    return @"张不坏";
}
    
// 注意，这是C语言函数（不是Objective-C方法）  
void setName(id self, SEL _cmd, id value) {
    NSLog(@"do nothing");
}
    
+ (BOOL)resolveInstanceMethod:(SEL)sel {
    
    if (sel == @selector(name)) {
        class_addMethod(self, sel, (IMP)name, "@@:");        // 添加getter
        return YES;
    } else if (sel == @selector(setName:)) {
        class_addMethod(self, sel, (IMP)setName, "v@:@:");   // 添加setter
        return YES;
    }
    return [super resolveInstanceMethod:sel];
}
```

`@@:`和`v@:@:`用来描述函数参数和返回值，更多内容参考《[Apple: Type Encodings](https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtTypeEncodings.html)》和《[NSHipster: Type Encodings](http://nshipster.com/type-encodings/)》。

执行效果：

```objc
Student *jason = [[Student alloc] init];
jason.name = @"Jason";
NSLog(@"name of this student：%@", jason.name);
    
/* 输出：
do nothing
name of this student：张不坏
*/
```

## 备援接收者

当前receiver还有第二次机会能处理unknown selector，在这一步中，runtime系统会问它：可否把这条消息转给其他对象处理？该步骤对应的处理方法是`forwardingTargetForSelector:    `，定义于`<objc/NSObject.h>`中：

```objc
- (id)forwardingTargetForSelector:(SEL)aSelector;
```

方法参数代表unknown selector，若当前receiver能找到备援对象，则将其返回，若找不到，则返回`nil`。

通过此方案，我们可以通过组合（composition）来模拟出多继承（multiple inheritance）的某些特性。

所谓**多继承**指一个类可以继承自多个类，即该类对象具有多个类的属性和方法，譬如A继承自B和C，则A的类对象a同时具有了B和C的方法和属性。

而Objective-C是不支持多继承了。因此A只能继承自B，而不能同时也继承C。若想让A也具备C的方法，基于**消息转发机制**的实现过程是这样的：在A中定义一个C对象（假设为c），当向A对象发送C才能处理的消息时，在A的`-(id)forwardingTargetForSelector:`方法实现中返回c（C对象）即可。如此这般，在外界看来，就感觉A能够处理这些C中定义的方法。

为了更好的阐述「通过转发模拟多继承」，以下图举个例子：

<div class="imagediv" style="width: 321px; height: 206px">{% asset_img QQ20150427-2.png %}</div>

在上图中，Warrior（武士）和Diplomat（外交官）没有继承关系，所以它自然不能处理Diplomat才能做的事情negotiate（谈判）。但是，通过「消息转发」，可以让Warrior也能够接受negotiate消息。具体做法是在Warrior中定义一个Diplomat对象（内部变量，假设名为aDiplomat），当Warrior对象接收到negotiate消息时，就转发给aDiplomat。这让人感觉武士（Warrior）也兼具谈判（negotiate）能力。

## 完整的消息转发机制

如果转发已经到了这一步的话，那么唯一能够做的就是启用**完整的消息转发机制**了。首先创建`NSInvocation`对象，将未知消息相关的全部细节都封装于其中。此对象包含选择子、目标（target）以及参数。在触发`NSInvocation`对象时，消息派发系统（message-dispatch system）将亲自出马，把消息派给目标对象。

此步骤会调用`forwardInvocation:`方法来转发消息，该方法定义于`<objc/NSObject.h>`中：

```objc
- (void)forwardInvocation:(NSInvocation *)anInvocation;
```

这个方法可以实现得很简单：只要改变调用目标，是消息在新目标上得以调用即可。然而这样实现出来的方法与「备援接收者」方案所实现的方法等效，所以很少有人采用这种实现方式。比较有用的实现方式为：在触发消息前，先以某种方式改变消息内容，比如追加另外一个参数，或是改换选择子，等等。

实现此方法时，若发现某调用方法不应由本类处理，则需调用超类的同名方法。这样的话，集成体系中的每个类都有机会处理此调用请求，直至NSObject。如果最后调用了NSObject类的方法，那么该方法还有继而调用`doesNotRecognizeSelector:`以抛出异常，此异常表明选择子最终未能得到处理。

## 消息转发全流程

下图是消息转发全流程图，描述了**消息转发机制**的各个步骤。

<div class="imagediv" style="width: 464px; height: 220px">{% asset_img QQ20150427-1.png %}</div>

Receiver在每一步中均有机会处理消息。步骤越往后，处理消息的代价就越大；最好能在第一步就处理完，这样的话，runtime系统就可以将此方法缓存起来，进而提高效率。若想在第三步里把消息转发给备援的receiver，那还不如把转发操作提前到第二步。因为第三步只是修改了调用目标，这项改动放在第二步会更为简单，不然的话，还得创建并处理完整的`NSInvocation`。