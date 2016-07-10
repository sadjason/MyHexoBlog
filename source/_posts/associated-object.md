title: 关联对象
date: 2015-04-29 14:47:10
categories: Objective-C

---

## 关联对象概述

Objective—C作为一门动态语言，它本身有一个非常大的弱点，即不能在类的category中方便地为类添加新的自定义属性。「关联对象」（Associated Objects）或曰「关联引用」（Associative References）是基于Objective-C 2.0的一个Runtime特性，它使得可以在Runtime为某个类对象绑定一个对象（通过简单的封装，可以让这个关联属性在使用上和普通属性有类似的体验）。

P.S：也可以绑定一个block。

与此相关有3个函数，都在`<objc/runtime.h>`中定义：

```objc
// 绑定对象，类似setter
objc_setAssociatedObject(id object, const void *key, id value, objc_AssociationPolicy policy);
// 获取对象，类似于getter
id objc_getAssociatedObject(id object, const void *key);
// 移除绑定关系
objc_removeAssociatedObjects(id object);
```

假设现在需要往对象A上绑定一个对象B（即B作为A的「属性」），本文称A为「宿主对象」，B为「寄生对象」，这个比喻可能不是很好，为了更简单地表述，先就这么将就着吧。

下面来对这三个函数进行简单的介绍！

上述3个函数的第一个参数都是id类型，都是指「宿主对象」；

objc_setAssociatedObject函数和objc_getAssociatedObject函数的第二个参数是一个整型值，笔者曾一度认为这个参数应该是一个int型指针，用来存放新创建的对象地址值（现在想来这个想法太傻逼了），Objective-C只是要求这个值是Runtime时唯一标识值即可，在实际应用中，这个值常常是static char型变量的指针，譬如这样：

```objc
static char kAssociatedObjectKey;
objc_getAssociatedObject(self, &kAssociatedObjectKey);
```
但是Mattt Thompson的作者似乎更喜欢这样使用`@selector()`生成一个指针值，`@selector(XXOO)`中的参数XXOO可以随便填，但必须得保证唯一性，且objc_setAssociatedObject和objc_getAssociatedObject中必须保持一致。

objc_setAssociatedObject的第三个参数即上文所述的「寄生对象」；

objc_setAssociatedObject的第四个参数policy描述的是「宿主对象」对「寄生对象」的
持有策略，有OBJC_ASSOCIATION_ASSIGN、OBJC_ASSOCIATION_RETAIN_NONATOMIC等几个值，这些值分别对应property修饰符assign、strong等。

对于objc_removeAssociatedObjects，注意后缀`s`，关于其说明，文档的描述是：Removes all associations for a given object. 但似乎不能直接使用它来remove关联的「寄生对象」们，文档是这么说的：
>The main purpose of this function is to make it easy to return an object to a "pristine state”. You should not use this function for general removal of associations from objects, since it also removes associations that other clients may have added to the object. Typically you should use objc_setAssociatedObject with a nil value to clear an association.

P.S：不太看得懂第一句！

## 关联对象应用场景

对于笔者而言，关联引用的最主要应用场景就是在category中为类动态添加**属性**，这方面的应用场景非常非常多，本文就不赘述了。但是《Effective Objective-C 2.0》举了另外一个应用场景的例子。

开发iOS时经常用到UIAlertView类，该类提供了一种标准视图，可向用户展示警告信息，当用户按下按钮关闭该视图时，需要用委托协议（delegate protocol）来处理此动作，但是，要想设置好这个委托机制，就得把创建警告视图和处理按钮动作的代码分开。由于代码分作两块，所以读起来有些乱。比如说，我们在使用UIAlertView时，一般都会这么写：

```objc
- (void)askUserAQuestion {
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Question"
                                                        message:@"What do you want to do?"
                                                       delegate:self
                                              cancelButtonTitle:@"Cancel"
                                              otherButtonTitles:@"Continue", nil];
    [alertView show];
}
    
// UIAlertViewDelegate protocol method
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    if (buttonIndex == 0) {
        [self doCancel];
    } else {
        [self doContinue];
    }
}
```

但是，如果在同一个ViewController中需要多次使用UIAlertView，那么UIAlertViewDelegate方法`- (void)alertView:clickedButtonAtIndex:;`的实现代码就非常复杂了，譬如会这样写：

```objc
// UIAlertViewDelegate protocol method
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    if (alertView == self.question1Alert) {
        if (buttonIndex == 0) {
            // 巴拉巴拉
        } else {
            // 巴拉巴拉
        }
    } else if (alertView == self.question2Alert) {
        if (buttonIndex == 0) {
            // 巴拉巴拉
        } else {
            // 巴拉巴拉
        }
    } else if (alertView == self.question3Alert) {
        if (buttonIndex == 0) {
            // 巴拉巴拉
        } else {
            // 巴拉巴拉
        }
    } 
}
```

要是在创建UIAlertView对象时就把相关的处理逻辑写好，那就简单多了。这可以通过关联对象来处理。简单来说，就是创建UIAlertView对象完成后，对其关联一个block，而在`alertView:clickedButtonAtIndex:`调用这个block即可。此方案的实现代码如下：

```objc
#import <objc/runtime.h>
    
static void *alertViewBlockKey = &alertViewBlockKey;
    
- (void)askUserAQuestion {
    
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Question"
                                                        message:@"What do you want to do?"
                                                       delegate:self
                                              cancelButtonTitle:@"Cancel"
                                              otherButtonTitles:@"Continue", nil];
    void (^block)(NSInteger) = ^(NSInteger buttonIndex) {
        if (buttonIndex == 0) {
            [self doCancel];
        } else {
            [self doContinue];
        }
    };
    objc_setAssociatedObject(alertView, alertViewBlockKey, block, OBJC_ASSOCIATION_COPY);
    [alertView show];
}
    
// UIAlertViewDelegate protocol method
    
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    
    void (^block)(NSInteger) = objc_getAssociatedObject(alertView, alertViewBlockKey);
    block(buttonIndex);
}   
```

## 本文参考

* 《[associated-objects](http://nshipster.com/associated-objects/)》
* 《Effective Objective-C 2.0》