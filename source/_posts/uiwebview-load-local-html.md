title: UIWebView加载本地HTML文件
date: 2015-06-16 16:38:39
tags: UIWebView
categories: iOS

---

## 写在前面

写本文的原因是今天被要求「调研在iOS中加载本地HTML的相关技术」，好记性不如烂笔头，将一些东西给记录下来吧。

应用场景是这样的：手头的iOS App有一部分UI需要使用HTML完成，好处是这部分UI可以更灵活，即可以在任何不需要升级App的情况下更新这部分UI（包括样式、操作等等）。当下非常火的技术 -- Facebook的React Native -- 正是用来解决这种问题的，毕竟在传统的开发模式下，更新UI必须要升级App，而升级App是一个非常耗时的过程。使用Web App部分代替Native App已经成为当下移动客户端开发的一种思潮了。为了简便起见，也为了不给项目的接盘者制造更高的门槛，暂时决定不使用React Native（原因是多方面的）等著名框架去这种事情，从0开始探究。话说胡来，也许走了这么一遭之后，以后学习React Native会有更多的体会呢！

总之，目前的构想是这样的：

1. App部分UI是由HTML+JS+CSS完成的，HTML+JS+CSS代码会随着App一起发布；
2. 当后端决定更新App的部分UI（HTML）时，客户端从服务器下载最新的HTML+JS+CSS包，解压后代替本地的、旧的HTML+JS+CSS资源，将最新的呈现给用户，至此完成UI的更新；

因此有这么些问题需要考虑：

1. 加载本地HTML+JS+CSS资源（在iOS中，通常使用UIWebView）；
2. 解压从服务端下载的压缩文件（HTML+JS+   CSS包）；
3. JS代码和OC代码的互相调用；

本文主要探究第一个问题！

## UIWebView介绍

UIWebView是iOS中一个非常常用的控件，是内置的浏览器控件，可能也是最强大复杂的控件。可以用它来浏览网页、打开文档（譬如PDF文档）等等。

UIWebView既可以用来显示本地文档，也可以用来显示网络文档。无论是显示本地文件还是网络文件，此过程都可以统称为「加载」。与「加载」相关的方法不多，只有三个：

```objc
// Loading Local Content or Loading Content From the Network
- (void)loadRequest:(NSURLRequest *)request;
    
// 嵌入HTML结构的字符串
- (void)loadHTMLString:(NSString *)string
               baseURL:(NSURL *)baseURL;
    
// Loading Local Content
- (void)loadData:(NSData *)data
        MIMEType:(NSString *)MIMEType
textEncodingName:(NSString *)textEncodingName
         baseURL:(NSURL *)baseURL;
```

与UIWebView相关的知识点并不多（从某种角度来看，也可以认为是Apple封装得比较好吧），除了上述的三个方法之外，还有必要了解的是UIWebViewDelegate定义的一些delegate方法：

```objc
// 开始加载前调用
- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request
 navigationType:(UIWebViewNavigationType)navigationType;
// 开始加载时调用
- (void)webViewDidStartLoad:(UIWebView *)webView;
// 加载成功时调用
- (void)webViewDidFinishLoad:(UIWebView *)webView;
// 加载失败时调用
- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error;
```

除此之外，UIWebView似乎有啥其他需要了解的内容，若有需要，以后再补充吧！

## 加载本地HTML文件

