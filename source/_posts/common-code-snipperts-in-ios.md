title: iOS常用代码片段
date: 2015-02-11 15:51:12
categories: iOS

---

## 写在前面

在开发过程中，总会有一些简短但使用频率非常高并且不太容易记住的代码，这里总结一下，方便使用。

## 判断系统版本

iOS8 之前版本常用方法：

```objc
// 变量v是NSString类型
#define SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(v) \
([[[UIDevice currentDevice] systemVersion] \
compare:v options:NSNumericSearch] != NSOrderedAscending)
```

记得从某篇博客中了解到iOS8提供了更直接的封装，但忘了方法名了，哎，好记性不如烂笔头啊，以后再补上吧！

## 根据字体和宽度计算高度

常用语计算UILabel实例的高度。已知文本内容、字体以及UILabel宽度，计算UILabel的高度。

```objc
+ (CGFloat)computeHeightForText:(NSString *)text
                    havingWidth:(CGFloat)widthValue
                        andFont:(UIFont *)font
{
#define SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(v)\
([[[UIDevice currentDevice] systemVersion] \
compare:v options:NSNumericSearch] != NSOrderedAscending)
    
    CGFloat result = font.pointSize + 4;
    if (text) {
        CGSize size;
        if (SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(@"7.0")) {
            //iOS 7
            CGRect frame = [text boundingRectWithSize:CGSizeMake(widthValue, CGFLOAT_MAX)
                                              options:NSStringDrawingUsesLineFragmentOrigin
                                           attributes:@{NSFontAttributeName:font}
                                              context:nil];
            size = CGSizeMake(frame.size.width, frame.size.height+1);
        } else {
            //iOS 6.0
        }
        result = MAX(size.height, result); //At least one row
    }
    return result;
}
```

## 收起键盘

在UIViewController中收起键盘，除了调用相应控件的resignFirstResponder方法外，还有另外三种办法：

1. 重载ViewController中的touchBegin方法，然后在里面执行`[self.view endEditing:YES];`，这样单击UIViewController的任意地方，就可以收起键盘；
2. 直接执行[[UIApplication sharedApplication] sendAction:@selector(resignFirstResponder) to:nil from:nil forEvent:nil]，用于在获得当前UIViewController比较困难的时候用；
3. 直接执行`[[[UIApplication sharedApplication] keyWindow] endEditing:YES];`。

## 取消UIButton被按下时的高亮效果

```objc
UIButton.adjustsImageWhenDisabled    = false;   // 取消高亮效果
UIButton.adjustsImageWhenHighlighted = false;   // 取消失能状态的高亮效果
```

## Button图片和title共存

很多时候我们在UI中需要实现同时具备image和title的button，最常见的场景如下：

<div class="imagediv" style="width: 320px; height: 90px">{% asset_img button-image-and-title.png Button With Image And Title %}</div>

这样的效果如何实现呢？很简单，使用UIButton的`titleEdgeInsets`和`imageEdgeInsets`属性控制image和title的位置，如下：

```objc
- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.view.backgroundColor = [UIColor lightGrayColor];
    
    CGFloat spacingBetweenIconAndTitle = 15.f;  // icon和title的间距
    UIButton *button                   = [[UIButton alloc] init];
    UIImage *icon                      = [UIImage imageNamed:@"order_order"];
    button.titleLabel.font             = [UIFont systemFontOfSize:15.f];
    button.backgroundColor             = [UIColor whiteColor];
    [button setImage:icon forState:UIControlStateNormal];
    [button setTitle:@"购物车" forState:UIControlStateNormal];
    [button setTitleEdgeInsets:UIEdgeInsetsMake(0, spacingBetweenIconAndTitle, 0, 0)];
    [button setImageEdgeInsets:UIEdgeInsetsMake(0, 0, 0, spacingBetweenIconAndTitle)];
    [button setTitleColor:[UIColor lightGrayColor] forState:UIControlStateNormal];
    [button sizeToFit];     // 匹配最合适的size
    CGRect buttonFrame     = button.frame;
    buttonFrame.size.width += spacingBetweenIconAndTitle;
    button.frame           = buttonFrame;
    button.center          = CGPointMake(self.view.center.x, 50);
    
    [self.view addSubview:button];
}
```