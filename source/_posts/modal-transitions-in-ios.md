title: Modal Transition
date: 2016-06-09 19:43:39
tags: Transition
categories: iOS
---

本文旨在分析modal transition自定义过场动画的实现过程，总结一些基本套路。

视图控制器的转场动画涉及几大组件：

1. **转场上下文**（transition context）
2. **转场代理**（transitioning delegation）
3. **动画控制器**（animation controller）
4. **交互控制器**（interaction controller）
5. **转场协调器**（transition coordination）
6. **呈现控制器**（presentation controller）

其中前5个是iOS 7中引入的，都以**协议**的形式定义，利用这5个接口可以实现丰富转场动画（不光是modal transition），详细内容参考[iOS转场动画概述](../viewcontroller-transitions-basics/)；第6个是Apple在iOS 8中引入的，以类（`UIPresentationController`）的形式定义，在modal transition中可能会用到。

下文若涉及如上这些概念，均以对应的英文代指。

在iOS中，modal transition涉及两个控制器：presenting view controller和presented view controller。transition结束后，后者盖在前者上面。

过场动画说到底是对view的动画处理，根据我的分析，相关的view包括：key window、presenting controller's view、container view、presented controller's view，完成transition后，它们的层次结构如下：

```txt
|--key window
   |--presenting controller's view
   |--container view
      |--presented controller's view

# 说明：
# a. 此处presenting view controller是keyWindow的rootViewController；
# b. container view并不一定是presented controller's view的super view，但一定是它的ancestor view；
```

为叙述简单起见，下文中的**presented view**等价于上述的**presented controller's view**，**presenting view**则等价于**presenting controller's view**。

如上的层次结构是我通过阅读文档和测试得出的结论，可以看出，presenting view和presented view并没有包含关系。container view是什么呢？在transition过程中，系统先创建一个临时的view，把它纳入到key window的层次结构中，然后将presented view加入到container view中。

对于modal transition，一般来说，container view是系统产生的。

>P.S: 根据我的认知，貌似只有在自定义容器控制器时，才需要指定container view。

## 简单的自定义Modal过场动画

把相关的view层次关系理清，实现简单的自定义过场动画就不难了，先来一个最简单的：presented view渐进显示（alpha从0.0到1.0），效果如下：

<div class="imagediv" style="width: 320px; height: 568px">{% asset_img simple-modal-transition-demo1.gif Simple Modal Transition Demo %}</div>

上述demo中涉及两个view controller，本文将它们对应的类命名为：PresentingViewController和PresentedViewController。

从PresentingViewController过渡到PresentedViewController非常容易，一行代码解决：

```swift
presentViewController(PresentedViewController(), animated: true)
```

默认情况下，PresentedViewController会从底部升起，现在想让它执行自定义动画（alpha从0.0到1.0），需要对PresentedViewController稍作处理，首先是设置其modal style为Custom，并指定其过场动画的代理：

```swift
class PresentedViewController: UIViewController {
  var presenter: UIViewControllerTransitioningDelegate?
  init() {
    super.init(nibName: nil, bundle: nil)
    commonSetup()
  }
  
  required init?(coder aDecoder: NSCoder) {
    super.init(coder: aDecoder)
    commonSetup()
  }
  
  private func commonSetup() {
    modalPresentationStyle = .Custom
    presenter = SimpleModalPresenter()
    transitioningDelegate = presenter
  }
  // ...
}
```

`modalPresentationStyle=.Custom`告诉系统自定义过场处理，而`transitioningDelegate`属性指定transitioning delegation，值得一提的是，`transitioningDelegate`属性被`weak`修饰，不强持有所指向的资源，故而`commonSetup()`不能写成如下这样（曾在这里栽过跟头）：

```swift
private func commonSetup() {
  modalPresentationStyle = .Custom
  transitioningDelegate = SimpleModalPresenter()
}
```

目前为止（当前最新版本为iOS 9），transitioning delegation有5个可选回调，作用有三个：

