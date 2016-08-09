title: AFNetworking使用笔记 第一弹
date: 2015-04-05 02:04:16
tags: AFNetworking
categories: iOS

---

## AFHTTPSessionManager

归根到底，使用AFNetworking的最终目的无非是处理网络任务，网络任务包括三类：

* download task
* upload task
* data task

P.S: 上传、下载都比较容易理解，data task是什么玩意儿？上传、下载一般处理的对象主要是文件，而data task处理的对象是文本数据。譬如登录、修改用户昵称，这类操作就是data task。

忽略开发语言以及框架之类的东东，回想一下，启动一个HTTP任务大概需要哪些过程？答案很简单：构建请求消息 -- 发送请求消息。

**发送请求消息**相对而言比较死板，这一部分往往由框架在底层实现（当然，这一部分也是最复杂的，因为涉及线程神马的）；但是**构建请求消息**相较而言就比较灵活了。

「请求消息」的结构是固定的，包括三个部分：请求行、消息头、消息体。（更多内容参考[HTTP学习笔记](/http/)）。

所以无论是什么HTTP任务，万变不离其宗，无非是针对请求行、消息头、消息体进行不同设置和构建。只是在`NSURLSession`框架里，Apple站在功能的角度做了更进一步的封装，它将request message根据功能封装成download task、upload task、data task。

这3种task分别对应三个类：`NSURLSessionDataTask`、`NSURLSessionUploadTask`、`NSURLSessionDownloadTask`，它们都是`NSURLSessionTask`的子类，继承关系如下：

<div class="imagediv" style="width:612px; height:294px">{% asset_img session-tasks@2x.png %}</div>

