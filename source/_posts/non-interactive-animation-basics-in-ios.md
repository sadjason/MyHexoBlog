title: 非交互式动画基础
date: 2016-06-13 09:35:14
tags: Animation
categories: iOS
---

## Animation Properties

所谓的动画，其本质是物体（譬如view）的某些属性或属性组合在时间线上的变化。[Motion Design for iOS](https://dn-motion-design.qbox.me/)将这些属性总结为6种：position、opacity、color、scale、rotation、3D。

这种说法对iOS环境下`UIView`同样适合，`UIView`中与animation相关的属性包括：

1. `UIView#frame`/`UIView#center`/`UIView#bounds`: position
2. `UIView#alpha`: alpha
3. `UIView#backgroundColor`: color
4. `UIView#transform`: position, scale, rotation
5. `UIView#layer.transform`: 3D

最后一个，`UIView#layer.transform`的类型是`CATransform3D`，能够控制layer级别的3D变换；其余几种能够控制`UIView`的2D变换，其中第4个，`UIView#transform`的类型是`CGAffineTransform`，能够控制`UIView`的平移、缩放、旋转。

这一部分内容着重于分析`CGAffineTransform`和`CATransform3D`，其他几个属性相对简单得多。

## Affine Transforms

国内博客在谈及`CGAffineTransform`时，常常会讲到一个词语：**仿射变换**，它是affine transformation的中文翻译。

`CGAffineTransform`是个结构体类型，属于Core Graphics框架，它有6个变量：a、b、c、d、tx、ty。这6个变量和另外3个固定值构成了一个3x3矩阵，如下图：

<div class="imagediv" style="width: 232px; height: 206px">{% asset_img affine-transform-matrix.png Affine Transform Matrix %}</div>

`UIView`的本质是由像素点组成的矩形块，每个像素点的相对位置可以抽象为坐标`(x, y)`，将这个坐标称之为某像素点的**原坐标**，**原坐标**的`x`、`y`以及`1`构成一个1x3的向量`(x, y, 1)`，所谓的仿射变换指的是让该1x3向量与3x3矩阵进行相乘，得到一个新的1x3向量，称之为`(_x, _y, 1)`，而`(_x, _y)`即`(x, y)`反射变换后的新坐标。将`UIView`的每一个像素点都进行这样的处理，即可完成整个`UIView`的反射变换。更形象的描述如下图所示：

<div class="imagediv" style="width: 676px; height: 220px">{% asset_img affine-transform-algorithm.jpeg Affine Transform Algorithm %}</div>

显然，仿射变换的关键是生成`CGAffineTransform`实例了，如果没学过计算机图形学，计算`CGAffineTransform`的参数值还是蛮麻烦的，不过好在iOS已经为我们提供了快捷API。

仿射变换实现的效果包括平移、缩放、旋转以及它们的组合，如下是这三种仿射变换的快速生成APIs：

```swift
// 平移
CGAffineTransformMakeTranslation(_:_:)
// CGAffineTransformMakeTranslation(tx, ty)

// 缩放
CGAffineTransformMakeScale(_:_:)
// CGAffineTransformMakeScale(sx, sy)

// 旋转
CGAffineTransformMakeRotation(_:)
// CGAffineTransformMakeRotation(angle)
```

除此之外还有一个其他APIs，此处略过。Wiki还提供了一些常见反射变换及对应的矩阵值，如下图：

<div class="imagediv" style="width: 512px; height: 683px">{% asset_img 2D_affine_transformation_matrix.png 2D Affine Transform Matrix %}</div>

>Note: 在iOS环境中，对`UIView`进行反射变换所参考的坐标点是`UIView.layer.archorPoint`，[《iOS图层几何学》](layer-geometry-in-ios.md)里有阐述。

## 3D Transforms

3D变换要复杂得多，恐怕得花很多时间才能搞清楚，暂时略过。

[iOS核心动画高级技巧--3D变换](https://zsisme.gitbooks.io/ios-/content/chapter5/3d-transform.html)有比较好的介绍。

`UIView#transform`和`UIView#layer.transform`的关系？将一个归位后另外一个会是啥变化？

## Planning Animations

如何规划动画？先问自己这么几个问题：

* What are the initial properties of the item?
* What are the final properties of the item?
* How long should the animation take?
* What’s happening while this item is animating?
* What will happen once this item is done animating?

## Animation Curve of An Easing-type Motion

所谓的动画，无非是animation properties在时间轴上的变化过程。

以时间为x轴，属性值为y轴，可以构建一个二维坐标系，属性值从`a`到`b`，有无数种走向。换句话说，动画类型的丰富程度是无限的。

但在实际应用中，可接受的（看起来比较爽的）动画类型是不多的，其中一种类型叫：**easing-type motion**，一图胜千言：

<div class="imagediv" style="width: 460px; height: 340px">{% asset_img easing.png Easing %}</div>

图中所展示的是4种最为常见的easing-type motion曲线，[缓动函数速查表](http://easings.net/zh-cn)里能够看到更多。

在iOS中使用`UIView.animateWithDuration(...)`系列方法处理动画时，可以通过设置`options`来选择对应的easing type，比如：

```swift
UIView.animateWithDuration(0.2, delay: 0.0, options: .CurveEaseInOut, animations: { /*...*/ }, completion: nil)
```

`options`可以设置结构体类型`UIViewAnimationOptions`的变量值，该类型有很多变量值，easing-type motion相关的有四个：

* `CurveLinear`, linear
* `CurveEaseInOut`, ease in, ease out
* `CurveEaseIn`, ease in
* `CurveEaseOut`, ease out

>Note: 这些easing curves本质上是3次贝塞尔曲线（cubic Bézier curve），通过控制4个变量可以得到各种各样的3次贝塞尔曲线，只是iOS貌似没有提供控制这些参数的API，[Cubic Bezier On Line](http://cubic-bezier.com/)提供了更为丰富的体验。

## Animation Curve of A Spring-like Motion

另一种动画类型叫：**spring-like motion**。[这里](https://dn-motion-design.qbox.me/guide-3.html)对spring-like motion有较为细致的描述，此处就不再赘述，这一部分重点在于分析控制spring-like motion的关键参数。

参考[Natural Motion](https://dn-motion-design.qbox.me/guide-3.html)的分析，控制spring-like motion的关键参数有三个：

* **Mass**. The mass is the weight or heft of the object attached to the end of the spring.
* **stiffness.** The stiffness is how difficult it is to stretch out the spring, which typically corresponds to its thickness and how tightly it’s coiled. 
* **damping.** The damping is the resistive force or friction, like when you drag your hand through water and feel the forces pushing against it as you try to move quicker through the water.

iOS自iOS 7起引入了支持spring-like motion的API：

```swift
UIView.animateWithDuration(duration: NSTimeInterval,
                           delay: NSTimeInterval, 
                           usingSpringWithDamping dampingRatio: CGFloat,
                           initialSpringVelocity velocity: CGFloat,
                           options: UIViewAnimationOptions,
                           animations: () -> Void,
                           completion: ((Bool) -> Void)?)
```

相对而言，该Spring Animation API对spring-like motion的控制简化了，只提供了两个参数：`dampingRatio`和`velocity`

`dampingRatio`的取值范围是0.0-1.0，数值越小，「弹簧」的震动效果越明显。下图演示了在`dampingRatio`为`0.0`的情况下，`dampingRatio`分别取`0.2`，`0.5`和`1.0`的情况。

<div class="imagediv" style="width: 314px; height: 321px">{% asset_img spring-2.gif Spring 2 %}</div>

`velocity`表示初识的速度，数值越大一开始移动越快。下图演示了在`dampingRatio`为`1.0`时，`velocity`分别取`5.0`，`15.0`和`25.0`的情况。值得注意的是，初始速度取值较高而时间较短时，也会出现反弹情况。

<div class="imagediv" style="width: 314px; height: 321px">{% asset_img spring-3.gif Spring 3 %}</div>

## 本文参考

* [Motion Design for iOS](https://dn-motion-design.qbox.me/)
* [iOS仿射变换CGAffineTransform详解](http://www.jianshu.com/p/6c09d138b31d)
* [仿射变换](https://zh.wikipedia.org/wiki/%E4%BB%BF%E5%B0%84%E5%8F%98%E6%8D%A2)
* [iOS核心动画高级技巧--仿射变换](https://zsisme.gitbooks.io/ios-/content/chapter5/affine-fransforms.html)
* [iOS核心动画高级技巧--3D变换](https://zsisme.gitbooks.io/ios-/content/chapter5/3d-transform.html)
* [缓动函数速查表](http://easings.net/zh-cn)
* [Cubic Bezier On Line](http://cubic-bezier.com/)
* [使用iOS 8 Spring Animation API创建动画](https://www.renfei.org/blog/ios-8-spring-animation.html)
