title: IBInspectable和IBDesignable
date: 2015-06-22 10:50:45
tags: Xcode
categories: iOS
---

## 写在前面

第一次遇到`IBInspectable`是在[RESideMenu](https://github.com/romaonthego/RESideMenu)中，RESideMenu是github上非常著名的一个开源库，主要是实现侧滑菜单，到今天已经有四千多stars了；关于RESideMenu的更多信息，可以移步到github中去查找。

在使用RESideMenu的过程中，看到这样的场景：

<div class="imagediv" style="width: 390px; height: 450px">{% asset_img 20150704-10.png %}</div>

当时就勾起了我的好奇心，因为我早就想着Apple啥时候能让自定义View或ViewController能够在Interface Builder中进行配置（编辑属性）。

后来查看RESideMenu.h文件，看到一个从未见过的关键字`IBInspectable`：

<div class="imagediv" style="width: 670px; height: 510px">{% asset_img 20150704-11.png %}</div>

问题似乎变得很清晰了，`IBInspectable`指令能够让我们在Interface Builder中编辑对应类的属性。

接着在搜索引擎中搜索`IBInspectable`关键字，才搞明白`IBInspectable`是Xcode6新引入的指令，同时引入的还有另外一个指令`IBDesignable`。

**IBInspectable和IBDesignable**

Apple的官方文档《[Using Swift With Cocoa and Objective-C](https://developer.apple.com/library/prerelease/ios/documentation/Swift/Conceptual/BuildingCocoaApps/WritingSwiftClassesWithObjective-CBehavior.html)》中有关于`IBInspectable`和`IBDesignable`的介绍，如下：

<div class="imagediv" style="width: 684px; height: 352px">{% asset_img 20150704-09.png %}</div>

简单来说，`IBInspectable`使得在Interface Builder的Attribute Inspector（属性检查器）中能够查看类的属性，而`IBDesignable`能实时更新视图，很厉害吧！

P.S: inspectable表示「可视的」。

下面将以实际例子更加生动说明`IBInspectable`和`IBDesignable`的使用方法和效果。

**使用IBInspectable**

举个栗子，定义一个继承自UIView的类CustomView，如下：

```objc
@interface CustomView : UIView
    
@property (nonatomic, assign) IBInspectable CGFloat customWidth;
@property (nonatomic, assign) IBInspectable CGFloat customHeight;
@property (nonatomic, strong) IBInspectable NSString *customTitle;
@property (nonatomic, strong) IBInspectable UIColor *customColor;
@property (nonatomic, assign) IBInspectable BOOL customHidden;
    
@end
```

再在Interface Builder中创建一个View，该View的Custom Class指向到刚刚创建的类Custom View，在其Attributes Inspector中果然能够实时看到如下信息：

<div class="imagediv" style="width: 304px; height: 143px">{% asset_img 20150704-12.png %}</div>

这使得我们能够添加一些自定义的运行时属性，这些属性将会在view加载时设置它的初始值。

<div class="imagediv" style="width: 304px; height: 151px">{% asset_img 20150704-13.png %}</div>

**使用IBDesignable**

上文已经讲过：`IBDesignable`能实时更新视图。具体是怎么回事儿呢？

关于`IBDesignable`的内容摘自[IBInspectable/IBDesignable]（http://www.cocoachina.com/ios/20150227/11202.html）

当应用到UIView或NSView子类中的时候，IBDesignable让Interface Builder知道它应该在画布上直接渲染视图。你会看到你的自定义视图在每次更改后不必编译并运行你的应用程序就会显示。

标记一个自定义视图为Designable，只需在类名前加上IBDesignable的前缀（Objective-C中加上IB_DESIGNABLE）。你的初始化、布置和绘制方法将被用来在画布上渲染你的自定义视图：

```objc
IB_DESIGNABLE
@interface CustomView : UIView
    ...
@end
```

<div class="imagediv" style="width: 452px; height: 148px">{% asset_img 20150704-14.png %}</div>

从这个功能上节约的时间是不能被低估的。加上`IBInspectable`，一个设计师或开发人员可以轻松地调整自定义控件的呈现，以得到她想要的确切的结果。任何改变，无论是从代码或属性检查器中，都将立即呈现在画布上。

由于在Interface Builder中呈现自定义视图不会有应用程序的完整上下文，你可能需要生成模拟数据以便显示，例如一个默认用户头像图片或仿制的天气数据。有两种方法可以为这个特殊的上下文添加代码。

第一种是使用`prepareForInterfaceBuilder()`方法。
`prepareForInterfaceBuilder()`方法与你代码的其余部分一起编译，但只有当视图正在准备在Interface Builder显示时执行。

第二种是使用`#if TARGET_INTERFACE_BUILDER`宏。
`#if TARGET_INTERFACE_BUILDER`预处理宏在Objective-C或Swift下都是工作的，它会视情况编译正确代码：

```objc
#if !TARGET_INTERFACE_BUILDER
    // this code will run in the app itself
#else
    // this code will execute only in IB
#endif
```

**IBDesignable和IBInspectable结合使用**

把自定义IBDesignable视图和视图里的IBInspectable属性结合在一起，你能干点啥？作为一个例子，让我们更新老式经典[Apple folklore](http://www.folklore.org/StoryView.py?story=Calculator_Construction_Set.txt)：在“Steve Jobs Roll Your Own Calculator Construction Set”，Xcode 6的风格：

<div class="imagediv" style="width: 445px; height: 460px">{% asset_img 20150704-15.gif %}</div>

## 写在最后

介绍`IBDesignable`的内容完全摘自《[IBInspectable/IBDesignable](http://www.cocoachina.com/ios/20150227/11202.html)》，还不是非常理解，至少对`IBDesignable`的强大功能的体会还不够深，希望以后能够补充更多的示例说明。

最后，再次让我感觉到iOS开发不是我想象中那么简单的，我甚至连很多工具都没使用好，路漫漫其修远兮！

## 本文参考

* [如何在iOS 8中使用Swift和Xcode 6制作精美的UI组件](http://www.cocoachina.com/industry/20140619/8883.html)
* [How to make awesome UI components in iOS 8 using Swift and XCode 6](http://www.weheartswift.com/make-awesome-ui-components-ios-8-using-swift-xcode-6/)
* [IBInspectable/IBDesignable](http://www.cocoachina.com/ios/20150227/11202.html)