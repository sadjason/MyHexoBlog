title: 使用UIScrollView
date: 2016-05-21 13:41:28
tags: UIScrollView
categories: iOS
---

## UIScrollViewDelegate

简单介绍一下UIScrollViewDelegate定义的一些方法，这里只涉及与scroll相关的方法，zoom相关的方法略过不表。

* `scrollViewDidScroll(_:)`

这个方法在任何方式触发`contentOffset`变化的时候都会被调用（包括用户拖动，减速过程，直接通过代码设置等），可以用于监控`contentOffset`的变化，并根据当前的`contentOffset`对其他view做出随动调整。

* `scrollViewWillBeginDragging(_:)`

用户开始拖动scroll view的时候被调用。

* `scrollViewWillEndDragging(_:withVelocity:targetContentOffset:)`

这个方法是iOS 5之后引入的，可在它的基础上实现更为丰富的自定义paging。

* `scrollViewDidEndDragging(_:willDecelerate:)`

在用户结束拖动后被调用，`willDecelerate`为YES时，结束拖动后会有减速过程（惯性）。

* `scrollViewWillBeginDecelerating(_:)`

减速动画开始前被调用。

* `scrollViewDidEndDecelerating(_:)`

减速动画结束时被调用，这里有一种特殊情况：当一次减速动画尚未结束的时候再次drag scroll view，该方法不会被调用，并且这时scroll view的`dragging`和`decelerating`属性都是`YES`。新的dragging如果有加速度，那么`scrollViewWillBeginDecelerating(_:)`会再一次被调用，然后才是`scrollViewDidEndDecelerating(_:)`；如果没有加速度，虽然`scrollViewWillBeginDecelerating(_:)`不会被调用，但前一次留下的`scrollViewDidEndDecelerating(_:)`会被调用。

>P.S: 以上的解释比较拗口，动手体验一下就明白了。

* `scrollViewDidEndScrollingAnimation(_:)`

当通过代码滑动scroll view时（`setContentOffset(_:animated:)`或者`scrollRectVisible(_:animated:)`），动画结束时会调用该方法；值得一提的是，当且仅当动画存在时才会调用该方法。

## Scrolling With Finger

我总结了一些手指控制UIScrollView滑动的场景，将相关的回调给列出来了。

**场景一：一次正常的滑动-减速-停止**

```swift
// 手指介入
scrollViewWillBeginDragging(_:)
// 开始滑动
scrollViewDidScroll(_:)                     // scrolling...
scrollViewDidScroll(_:)                     // scrolling...
// ...

// 手指离开屏幕（scrolling继续，并未停滞）
scrollViewWillEndDragging(scrollView:withVelocity:targetContentOffset:)
scrollViewDidEndDragging(_:willDecelerate:) // 拖拽结束（willDecelerate=true）
scrollViewWillBeginDecelerating(_:)         // 减速
scrollViewDidScroll(_:)                     // scrolling...
scrollViewDidScroll(_:)                     // scrolling...
// ...

scrollViewDidEndDecelerating(_:)            // 减速结束
```

**场景二：减速过程中的手指介入（点停）**

```swift
// 手指介入
scrollViewWillBeginDragging(_:)
// 开始滑动
scrollViewDidScroll(_:)             // scrolling...
scrollViewDidScroll(_:)             // scrolling...
// ...

// 手指离开屏幕（scrolling继续，并未停滞）
scrollViewWillEndDragging(scrollView:withVelocity:targetContentOffset:)
scrollViewDidEndDragging(_:willDecelerate:) // 拖拽结束（willDecelerate=true）
scrollViewWillBeginDecelerating(_:)         // 减速
scrollViewDidScroll(_:)             // scrolling...
scrollViewDidScroll(_:)             // scrolling...
// ...

// 手指介入（scrolling停止 -- 之前的操作被取消）
scrollViewWillBeginDragging(_:)
// 手指离开屏幕
scrollViewWillEndDragging(scrollView:withVelocity:targetContentOffset:)
scrollViewDidEndDragging(_:willDecelerate:)     // 拖拽结束（willDecelerate=false）
scrollViewDidEndDecelerating(_:)                // 减速结束
```

**场景三：减速过程中的手指介入（乱滑）**

```swift
// 手指介入
scrollViewWillBeginDragging(_:)
// 开始滑动
scrollViewDidScroll(_:)             // scrolling...
scrollViewDidScroll(_:)             // scrolling...
// ...

// 手指离开屏幕（scrolling继续，并未停滞）
scrollViewWillEndDragging(scrollView:withVelocity:targetContentOffset:)
scrollViewDidEndDragging(_:willDecelerate:) // 拖拽结束（willDecelerate=true）
scrollViewWillBeginDecelerating(_:)         // 减速
scrollViewDidScroll(_:)             // scrolling...
scrollViewDidScroll(_:)             // scrolling...
// ...

// 手指介入--重新拖拽（scrolling停止 -- 之前的操作被取消）
scrollViewWillBeginDragging(_:)
// 开始滑动
scrollViewDidScroll(_:)             // scrolling...
scrollViewDidScroll(_:)             // scrolling...
// ...

// 手指离开屏幕（scrolling继续，并未停滞）
scrollViewWillEndDragging(scrollView:withVelocity:targetContentOffset:)
scrollViewDidEndDragging(_:willDecelerate:) // 拖拽结束（willDecelerate=true）
scrollViewWillBeginDecelerating(_:)         // 减速
scrollViewDidScroll(_:)             // scrolling...
scrollViewDidScroll(_:)             // scrolling...
// ...

// 手指介入--点停（scrolling停止 -- 之前的操作被取消）
scrollViewWillBeginDragging(_:)

// 手指离开屏幕
scrollViewWillEndDragging(scrollView:withVelocity:targetContentOffset:)
scrollViewDidEndDragging(_:willDecelerate:)     // 拖拽结束（willDecelerate=false）
scrollViewDidEndDecelerating(_:)                // 减速结束
```

