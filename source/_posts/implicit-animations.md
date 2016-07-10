title: 隐式动画
date: 2016-06-26 23:38:26
tags: Animation
categories: iOS
---

根据我粗浅的理解，「隐式动画」中的所谓「隐式」，是相对于「显式动画」中的显式而言的。

实现显式动画时，往往会创建一个动画对象，譬如`CAAnimation`、`CABasicAnimation`、`CAKeyframeAnimation`，然后通过`CALayer#addAnimation(_:forKey:)`方法该动画对象绑定到layer中，简单来说，我们所选的动画类型是确定的。

>P.S: 关于显式动画更多内容详见[这里](../explicit-animations/)。

而实现隐式动画时，无需指定动画类型，仅仅改变了一个属性，然后Core Animation来决定如何并且何时去做动画。

## 事务

隐式动画中有一个「事务」的概念。

**事务**（transaction）实际上是Core Animation用来包含一系列属性动画集合的机制，用指定事务去改变可以做动画的图层属性，不会立刻发生变化，而是提交事务时用一个动画过渡到新值。

Core Animation中的事务通过`CATransaction`类来做管理，这个类有些奇怪，它没有属性或实例方法，并且也不能创建实例，但可以用类方法`begin()`或`commit()`分别来入栈或出栈。

### 使用CATransaction

`CATransaction`没有任何实例方法，只有类型方法。`CATransaction.begin()`和`CATransaction.commit()`构成了一个**动画块**：

```swift
CATransaction.begin()
/* animation block */
CATransaction.commit()
```

在**动画块**中配置动画（譬如指定duration）、设置animated properties，如下是一个简单的应用示例（平移动画）：

```swift
CATransaction.begin()
CATransaction.setAnimationDuration(1.0) // duration = 1.0
targetLayer.transform = CATransform3DMakeTranslation(0, 150, 0)
CATransaction.commit()
```

除了`begin()`和`commit()`，`CATransaction`还有一些其他的类型方法：

```swift
func animationDuration() -> CFTimeInterval  // get duration, defaults to 0.25s
func setAnimationDuration(dur: CFTimeInterval)  // set duration

func animationTimingFunction() -> CAMediaTimingFunction?  // get timing function
func setAnimationTimingFunction(function: CAMediaTimingFunction?)  // set timing function

func disableActions() -> Bool  // get disable actions state
func setDisableActions(flag: Bool)  // set disable actions state

func completionBlock() -> (() -> Void)?  // get completion block
func setCompletionBlock(block: (() -> Void)?)  // set completion block
```

共有4组可配置项：duration、timing function、disable actions、completion block。如上的4组getters/setters可以用如下两个类型方法代替：

```swift
func valueForKey(key: String) -> AnyObject?
func setValue(anObject: AnyObject?, forKey key: String)

// key的可选值共有4种：
// 1. kCATransactionAnimationDuration, duration
// 2. kCATransactionAnimationTimingFunction, timing function
// 3. kCATransactionDisableActions, disable actions
// 4. kCATransactionCompletionBlock, completion block
```

需要注意的是：

* `CATransaction`没办法配置delay、repeat count等；
* `CATransaction`动画块只能处理`CALayer`相关动画，无法正确处理`UIView`的动画，甚至`UIView#layer`（与`UIView`相关联的`CALayer`）也不行；

既然`CATransaction`只能处理`CALayer`相关动画，`UIView`的隐式动画怎么实现？

对应`CATransaction.begin()`和`CATransaction.commit()`，`UIView`也有两个类型方法：`beginAnimations(_:context:)`和`UIView.commitAnimations()`。换句话说，这两个方法一前一后也构成了一个针对`UIView`的动画块：

```swift
UIView.beginAnimations(nil, context: nil)
/* animation block */
UIView.commitAnimations()
```

可以通过`UIView.setAnimationDuration()`等方法对动画进行配置，值得一提的是，相对于`CATransaction.setXXX`，`UIView.setAnimationXXX`要更丰富一些，可以额外配置delay、repeat count等。