* 提供animation controllers（遵循`UIViewControllerAnimatedTransitioning`协议的对象）；
* 提供interaction controllers（遵循`UIViewControllerInteractiveTransitioning`协议的对象）；
* 提供presentation controller（`UIPresentationController`对象）；

`SimpleModalPresenter`的实现非常简单，实现两个回调，分别为presentation transition和dismissal transition提供animation controller（暂时不考虑交互，因此略过interaction controller，`UIPresentationController`下文会提到）：

```swift
class SimpleModalPresenter: NSObject, UIViewControllerTransitioningDelegate {
  
  // 非交互动画
  func animationControllerForPresentedController(presented: UIViewController, presentingController presenting: UIViewController, sourceController source: UIViewController) -> UIViewControllerAnimatedTransitioning? {
    let animator = SimpleModalAnimator()
    animator.isPresentation = true
    return animator
  }
  
  func animationControllerForDismissedController(dismissed: UIViewController) -> UIViewControllerAnimatedTransitioning? {
    let animator = SimpleModalAnimator()
    animator.isPresentation = false
    return animator
  }
}
```

Animation controller的实现也非常简单，无非是利用系统提供transition context信息进行简单的动画处理：

```swift
class SimpleModalAnimator: NSObject, UIViewControllerAnimatedTransitioning {

  var isPresentation: Bool = false
  
  // 定义过场动画时间，其他地方可能会用到这个
  func transitionDuration(transitionContext: UIViewControllerContextTransitioning?) -> NSTimeInterval {
    return 1.0
  }
  
  // 过场动画的具体执行
  func animateTransition(transitionContext: UIViewControllerContextTransitioning) {
    
    let containerView = transitionContext.containerView()
    
    let presentedView: UIView?
    if isPresentation {
      presentedView = transitionContext.viewForKey(UITransitionContextToViewKey)
      presentedView?.frame = (containerView?.bounds)!
      presentedView?.alpha = 0.0
      containerView?.addSubview(presentedView!)
    } else {
      presentedView = transitionContext.viewForKey(UITransitionContextFromViewKey)
    }
    
    UIView.animateWithDuration(transitionDuration(transitionContext), animations: {
      presentedView?.alpha = self.isPresentation ? 1.0 : 0.0
      }) { (finished) in
        transitionContext.completeTransition(finished)
        if !self.isPresentation {
          presentedView?.removeFromSuperview()
        }
    }
  }
}
```

`SimpleModalAnimator`将presentation和dismissal二合一，定义`isPresentation`属性以便区分。

综上，实现简单的modal transition只需要三个步骤：

1. 在presented view controller中指定modal style和transition delegate；
2. 实现transition delegate，即定义遵循`UIViewControllerTransitioningDelegate`协议的类，transition delegate为presented view controller提供animation controllers；
3. 实现animation controllers，即定义遵循`UIViewControllerAnimatedTransitioning`协议的类，animation controller利用系统所提供的transition context信息，处理一些简单的view animation。

## UIPresentationController

Presentation controller是iOS 8引入的新概念，能够帮助我们更方便快捷地实现view controller的自定义过渡效果，与animation controller等不同，它不是以协议的方式定义，而是定义了一个名为`UIPresentationController`的类。

>P.S: 很好奇为什么不把presentation controller也定义成protocol。

简单来说，presentation controller和animation controller有些类似，它们都不是视图控制器，没有`view`属性，都拥有container view、presented view等访问权，都由transitioning delegation提供；presentation controller的主要使命是提供一些回调，系统在presentation transition的开始/结束、dismissal transition的开始/结束会调用这些回调，有点类似视图控制器的`viewWillAppear(_:)`、`viewDidAppear(_:)`，常用的回调如下：

```swift
// presentation transition
func presentationTransitionWillBegin()
func presentationTransitionDidEnd(completed: Bool)
// dismissal transition
func dismissalTransitionWillBegin()
func dismissalTransitionDidEnd(completed: Bool)

/* Position of the presented view in the container view 
 * by the end of the presentation transition.
 * (Default: container view bounds)
 */
func frameOfPresentedViewInContainerView() -> CGRect

/* Indicate whether the view controller's view we are transitioning from 
 * will be removed from the window in the end of the presentation transition
 * (Default: NO)
 */
func shouldRemovePresentersView() -> Bool
```