## Scrolling Programmatically

**Scrolling to a Specific Offset**

可通过设置content offset让scrollView滚动到某个指定位置，有两种方式：一是直接给`contentOffset`属性赋值，二是调用`setContentOffset(_:animated:)`方法。对于第二种，当`animated`参数设置为NO，等效于第一种方式。

无论是哪一种情况，`delegate`都会接收到`scrollViewDidScroll(_:)`消息。

只是当禁止动画时，`delegate`只会收到一次`scrollViewDidScroll(_:)`消息；当允许动画时，`delegate`会在动画过程中接收到一系列`scrollViewDidScroll(_:)`消息，并且当动画结束时，`delegate`还会接收到`scrollViewDidEndScrollingAnimation(_:)`消息。

**Making a rectangle visible**

还可以调用`scrollRectToVisible(_:animated:)`方法让scrollView滚动到一个合适的位置以使指定区域可见，`animated`参数依然是用来控制动画开关的。当关闭动画时，`delegate`会接收到一次`scrollViewDidScroll(_:)`消息；当允许动画时，`delegate`会在动画过程中接收到一系列`scrollViewDidScroll(_:)`消息，并且当动画结束时，`delegate`还会接收到`scrollViewDidEndScrollingAnimation(_:)`消息。

**Scroll To Top**

`UITableView`等子类还实现了Scroll To Top功能，这里不赘述了，详细参考[这里](https://developer.apple.com/library/ios/documentation/WindowsViews/Conceptual/UIScrollView_pg/ScrollingViewContent/ScrollingViewContent.html)。

## Tracking The Start and Completion Of A Scroll Action

Implement the scrollViewWillBeginDragging: method to receive notification that dragging will begin.

当编程实现滚动（设置`contentOffset`、）时，`scrollViewWillBeginDragging:`也会被调用吗？

To determine when scrolling is complete you must implement two delegate methods: scrollViewDidEndDragging:willDecelerate: and scrollViewDidEndDecelerating:. Scrolling is completed either when the delegate receives the scrollViewDidEndDragging:willDecelerate: message with NO as the decelerate parameter, or when your delegate receives the scrollViewDidEndDecelerating: method. In either case, scrolling is complete.

**编程实现滚动时抗干扰**

在开始和结束时设置`userInteractionEnabled`属性即可：

```swift
scrollView.userInteractionEnabled = false
scrollView.setContentOffset(CGPointZero, animated: true)

func scrollViewDidEndScrollingAnimation(scrollView: UIScrollView) {
  scrollView.userInteractionEnabled = true
}
```

## 经验之谈

**不可描述的问题**

一般来说，在UIScrollView静止状态下，点击（只是轻点一下）不会触发UIScrollViewDelegate相关的回调，但曾在开发中遇到了一件诡异的事情，点击静止状态下的UIScrollView会触发`scrollViewWillBeginDecelerating(_:)`和`scrollViewDidEndDecelerating(_:)`回调，代码如下：

```objc
- (void)viewDidLoad {
  [super viewDidLoad];
  
  self.view.backgroundColor = [UIColor whiteColor];
  
  UIScrollView *scrollView = [[UIScrollView alloc] initWithFrame:CGRectMake(10, 30, 100, 200)];
  scrollView.backgroundColor = [UIColor lightGrayColor];
  scrollView.bounces = NO;
  scrollView.contentSize = CGSizeMake(160, 200);
  scrollView.pagingEnabled = YES;
  scrollView.showsHorizontalScrollIndicator = NO;
  scrollView.delegate = self;
  
  [self.view addSubview:scrollView];
  
  UIView *view1 = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 60, 200)];
  view1.backgroundColor = [UIColor redColor];
  [scrollView addSubview:view1];
  
  UIView *view2 = [[UIView alloc] initWithFrame:CGRectMake(60, 0, 100, 200)];
  view2.backgroundColor = [UIColor greenColor];
  [scrollView addSubview:view2];
}
```

排查了好久，最终发现问题出在`scrollView.pagingEnabled = YES`这里，以上的scroll view允许paging，然而，其`contentSize.width`并不是`size.width`的整数倍，这会让它显式最后一页时没办法达到稳定状态，该情况下点击UIScrollView会触发`scrollViewWillBeginDecelerating(_:)`和`scrollViewDidEndDecelerating(_:)`。该情景不太容易描述...

**UIScrollView Custom Paging**

通过回调`scrollViewWillEndDragging(_:withVelocity:targetContentOffset:)`可以在`UIScrollView#pagingEnabled`设为`false`的情况下自己实现paging效果。然而，用户体验并不好，至少不如`UIScrollView#pagingEnabled = true`顺畅，参考[这里](http://stackoverflow.com/questions/6945964/uiscrollview-custom-paging)，不晓得有没什么办法改善操作体验。


## 本文参考

* [Scroll View Programming Guide for iOS](https://developer.apple.com/library/ios/documentation/WindowsViews/Conceptual/UIScrollView_pg/ScrollingViewContent/ScrollingViewContent.html)
* [UIScrollView实践经验](http://tech.glowing.com/cn/practice-in-uiscrollview/)