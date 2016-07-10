title: 使用UIPercentDrivenInteractiveTransition
date: 2016-06-10 17:06:55
tags: Transition
categories: iOS
---

实现可交互的自定义转场动画的关键在于：定义遵循`UIViewControllerInteractiveTransitioning`协议的类。

`UIViewControllerInteractiveTransitioning`协议的内容如下：

```swift
public protocol UIViewControllerInteractiveTransitioning : NSObjectProtocol {
    public func startInteractiveTransition(transitionContext: UIViewControllerContextTransitioning)
    
    optional public func completionSpeed() -> CGFloat
    optional public func completionCurve() -> UIViewAnimationCurve
}
```

到现在我都没搞明白如何定义遵循`UIViewControllerInteractiveTransitioning`的类。

但正如Apple[文档](https://developer.apple.com/library/prerelease/content/featuredarticles/ViewControllerPGforiPhoneOS/CustomizingtheTransitionAnimations.html)所言：
>The easiest way to make your animations interactive is to use a `UIPercentDrivenInteractiveTransition` object.

简单来说，创建遵循`UIViewControllerInteractiveTransitioning`协议的类型的最简单方式是继承`UIPercentDrivenInteractiveTransition`类。

这是一个遵循`UIViewControllerInteractiveTransitioning`协议的类，为我们预先实现和提供了一系列便利的方法，可以用一个百分比来控制交互式切换的过程。

该类的内容及介绍如下：

```swift
public class UIPercentDrivenInteractiveTransition : NSObject, UIViewControllerInteractiveTransitioning {
    
  // 只读，对应的是animation controller的transitionDuration(_:)返回值
  public var duration: CGFloat { get }
    
  // 交互动画完成度（百分比），updateInteractiveTransition(_:)会改变该值
  public var percentComplete: CGFloat { get }
    
  // 影响completion animation duration，一般设置为1.0（默认值），
  // 对应的completion animation duration为(1 - percentComplete)*duration.
  public var completionSpeed: CGFloat
    
  // completion animation的timing curve，默认是.EaseInOut
  public var completionCurve: UIViewAnimationCurve

  /* 这三个方法在gesture recognizer的处理逻辑中调用 */
  // 更新百分比，一般通过手势识别的长度之类的来计算一个值，然后进行更新
  public func updateInteractiveTransition(percentComplete: CGFloat)
  // 报告交互取消，返回切换前的状态
  public func cancelInteractiveTransition()
  // 报告交互完成，更新到切换后的状态
  public func finishInteractiveTransition()
}
```

`UIPercentDrivenInteractiveTransition`的内容并不复杂，使用时需要注意的是，interaction controller不能孤立存在，需要配合animation controller工作；换句话说，具体的transition仍然在animation controller的`animateTransition(_:)`中完成。

此外，在定义animation controller的`animateTransition(_:)`方法时，不要忘记调用`UIViewControllerContextTransitioning`的`completeTransition(_:)`方法。

>P.S: 根据我的理解并验证，`UIViewControllerContextTransitioning#completeTransition(true)`的作用效果之一是将from view从container view中移除掉。

基于`UIPercentDrivenInteractiveTransition`做了个简单的Demo，如下图：

<div class="imagediv" style="width: 320px; height: 568px">{% asset_img shrinking-modal-interactive-transition.gif Shrinking Modal Interactive Transition %}</div>

代码详见[Github](https://github.com/sadjason/ShrinkingModalInteractiveTransitionDemo)。

`UIViewControllerTransitioning.h`中对`UIPercentDrivenInteractiveTransition`有一段说明，不是特别理解：
>This style of interaction controller should only be used with an animator that implements a CA style transition in the animator's animateTransition: method.

掌握使用`UIPercentDrivenInteractiveTransition`并不难，但是该类有哪些限制（或缺陷）呢？我有该如何抛开该类自定义实现interaction controller呢？

## 本文参考

* [WWDC 2013 Session笔记 - iOS7中的ViewController切换](https://onevcat.com/2013/10/vc-transition-in-ios7/)
* [UIViewControllerInteractiveTransitioning Protocol Reference](https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIViewControllerInteractiveTransitioning_protocol/)
* [Interactive Animated Transitions on iOS](http://initwithfunk.com/blog/2014/05/22/interactive-animated-transitions-on-ios/)