可以看到，相较于animation controller，presentation controller所涉及的过场讯息更加精细。

>Note: `UIPresentationController`实例的初始化不能使用`init()`，必须要使用`init(presentedViewController:presentingViewController:)`。

刚开始接触presentation controller时有些疑点：

* Presentation controller一定要配合animation controller使用？可以单独使用吗？
* Presentation controller和animation controller都可以访问container view和presented view，都能处理过场动画，如何让它们俩协调工作？

首先，presentation controller是可以单独使用的！也就是说，transitioning delegation可以只提供presentation controller，而不提供animation controller，如下这样：

```swift
// 转场代理只提供presentation controller
class SimpleModalPresenter: NSObject, UIViewControllerTransitioningDelegate {

  func presentationControllerForPresentedViewController(presented: UIViewController, presentingViewController presenting: UIViewController, sourceViewController source: UIViewController) -> UIPresentationController? {
    return SimplePresentationController(presentedViewController: presented, presentingViewController: presenting)
  }
}
```

StackOverFlow中有一个简单的[用例](http://stackoverflow.com/questions/29219688/present-modal-view-controller-in-half-size-parent-controller/29220983#29220983)：让presented view占据屏幕的下半部分，而不是全屏。重载自定义presentation controller的`frameOfPresentedViewInContainerView() -> CGRect`方法即可实现。

>Note: 当transitioning delegation同时提供自定义的animation controller和presentation controller时，presentation controller的`frameOfPresentedViewInContainerView() -> CGRect`方法不管用了，不晓得什么原因，非常诡异！除了这种方式，还可以在animation controller的`animateTransition(_:)`设置presented view的`frame`。

然后来分析presentation controller和animation controller如何协同工作。

如上文所述，二者拥有对presented view和container view的访问权...在我看来，它们所能做的事情有些重叠，modal呈现presented view controller的一个重要步骤是`containerView.addSubview(presentedView)`，这行代码在哪里执行呢？可以在animation controller的`animateTransition(_:)`方法中处理，也可以在presentation controller的回调`presentationTransitionWillBegin()`中处理。此外，presentation controller的很多用法是在container view和presented view中加入一层view（充当遮罩或者蒙板），比如[这样](http://www.cocoachina.com/industry/20140707/9053.html)；这种事情显然也可以在animation controller的`animateTransition(_:)`方法中完成。

总之，在我看来，presentation controller不是必要的，它所能做的事情，animation controller的`animateTransition(_:)`都能做，只是presentation controller把工作给简化了一些。

>Note: 把presentation controller看作是animation controller和interaction controller的扩展。

个人感觉，presentation controller虽然简化了自定义modal transition的实现，但是让逻辑更复杂了，甚至更混乱了。

因此，有必要将presentation controller和animation controller的工作给分清楚。我是这么想的：presented view相关的设置与动画处理都放在animation controller中完成，而presentation view（一般是夹在container view和presented view之间的临时view）相关设置和处理都放在presentation controller中处理。

在presentation transition和dismissal transition过程中，animation controller的`animateTransition(_:)`和presentation controller的几个回调的执行顺序是：

```txt
# presentation transition
presentationTransitionWillBegin()
animateTransition(_:)
presentationTransitionDidEnd(_:)

# dismissal transition
dismissalTransitionWillBegin()
animateTransition(_:)
dismissalTransitionDidEnd(_:)
```

## Presentation Style

`UIViewController`有个属性`modalPresentationStyle`，上文中用到了该属性，只是用得稀里糊涂，这一部分对它进行进一步说明。

Apple文档对`modalPresentationStyle`属性的解释如下：

>The presentation style determines how a modally presented view controller is displayed onscreen. In a horizontally compact environment, modal view controllers are always presented full-screen. In a horizontally regular environment, there are several different presentation options. For a list of possible presentation styles, and their compatibility with the available transition styles, see the [UIModalPresentationStyle](https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIViewController_Class/#//apple_ref/c/tdef/UIModalPresentationStyle) constant descriptions.

该属性是个枚举值，包括9个case，其中大多数都与iPad环境有关，这里只介绍iPhone开发环境中会常常涉及到的3个枚举值：

* `.FullScreen` -- A presentation style in which the presented view covers the screen. The views belonging to the presenting view controller are removed after the presentation completes.
* `.OverFullScreen` -- A view presentation style in which the presented view covers the screen. The views beneath the presented content are not removed from the view hierarchy when the presentation finishes. So if the presented view controller does not fill the screen with opaque content, the underlying content shows through.
* `.Custom` -- A custom view presentation style that is managed by a custom presentation controller and one or more custom animator objects. All of these objects are provided by the presented view controller’s transitioning delegate.

简单来说，当`modalPresentationStyle`属性赋值`.FullScreen`或者`.OverFullScreen`时，transitioning delegation提供的自定义presentation controller就没什么卵用。换句话说，如果想使用自定义的presentation controller，必须对`modalPresentationStyle`属性赋值`.Custom`。

它们仨有啥不同呢？

`.FullScreen`所对应的内置presentation controller，会在presentation transition结束时，将presenting view从key window的view hierarchy中移除掉。

而`.OverFullScreen`所对应的内置presentation controller，则不会在presentation transition结束时移除掉presenting view。

在`.Custom`模式下，不会使用任何一种系统内置presentation controller，它依赖于用户自定义presentation controller，即实现一个`UIPresentationController`子类，该子类有一个`shouldRemovePresentersView() -> Bool`方法。单从对待presenting view的角度来看，当该方法返回`false`时，`.Custom`与`.OverFullScreen`一致，反之，当该方法返回`true`时，`.Custom`与`.FullScreen`一致。

我为什么会关注`modalPresentationStyle`属性呢？

这是因为我在animation controller的`animateTransition(_:)`方法中无法访问presenting view，如下：

```swift
func animateTransition(transitionContext: UIViewControllerContextTransitioning) {
  //...
  let presentingView = transitionContext.viewForKey(UITransitionContextFromViewKey)
  //...
}
```

查了半天资料，发现这个[说法](http://stackoverflow.com/questions/24338700/from-view-controller-disappears-using-uiviewcontrollercontexttransitioning/25901154#25901154)比较靠谱。大概的意思是：

* 如果在presentation transition结束时仍然将presenting view保留到key window中，iOS（iOS 8及以后版本）认为没有必要让你再操控presenting view，故而通过`viewForKey(UITransitionContextFromViewKey)`访问presenting view得到的结果为`nil`；
* 如果在presentation transition结束时将presenting view从key window中移除掉，iOS认为你可能需要在presentation transition过程中对presenting view做一个动画，故而通过`viewForKey(UITransitionContextFromViewKey)`能够访问到presenting view。

UIKit的UIViewControllerTransitioning.h文件中也有对这种说法的佐证：

```swift
// Currently only two keys are defined by the system -
// UITransitionContextFromViewKey, and UITransitionContextToViewKey
// viewForKey: may return nil which would indicate that the animator should not
// manipulate the associated view controller's view.
@available(iOS 8.0, *)
public func viewForKey(key: String) -> UIView?
```

对于这种做法，我的下意识是：What The Fuck!

当然，如果非要访问presenting view，除了通过`viewForKey(_:)`，还可以曲线救国：先通过`viewControllerForKey(_:)`访问presenting view controller，然后再访问view。

## 本文参考

* [iOS 8的PresentationController](http://www.cocoachina.com/industry/20140707/9053.html)
* [UIPresentationController Class Reference](https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIPresentationController_class/)
* [Customizing the Transition Animations](https://developer.apple.com/library/prerelease/content/featuredarticles/ViewControllerPGforiPhoneOS/CustomizingtheTransitionAnimations.html)

