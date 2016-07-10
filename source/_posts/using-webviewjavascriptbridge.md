title: 使用WebViewJavascriptBridge
date: 2015-06-16 16:53:35
categories: iOS

---

## 写在前面

在App中使用Web代替一些Native UI已经成为移动客户端开发的一种思潮。在App中嵌入Web有一个重要的基础问题：Objective-C和JavaScript的交互。

近期的项目需要，笔者开始着手这方面的问题学习。很容易想到：一定存在某个引擎能够在OC和JS之间转换。去github中搜索关键字`iOS JavaScript`得到的选择并不多，第三方库[WebViewJavascriptBridge](https://github.com/marcuswestin/WebViewJavascriptBridge)的Stars遥遥领先，自然选择它作为OC和JS的交互引擎了。

本文是笔者了解WebViewJavascriptBridge过程中的一些学习记录，好记性不如烂笔头嘛！

## 使用WebViewJavascriptBridge

WebViewJavascriptBridge的使用不难，[WebViewJavascriptBridge](https://github.com/marcuswestin/WebViewJavascriptBridge)提供的Example比较直观的展示了各种接口的使用。

App中嵌入Web一般需要使用UIWebView（除非你自己写一个），WebViewJavascriptBridge正是配合UIWebView进行工作的。

**初始化**

和其他第三库一样，使用WebViewJavascriptBridge需要做一些初始化，只是WebViewJavascriptBridge的初始化包括两部分：「Objective-C初始化」和「JavaScript初始化」。

* OC初始化

OC中初始化WebViewJavascriptBridge的前提是存在一个UIWebView对象，每个WebViewJavascriptBridge对象都应该与一个UIWebView对象绑定。

```objc
// 创建UIWebView对象
UIWebView * webView = [[UIWebView alloc] initWithFrame:self.view.bounds];
    
// 配置logging
[WebViewJavascriptBridge enableLogging];
    
// 创建WebViewJavascriptBridge对象并与UIWebView对象绑定
self.bridge = ({
    WebViewJavascriptBridge* bridge =
    [WebViewJavascriptBridge bridgeForWebView:webView
                              webViewDelegate:self
                                      handler:^(id data, WVJBResponseCallback responseCallback) {
                                          // do something
                                          // responseCallback(responseData)
                                      }
     ];
    bridge;
});
```

使用类方法创建一个WebViewJavascriptBridge对象，其中有一个`block`类型的handler。这个handler用来处理来自于JavaScript发送的消息，**handler的形参与JavaScript中的send方法的形参对应**，一般有两个参数，第一个参数是JS send传入的参数（可以是任意类类型），第二个是JS send传入的回调handler。

P.S：看客可能像我当初一样不太理解第二个参数：responseCallback，后文会对此进行详细说明。

* JS初始化

除了OC外，JS中也得执行针对WebViewJavascriptBridge的初始化代码。这意味着，除了客户端（iOS开发人员）外，服务端（后端写JS的开发人员）也得对WebViewJavascriptBridge有所了解。好在需要理解的内容不多，十分钟就可以搞定。JavaScript对WebViewJavascriptBridge初始化过程（这部分代码几乎是固定的）如下：

```js
// 创建了一个connectWebViewJavascriptBridge方法，该方法名是固定的
function connectWebViewJavascriptBridge(callback) {
    if (window.WebViewJavascriptBridge) {
        callback(WebViewJavascriptBridge)
    } else {
        document.addEventListener('WebViewJavascriptBridgeReady', function() {
            callback(WebViewJavascriptBridge)
        }, false)
    }
}
    
// 调用connectWebViewJavascriptBridge方法
connectWebViewJavascriptBridge(function(bridge) {
    bridge.init(function(message, responseCallback) {
        // do something
        responseCallback(responseData)
    })
})
```

先创建了一个connectWebViewJavascriptBridge方法，该方法注册了一个WebViewJavascriptBridgeReady事件，同时声明了一个全局的WebViewJavascriptBridge变量，这样我们可以在外部通过WebViewJavascriptBridge调用相关方法。

在`bridge.init`里面同样定义了一个匿名function，这个function用来接收Objective-C里面通过send方法发送的消息的，参数与OC里的send方法参数对应。同样，一般有两个参数，第一个参数是OC send传入的参数（可以是任意类类型），第二个是OC send传入的回调handler。

可以简单总结一下。初始化的根本目的是啥？
根据我的理解，初始化的根本目的是：消息接收者定义`message handler`。

P.S：请记住`message handler`这个名词，后文会经常用到。

**发送消息**

上述「初始化」操作的目的是为了确保OC和JS能够相互处理来自对方的消息。

除了「初始化」操作之外，WebViewJavascriptBridge对发送消息也有所约束，这意味着OC和JS发送消息必须得遵守一定的格式。

* OC向JS发送消息

OC向JS发送消息，定义了两个用于「发送消息」的接口：

```objc
// APIs
- (void)send:(id)data;
- (void)send:(id)data responseCallback: (WVJBResponseCallback)responseCallback;
```

两个接口的区别只是参数个数不同，参数data指的是「传给JS的参数」，responseCallback参数是一个`block`，给JS发送消息后，JS的`message handler`可能会返回一些值，responseCallback就是用来处理***返回值***的。

P.S：data可以为空；这里的「返回值」并不是非常准确的说法，只是一种参考「函数」的说法，准确来讲应该叫`response data`。

* JS向OC发送消息

JS向OC发送消息，WebViewJavascriptBridge也定义了两种格式：

```js
bridge.send(data)
bridge.send(data, function responseCallback(responseData) { ... })
```

显然，无论是「OC向JS发送消息」，还是「JS向OC发送消息」，都有两种格式，含有`responseCallback`和不含`responseCallback`。

P.S：data可以为空；

**理解responseCallback**

上文已经多次提到了`responseCallback`，可能是由于笔者对跨平台了解得比较少，也可能是对函数式编程了解不多，刚开始对`responseCallback`不甚理解。这一小段将对`responseCallback`进行详细阐述。

关于「消息处理」和「消息发送」，我是参考「函数定义」和「函数调用」这两个概念来理解的。「函数定义」定义了函数的具体工作（即说明这段代码块都干了些啥），「函数调用」指示执行具体代码块；根据我的理解，从概念上讲，「消息处理」对应「函数定义」，「消息发送」对应「函数调用」。

这段话非常啰嗦，但引入这么一种对应关系是为了更好说明`responseCallback`。

对于函数（广义上的「函数」，而不仅仅指JavaScript function）来说，函数可能有返回值，也可能没有返回值。当有返回值时，调用者往往会定义变量接收返回值，方便之后使用返回值...而上文中反复出现的`responseCallback`有些类似于对函数返回值的处理，消息发送方向消息接收方发出一个消息，除了希望对方处理某些事情之外，可能还期待对方返回一些数据（`response data`），这些数据往往会在后续的处理中起作用。

因此，若「消息接收方」的`message handler`中可能有需要传给「消息发送方」的`response data`时，「消息发送方」还需要定义一个handler用来处理这些值，即所谓的`responseCallBack`。

P.S：为什么函数处理返回值使用`ret = aFunction(variable)`这样的格式，而这里使用`responseCallBack`处理呢？我想是因为这里处理的是两种不同语言，过程中难免存在类型转换，况且，还有可能是由于并发。

对于函数而言，若某个函数有返回值，但调用者不想要保存该返回值，此时往往不会定义变量接收该返回。这在大多属于语言中是被允许的。

对于WebViewJavascriptBridge也一样，「消息发送方」发送消息时，可以不传入responseCallback参数，表示对「消息接收方」的`response data`不care。

在定义`message handler`时，在handler的`responseCallback(responseData)`好比函数中`return ret`。

P.S：在定义`message handler`时，并不要求一定有`responseCallback(responseData)`这么一句代码；只是个人觉得，有必要写上，哪怕没有任何`response data`需要返回，也得加上`responseCallback(null)`。类似于函数，若某个函数没有返回值，也没有显式调用`return`语句，在编译阶段，编译器也会帮助在末尾加上`return void`。

理清了`responseCallback`这个概念，就基本上算是学会使用WebViewJavascriptBridge了。

**OC和JS互相调用**

「OC和JS互相调用」指的是OC和JS互相对应对方的handler（block或function）。

笔者刚开始觉得啰嗦：既然「消息机制」能够解决OC和JS交互问题，为啥还需要OC和JS互相调用对方的handler呢？

我还没有找到比较权威的的说法，但这里也谈谈自己的一点理解。

先说「函数」，「函数」的本质不过是代码的一种组织结构，它使得代码块具有了更好的可读性，同时极大加强代码复用。

基于「消息机制」，我们可以尽可能实现任何基于文本交互。可以做的事情非常丰富，譬如「消息发送者」传入参数`1`，`message handler`执行A段代码，传入参数`2`，执行B段代码。但问题是：基于「消息机制」，OC和JS之间的几乎所有交互任务都得写在`message handler`中。当交互任务变得复杂时，代码组织将是一种灾难（会充斥很长并且嵌套很深的`if`语句）。以消息的第一个参数data为例，有时候，data可能是一个URL字符串，有时候可能是一个JSON字符串，有时候可能只是一个数值，光是解析这些参数，都需要一个非常复杂的`if`语句...写到这里，「在OC和JS中定义能被对方调用的handler」的意义就不需要多讲了。

关于「OC和JS互相调用」，WebViewJavascriptBridge也定义了一些约束。约束包括两部分：定义handler的姿势，调用handler的姿势。

**OC定义和调用JS handler**

所谓定义handler，其实是向bridge注册一个handler，如下：

```objc
// API
// - (void)registerHandler:(NSString *)handlerName handler:(WVJBHandler)handler
    
// eg:
[self.bridge registerHandler:@"OCHandlerName"
                     handler:^(id data, WVJBResponseCallback responseCallback) {
                         // do something
                         responseCallback(responseData);
                 }
 ];
```

向bridge注册handler包括两部分内容：name和handler body。

调用JS的handler也简单，如下：

```objc
// APIs
// - (void)callHandler:(NSString *)handlerName;
// - (void)callHandler:(NSString*)handlerName data:(id)data;
// - (void)callHandler:(NSString*)handlerName data:(id)data responseCallback:(WVJBResponseCallback)responseCallback;
    
// eg：
[self.bridge callHandler:@"JSHandlerName" data:data];
```

**JS定义和调用OC handler**

在JS定义（注册）handler的姿势如下：

```js
bridge.registerHandler("handlerName", function(responseData) { ... })
```

调用OC的handler也简单，和OC调用JS handler类似。

。。。。。。

本文写得好啰嗦啊！

## 本文参考

* [WebViewJavascriptBridge - github](https://github.com/marcuswestin/WebViewJavascriptBridge)
* [《WebViewJavascriptBridge使用说明（iOS）》](http://dxldy.iteye.com/blog/2078350?utm_source=tuicool)
* [《WebViewJavascriptBridge使用》](http://honglu.me/2014/09/27/WebViewJavascriptBridge使用/)