关于这3种task，《[从NSURLConnection到NSURLSession](https://objccn.io/issue-5-4/)》有比较清晰的描述：
>当一个`NSURLSessionDataTask`完成时，它会带有相关联的数据，而一个`NSURLSessionDownloadTask`任务结束时，它会带回已下载文件的一个临时的文件路径。因为一般来说，服务端对于一个上传任务的响应也会有相关数据返回，所以`NSURLSessionUploadTask`继承自`NSURLSessionDataTask`。

所以，在`NSURLSession`框架下，启动一个HTTP任务的过程变成了：

1. 创建一个`NSURLSession`实例，并配置之；
2. 创建一个`NSURLSessionTask`实例，并将之关联到`NSURLSession`实例中；
3. 激活`NSURLSessionTask`实例；

P.S: 对于单客户单-单服务器，`NSURLSession`创建一次即可；激活`NSURLSessionTask`调用其`resume`方法即可。

所以，在`AFHTTPSessionManager`中，创建各种HTTP任务流程（参考[github:AFNetworking](https://github.com/AFNetworking/AFNetworking/)）如下：

```objc
NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
AFURLSessionManager *manager = [[AFURLSessionManager alloc] initWithSessionConfiguration:configuration];

NSURL *URL = [NSURL URLWithString:@"http://example.com/download.zip"];
NSURLRequest *request = [NSURLRequest requestWithURL:URL];
// 这里没有指定request的方法名，我猜是使用默认方法POST

NSURLSessionDownloadTask *downloadTask =
[manager downloadTaskWithRequest:request progress:nil destination:^NSURL *(NSURL *targetPath, NSURLResponse *response) {
    NSURL *aDirectoryURL =
    [[NSFileManager defaultManager] URLForDirectory:NSDocumentDirectory
                                           inDomain:NSUserDomainMask
                                  appropriateForURL:nil
                                             create:NO
                                              error:nil];
    return [aDirectoryURL URLByAppendingPathComponent:response.suggestedFilename];
} completionHandler:^(NSURLResponse *response, NSURL *filePath, NSError *error) {
    NSLog(@"File downloaded to: %@", filePath);
}];

[downloadTask resume];
```

这段代码中的`NSURLRequest`对象的创建全部采用默认值，猜测其默认method是POST，在实际应用中，会对`NSURLRequest`做更多的配置；再就是download task的创建显然不止一种，根据我的理解，基于`NSURLSessionDataTask`，GET请求消息、POST请求消息都可以完成download task，只是自己要写更多的代码，譬如将download结果从response中取出来。

```objc
// data task
NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
AFURLSessionManager *manager = [[AFURLSessionManager alloc] initWithSessionConfiguration:configuration];
    
NSURL *URL = [NSURL URLWithString:@"http://example.com/upload"];
NSURLRequest *request = [NSURLRequest requestWithURL:URL];
// 这里没有指定request的方法名，我猜是使用默认方法POST
    
NSURLSessionDataTask *dataTask =
[manager dataTaskWithRequest:request
           completionHandler:^(NSURLResponse *response, id responseObject, NSError *error) {
               if (error) {
                   NSLog(@"Error: %@", error);
               } else {
                   NSLog(@"%@ %@", response, responseObject);
               }
}];
[dataTask resume];
```

实际应用中，处理data task时，不会直接像这样写代码，更多的时候会直接使用`AFHTTPSessionManager`中定义的POST、GET等方法，虽然这些方法的实现和这段代码类似...

Data task和download task相对来说比较简单一些，但upload task就复杂多了。根据复杂程度来看，upload task可以分为**单文件上传**和**多文件上传**。对于前者，相较而言比较简单，直接把要上传的文件作为POST请求消息的消息体即可。对于多文件上传，就复杂得多了。

P.S: 多文件上传并不是很复杂，只要熟读了HTTP相关文档，了解了`multipart/form-data`相关概念之后，实现多文件上传代码也是挺容易的，关于`multipart/form-data`，[HTTP学习笔记](/http/)有介绍。

先说单文件上传，单文件上传根据文件来源构建`NSURLSessionUploadTask`对象的方式有这么几种：

```objc
// 1. 单文件（file路径）上传，进度信息，completion handler
uploadTaskWithRequest:fromFile:progress:completionHandler:
// 2. 单文件（file data）上传，进度信息，completion handler
uploadTaskWithRequest:fromData:progress:completionHandler:
```

具体实现如下：

```objc
// 单文件（file路径）上传
NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
AFURLSessionManager *manager = [[AFURLSessionManager alloc] initWithSessionConfiguration:configuration];

NSURL *URL = [NSURL URLWithString:@"http://example.com/upload"];
NSURLRequest *request = [NSURLRequest requestWithURL:URL];

NSURL *filePath = [NSURL fileURLWithPath:@"file://path/to/image.png"];
NSURLSessionUploadTask *uploadTask =
[manager uploadTaskWithRequest:request
                      fromFile:filePath
                      progress:nil
             completionHandler:^(NSURLResponse *response, id responseObject, NSError *error) {
                 if (error) {
                     NSLog(@"Error: %@", error);
                 } else {
                     NSLog(@"Success: %@ %@", response, responseObject);
                 }
             }];
[uploadTask resume];
```

多文件上传时，需要自行写更多代码构建消息体，具体实现如下：

```objc
NSMutableURLRequest *request =
[[AFHTTPRequestSerializer serializer] multipartFormRequestWithMethod:@"POST"
                                                           URLString:@"http://example.com/upload"
                                                          parameters:nil
                                           constructingBodyWithBlock:^(id<AFMultipartFormData> formData) {
                                               [formData appendPartWithFileURL:[NSURL fileURLWithPath:@"file://path/to/image.jpg"]
                                                                          name:@"file"
                                                                      fileName:@"filename.jpg"
                                                                      mimeType:@"image/jpeg" error:nil];
                                           }
                                                               error:nil];
    
AFURLSessionManager *manager =
[[AFURLSessionManager alloc] initWithSessionConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration]];
NSProgress *progress = nil;
    
NSURLSessionUploadTask *uploadTask =
[manager uploadTaskWithStreamedRequest:request
                              progress:&progress
                     completionHandler:^(NSURLResponse *response, id responseObject, NSError *error) {
                         if (error) {
                             NSLog(@"Error: %@", error);
                         } else {
                             NSLog(@"%@ %@", response, responseObject);
                         }
                     }];
    
[uploadTask resume];
```

当然，多文件上传代码也可用来实现单文件上传。

本文站在应用的角度对AFNetworking使用进行了简单的描述。但对`NSURLSession`的阐述不够，希望以后补充吧。

## 本文参考

1. [从NSURLConnection到NSURLSession](https://objccn.io/issue-5-4/)，这篇文章的作者是Mattt Thompson，也就是AFNetworking的作者
2. [AFNetworking使用文档](https://github.com/AFNetworking/AFNetworking/)