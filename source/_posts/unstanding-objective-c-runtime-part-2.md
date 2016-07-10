title: 理解Objective-C Runtime（二）消息传递机制
date: 2015-04-27 11:16:36
tags: 
- Runtime
- Objective-C
categories: iOS

---

在对象上调用方法是包括Objective-C的众多语言都具备的功能。但在Objective-C中，这个术语叫「传递消息」（pass a message）。「消息」有可以接受参数，也可能有返回值。

## 静态绑定和动态绑定

由于Objective-C是C的超集，所以最好先理解C语言的函数调用方式。C语言使用「静态绑定」（static binding），也就是说，在编译期间就能决定运行时所应调用的函数。以下代码为例：

```c
#import <stdio.h>
    
void printHello() {
    printf("Hello, world!\n");
}
    
void printGoodbye() {
    printf("Goodbye!\n");
}
    
void doTheThing(int type) {
    if (type == 0) {
        printHello();
    } else {
        printGoodbye();
    }
    return 0;
}
```

若不考虑「内联」（inline），那么编译器在编译代码时就已经知道程序中有printHello与printGoodbye这两个函数了，于是会直接生成调用这些函数的指令（站在汇编的角度，call命令）。而函数地址实际上是与硬编码在指令之中的。

如果将上述代码写成下面这样，会如何呢？

```c
#import <stdio.h>
    
void printHello() {
    printf("Hello, world!\n");
}
    
void printGoodbye() {
    printf("Goodbye!\n");
}
    
void doTheThing(int type) {
    void (*fnc)();
    if (type == 0) {
        fnc = printHello;
    } else {
        fuc = printGoodbye;
    }
    fnc();
    return 0;
}
```

这就是「动态绑定」（dynamic binding）！因为所要调用的函数知道运行期才能确定。编译器在这种情况下生成的指令与刚才的那个例子不同，在第一个例子（静态绑定）中，if与else语句里都有函数调用指令（汇编中的call命令）；而在第二个例子（动态绑定）中，只有一个函数调用指令，不过待调用的函数地址无法硬编码之中，而是要在运行期读出来。

## 消息传递机制

在OC中，如果向某对象传递消息，那就会使用**动态绑定机制**来决定需要调用的方法。在底层，所有方法都是普通的C语言函数，然而对象收到消息之后，究竟该调用哪个方法则完全在runtime决定，甚至可以在程序运行时改变，这些特性使得**Objective-C成为一门真正的动态语言**。

给对象发生消息可以这样写：
`id returnValue = [someObject messageName:parameter];`

someObject是「接收者」（receiver），messageName叫做「选择子」（selector）。二者合起来称为「消息」（message）。

P.S: 「选择子」和「方法」这两个名词经常交替使用，都是一个意思。

编译器看到此消息后，将其转换为一条标准的C语言函数调用，所调用的函数乃是「消息传递机制」中的核心函数，即鼎鼎有名的`objc_msgSend`，其原型可以在`<objc/message.h>`中找到：

```c
id objc_msgSend(id self, SEL op, ...)
```

`id returnValue = [someObject messageName:parameter];`会被编译器翻译成如下形式：

```objc
id returnValue = objc_msgSend(someObject, @selector(messageName:), parameter);
```

在runtime，`objc_msgSend`会依据receiver（即上述对象someObject）和选择子类型来调用适当的方法。为了完成此操作，该方法需要在接收者所属的类中搜寻其「方法列表」（关于方法列表，详见[methodLists](http://localhost:4000/2015/04/26/unstanding-the-Objective-C-Runtime-part1/#objc_class和isa)），如果能找到与「选择子」匹配的方法，就调至其实现的代码。若找不到，那就沿着继承体系继续向上查找，等找到合适的方法之后再跳转。**如果最终还找不到相匹配的方法，那就执行「消息转发」（message forwarding）操作**，这会在下一篇[博客](/2015/04/27/unstanding-the-Objective-C-Runtime-part3/)中阐述。

如此看来，Objective-C在runtime调用一个方法似乎需要很多步骤。所幸的是，`objc_msgSend`会将匹配结果缓存在「快速映射表」（fast map）里面，每个类都有这样一块缓存，若是稍后还想该类发送与「选择子」相同的方法，那么查找起来就很快了。

P.S: 上一篇博客《[理解Objective-C Runtime（一）预备知识](/unstanding-objective-c-runtime-part-1/)》中在介绍objc_class结构体时谈到了其中一个变量cache，但简单忽略飘过；其实，根据我的理解，这里的「快速映射表」所对应的就是objc_class中的变量cache。

诚然，即便有这种「快速映射表」机制，执行速度还是不如「静态绑定」。实际上，对于当前这种硬件平台，这点速度差根本不值一提。

在`<objc/message.h>`中，除了`objc_msgSend`函数原型之外，还可以看到其他的objc_msgSendXXOO函数，这些方法的详细作用，《Effective Objective-C 2.0》item 11中有详细说明，本文就不赘述了。


只要理解了Objective-C的对象模型，理解「消息传递机制」还是非常容易的。

## 本文参考资料

* 《Effective Objective-C 2.0》