在iOS 4中，苹果对`UIView`添加了一种基于block的动画方法，即`UIView.animateXXX`系列方法，在做一堆的属性动画时，这些方法在语法上会更加简单，但实质上它们都是在做同样的事情。

## 图层行为

这一部分着重解释`UIView#layer`动画为什么会在`CATransaction`动画块中失效。这一部分内容几乎拷贝自[图层行为](https://zsisme.gitbooks.io/ios-/content/chapter7/layer-actions.html)。

先来做个试验，尝试直接对`UIView`关联的图层（`UIView#layer`）而不是一个单独的图层（`CALayer`）做动画：

```objc
@interface ViewController ()

@property (nonatomic, weak) IBOutlet UIView *layerView;

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    //set the color of our layerView backing layer directly
    self.layerView.layer.backgroundColor = [UIColor blueColor].CGColor;
}

- (IBAction)changeColor
{
    //begin a new transaction
    [CATransaction begin];
    //set the animation duration to 1 second
    [CATransaction setAnimationDuration:1.0];
    //randomize the layer background color
    CGFloat red = arc4random() / (CGFloat)INT_MAX;
    CGFloat green = arc4random() / (CGFloat)INT_MAX;
    CGFloat blue = arc4random() / (CGFloat)INT_MAX;
    self.layerView.layer.backgroundColor = [UIColor colorWithRed:red green:green blue:blue alpha:1.0].CGColor;
    //commit the transaction
    [CATransaction commit];
}
```

运行程序，你会发现当按下按钮，图层颜色瞬间切换到新的值，而不是之前平滑过渡的动画。发生了什么呢？隐式动画好像被`UIView`关联图层给禁用了。

试想一下，如果`UIView`的属性都有动画特性的话，那么无论在什么时候修改它，我们都应该能注意到的。所以，如果说`UIKit`建立在Core Animation（默认对所有东西都做动画）之上，那么隐式动画是如何被`UIKit`禁用掉呢？

我们知道Core Animation通常对`CALayer`的所有属性（可动画的属性）做动画，但是`UIView`把它关联的图层的这个特性关闭了。为了更好说明这一点，我们需要知道隐式动画是如何实现的。

我们把改变属性时CALayer自动应用的动画称作行为，当`CALayer`的属性被修改时候，它会调用`-actionForKey:`方法，传递属性的名称。剩下的操作都在`CALayer`的头文件中有详细的说明，实质上是如下几步：

* 图层首先检测它是否有委托，并且是否实现`CALayerDelegate`协议指定的`-actionForLayer:forKey:`方法。如果有，直接调用并返回结果。
* 如果没有委托，或者委托没有实现`-actionForLayer:forKey:`方法，图层接着检查包含属性名称对应行为映射的actions字典。
* 如果actions字典没有包含对应的属性，那么图层接着在它的style字典接着搜索属性名。
* 最后，如果在style里面也找不到对应的行为，那么图层将会直接调用定义了每个属性的标准行为的`-defaultActionForKey:`方法。

所以一轮完整的搜索结束之后，`-actionForKey:`要么返回空（这种情况下将不会有动画发生），要么是`CAAction`协议对应的对象，最后`CALayer`拿这个结果去对先前和当前的值做动画。

于是这就解释了`UIKit`是如何禁用隐式动画的：每个UIView对它关联的图层都扮演了一个委托，并且提供了`-actionForLayer:forKey:`的实现方法。当不在一个动画块的实现中，`UIView`对所有图层行为返回`nil`，但是在动画block范围之内，它就返回了一个非空值。我们可以用一个demo做个简单的实验：

```objc
@interface ViewController ()

@property (nonatomic, weak) IBOutlet UIView *layerView;

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    //test layer action when outside of animation block
    NSLog(@"Outside: %@", [self.layerView actionForLayer:self.layerView.layer forKey:@"backgroundColor"]);
    //begin animation block
    [UIView beginAnimations:nil context:nil];
    //test layer action when inside of animation block
    NSLog(@"Inside: %@", [self.layerView actionForLayer:self.layerView.layer forKey:@"backgroundColor"]);
    //end animation block
    [UIView commitAnimations];
}

@end
```

运行程序，控制台显示结果如下：

```txt
$ LayerTest[21215:c07] Outside: <null>
$ LayerTest[21215:c07] Inside: <CABasicAnimation: 0x757f090>
```

于是我们可以预言，当属性在动画块之外发生改变，`UIView`直接通过返回`nil`来禁用隐式动画。但如果在动画块范围之内，根据动画具体类型返回相应的属性，在这个例子就是`CABasicAnimation`。

当然返回`nil`并不是禁用隐式动画唯一的办法，`CATransacition`有个方法叫做`+setDisableActions:`，可以用来对所有属性打开或者关闭隐式动画。

总结一下，我们知道了如下几点：

* `UIView`关联的图层禁用了隐式动画，对这种图层做动画的唯一办法就是使用`UIView`的动画函数（而不是依赖`CATransaction`），或者继承`UIView`，并覆盖`-actionForLayer:forKey:`方法，或者直接创建一个显式动画。
* 对于单独存在的图层，我们可以通过实现图层的`-actionForLayer:forKey:`委托方法，或者提供一个actions字典来控制隐式动画。

如上文所述，`UIView`和`CALayer`处理隐式动画的逻辑不一样，那么如何同时实现它们俩的动画呢？这个问题并不复杂，懒得叙述了，直接参考：

* [How to synchronously animate a UIView and a CALayer](http://stackoverflow.com/questions/10801414/how-to-synchronously-animate-a-uiview-and-a-calayer)
* [Mixing UIView and CALayer animations](http://khanlou.com/2012/10/mixing-uiview-and-calayer-animations/)
* [Animating View and Layer Changes Together](https://developer.apple.com/library/ios/documentation/WindowsViews/Conceptual/ViewPG_iPhoneOS/AnimatingViews/AnimatingViews.html#//apple_ref/doc/uid/TP40009503-CH6-SW12)

## UIView.animateXXX汇总

```swift
func animateWithDuration(duration: NSTimeInterval,
                         animations: () -> Void)
                         
func animateWithDuration(duration: NSTimeInterval,
                         animations: () -> Void,
                         completion: ((Bool) -> Void)?)

func animateWithDuration(duration: NSTimeInterval,
                         delay: NSTimeInterval,
                         options: UIViewAnimationOptions,
                         animations: () -> Void,
                         completion: ((Bool) -> Void)?)

func animateWithDuration(duration: NSTimeInterval,
                         delay: NSTimeInterval,
                         usingSpringWithDamping dampingRatio: CGFloat,
                         initialSpringVelocity velocity: CGFloat,
                         options: UIViewAnimationOptions,
                         animations: () -> Void,
                         completion: ((Bool) -> Void)?)

func transitionWithView(view: UIView,
                        duration: NSTimeInterval,
                        options: UIViewAnimationOptions,
                        animations: (() -> Void)?,
                        completion: ((Bool) -> Void)?)

func transitionFromView(fromView: UIView,
                        toView: UIView,
                        duration: NSTimeInterval,
                        options: UIViewAnimationOptions,
                        completion: ((Bool) -> Void)?)

func performSystemAnimation(animation: UISystemAnimation,
                            onViews views: [UIView],
                            options: UIViewAnimationOptions,
                            animations parallelAnimations: (() -> Void)?,
                            completion: ((Bool) -> Void)?)
```

```swift
/* 关键帧动画 */
func animateKeyframesWithDuration(duration: NSTimeInterval,
                                  delay: NSTimeInterval,
                                  options: UIViewKeyframeAnimationOptions,
                                  animations: () -> Void,
                                  completion: ((Bool) -> Void)?)

func addKeyframeWithRelativeStartTime(frameStartTime: Double,
                                      relativeDuration frameDuration: Double,
                                      animations: () -> Void)
```

如上这些API能够做到的事情，使用Core Animation的显式动画（基于`CAAnimation`）都能做到。