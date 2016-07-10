title: Navigation Transition
date: 2016-06-11 10:30:03
tags: Transition
categories: iOS
---

[Modal Transition概述](../modal-transitions-in-ios/)对自定义transition进行了简单概述，[Using UIPercentDrivenInteractiveTransition](../using-uipercentdriveninteractivetransition)介绍了可交互转场的基本实现思路。本文旨在总结实现**自定义navigation转场**的基本套路。

此外，还对自己写的相关demo做一个汇总。

## Basic Methods Of Custom Navigation Transition

在`UINavigationController`的基础上实现自定义custom navigation transition，只需要让自定义view controller遵循`UINavigationControllerDelegate`协议，如下：

```swift
class MainViewController: UIViewController, UINavigationControllerDelegate {
  override func viewDidLoad() {
    super.viewDidLoad()
    navigationController?.delegate = self
    // ...
  }
}
```

默认情况下，`UINavigationController#delegate`为`nil`，此时push和pop动画全部采用系统默认transition。

`UINavigationControllerDelegate`定义了6个方法，与transition直接相关的有两个：

```swift
// 为push和pop操作提供animation controller
func navigationController(navigationController: UINavigationController,
  animationControllerForOperation operation: UINavigationControllerOperation,
  fromViewController fromVC: UIViewController,
  toViewController toVC: UIViewController)
  -> UIViewControllerAnimatedTransitioning?

// 为push和pop操作提供interaction controller
func navigationController(
  navigationController: UINavigationController,
  interactionControllerForAnimationController animationController: UIViewControllerAnimatedTransitioning)
  -> UIViewControllerInteractiveTransitioning?
```

站在这两个方法的角度来看，`UINavigationControllerDelegate`有点类似于`UIViewControllerTransitioningDelegate`，后者为transition动画提供animation controller和interaction controller，前者的作用也类似。

## Some Demos

**可交互的简单push/pop实现 -- 仿知乎**

<div class="imagediv" style="width: 320px; height: 566px">{% asset_img BasicInteractiveCustomNavigationTransitionDemo.gif Basic Interactive Custom Navigation Transition Demo %}</div>

代码详见[这里](https://github.com/sadjason/BasicInteractiveCustomNavigationTransitionDemo)。

如下是一些不错的开源custom navigation transition实现：

* [RMPZoomTransitionAnimator](https://github.com/recruit-mp/RMPZoomTransitionAnimator)
* [ZoomInteractiveTransition](https://github.com/DenHeadless/ZoomInteractiveTransition)
* [ZOZolaZoomTransition](https://github.com/NewAmsterdamLabs/ZOZolaZoomTransition)
* [HAPaperViewController](https://github.com/hebertialmeida/HAPaperViewController)
* [BCMagicTransition](https://github.com/boycechang/BCMagicTransition)
* [IBAnimatable](https://github.com/JakeLin/IBAnimatable)

## 本文参考

* [Navigation Controller Custom Transition Animation](http://stackoverflow.com/questions/26569488/navigation-controller-custom-transition-animation)