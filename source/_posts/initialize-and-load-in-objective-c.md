title: Objective-C中的+initialize和+load
date: 2015-04-28 14:14:13
tags: Objective-C
categories: iOS

---

## 写在前面

近几天花了一些时间了解了一下Objective-C runtime相关的东西，其中涉及到了`+load`方法，譬如method swizzling通常在category的`+load`方法中完成。之前对initializer和load的使用就比较疑惑，但一直没有详细去对比了解，以此为契机，集各方资源，分析一下吧！

关于了解`+initialize`和`+load`，个人感觉参考官方文档《NSObject Class Reference》就够了。

## +initialize

关于`+initialize`方法，《NSObject Class Reference》的介绍如下：

>Initializes the class before it receives its first message.

可以理解`+initialize`的作用是为了该Class在使用前创建合适的环境；

关于其使用，《NSObject Class Reference》的说明如下：

>The runtime sends initialize to each class in a program just before the class, or any class that inherits from it, is sent its first message from within the program. The runtime sends the initialize message to classes in a thread-safe manner. Superclasses receive this message before their subclasses. The superclass implementation may be called multiple times if subclasses do not implement initialize—the runtime will call the inherited implementation—or if subclasses explicitly call [super initialize].

这上面这段话，可以得出如下这么一些意思：

* `+initialize`方法是在runtime被调用的；
* 对于某个类，其类`+initialize`方法都会在该对象接受任何消息之前被调用；
* 如果父类和子类的`+initialize`方法都被调用，父类的调用一定在子类之前，这是系统自动完成的，子类`+initialize`中没必要显式调用`[super initialize];`；
* runtime系统处理`+initialize`消息的方式是线程安全的，所以没必要在`+initialize`中为了保证线程安全而使用lock、mutex之类的线程安全工具；
* 某个类的`+initialize`的方法不一定只被调用一次，至少有两种情况会被调用多次：
    * 子类显式调用`[super initialize];`；
    * 子类没有实现`+initialize`方法；

下面以示例演示某个类的`+initialize`被多次执行的现象。

定义三个类：`Person`、`Student`、`Teacher`，`Student`和`Teacher`继承自`Person`，`Person`继承自`NSObject`。`Person`和`Student`都实现了`+initialize`方法，`Teacher`没有实现该方法，如下：

```objc
// Person的+initialize方法的实现
+ (void)initialize {
    NSLog(@"Person initialize");
}
    
// Student的+initialize方法的实现
+ (void)initialize {
    NSLog(@"Student initialize");
}
```

执行效果如下：

```objc
- (void)viewDidLoad {
    Student *aStudent = [[Student alloc] init];
    Teacher *aTeacher = [[Teacher alloc] init];
    
    [super viewDidLoad];
}
    
/* 输出：
Person initialize
Student initialize
Person initialize
*/
```

可以看到，对于`Student`，在其`+initialize`方法被调用之前，其super class（`Person`）的`+initialize`方法被率先调用；对于`Teacher`，没有定义`+initialize`方法，所以它会直接调用super class（Person）的`+initialize`方法，这就导致了Person的`+initialize`方法被执行两次。

有没有办法避免`Person`的`+initialize`方法被多次调用？当然可以：

```objc
// Person的+initialize方法的实现
+ (void)initialize {
    static BOOL b = false;
    if (!b) {
        NSLog(@"Person initialize");
        b = true;
    }
}
```

也可以这样：

```objc
// Person的+initialize方法的实现
+ (void)initialize {
    if (self == [Person class]) {
        NSLog(@"Person initialize");
    }
}
```

《NSObject Class Reference》中还对`+initialize`方法的使用做了一些警告：
>Because initialize is called in a thread-safe manner and the order of initialize being called on different classes is not guaranteed, it’s important to do the minimum amount of work necessary in initialize methods. Specifically, any code that takes locks that might be required by other classes in their initialize methods is liable to lead to deadlocks. Therefore you should not rely on initialize for complex initialization, and should instead limit it to straightforward, class local initialization.

总结一下，就是这样：不要在`+initialize`中处理复杂的逻辑！

那么`+initialize`可以做些什么事情呢？可以做一些简单的初始化工作，譬如对于某个继承自`UICollectionViewCell`的自定义类`PhotoViewCell`，`PhotoViewCell`的对象可能会有一些公用资源，譬如label color，label font等等，没必要在`-initXXOO`方法中创建这些完全一样的资源，此时就可以放在`PhotoViewCell`中的`+initialize`中完成，如下：

```objc
+ (void)initialize {
    titleFont       = [UIFont systemFontOfSize:12];
    titleHeight     = 20.0f;
    videoIcon       = [UIImage imageNamed:@"CTAssetsPickerVideo"];
    titleColor      = [UIColor whiteColor];
    checkedIcon     = [UIImage imageNamed:@"CTAssetsPickerChecked"];
    selectedColor   = [UIColor colorWithWhite:1 alpha:0.3];
}
```

`+initialize`终究还是带来惊人的信息量，颇为失望。

## +load

关于`+load`方法，《NSObject Class Reference》的介绍如下：
>Invoked whenever a class or category is added to the Objective-C runtime; implement this method to perform class-specific behavior upon loading.

关于其使用，《NSObject Class Reference》的说明如下：

>The load message is sent to classes and categories that are both dynamically loaded and statically linked, but only if the newly loaded class or category implements a method that can respond.

>The order of initialization is as follows:
>>1. All initializers in any framework you link to.
>>2. All +load methods in your image.
>>3. All C++ static initializers and C/C++ __attribute__(constructor) functions in your image.
>>4. All initializers in frameworks that link to you.

>In addition:
* A class’s +load method is called after all of its superclasses’ +load methods.
* A category +load method is called after the class’s own +load method.
In a custom implementation of load you can therefore safely message other unrelated classes from the same image, but any load methods implemented by those classes may not have run yet.

从这段文字可以读出如下信息：

* 在一个程序（main函数）运行之前，所用到的库被加载到runtime之后，被添加到的runtime系统的各种类和category的`+load`方法就被调用；（关于这点很容易通过打印语句来验证）；
* 如果父类和子类的`+load`方法都被调用，父类的调用一定在子类之前，这是系统自动完成的，子类`+load`中没必要显式调用`[super load];`；
* 文档没有讲明`+load`的执行是否是线程安全的，但考虑到它是在runtime之前就调用，所以谈论它是否是线程安全没啥必要，根据我的理解，多线程在runtime才有谈论意义；
* 若某个类由一个主类和多个category组成，则允许主类和category中各自有自己的`+load`方法，只是category中的`+load`的执行在主类的`+load`之后；

关于`+load`的使用场景，笔者知道的至少有一个，method swizzling的处理一般都在category的`+load`中完成的，参考[这里](/unstanding-objective-c-runtime-part-4/)。

## 本文参考

* 《NSObject Class Reference》