加载本地HTML文件不是多么难的事情，但再简单的事情也有一个1-2-3-4，不是嘛？何况笔者比较笨，参考[UIWebView加载本地HTML5文件](http://blog.csdn.net/kaitiren/article/details/17115085)，也分几个步骤：
1. 准备HTML文件及其资源文件（主要是CSS文件和JS文件以及image文件）；
2. 加载本地HTML文件；

第一种：
loadRequest:
第二种：
loadHTMLString:baseURL:

**第一步，准备HTML文件及其资源文件。**

HTML及其资源文件比较容易获取，随便找个比较漂亮的web页面，将它保存起来即可！

显然HTML文件及其资源文件是需要添加到工程的，问题是如何添加呢？如何组织管理这些文件呢？沙盒？Bundle？

**Xcode工程添加文件有两种方式**

向项目中添加已有的文件（或资源文件，或第三方库）不是什么稀罕的事情，只是平时都没怎么注意。实际上，将「文件」或者「目录」添加到项目时，Xcode给了我们两个选项：Create groups以及Create folder references，如下图：

<div class="imagediv" style="width: 730px; height: 126px;">{% asset_img 20150616-01.png %}</div>

简单来说，若选择Create groups的方式添加文件夹，则文件夹的颜色是黄色的；若选择Create folder references的，则发现添加的文件夹在工程目录中的的颜色是蓝色的：

<div class="imagediv" style="width: 400px; height: 120px;">{% asset_img 20150616-02.png %}</div>

当然不只是颜色上的区别！主要是Xcode对加入的文件的路径处理不一样。

如果文件/文件夹以Create groups的方式添加到工程中，则不管加入项目的文件的目录结构如何，在APP中都可以通过mainBundlePath/filename这样结构来访问文件；反之，若文件/文件夹以Create folder references的方式添加到工程中，则会保留相对路径，访问文件的结构就变成了mainBundlePath/path/filename。

理解Create groups以及Create folder references并不难，自己动手试试就能感受二者的不同了。

**Bundle介绍**

上文谈到了向工程添加文件的两种方式，由此延伸，不得不提bundle，一直以来对bundle的理解非常模糊，知道它的本质是一个目录，也知道在OS X中就有此概念，但一直没get到`bundle`的存在意义，总感觉它没必要存在，事实上也没主动使用它。

借撰写本文这个机会，驱动一下自己，查查资料，争取对bundle理解更深入一点点吧！

参考《[Bundle in iOS](http://avatar-matrix.lofter.com/post/e4689_236e82)》和《[iOS开发里的Bundle是个啥玩意](http://www.cnblogs.com/BigPolarBear/archive/2012/03/28/2421802.html)》。

简单地讲，bundle就是一个内部结构按照标准规则组织的特殊目录，常用来存放一些资源文件，譬如图片，plist文件等，它不会成为编译的一部分，所以它不存放能够被编译的文件，譬如`.m`、`.h`文件等。

Bundle的主要用途是软件的国际化，想象一个应用场景：软件有美国、中国版本，软件的logo等图片不同，这时候，我们把相应的图片资源放到一个文件夹下，然后修改文件夹的名字，以`.bundle`作为后缀，然后添加到xcode中。

有过iOS开发经历的人都应该知道，每个project都有个mainBundle，那如何去获取自己定义的bundle内容呢？我们把自定义的bundle当成项目的一种资源，由mainBundle去获取`~`，然后我们用相同的方式获取自定义的bundle下的资源。即：

1、通过mainBundle去加载自定义的bundle；
2、通过获取到的自定义bundle去获取资源；

根据上述场景来写的demo项目结构如下：

<div class="imagediv" style="width: 340px; height: 280px;">{% asset_img 20150616-03.png %}</div>

此时，我容易联想到了在Windows世界经常使用的「汉化包」。根据我的理解，我们通过某种手段得到的「汉化包」和本文所谈及的bundle是类似的概念，它们的本质是一个目录，只是这个目录有些特别，特别之处在于它只存放一些资源文件，以及目录名后缀为`.bundle`，这让它看起来像个文件，但实际上又不是文件，开发者又特别想让普通用户以为它是一个文件...

总之，bundle就是这么个东东。

到了这里，应该弄清楚了该把HTML文件及其资源放在哪里了。

沙盒？显然不可能，沙盒可是App到设备之后才有的概念；所以我们只能让HTML资源成为工程的一部分呢，编译打包时作为安装包的一部分。

放在普通目录下还是组织成bundle，个人以为没必要组织成bundle，因为后者主要用来处理国际化问题，国际化问题面对的一般是图片资源、字符串资源等等，所以不需要组成成bundle，而是直接添加到工程的mainBundle中。

上文已经提到了Xcode工程添加文件有两种方式：Create groups和Create folder references，个人以为应该选择后者！

值得注意是，当被加入到项目中的文件包含js文件时，有可能因为操作不慎等原因，js文件成为了编译文件，若是这样，应该将它从`Compile Sources`给踢掉，让它成为`Copy Bundle Resources`的一部分，具体做法是在TARGETS->Build Phases中的`Compile Sources`中找到该js文件，并将其移到`Copy Bundle Resources`中，如下图所示：

<div class="imagediv" style="width: 699px; height: 364px;">{% asset_img 20150616-05.png %}</div>

**第二步，加载本地HTML。**

本文示例中，HTML文件及其资源文件被组织在一个叫`html`的目录中，目录结构如下图，index.html是要被加载的目标文件，index.html中访问css、js以及图片资源的方式都是使用相对路径，实现代码也给展示在下图中。

<div class="imagediv" style="width: 926px; height: 400px;">{% asset_img 20150616-04.png %}</div>

上图右侧代码区域加载策略是，先获取本地HTML文本，转为字符串，然后使用`loadHTMLString:baseURL:`方法加载；当然也可以使用`loadRequest:`方法，哪个更好？我暂时也不知道！

**关于baseURL**

上文`loadHTMLString:baseURL:`方法的第二个参数是baseURL，baseURL是HTML字符串中引用到资源的查找路径，当HTML中没有引用外部资源时，可以指定为`nil`；若引用了外部资源，一般情况下使用mainBundle的路径即可。在实际操作中，常常会出现「文本显示正常，图片无法显示」等情况，若HTML文本中引用外部资源都是使用相对路径，则出现这种问题的原因一般都是baseURL参数错误（有时甚至是`nil`）。

最后，本文的Demo详见[这里](https://github.com/sadjason/iOSDemos/tree/master/UIWebView%E9%9D%9E%E5%88%9D%E4%BD%93%E9%AA%8C)。