title: 深入理解Auto Layout 第一弹
date: 2015-07-16 09:10:31
tags: Auto Layout
categories: iOS

---

## 写在前面

iOS的的布局机制「auto layout」不是一个新概念，它早在iOS 6中就推出来了，当下距离iOS 9正式版面世已不远矣，我却对它的了解还比较初级。

我之前对「auto layout」机制的理解非常粗浅，几乎把它和「constraint」对等。对它的使用有这么几个阶段：
1. Storyboard中通过拖拽设置constraints；
2. 学习VFL（Visual Format Language）语法使用代码设置constraints；
3. 使用大杀器[Masonry](https://github.com/SnapKit/Masonry)。

>P.S: iOS的VFL语法实在太罗嗦了，又臭又长且可读性差难于调试，无法忍受，Masonry正是解决这一痛点的第三方库，非常好用，学习成本非常低。

近期因为项目，我需要实现一个能够自适应文本自动调整高度的table view cell。网上有相关丰富的资源（category、博客等），思路都非常简单，比如这篇：[动态计算UITableViewCell高度详解](http://www.cocoachina.com/industry/20140604/8668.html)。但即便如此，我对有些东西理解起来还有障碍，譬如`systemLayoutSizeFittingSize:`和`sizeThatFits:`之类的，想到我都将近有一年的开发经验了，居然还无法「理解」这么简单的东西，不能忍！

导致这种结果的主要原因有俩：一是之前项目比较简单，涉及auto layout相关的知识无非是add/update/remove constraints；二是自己太轻浮，把auto layout想得太简单；

通过对各种资讯梳理，大概搞明白了自己最大的问题是对auto layout相关的各种API不熟悉或者完全陌生，这导致了无法在一些实际问题中使用正确的策略解决问题。本文的着重点正是结合各种资料加上自己的理解对这些API进行分析。
本文涉及的API包括：

* `sizeThatFits:`和`sizeToFit`
* `systemLayoutSizeFittingSize:`
* `intrinsicContentSize`

>P.S: 这几个API都是与size相关，该如何使用它们呢？这曾让笔者一时非常困惑。以上这几个API都是UIView的实例方法，除此之外，本文还涉及一些属性，譬如`preferredMaxLayoutWidth`。


## iOS布局机制

iOS布局机制大概分这么几个层次：

* frame layout
* autoresizing
* auto layout

**frame layout**

frame layout最简单直接，简单来说，即通过设置view的`frame`属性值进而控制view的位置（相对于superview的位置）和大小。

**autoresizing**

autoresizing和frame layout一样，从一开始存在，它算是后者的补充，基于autoresizing机制，能够让subview和superview维持一定的布局关系，譬如让subview的大小适应superview的大小，随着后者的改变而改变。

站在代码接口的角度来看，autoresizing主要体现在几个属性上，包括（但不限于）：

1. `translatesAutoresizingMaskIntoConstraints`
2. `autoresizingMask`

第一个属性标识view是否愿意被autoresize；
第二个属性是一个枚举值，决定了当superview的size改变时，subview应该做出什么样的调整；

关于autoresizing的更详细使用说明，参考：[自动布局之autoresizingMask使用详解](http://www.cocoachina.com/ios/20141216/10652.html)。

**auto layout**

autoresizing存在的不足是非常显著的，通过`autoresizingMask`的可选枚举值可以看出：基于autoresizing机制，我们只能让view在superview的大小改变时做些调整；而无法处理兄弟view之间的关系，譬如处理与兄弟view的间隔；更无法反向处理，譬如让superview依据subview的大小进行调整。

Auto Layout是随着iOS 6推出来的，关于它的介绍，官方文档《Auto Layout Guide》的描述非常精炼：
>Auto Layout is a system that lets you lay out your app’s user interface by creating a mathematical description of the relationships between the elements. You define these relationships in terms of constraints either on individual elements, or between sets of elements. Using Auto Layout, you can create a dynamic and versatile interface that responds appropriately to changes in screen size, device orientation, and localization.

简单来说，它是一种基于约束的布局系统，可以根据你在元素（对象）上设置的约束自动调整元素（对象）的位置和大小。

值得一提的是，对于某个view的布局方式，autoresizing和auto layout只能二选一，简单来说，若要对某个view采用auto layout布局，则需要设置其`translatesAutoresizingMaskIntoConstraints`属性值为`NO`。

## 几个重要的API

**intrinsicContentSize方法**

在介绍`intrinsicContentSize`方法之前，先来看一个应用场景：

场景一：某个UILabel用于显示**单行**文本，让其能够自适应文本，即根据文本自动调整其大小。

让UILabel自适应文本，在auto layout之前，一般做法是先给定字体，进而计算文本内容所占据的宽度width和高度height，然后使用得来的width和height设置其`frame`属性值。

但是使用auto layout非常简单，如下：

```objc
@interface ViewController () {
    UILabel *testLabel;
}
    
- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.view.backgroundColor = [UIColor whiteColor];
    
    testLabel = ({
        UILabel *label        = [[UILabel alloc] init];
        label.textAlignment   = NSTextAlignmentCenter;
        label.font            = [UIFont systemFontOfSize:14.0];
        label.textColor       = [UIColor whiteColor];
        label.backgroundColor = [UIColor lightGrayColor];
        label;
    });
    [self.view addSubview:testLabel];
    
    // 使用Masonry添加constraints
    [testLabel mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(self.view.mas_top).offset(40);
        make.left.equalTo(self.view.mas_left).offset(10);
    }];
    testLabel.text = @"天地玄黄 宇宙洪荒 日月盈昃 辰宿列张";
}
    
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    NSLog(@"testLabel.origin = %@", NSStringFromCGPoint(testLabel.frame.origin));
    NSLog(@"testLabel.size = %@", NSStringFromCGSize(testLabel.frame.size));
    // print: "testLabel.origin = {10, 40}"
    // print: "testLabel.size = {236.5, 17}"
}
```

效果如下：

<div class="imagediv" style="width: 320px; height: 100px">{% asset_img 20150716-01.png %}</div>

问题来了，auto layout system知道testLabel的size呢？

OK，就此引入API `intrinsicContentSize`。

`intrinsicContentSize`是UIView的基础方法（Available in iOS 6.0 and later），[UIView Class References](https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIView_Class/)对它的描述如下：
>Returns the natural size for the receiving view, considering only properties of the view itself.
&nbsp;
**Return Value**
A size indicating the natural size for the receiving view based on its intrinsic properties.
&nbsp;
**Discussion**
Custom views typically have content that they display of which the layout system is unaware. Overriding this method allows a custom view to communicate to the layout system what size it would like to be based on its content. This intrinsic size must be independent of the content frame, because there’s no way to dynamically communicate a changed width to the layout system based on a changed height, for example.
If a custom view has no intrinsic size for a given dimension, it can return UIViewNoIntrinsicMetric for that dimension.

「intrinsic content size」在中文世界里常被译作：「固有内容大小」，简单来说，**它被用来告诉auto layout system应该给它分配多大的size**。

所以呢，在上文代码（**场景一**的Solution）中，根据我的理解，layout工作流程是这样的：在layout时，auto layout system会去回调testLabel的实例方法`intrinsicContentSize`，该方法能够根据「文本内容+字体」计算出content的size，进而根据此size对testLabel进行布局。

为了验证这个说法，对上述代码做些改变。

首先定义一个继承自UILabel的类ZWLabel方法，重写ZWLabel的`intrinsicContentSize`方法，如下：

```objc
@interface ZWLabel : UILabel
    
@end
    
@implementation ZWLabel
    
- (CGSize)intrinsicContentSize {
    CGSize size = [super intrinsicContentSize];
    size.width  += 20;
    size.height += 20;
    return size;
}
    
@end
```

然后让上文的testLabel从UILabel的实例改为ZWLabel的实例，其余不变：

```objc
testLabel = ({
    ZWLabel *label = [[ZWLabel alloc] init];
    ...
    label;
});
```

效果如下：

<div class="imagediv" style="width: 320px; height: 110px">{% asset_img 20150716-02.png %}</div>

效果明显，本示例较为直观说明了`intrinsicContentSize`这个API的作用了，这个API是为auto layout system的callback提供的！

P.S：笔者刚开始对`intrinsicContentSize`这个API的用法感到非常疑惑，在我的理解里，它有两种可能：
1. Auto Layout System会根据content为view设置一个合适的size，开发者有时需要知道这个size，因此可以通过`intrinsicContentSize`获取；
2. Auto Layout System在layout时，不知道该为view分配多大的size，因此回调view的`intrinsicContentSize`方法，该方法会给auto layout system一个合适的size，system根据此size对view的大小进行设置；

现在看来，第二种理解更靠谱！

对于上文所用到的UILabel，想必Cocoa在实现的`intrinsicContentSize`方法时已经根据`text`属性值和`font`属性值进行了计算。那是不是每个view都实现了`intrinsicContentSize`呢？

NO！《Auto Layout Guide》在谈论「intrinsic content size」时，总会与另外一个词语「leaf-level views」相关联，譬如：
>**Intrinsic Content Size**
Leaf-level views such as buttons typically know more about what size they should be than does the code that is positioning them. This is communicated through the **intrinsic content size**, which tells the layout system that a view contains some content that it doesn’t natively understand, and indicates how large that content is, intrinsically.

「leaf-level views」指的是那种一般不包含任何subview的view，譬如UILabel、UIButton等，这类的view往往能够直接计算出content（譬如UILabel的text、UIButton的title，UIImageView的image）的大小。

但是有些view不包含content，譬如view，这种view被认为「has no intrinsic size」，它们的`intrinsicContentSize`返回的值是`(-1,-1)`。

>P.S: 官方文档中说的是：UIView's default implementation is to return (UIViewNoIntrinsicMetric, UIViewNoIntrinsicMetric)，而UIViewNoIntrinsicMetric等于`-1`，为什么是`-1`而不是`0`，我猜是`0`是一个有效的width/height，而`-1`不是，更容易区分处理。

还有一种view虽然包含content，但是`intrinsicContentSize`返回值也是`(-1,-1)`，这类view往往是UIScrollView的子类，譬如UITextView，它们是可滚动的，因此auto layout system在对这类view进行布局时总会存在一些未定因素，Cocoa干脆让这些view的`intrinsicContentSize`返回`(-1,-1)`。

**preferredMaxLayoutWidth属性**

基于上述**场景一**，我们来分析更复杂一点的UILabel自适应问题。

场景二：某个UILabel用于显示**多行**文本，让其能够自适应文本，即根据文本自动调整其大小；

对于单行文本UILabel，UILabel的`intrinsicContentSize`在计算content size时比较容易；但对于多行文本的UILabel，同样的content，譬如「天地玄黄宇宙洪荒」这八个字，摆放方式可以是1x8，可以是2x4，可以是4x2，auto layout system该如何处理呢？UILabel的属性`preferredMaxLayoutWidth`正是用来应对这个问题的。

[UILabel Class References](https://developer.apple.com/library/ios/documentation/UIKit/Reference/UILabel_Class/)对它的描述如下：
>The preferred maximum width (in points) for a multiline label.
**Discussion**
This property affects the size of the label when layout constraints are applied to it. During layout, if the text extends beyond the width specified by this property, the additional text is flowed to one or more new lines, thereby increasing the height of the label.

`preferredMaxLayoutWidth`的作用顾名思义，用来限制UILabel content size的最大宽度值。如下代码：

```objc
testLabel = ({
    UILabel *label                = [[UILabel alloc] init];
    label.textAlignment           = NSTextAlignmentCenter;
    label.font                    = [UIFont systemFontOfSize:14.0];
    label.textColor               = [UIColor whiteColor];
    label.numberOfLines           = 0;      // mark
    label.preferredMaxLayoutWidth = 100;    // mark
    label.backgroundColor         = [UIColor lightGrayColor];
    label;
});
[self.view addSubview:testLabel];
    
// 使用Masonry添加constraints
[testLabel mas_makeConstraints:^(MASConstraintMaker *make) {
    make.top.equalTo(self.view.mas_top).offset(40);
    make.left.equalTo(self.view.mas_left).offset(10);
}];
    
testLabel.text = @"天地玄黄 宇宙洪荒 日月盈昃 辰宿列张";
```

效果如下：

<div class="imagediv" style="width: 320px; height: 120px">{% asset_img 20150716-03.png %}</div>

那么最后testLabel的width是不是就是`preferredMaxLayoutWidth`的属性值呢？No，最终testLabel的属性值小于等于`preferredMaxLayoutWidth`的属性值。


**sizeThatFits:方法和sizeToFit方法**

上文已经提到，UITextView继承自UIScrollView，是可以滚动的，它的`intrinsicContentSize`方法返回值是`(-1,-1)`，auto layout system在处理UITextView对象时，为其设置的size是`(0,0)`。如此看来，似乎UITextView无法体会到auto layout带来的好处了。

继续结合应用场景引出`sizeThatFits:`方法和`sizeToFit`方法。

场景三：某个UITextView用于显示文本，让其能够自适应文本，即根据文本自动调整其大小；

既然UITextView的content计算方法`intrinsicContentSize`无法向auto layout system传递我们想要传达的值，我们就应该另想别的方法。

好在iOS有直接的接口可供我们使用。

先谈`sizeThatFits:`方法，[UIView Class References](https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIView_Class/)对它的描述如下：
>Asks the view to calculate and return the size that best fits the specified size.
&nbsp;
**Return Value**
A new size that fits the receiver’s subviews.
&nbsp;
**Discussion**
The default implementation of this method returns the existing size of the view. Subclasses can override this method to return a custom value based on the desired layout of any subviews. For example, a UISwitch object returns a fixed size value that represents the standard size of a switch view, and a UIImageView object returns the size of the image it is currently displaying.

简单来说，调用`sizeThatFits:`方法意味着「根据文本计算最适合UITextView的size」。从功能来讲，`sizeThatFits:`和`intrinsicContentSize`方法比较类似，都是用来计算view的size的。笔者曾一度对二者的关系非常疑惑，甚至觉得二者存在相互调用的关系。后来通过验证发现不是这么回事儿，后文会通过示例说明。

对于显示多行文本的UILabel，为了方便`intrinsicContentSize`方法更方便计算content size，需要指定`preferredMaxLayoutWidth`属性值；对于UITextView的`sizeThatFits:`，似乎有类似的需求，毕竟UITextView也可能会显示多行啊，这样说来，UITextView也有一个`preferredMaxLayoutWidth`属性？

No！`preferredMaxLayoutWidth`属性是iOS 6才引入的，`sizeThatFits:`方法则早得多，况且，UITextView是可以滚动的，哪怕文本不会全部呈现出来，但也可以通过左右或者上下滚动浏览所有内容；传给`sizeThatFits:`的参数（假设为size）是CGSize类型，size.width的功能和UILabel的`preferredMaxLayoutWidth`差不多，指定了UITextView区域的最大宽度，size.height则指定了UITextView区域的最大高度；可能有人问，若传给`sizeThatFits:`的size小于UITextView.text面积怎么办，岂不是有些内容无法显示出来？傻啊，可以滚啊！

值得一提的是，调用`sizeThatFits:`并不改变view的size，它只是让view根据已有content和给定size计算出最合适的view.size。

那么`sizeToFit`方法是干嘛的呢？很简单：
>calls sizeThatFits: with current view bounds and changes bounds size.

P.S：有点不太理解，这个「current view」指的是啥？self？还是superview？
P.P.S：经过验证，这里的「current view」指的是`self`。简单来说，`sizeToFit`等价于：

```objc
// calls sizeThatFits
CGSize size = [self sizeThatFits:self.bounds.size];
// change bounds size
CGRect bounds = self.bounds;
bounds.size.width = size.width;
bounds.size.height = size.width;
```

P.S：值得一提的是，经过测试发现，当调用`sizeThatFits:`的`size=(width, height)`，当width/height的值为0时，width/height似乎就被认为是无穷大！

**systemLayoutSizeFittingSize:方法**

首先来看一个应用场景。

场景四：某个UIView，宽度等于屏幕宽度，包含两个UILabel，两个Label都可能显示多行文本，要求：结合auto layout让UIView大小能够自适应subviews。

Easy，给出如下代码：

```objc
- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor lightGrayColor];
    
    bgView = ({
        UIView *view         = [[UIView alloc] init];
        view.backgroundColor = [UIColor grayColor];
        view;
    });
    [self.view addSubview:bgView];
    
    label1 = ({
        UILabel *label                = [[UILabel alloc] init];
        label.font                    = [UIFont systemFontOfSize:14.0];
        label.preferredMaxLayoutWidth = self.view.frame.size.width-20;
        label.numberOfLines           = 0;
        label.textColor               = [UIColor whiteColor];
        label.backgroundColor         = [UIColor purpleColor];
        label;
    });
    [bgView addSubview:label1];
    
    label2 = ({
        UILabel *label                = [[UILabel alloc] init];
        label.font                    = [UIFont systemFontOfSize:18.0];
        label.preferredMaxLayoutWidth = self.view.frame.size.width-20;
        label.numberOfLines           = 0;
        label.textColor               = [UIColor whiteColor];
        label.backgroundColor         = [UIColor redColor];
        label;
    });
    [bgView addSubview:label2];
    
    // 添加约束（基于Masonry）
    [bgView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(self.view.mas_left);
        make.top.equalTo(self.view.mas_top).offset(10);
        make.width.equalTo(self.view.mas_width);
    }];
    
    [label1 mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(bgView.mas_left).offset(10);
        make.right.lessThanOrEqualTo(bgView.mas_right).offset(-10);
        make.top.equalTo(bgView.mas_top).offset(10);
    }];
    
    [label2 mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(label1.mas_left);
        make.right.lessThanOrEqualTo(bgView.mas_right).offset(-10);
        make.top.equalTo(label1.mas_bottom).offset(10);
        make.bottom.equalTo(bgView.mas_bottom).offset(-10);
    }];
    
    label1.text = @"天地玄黄 宇宙洪荒 日月盈昃 辰宿列张 寒来暑往 秋收冬藏 闰余成岁 律吕调阳";
    label2.text = @"天地玄黄 宇宙洪荒 日月盈昃 辰宿列张 寒来暑往 秋收冬藏 闰余成岁 律吕调阳";
}
```

代码看得有些枯燥，简单来说，bgView（UIView）中嵌入两个能显示多行文本的label1（UILabel）和label2（UILabel），设置约束如下：

<div class="imagediv" style="width: 280px; height: 310px">{% asset_img 20150716-05.png %}</div>

代码运行后的显示效果：

<div class="imagediv" style="width: 320px; height: 170px">{% asset_img 20150716-04.png %}</div>

代码中除了添加各种各样的constraints，没有任何设置frame的代码，显然都是基于auto layout的。

那么问题来了，理解label1和label2的布局没啥子问题，因为它们的`intrinsicContentSize`方法会将content size告诉auto layout system，进而后者会为它们的size设置对应值；但对于bgView，它可是一个UIView对象，它的`intrinsicContentSize`回调方法的返回值为`(-1,-1)`，那么auto layout system是如何为它设置合适的size的呢？

根据我的理解，auto layout system在处理某个view的size时，参考值包括：
* 自身的`intrinsicContentSize`方法返回值；
* subviews的`intrinsicContentSize`方法返回值；
* 自身和subviews的constraints；

<div class="imagediv" style="width: 570px; height: 340px">{% asset_img 20150716-06.png %}</div>

OK，根据笔者理解，结合上图，我认为auto layout system是这样计算一下bgView的size的：

width=max{10+size1.width+10, 10+size2.width+10, size3.width}
height=max{10+size1.height+10+size2.height+10, size3.height}

我们在`viewDidAppear:`方法中将相关值打印出来瞧瞧看：

```objc
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    CGSize size1 = [label1 intrinsicContentSize];
    CGSize size2 = [label2 intrinsicContentSize];
    CGSize size3 = [bgView intrinsicContentSize];
    
    NSLog(@"size1 = %@", NSStringFromCGSize(size1));    // print: "size1 = {300, 33.5}"
    NSLog(@"size2 = %@", NSStringFromCGSize(size2));    // print: "size2 = {290.5, 64.5}"
    NSLog(@"size3 = %@", NSStringFromCGSize(size3));    // print: "size3 = {-1, -1}"
    
    CGSize bgViewSize = bgView.frame.size;
    NSLog(@"bgViewSize = %@", NSStringFromCGSize(bgViewSize));  // print: "bgViewSize = {320, 128}"
}
```

完全吻合我理解的auto layout size计算公式。

P.S：然而，我知道，事实往往并没有这么简单，当处理自定义View时，当constraints设置不完整或者冲突时，事情总会变得复杂起来，也总会得到意想不到的结果。但，暂且就这么理解吧！

罗莉啰嗦写了这么多，还没引出`systemLayoutSizeFittingSize:`方法...

OK，再来看另外一个应用场景。

场景五：某个UIView，宽度等于屏幕宽度，包含一个UILabel和一个UITextView，二者都可能显示多行文本，要求：结合auto layout让UIView大小能够自适应subviews。

在场景四代码基础上将label2改为UITextView对象textView1，如下：

```objc
- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor lightGrayColor];
    
    bgView = ({
        UIView *view         = [[UIView alloc] init];
        view.backgroundColor = [UIColor grayColor];
        view;
    });
    [self.view addSubview:bgView];
    
    label1 = ({
        UILabel *label                = [[UILabel alloc] init];
        label.font                    = [UIFont systemFontOfSize:14.0];
        label.preferredMaxLayoutWidth = self.view.frame.size.width-20;
        label.numberOfLines           = 0;
        label.textColor               = [UIColor whiteColor];
        label.backgroundColor         = [UIColor purpleColor];
        label;
    });
    [bgView addSubview:label1];
    
    textView1 = ({
        UITextView *label     = [[UITextView alloc] init];
        label.font            = [UIFont systemFontOfSize:18.0];
        label.textColor       = [UIColor whiteColor];
        label.backgroundColor = [UIColor redColor];
        label;
    });
    [bgView addSubview:textView1];
    
    // 添加约束（基于Masonry）
    [bgView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(self.view.mas_left);
        make.top.equalTo(self.view.mas_top).offset(10);
        make.width.equalTo(self.view.mas_width);
    }];
    
    [label1 mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(bgView.mas_left).offset(10);
        make.right.lessThanOrEqualTo(bgView.mas_right).offset(-10);
        make.top.equalTo(bgView.mas_top).offset(10);
    }];
    
    [textView1 mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(label1.mas_left);
        make.right.lessThanOrEqualTo(bgView.mas_right).offset(-10);
        make.top.equalTo(label1.mas_bottom).offset(10);
        make.bottom.equalTo(bgView.mas_bottom).offset(-10);
    }];
    
    label1.text = @"天地玄黄 宇宙洪荒 日月盈昃 辰宿列张 寒来暑往 秋收冬藏 闰余成岁 律吕调阳";
    textView1.text = @"天地玄黄 宇宙洪荒 日月盈昃 辰宿列张 寒来暑往 秋收冬藏 闰余成岁 律吕调阳";
}
```

运行效果如下：

<div class="imagediv" style="width: 320px; height: 110px">{% asset_img 20150716-07.png %}</div>

这显然不是我们想要的结果，至少存在这么两个问题：
1. textView1不见了；
2. bgView的大小不是我们想要的；

为什么会有出现这样的问题呢？

首先正如上文所提到的那样，textView1是UITextView的对象，而UITextView是UIScrollView的子类，它的`intrinsicContentSize:`方法的返回值是`(-1,-1)`，这意味着textView1对bgView的auto layout size没有产生影响。

那textView1为什么不见了呢？或者说，为什么textView1的size为CGSizeZero呢？根据我对auto layout system的理解，auto layout system在处理view的size时，受三个因素影响：
* 自身的`intrinsicContentSize`方法返回值；
* subviews的`intrinsicContentSize`方法返回值；
* 自身和subviews的constraints；

对于textView1，前两个因素可以忽略掉，简而言之，textView1的size由它自身的constraints决定。而根据上述代码的约束可以计算出textView1的height：
height = bgView.height-10-label1.height-10-10;

而bgView.height=10+label1.height+10+10；意味着textView1的height值为0；当然就看不到了textView1了。

因此，若想要让textView1正常可见，至少有这么一种策略：直接为textView1添加约束设置width和height；

简单来说，override回调方法`viewWillLayoutSubviews`，如下：

```objc
- (void)viewWillLayoutSubviews {
    [super viewWillLayoutSubviews];

    CGSize textViewFitSize = [textView1 sizeThatFits:CGSizeMake(self.view.frame.size.width-20, 0)];
    [textView1 mas_updateConstraints:^(MASConstraintMaker *make) {
        make.width.equalTo([NSNumber numberWithFloat:textViewFitSize.width]);
        make.height.equalTo([NSNumber numberWithFloat:textViewFitSize.height]);
    }];
}
```

这种做法是有效的，运行效果如下：

<div class="imagediv" style="width: 320px; height: 190px">{% asset_img 20150716-09.png %}</div>

若将这种场景切换到table view cell中会如何呢？简单来说，如果将上述的bgView换成UITableViewCell（或其子类）对象又会如何呢？

在table view中，我们可以使用`- (CGFloat)tableView:heightForRowAtIndexPath:`接口，该回调方法会返回CGFloat值，该值指示了对应cell的高度；假设auto layout system为cell分配的size是autoSize，在处理返回值时额外加上textViewFitSize.height即可。

但问题是，我们如何获取这个autoSize的值呢？毕竟此时cell还未布局完成啊，直接读取cell.frame.size肯定是不行的。

`systemLayoutSizeFittingSize:`方法正是用于处理这个问题的。

[UIView Class References](https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIView_Class/)对该方法描述如下：
>Returns the size of the view that satisfies the constraints it holds.
**Return Value**
The size of the view that satisfies the constraints it holds.
&nbsp;
**Discussion**
Determines the best size of the view considering all constraints it holds and those of its subviews.

我是这么理解`systemLayoutSizeFittingSize:`的：对于使用auto layout机制布局的view，auto layout system会在布局过程中综合各种约束的考虑为之设置一个size，在布局完成后，该size的值即为view.frame.size的值；这包含的另外一层意思，即在布局完成前，我们是不能通过view.frame.size准确获取view的size的。但有时候，我们需要在auto layout system对view完成布局前就知道它的size，`systemLayoutSizeFittingSize:`方法正是能够满足这种要求的API。`systemLayoutSizeFittingSize:`方法会根据其constraints返回一个合适的size值。

`systemLayoutSizeFittingSize:`方法可传入一个参数，目前有两个值可以传入：
* UILayoutFittingCompressedSize : The option to use the smallest possible size.
* UILayoutFittingExpandedSize : The option to use the largest possible size.

值得一提的是，在使用`[view systemLayoutSizeFittingSize:]`时，要注意尽量确保view的constraints的完整性，这样参数UILayoutFittingCompressedSize和UILayoutFittingExpandedSize得到的结果是一样的。否则，举个例子，若view的right属性没有设置，则这两个参数得到`systemLayoutSizeFittingSize:`返回值size是不一样的，前者size.width=0，后者size.width=1000。
P.S：这纯属个人使用体验。

至于`systemLayoutSizeFittingSize:`的使用场景，[动态计算UITableViewCell高度详解](http://www.cocoachina.com/industry/20140604/8668.html)非常值得参考！

**对比几种API**

在刚开始接触这几个API时感到非常困惑。分不清`intrinsicContentSize`、`sizeThatFits:`以及`systemLayoutSizeFittingSize:`的区别。经过这么将近一天的折腾，现在大概有了基本的判断。

首先说`intrinsicContentSize`，它的最主要作用是告诉auto layout system的一些信息，可以认为它是后者的回调方法，auto layout system在对view进行布局时会参考这个回调方法的返回值；一般很少像`CGSize size = [view intrinsicContentSize]`去使用`intrinsicContentSize` API。

再来看`sizeThatFits:`和`systemLayoutSizeFittingSize:`，它们俩非常相似，都是为开发者直接服务的API（而不是回调方法）。所不同的是，`sizeThatFits:`是auto layout之前就存在的，一般在leaf-level views中用得比较多，在计算size过程中，它可不会考虑constraints神马的；对于`systemLayoutSizeFittingSize:`，它是随着auto layout（iOS 6）引入的，用于在view完成布局前获取size值，如果view的constraints确保了完整性和正确性，通常它的返回值就是view完成布局之后的view.frame.size的值。

它们之前存在相互调用的关系吗？经过测试发现，三者之前没有直接的调用关系。但是能得出这样的结论：`intrinsicContentSize`的返回值会直接影响`systemLayoutSizeFittingSize:`的返回值。至于底层是如何处理的不得而知。

## 写在后面

本博客写了好多个小时，非常勉强，写完后不忍直视，臭又长。进一步意识到把博客写长不难，难的是把它写短同时传递足够多的信息。这在方面，我需要极大的提升啊！

用人话把技术讲清楚。

## 参考资料

* [Proper usage of intrinsicContentSize and sizeThatFits: on UIView Subclass with autolayout](http://stackoverflow.com/questions/24127032/proper-usage-of-intrinsiccontentsize-and-sizethatfits-on-uiview-subclass-with-a)
* [How to set a label's preferredMaxLayoutWidth to automatic programmatically?](http://stackoverflow.com/questions/27711853/how-to-set-a-labels-preferredmaxlayoutwidth-to-automatic-programmatically)
* [How to resize superview to fit all subviews with autolayout?](http://stackoverflow.com/questions/18118021/how-to-resize-superview-to-fit-all-subviews-with-autolayout/18155803#18155803)
* [Using Auto Layout in UITableView for dynamic cell layouts & variable row heights](http://stackoverflow.com/questions/18746929/using-auto-layout-in-uitableview-for-dynamic-cell-layouts-variable-row-heights/18746930#18746930)
* [iOS开发实践之Auto Layout](http://xuexuefeng.com/autolayout/)
* [动态计算UITableViewCell高度详解](http://www.cocoachina.com/industry/20140604/8668.html)