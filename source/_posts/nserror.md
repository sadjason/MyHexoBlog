title: NSError分析
date: 2015-01-19 15:29:44
tags: Error
categories: iOS

---

## 写在前面

在iOS开发中，`NSError`的使用非常常见，使用也比较简单，也正因为简单，所以对这一部分知识不甚注重。但是近期在做app底层网络封装时发现了一些问题。我使用的网络框架是AFNetworking，AFNetworking常常会返回一些错误信息，有时需要将这些错误信息告诉用户，通常做法是将`NSError#localizedDescription`以弹框的形式原原本本展现给用户（譬如“网络不畅”之类的），但是这样非常不友好，一是这些信息往往是英文，二是这些信息过于冗长，不够简洁。所以自然想到了对这些error信息进行包装。这就迫使我不得不去了解更多关于NSError相关的信息，本文着重叙述Error Domain和Error Code。

## Error Domain

首先，error domain是一个字符串。因为历史原因，在OS X中将errors分为不同的domains。譬如，对于Carbon框架的error，归于OSStatus domain（`NSOSStatusErrorDomain`），对于POSIX error，归于`NSPOSIXErrorDomain`，而对于我们的iOS开发，一般使用`NSCocoaErrorDomain`。NSError.h定义了四个domain，如下：

```objc
// Predefined domain for errors from most AppKit and Foundation APIs.
FOUNDATION_EXPORT NSString *const NSCocoaErrorDomain;
    
// Other predefined domains; value of "code" will correspond to preexisting values in these domains.
FOUNDATION_EXPORT NSString *const NSPOSIXErrorDomain;
FOUNDATION_EXPORT NSString *const NSOSStatusErrorDomain;
FOUNDATION_EXPORT NSString *const NSMachErrorDomain;
```

除了上述的四个domain之外，不同的framework甚至一些classes group（相关的几个classes）也定义了自己的domain，譬如对于Web Kit framework，定义了`WebKitErrorDomain`，而更常见的，URL相关的classes定义了`NSURLErrorDomain`。

Domains非常有用，特别当程序非常复杂庞大时，官方文档是这么说的：

>Domains serve several useful purposes. They give Cocoa programs a way to identify the OS X subsystem that is detecting an error. They also help to prevent collisions between error codes from different subsystems with the same numeric value. In addition, domains allow for a causal relationship between error codes based on the layering of subsystems; for example, an error in the NSOSStatusErrorDomain may have an underlying error in the NSMachErrorDomain.

用户也可以为自己的framework或者app定义自己的domain，官方推荐的domain命名规则是：
`com.company.framework_or_app.ErrorDomain`。

## Error Code

Error Code的类型是signed integer。Error Code指定了特殊的错误。这个信息对于程序开发来说极为有用。比如访问URL资源timeout错误对应的是`NSURLErrorTimedOut`（`-1001`）。

那么如何知道各个error code对应什么样的值呢？iOS开发中常用的error code所对应的头文件如下：

* Foundation/FoundationErrors.h - Generic Foundation error codes
* CoreData/CoreDataErrors.h - Core Data error codes
* Foundation/NSURLError.h - URL error codes

以Foundation/NSURLError.h为例，其中的URLError Code值从`NSURLErrorDataLengthExceedsMaximum`到`NSURLErrorCancelled`，二者分别对应`-1103`和`-999`。如果对所有网络error笼统处理，这两个值可以为我所用。

## The User Info Dictionary

Every NSError object has a “user info” dictionary to hold error information beyond domain and code.

User info可以包含很多自定义信息，最常用的或许是localized error information。访问localized error information有两种方式，其一是访问NSError的localizedDescription属性，其二是访问`NSError#userInfo`的`NSLocalizedDescriptionKey`域。

关于user info dictionary，比较常见，这里不多讲了，更多内容参考《Error Handling Programming Guide》。