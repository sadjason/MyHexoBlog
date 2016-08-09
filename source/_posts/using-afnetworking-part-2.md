title: AFNetworking使用笔记 第二弹
date: 2015-04-05 02:04:32
tags: AFNetworking
categories: iOS

---

## 写在前面

接着[AFNetworking使用笔记 第一弹](/using-afnetworking-part-1/)继续写。本文分析两个问题：

* AFNetworking网络任务的创建是在哪个线程执行的？
* 网络任务的completion handler在哪一个线程被调用？

## 发送请求消息

这一部分内容的着重点在于回答「AFNetworking网络任务的创建是在哪个线程执行的？」这个问题。其实这个问题也可以这样问「AFNetworking网络任务的创建是在main thread中完成的吗？」。

开始分析，以`AFHTTPSessionManager`的POST方法为例，如下：

```objc
- (NSURLSessionDataTask *)POST:(NSString *)URLString
                    parameters:(id)parameters
                       success:(void (^)(NSURLSessionDataTask *task, id responseObject))success
                       failure:(void (^)(NSURLSessionDataTask *task, NSError *error))failure
{
    NSURLSessionDataTask *dataTask = [self dataTaskWithHTTPMethod:@"POST"
                                                        URLString:URLString
                                                       parameters:parameters
                                                          success:success
                                                          failure:failure];
    
    [dataTask resume];
    
    return dataTask;
}
```

其所作的事情很简单，创建一个`NSURLSessionDataTask`实例，并将它激活（resume），然后返回它。

进入`NSURLSessionDataTask`实例创建过程，最终进入如下方法：

```objc
- (NSURLSessionDataTask *)dataTaskWithRequest:(NSURLRequest *)request
                            completionHandler:(void (^)(NSURLResponse *response,
                                                        id responseObject,
                                                        NSError *error))completionHandler
{
    __block NSURLSessionDataTask *dataTask = nil;
    dispatch_sync(url_session_manager_creation_queue(), ^{
        dataTask = [self.session dataTaskWithRequest:request];
    });
    
    [self addDelegateForDataTask:dataTask completionHandler:completionHandler];
    
    return dataTask;
}
```

显然`url_session_manager_creation_queue()`是关键：

```objc
static dispatch_queue_t url_session_manager_creation_queue() {
    static dispatch_queue_t af_url_session_manager_creation_queue;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        af_url_session_manager_creation_queue =
        dispatch_queue_create("com.alamofire.networking.session.manager.creation",
                              DISPATCH_QUEUE_SERIAL);
    });
    
    return af_url_session_manager_creation_queue;
}
```

现在应该可以得出答案了：

* 根据上述的`dataTaskWithRequest:completionHandler:`里的代码段`dispatch_sync(...)`可以知道，创建网络任务的过程是同步完成的；
* 根据`url_session_manager_creation_queue()`的实现代码可以知道，创建网络任务的过程是并不是在main thread中完成的；

这里分析的是基于POST请求消息的data task，其他task也是差不多，总之，我们可以得出结论，网络任务的创建并不是在main thread中进行的。

## 处理响应消息

这一部分内容的着重点在于回答「网络任务的completion handler在哪一个线程被调用？」。

依旧以POST请求消息的响应为例，`dataTaskWithHTTPMethod:URLString:parameters:success:failure:`方法代码如下：

```objc
- (NSURLSessionDataTask *)dataTaskWithHTTPMethod:(NSString *)method
                                       URLString:(NSString *)URLString
                                      parameters:(id)parameters
                                         success:(void (^)(NSURLSessionDataTask *, id))success
                                         failure:(void (^)(NSURLSessionDataTask *, NSError *))failure
{
    NSError *serializationError = nil;
    NSMutableURLRequest *request =
        [self.requestSerializer requestWithMethod:method
                                    URLString:[[NSURL URLWithString:URLString
                                                      relativeToURL:self.baseURL] absoluteString]
                                   parameters:parameters
                                        error:&serializationError];
    if (serializationError) {
        if (failure) {
            dispatch_async(self.completionQueue ?: dispatch_get_main_queue(), ^{
                failure(nil, serializationError);
            });
        }
    
        return nil;
    }
    
    __block NSURLSessionDataTask *dataTask = nil;
    dataTask = [self dataTaskWithRequest:request
                       completionHandler:^(NSURLResponse * __unused response,
                                           id responseObject, NSError *error) {
        if (error) {
            if (failure) {
                failure(dataTask, error);
            }
        } else {
            if (success) {
                success(dataTask, responseObject);
            }
        }
    }];
    
    return dataTask;
}
```

有上述代码可以看到，当构造请求消息失败时，会异步在main thread中调用completionHandler block。

当接收到响应消息时，相应的处理在`AFURLSessionManager`类的
`URLSession:task:didCompleteWithError:`方法中完成，该方法代码如下：

```objc
- (void)URLSession:(__unused NSURLSession *)session
              task:(NSURLSessionTask *)task
didCompleteWithError:(NSError *)error
{
    if (error) {
        dispatch_group_async(manager.completionGroup ?: url_session_manager_completion_group(), manager.completionQueue ?: dispatch_get_main_queue(), ^{
            if (self.completionHandler) {
                self.completionHandler(task.response, responseObject, error);
            }
    
            dispatch_async(dispatch_get_main_queue(), ^{
                [[NSNotificationCenter defaultCenter] postNotificationName:AFNetworkingTaskDidCompleteNotification object:task userInfo:userInfo];
            });
        });
    } else {
        dispatch_async(url_session_manager_processing_queue(), ^{
            NSError *serializationError = nil;
            responseObject = [manager.responseSerializer responseObjectForResponse:task.response data:[NSData dataWithData:self.mutableData] error:&serializationError];
    
            if (self.downloadFileURL) {
                responseObject = self.downloadFileURL;
            }
    
            if (responseObject) {
                userInfo[AFNetworkingTaskDidCompleteSerializedResponseKey] = responseObject;
            }
    
            if (serializationError) {
                userInfo[AFNetworkingTaskDidCompleteErrorKey] = serializationError;
            }
    
            dispatch_group_async(manager.completionGroup ?: url_session_manager_completion_group(), manager.completionQueue ?: dispatch_get_main_queue(), ^{
                if (self.completionHandler) {
                    self.completionHandler(task.response, responseObject, serializationError);
                }
    
                dispatch_async(dispatch_get_main_queue(), ^{
                    [[NSNotificationCenter defaultCenter] postNotificationName:AFNetworkingTaskDidCompleteNotification object:task userInfo:userInfo];
                });
            });
        });
    }
}
```

上述代码中出现了`url_session_manager_processing_queue()`方法，其定义如下：

```objc
static dispatch_queue_t url_session_manager_processing_queue() {
    static dispatch_queue_t af_url_session_manager_processing_queue;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        af_url_session_manager_processing_queue = dispatch_queue_create("com.alamofire.networking.session.manager.processing", DISPATCH_QUEUE_CONCURRENT);
    });

    return af_url_session_manager_processing_queue;
}
```

通过这段代码可以看到，当response出现错误时，AFNetworking会在main thread异步调用completionHandler block；若response没有错误时，就会在异步并行队列中对response进行处理，处理完之后，再在main thread中调用completionHandler block。

总之，请求消息的构建工作是以同步形式在非main thread中进行的；对response的处理是以异步形式在非main thread中处理的；无论是哪个步骤的处理失败和成功，都会在main thread中执行completionHandler block。