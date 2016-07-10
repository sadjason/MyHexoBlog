title: 显式动画
date: 2016-06-26 10:16:26
tags: Animation
categories: iOS
---

**隐式动画**是在iOS平台创建动态用户界面的一种直接方式，也是`UIKit`动画机制的基础，不过它并不能涵盖所有的动画类型。本文旨在要研究**显式动画**，它能够对一些属性做指定的自定义动画，或者创建非线性动画，比如沿着任意一条曲线移动。

## CAAnimation简介

研究显式动画的关键是分析`CAAnimation`及其子类，`CAAnimation`和其子类的继承体系如下：

```txt
|--CAAnimation: CAMediaTiming
   |--CAPropertyAnimation
      |--CABasicAnimation
      |--CAKeyframeAnimation
   |--CATransition
   |--CAAnimationGroup
```

一般情况下不直接使用`CAAnimation`，而使用其子类；并且`CAPropertyAnimation`也不能直接使用。

`CAAnimation`遵循`CAMediaTiming`协议，常用属性包括：

* `beginTime` -- 控制动画开始时间，譬如`CACurrentMediaTime()+1.0`对应的是1秒的delay；
* `duration` -- 控制动画的持续时间；
* `repeatCount` -- 动画的重复次数，永久重复对应`HUGE_VALF`，`1`表不重复；
* `timingFunction` -- 控制动画运行的节奏，一般使用`CAMediaTimingFunction(name:)`赋值，`name`可设置为：
    * `kCAMediaTimingFunctionLinear`
    * `kCAMediaTimingFunctionEaseIn`
    * `kCAMediaTimingFunctionEaseOut`
    * `kCAMediaTimingFunctionEaseInEaseOut`
    * `kCAMediaTimingFunctionDefault`
* `delegate` -- 动画代理，用来监听动画的执行过程，两个方法：
    * `func animationDidStart(animation: CAAnimation)`
    * `func animationDidStop(animation: CAAnimation, finished flag: Bool)`
* `removedOnCompletion` -- 当设置为`true`时，动画完成后，layer又会回到原处；当设置为`false`，且`fillMode`设置为`kCAFillModeForwards`时，layer动画完成后不动；
* `fillMode` -- determine how the timed object behaves once its active duration has completed.

额外阐述一下`fillMode`属性。在此之前先引入几个概念，经历一个完整动画的layer的时间线可分为3个部分：

* **动画前** -- 添加动画到动画开始前的时间段；
* **动画中** -- 动画开始到结束的时间段；
* **动画后** -- 动画结束后的时间；

动画的实质是layer的属性值从某个值到另一个值的变化过程，站在属性值的角度同样有3个概念：

* origin value -- 添加动画前的属性值
* from value -- 动画的初始值
* to value -- 动画的结束值

`fillMode`的属性值会影响layer在**动画前**和**动画后**的行为，有4种可设置的属性值：

* `kCAFillModeRemoved` -- 此为默认值，layer在**动画前**的属性值一直未origin value，动画开始时属性值立马切换到from value，然后动画变化到to value，动画结束时属性值又立马切换到origin value；
* `kCAFillModeForwards` -- 相对于`kCAFillModeRemoved`，layer在动画结束时仍然保持属性值为to value；
* `kCAFillModeBackwards` -- 相对于`kCAFillModeRemoved`，layer在动画前就将属性值切换到origin value；
* `kCAFillModeBoth` -- `kCAFillModeForwards`和`kCAFillModeBackwards`的合体；

