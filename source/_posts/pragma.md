title: 预处理指令#pragma
date: 2015-04-29 16:12:35
categories: iOS

---

## #pragma介绍

`#pragma`是一个预处理指令，pragma的中文意思是『编译指示』。它不是Objective-C中独有的东西（貌似在C/C++中使用比较多），最开始的设计初衷是为了保证代码在不同编译器之间的兼容性，但随着时间推移，它出现在了更多更丰富的应用场景中。

`#pragma`代码是在编译期间处理的；它既不属于注释，也不属于逻辑代码的一部分；并且它和其他预处理命令譬如`#ifdef ... #endif`不同，它不会影响代码在运行时的逻辑处理，所以`#pragma`指令丝毫不会影响到程序在运行时的性能。根据Mattt Thompson大神的描述，在当前Xcode开发环境中，`#pragma`主要有两个应用场景：组织代码和屏蔽编译警告。

## #pragma mark组织代码

组织代码是个人卫生问题，个人卫生不好（代码组织不好）不能反映人品（技术能力），但它在某种程度中影响了别人是否愿意和他搞基（合作）。在一个项目（尤其是多人合作的项目）中，应该有一个比较好的内部一致性编码习惯，不好的习惯或者缺乏一致性，会使得项目难以维持，协作也不便（这点笔者深有体会，笔者目前的项目有一个同事，编码能力尚可，但编码习惯实在太糟糕，代码紧凑，从来不空格，更妄谈空行和注释了，甚至经常不对齐，看他的代码，一点修改的欲望都没有，想的只是自己重写一遍=_=）。

在组织代码时充分使用`#pragma mark`就是写出『干净代码』的一个重要环节，就像这样：

```objc
#pragma mark - UIViewController
    
- (void)viewDidLoad {
    
    [super viewDidLoad];
}
    
- (void)dealloc {
    
}
    
#pragma mark - IBAction
    
- (IBAction)loginNow:(id)sender {
    
}
    
#pragma mark - UITableViewDataSource
    
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return 0;
}
    
#pragma mark - UITableViewDelegate
    
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    
}
```

上述代码中使用`#pragma mark`将@implementation中的代码（方法）分成了几个逻辑section，这种处理并没有改变程序逻辑，但我们在使用Xcode代码导航工具时可以在视觉上汲取片刻的欢愉：

<div class="imagediv" style="width: 402px; height: 256px;">{% asset_img using-pragma.png 使用代码导航器%}</div>

组织N个方法为一个section的依据是什么呢？这个就见仁见智了。一般来说：

* 将一个protocol的方法组织成一个section；
* 将target-action类型方法组织成一个section；
* 将notification相关方法组织成一个section；
* 将需要override的父类方法组成成一个section；


## #pragma屏蔽编译警告

使用`#pragma mark`来组织代码使用比较普遍，相对而言，使用`#pragma`指令屏蔽编译器和静态分析器的警告相对来说就比较新鲜了。

You know what is even more annoying than poorly-formatted code? Code that generates warnings. 编译警告很可恶，应该尽可能修改代码干掉这些警告，但是有些时候有些警告无法避免，譬如我们在编写`@selector(aMethodName)`这样的代码时，如果aMethodName没有在上下文中出现，可能会出现含有『undeclared-selector』关键词的warning，有洁癖的程序员会想到干掉这个warning，此时`#pragma`指令就派上用场了。譬如：

```objc
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundeclared-selector"
    if ([self.selectedViewController respondsToSelector:@selector(isReadyForEditing)]) {
        boolNumber = [self.selectedViewController performSelector:@selector(isReadyForEditing)];
    }
#pragma clang diagnostic pop
```

这是Clang编译器提供的一种解决方案，通过使用`#pragma clang diagnostic push/pop`来告诉编译器仅仅为某一特定部分代码（记得要在代码片段末尾使用pop将最初的diagnostic设置恢复哦）忽视特定警告。

上述示例中`#pragma clang diagnostic ignored`后面的`"-Wundeclared-selector"`指示的是『特定警告』，再别的应用场景中肿么知道该填写什么呢？网络是强大的，有一个（可能是N个）愤怒的网友对clang警告消息做了一个总结，之所以说这个网友“愤怒”，是因为其链接非常有意思：http://fuckingclangwarnings.com/。

Finally, you can read more about the LLVM's use of #pragma in the [Clang Compiler User's Manual](http://clang.llvm.org/docs/UsersManual.html#diagnostics_pragmas).

最后，感谢大神Mattt Thompson，本文的参考资料完全来自于《[#pragma](http://nshipster.com/pragma/)》，本来想用自己的语言完成这篇博客，边读边写，最后发现几乎差不多了，没留下啥自己的东西，就这样吧！


## 本文参考

* 《[#pragma](http://nshipster.com/pragma/)》
* 《[clang diagnostics](http://nshipster.com/clang-diagnostics/)》