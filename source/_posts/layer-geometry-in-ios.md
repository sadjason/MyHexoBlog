title: iOS图层几何学
date: 2016-06-13 19:41:36
tags: Animation
categories: iOS
---

## frame v.s bounds v.s center

视图（`UIView`）有三种比较重要的布局属性：`frame`，`bounds`，`center`，图层（`CALayer`）对应叫做：`frame`，`bounds`，`position`。众所周知，视图是对图层的包装，名称虽然不完全相同，但都是指向相同的值，本文中`position`和`center`表示一个意思。

`bounds`和`position`是存储型属性，`bounds`标识了两个重要讯息：自身坐标系的起点（一般`(0, 0)`对应左上角）和图层的固有size；而`position`标识了图层中心点相对于父图层坐标系的位置。

`frame`是计算型属性（虚拟属性），它的值根据`bounds`、`position`和`transform`计算而来。

>Note: 这里的`transform`指的是图层（`CALayer`）的`transform`属性，而不是视图的`transform`属性。`UIView#transform`是`CATransform3D`类型，而`CALayer#transform`是`CGAffineTransform`类型，虽然类型不同，但根据我的理解，视图的`transform`和其图层的`transform`直接相关，换句话说，`UIView#transform`是`CALayer#transform`的简化版。

当`CALayer#transform`的值等于`CATransform3DIdentity`，或者当`UIView#transform`的值等于`CGAffineTransformIdentity`时，`frame`、`position`和`bounds`存在着如下关系：

* frame.origin = position - (bounds.size / 2.0)
* frame.size = bounds.size

在这种情况下：

* `position`和`bounds`彼此独立，改变任意一个值不会影响另外一个；
* `frame.size`和`bounds.size`直接相关，修改任何一个会影响另外一个；
* `frame.origin`和`position`直接相关，修改任何一个会影响另外一个；
* `bounds.origin`不会对`frame`和`center`产生影响，但会影响到子图层的位置；

当视图发生仿射变换时，比如平移、旋转、缩放，经验证发现：`position`和`bounds`都不会改变，而`frame`会改变。并且当对图层进行缩放时，`frame.size`会和`bounds.size`不一致，详细说明参考[《iOS核心动画高级技巧》](https://zsisme.gitbooks.io/ios-/content/chapter3/layout.html)。

>P.S: 还没测试图层3D变换时`bounds`和`position`的变化情况，但我估计结果是一样的。

我认为：把视图和图层的`frame`属性拿掉，iOS布局系统也能照样运转，`frame`的作用不过是为了迎合大部分人的布局惯性思维罢了。

## anchorPoint

按理说，图层的`position`和`bounds.size`就足以告诉布局系统它在父图层中所占据的位置和面积了，前者指示了其**中心点**的坐标，后者指示了其大小。

但问题出在**中心点**上，图层的`position`之所以叫`position`而不叫`center`是有原因的。

可以想象图层和很多其他现实物体一样，有一个*把柄*，布局系统在摆放图层时，直接“拖”着这个*把柄*，把它杵到`position`对应的坐标上即可。在大多数时候，这个*把柄*所在的位置就是图层的中心点。然而，iOS允许自定义图层的*把柄*位置，修改`CALayer#anchorPoint`值即可，`anchorPoint`是`CGPoint`类型，其默认值为`(0.5, 0.5)`（归一化值），当修改其值为`(0, 0)`，这意味着图层的*把柄*被移到左上角。换句话说，图层的`anchorPoint`属性值会影响到它的摆放位置，但该值和`position`、`bounds`彼此独立，没有任何关系，和`frame`更是谈不上啥关系了。

在什么时候需要修改`anchorPoint`属性值呢？[《ios核心动画高级技巧》](https://zsisme.gitbooks.io/ios-/content/chapter3/anchor.html)有一个非常不错的应用实例。

`anchorPoint`所对应的位置，常常被称为图层的**锚点**。

更值得一提的是，视图的仿射变换和图层的3D变换，都是围绕图层的锚点进行的。

具体来说，当`anchorPoint`的值为默认值时，对视图进行缩放或旋转，其中心点都不会发生迁移，如下：

<div class="imagediv" style="width: 316px; height: 142px">{% asset_img default-anchor-point.gif Default Anchor Point %}</div>

当修改两个view的`anchorPoint`属性值为`(0.0, 0.0)`后，结果如下：

<div class="imagediv" style="width: 318px; height: 126px">{% asset_img new-anchor-point.gif New Anchor Point %}</div>

显然，此时视图的缩放和旋转都以左上角为基准点。

## Z坐标轴

`UIView`使用的是严格的二维坐标系，而`CALayer`存在于一个三维空间中。

## Hit Testing

在什么情况下需要使用hit testing？
未完待续。

## 本文参考

* [UIView frame, bounds and center](http://stackoverflow.com/questions/5361369/uiview-frame-bounds-and-center)
* [iOS核心动画高级技巧](https://zsisme.gitbooks.io/ios-/content/)