>Note: `kCAFillModeForwards`起作用的前提是`removedOnCompletion`的属性值被设置为`false`，`kCAFillModeBackwards`的作用在delay不为0时才能直观感受到，更多内容参考[CAMediaTiming -- Fill Modes](https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/CAMediaTiming_protocol/index.html#//apple_ref/doc/constant_group/Fill_Modes)。

**使用CAAnimation的基本套路**

使用`CAAnimation`（或者其子类）的基本套路是：

1. 初始化一个`CAAnimation`对象，并设置一些动画相关属性；
2. 调用`CALayer#addAnimation(_:forKey:)`方法添加`CAAnimation`对象到`CALayer`中，这样就能开始执行动画了；
3. 调用`CALayer#removeAnimationForKey(_:)`方法停止动画；

`CALayer#addAnimation(_:forKey:)`里的参数`forkey`没有太多别的要求，可以是任意字符串，甚至可以是`nil`。

**CAPropertyAnimation**

`CAPropertyAnimation`继承`CAAnimation`，额外定义了一些属性：

* `keyPath` -- Specifies animated property. 譬如`"frame"`、`"position"`等；
* `additive` -- Determines if the value of the property is the value at the end of the previous repeat cycle, plus the value of the current repeat cycle. 不太懂！
* `cumulative` -- Determines if the value specified by the animation is added to the current render tree value to produce the new render tree value. 不太懂！
* `valueFunction` -- An optional value function that is applied to interpolated values. 很少会用到！

>Note: `CAPropertyAnimation`里的`keyPath`和`CALayer#addAnimation(_:forKey:)`里的`key`不一样，前者只能是`"frame"`、`"position"`之类的对应`CALayer`属性的字符串，后者可以是任意字符串，可以认为是给animation贴个ID标签。

## CABasicAnimation

`CABasicAnimation`在继承`CAPropertyAnimation`的基础上又定义了三个属性：

```objc
@interface CABasicAnimation : CAPropertyAnimation

@property(nullable, strong) id fromValue;
@property(nullable, strong) id toValue;
@property(nullable, strong) id byValue;

@end
```

`CAAnimation.h`里介绍了几种使用模式，一般来说，使用`fromValue`/`toValue`或`fromValue`/`byValue`组合即可，或者直接使用`byValue`或`toValue`。

## CAKeyframeAnimation

Key frame animation常被翻译为：**关键帧动画**。顾名思义，`CAKeyframeAnimation`可以实现让animated properties按照一串数值进行动画，就好像逐帧制作动画一样。

>P.S: 不要被`CAKeyframeAnimation`中的frame所迷惑，它和`CALayer#frame`中的frame不是一回事儿。

相较而言，`CABasicAnimation`处理动画更粗糙一些，只是单纯地让layer的属性值从a到b；考虑到`CAKeyframeAnimation`的复杂性，这一部分对它的阐述会更详细一些。

**Providing Keyframe Values**

动画处理的本质是改变属性值，`CABasicAnimation`提供了`fromValue`、`byValue`、`toValue`来「装载」待处理的属性值。

`CAKeyframeAnimation`处理动画更精细一些，动画过程中所需要指定的属性值自然更多（一般来说），因此提供了数组类型属性`values`来「装载」待处理的属性值。

举个例子，现在沿正方形路径移动一个view，所经过的路径是`(100, 100)`->`(200, 100)`->`(200, 200)`->`(100, 200)`->`(100, 100)`，代码如下：

```swift
let kfa = CAKeyframeAnimation(keyPath: "position")

kfa.values = [NSValue(CGPoint: CGPoint(x: 100, y: 100)),    // 左上角
              NSValue(CGPoint: CGPoint(x: 200, y: 100)),    // 右上角
              NSValue(CGPoint: CGPoint(x: 200, y: 200)),    // 右下角
              NSValue(CGPoint: CGPoint(x: 100, y: 200)),    // 左下角
              NSValue(CGPoint: CGPoint(x: 100, y: 100))]    // 左上角
kfa.duration = 4.0
kfa.removedOnCompletion = false
kfa.fillMode = kCAFillModeBoth

animationView.layer.addAnimation(kfa, forKey: nil)
```

除了`values`，`CAKeyframeAnimation`还提供了`path`属性，当使用`CAKeyframeAnimation`构建point-based的动画时，使用`path`是个不错的选择。

>P.S: 显然，`values`和`path`只需要使用一个即可，然而，当同时赋予`values`和`path`以有效值，哪个会起作用呢？验证发现，当给`path`设置了有效值后，`values`的值就不会起作用。

**Keyframe Timing**

`values`将一个完整的动画分为n个关键帧（假设数组长度为n），默认情况下，相邻帧之间的时间间隔为：duration/(n-1)。

但是`CAKeyframeAnimation`提供了丰富的接口以便进行更精密更丰富的自定义，相关属性包括：`keyTimes`、`calculationMode`以及`timingFunctions`。

`keyTimes`也是数组类型，简单来说，`keyTimes`中的元素将动画时间切割成几部分，譬如`[0, 0.5, 1.0]`将时间切割为两部分：0-0.5*duration、0.5*duration-1.0*duration。

>Note: 一般来说，`keyTimes`的元素个数和`values`的元素个数一致，或者与`path`的点的个数一致，否则，动画会出现想象不到的结果；但有特殊情况，下文会提到。

`calculationMode`指示了关键帧之间的值如何计算，可选的值包括：

* `kCAAnimationLinear`，默认值，线性过渡；
* `kCAAnimationDiscrete`，从一个关键帧跳到下一个关键帧，直接跳跃，无过渡；
* `kCAAnimationPaced`
* `kCAAnimationCubic`
* `kCAAnimationCubicPaced`

>P.S: `kCAAnimationLinear`和`kCAAnimationDiscrete`相对比较容易理解，后3种没怎么用到过，理解不深刻，希望以后能够补充。

第三个属性`timingFunctions`和上文提到的`timingFunction`类似，只是这里是一个数组类型，它的每个元素控制的是两个相邻关键帧之间timing curve。

>Note: 若属性`values`中指定的关键帧个数为`n`（元素个数），则`timingFunctions`的元素个数应该为`n-1`。

再着重介绍`keyTimes`配合`calculationMode`使用的注意事项：

* 如果`calculationMode`的值为`kCAAnimationLinear`或`kCAAnimationCubic`，`keyTimes`的第一个元素值必须为`0.0`，最后一个元素值必须为`1.0`，其他的元素值（如果有）范围在`0.0-1.0`之间；
* 如果`calculationMode`的值为`kCAAnimationDiscrete`，`keyTimes`的第一个元素值必须为`0.0`，最后一个元素值必须为`1.0`；此外，若`value`元素个数为`n`，则`keyTimes`的元素数量应该为`n+1`；
* 如果`calculationMode`的值为`kCAAnimationPaced`或`kCAAnimationCubicPaced`，`keyTimes`的值会被忽略。

**Rotation Mode Attribute**

`rotationMode`, it determines whether objects animating along the path rotate to match the path tangent.

该属性配合`path`使用，有三个可选值：`nil`、`kCAAnimationRotateAuto`以及`kCAAnimationRotateAutoReverse`。设置一段曲线，配置相应的属性值，能较直观的感受到它们的作用。

**Cubic Mode Attributes**

还有三个不常用到的属性：`tensionValues`、`continuityValues`、`biasValues`。

它们仅在`calculationMode`属性值为`kCAAnimationCubic`或`kCAAnimationCubicPaced`时起作用。还没详细体会过，暂时略过。

关于`CAKeyframeAnimation`的更多内容参考[CAKeyframeAnimation Class Reference](https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/CAKeyframeAnimation_class/)。

## CATransition

暂时略过，详细内容参考[CATransition Class Reference](https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/CATransition_class/)。

[iOS开发之各种动画各种页面切面效果](http://www.cnblogs.com/ludashi/p/4160208.html)中还介绍了更多，只是使用这些私有资源会存在被Apple Store打回的风险。

>P.S: 曾以为`CATransition`在转场动画中会用得比较多，但事实上，似乎用得并不多。

## CAAnimationGroup

每个`CAAnimation`只能处理一个animated property，但有时候需要同时处理多个animated properties，此时就会用到`CAAnimationGroup`。其定义非常简单：

```swift
public class CAAnimationGroup : CAAnimation {  
  public var animations: [CAAnimation]?
}
```

其用法就不消多说了，easy。

## 本文参考

* [CAMediaTiming Protocol Reference](https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/CAMediaTiming_protocol/)
* [CAAnimation Class Reference](https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/CAAnimation_class/)
* [CABasicAnimation Class Reference](https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/CABasicAnimation_class/)
* [CAKeyframeAnimation Class Reference](https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/CAKeyframeAnimation_class/)
* [CATransition Class Reference](https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/CATransition_class/)
* [CAAnimationGroup Class Reference](https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/CAAnimationGroup_class/)
* [iOS开发之各种动画各种页面切面效果](http://www.cnblogs.com/ludashi/p/4160208.html)