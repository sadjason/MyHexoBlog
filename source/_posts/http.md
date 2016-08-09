title: HTTP学习笔记
date: 2015-04-04 15:57:35
tags:
- HTTP
categories: Others

---

## 写在前面

对于程序员而言，HTTP协议是一个非常熟悉的名词。每个人或多或少都对此有所了解。知道它是Web设计、客户端开发等的基础协议。通过它可以让服务器和客户端进行XML、JSON甚至是二进制数据的传输。笔者近期在对之前的程序（iOS项目）的梳理过程中，发现对HTTP协议的了解程度还远远谈不上熟悉，更多的时候是能够凭借网络资源找到某种问题的解决方案，譬如图片的上传与下载，多个文件的上传与下载等等之类的。但是很难从原理层面对这些或有用或无用的信息进行甄别，更谈不上完全独立针对某个网络问题进行合理的程序设计。于是，决定对HTTP相关知识进行一次梳理。

本文的内容只是站在自身的角度，将那些不是特别明白但是可能会经常涉及的知识点进行整理。关于HTTP的更多内容还得看[RFC2616文档](http://www.ietf.org/rfc/rfc2616.txt)。

说明：本文会经常出现「客户端」「服务器」「发送端」「接收端」这些概念；就本文而言，「服务端」指的是管理HTTP资源的端点，「客户端」指的是请求HTTP资源的端点；而「发送端」和「接收端」是相对的，譬如客户端发送一个GET请求到服务器，则针对这条消息，客户端是发送端，服务器是接收端；然后服务器返回一个消息给客户端，针对这条响应消息，服务器是发送端，客户端是接收端。

## HTTP协议须知

HTTP，即所谓的超文本传输协议，全称叫**HyperText Transfer Protocol**，是一个客户端和服务端请求和应答的标准（TCP）：

* HTTP是应用层的协议
* HTTP协议是建立在TCP协议之上的协议，后者是传输层的协议

## HTTP消息概述

HTTP消息，顾名思义，指的是遵循HTTP协议，客户端和服务端的交流语言（笔者的一家之言，注意，在HTTP协议中，客户端和服务端是相对的，称**请求端**和**应答端**或许更好）。

从方向或应答角度来看，HTTP消息有两种：**请求消息（Request）**、**响应消息（Response）**；

HTTP协议定义了HTTP消息的格式，请求消息和响应消息都由1个开始行（start-line）、0个或多个消息头（headers）、可有可无的消息主体（message-body）组成。

```
generic-message =
    start-line                 ; 开始行
    *(message-header CRLF)     ; 消息头
    CRLF
    [message-body]             ; 消息主体
    
;  其中CRLF表示“结束符”
;  *表示“0个或多个“”
;  []表示“可有可无”
```

下面将围绕**开始行**、**消息头**、**消息主体**这几个概念进行详细阐述。

### 开始行

首先是**开始行**，开始行是什么样的格式？这可不一定，因为对于不同的消息类型（请求消息和响应消息），开始行的格式是不同的：

* 对于请求消息，**start-line**是Request-Line（请求行），请求行的格式后文会介绍；
* 对于响应消息，**start-line**是Status-Line（状态行），状态行的格式后文会介绍；

### 消息头

然后是**消息头**，根据作用域来分，消息头分为：常用头（general-header）、请求头（request-header）、响应头（response-header）、实体头（entity-header）。无论如何，它们的格式总是这样：

```
message-header = field-name ":" [field-value]
```

其中`field-name`对大小写不敏感。

### 消息主体

接着是**消息主体**，RFC2616中讲：
>The message-body (if any) of an HTTP message is used to carry the entity-body associated with the request or response.

如下：

```
message-body = entity-body | <entity-body encoded as per Transfer-Encoding>
```

这是什么意思呢？

HTTP协议中有两个概念非常容易混淆，消息主体（message-body）和实体主体（entity-body）：

* entity-body可以被理解为客户端想让服务端看到的内容
* message-body指的是服务端接收到的（来自于客户端）实际内容

二者的区别在于传输过程中可能会对entity-body进行编码；
                    
并非任何**请求消息**或者**响应消息**都可以有**消息主体**这一部分内容。**消息主体**可否存在于某个消息中由**请求类型**和**响应类型**决定的。

P.S: 或许其他资料没有「请求类型」「响应类型」这种说法，不同的**请求类型**在本文的意思是指不同方法的请求，譬如我们熟悉的GET请求、POST请求等；同样，不同的**响应类型**指不同状态码的请求，譬如404响应、403响应、200响应等；

**Note:** 对于请求消息，某些情况是不允许包含消息主体的，譬如HEAD请求；对于服务端而言，在处理不允许包含消息主体的请求消息时，应该忽略不理会这些消息的消息主体，哪怕这些消息包含了消息主体。但对于某些允许包含消息主体的请求消息，也可以不包含消息主体，譬如对于POST请求消息，本身是被允许包含消息主体的，但真正发送请求消息时，也可以不包含任何的消息主体。

那么问题来了，服务器如何识别某个可能包含消息主体的请求消息是否真的包含消息主体呢？

根据RFC2616的描述，服务器可以通过两个message-header来判别某个请求消息中是否包含请求主体，这两个message-header分别是Content-Length和Transfer-Encoding，前者记录entity-body的长度，后者记录对entity-body的编码标准。

更细化来看，客户端或者服务器的处理逻辑消息主体的逻辑如下：

1. 对于不能包含消息主体的消息（譬如HEAD请求消息、1xx响应消息），完全忽略消息主体，当它不存在；
2. 如果消息中出现「Transfer-Encoding」头，并且其值不是「identity」时：then the transfer-length is defined by use of the “chunked” transfer-coding (section 3.6), unless the message is terminated by closing the connection.
P.S: 不太理解这一句，但感觉的意思是，如果所接收到的消息中包含「Transfer-Encoding」头且其值不是「identity」，则计算所接收到消息主体并处理之；
3. 如果消息中出现「Content-Length」头，则认为所接收到的消息主体的长度为Content-Length的值，并且该值也是entity-body的值（即认为entity-body没有进行特别的传输编码）；
P.S: 如果entity-body进行了特别的传输编码，千万不要定义「Content-Length」这个header，因为这会让服务器造成误解；
P.S: 如果消息中同时包含「Transfer-Encoding」头和「Content-Length」头，则后者会被忽略不处理；
4. If the message uses the media type “multipart/byteranges”, and the transfer-length is not otherwise specified, then this self-delimiting media type defines the transfer-length. This media type MUST NOT be used unless the sender knows that the recipient can parse it; the presence in a request of a Range header with multiple byte-range specifiers from a 1.1 client implies that the client can parse multipart/byteranges responses
P.S: 看不懂！

## 请求消息

### 请求行

正如前文所述，对于请求消息，**开始行**被称为**请求行**，其格式如下：

```
Request-Line = Method SP Request-URL SP HTTP-Version CRLF
;  
;  SP表示“分隔符”
;  CRLF表示“结束符”
```
其中，Method包括：
```
Method = "OPTIONS"
         | "GET"
         | "HEAD"
         | "POST"
         | "PUT"
         | "DELETE"
         | "TRACE"
         | "CONNECT"
         | extension-method
```
关于Method这部分内容，相对比较熟悉，直接摘抄RFC2616：
>The list of methods allowed by a resource can be specified in an Allow header field (section 14.7). The return code of the response always notifies the client whether a method is currently allowed on a resource, since the set of allowed methods can change dynamically.
&nbsp;
An origin server SHOULD return the status code 405 (Method Not Allowed) if the method is known by the origin server but not allowed for the requested resource, and 501 (Not Implemented) if the method is unrecognized or not implemented by the origin server.
&nbsp;
The methods GET and HEAD MUST be supported by all general-purpose servers. All other methods are OPTIONAL;

## 响应消息

### 状态行

正如前文所述，对于响应消息，**开始行**被称为**状态行**，其格式如下：

```
Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
;  
;  SP表示“分隔符”
;  CRLF表示“结束符”
;  Status-Code表示“状态码”，譬如经典的404
;  Reason-Phrase表示“状态码的简单文字描述”
```

### 状态码

状态码都是三位数，第一位数（最高位）定义响应类别，第一位有5种值：

* 1xx: Informational - Request received, continuing process
* 2xx: Success - The action was successfully received, understood, and accepted
* 3xx: Redirection - Further action must be taken in order to complete the request
* 4xx: Client Error - The request contains bad syntax or cannot be fulfilled
* 5xx: Server Error - The server failed to fulfill an apparently valid request


## 实体（entity）

如果不被请求方法和响应状态码限制，**请求消息**和**响应消息**都可以传输entity。实体包括**实体头**（entity-header）和**实体主体**（entity-body），有些响应只包括实体头（譬如针对HEAD请求的响应）。

### 实体类型

实体可能是一个文件，也可能是一段文本，所以必然有一个重要的**属性**用来描述**实体类型**，这个**属性**是**实体头**Content-Type，实体类型比较繁多，譬如.png文件对应的实体类型是`image/png`，更多Content-Type参考[这里](http://tool.oschina.net/commons)。


## POST、GET、HEAD等方法

RFC2616对请求消息的方法进行了分类：**安全方法**（Safe Methods）和**幂等方法**（Idempotent Mehtods）。

要理解**安全方法**，先介绍一个概念：副作用，**副作用**指当你发送完一个请求以后，网站上的资源状态没有发生修改，即认为这个请求是无副作用的。比如注册用户这个请求是有副作用的，获取用户详情可以认为是无副作用的。

对于**幂等方法**，**幂等**是说一个请求原封不动的发送N次和M次（N不等于M，N和M都大于1）服务器上资源的状态最终是一致的。比如发贴是非幂等的，重放10次发贴请求会创建10个帖子。但修改帖子内容是幂等的，一个修改请求重放无论多少次，帖子最终状态都是一致的。

**请求消息**的方法众多，有些的方法只是读取服务器的资源，有的方法可能会修改服务器的资源。GET和HEAD属于前者，它们只是获取资源，这些方法被称为**安全方法**；POST、PUT、DELETE属于后者，它们可能使服务器的资源发生变化，这些方法被称为**幂等方法**。

P.S: 根据我的理解，所谓的**安全方法**和**幂等方法**只是一种臆想，不是绝对的。举个例子，服务器有某篇文章，现在浏览器通过GET方法获取这篇文章，当然，客户端并没有修改这篇文章，但是，服务器可能做了这样的处理：将这篇文章的浏览次数+1；客观来讲，这个GET方法还是修改了服务器的资源；所以，知道**安全方法**和**幂等方法**这两个概念就好，不必当真。

### GET方法

GET方法的意思是获取被请求URI（Request-URI）指定的信息（以实体的格式）。如果请求URI 涉及到一个数据生成过程，那么这个过程生成的数据应该被作为实体在响应中返回而不是过程的源文本，除非源文本恰好是过程的输出。

如果请求消息包含If-Modified-Since、If-Unmodified-Since、If-Match、If-None-Match或者If-Range头，GET的语义将变成**条件（conditionall）GET**。一个条件GET方法会请求满足条件头域的实体。条件GET方法的目的是为了减少不必要的网络使用，这通过允许利用缓存里仍然保鲜的实体而不用多次请求或传输客户端已经拥有的实体来实现的。

如果请求方法包含一个Range头域，那么GET方法就变成“部分Get”（partial GET）方法。 一个部分GET会请求实体的一部分。部分GET方法的目的是为了减少不必要的网络使用，可以允许客户端从服务器获取实体的部分数据，而不需要获取客户端本地已经拥有的部分实体数据。

P.S: 关键词 -- **条件GET**和**部分GET**。

### HEAD方法
除了服务器不能在响应里返回消息主体，HEAD方法和GET方法基本一致。HEAD请求的「响应消息」里的「消息头」应该和GET请求的「响应消息」里的「消息头」一致。此方法被用来获取请求实体的元信息而不需要传输实体主体（entity-body）。此方法经常被用来测试超文本链接的有效性、可访问性以及最近的改变等，对于涉及下载的服务，HEAD方法还用来获取「欲下载文件的大小」。

### POST方法

在实际应用中，GET和POST是用得最多的。相对于GET方法，POST方法在Request-URI所标识的资源后附加新的数据，而GET方法直接将数据放在URI中；此外，POST还可以携带「消息主体」，而GET不成。

简而言之：

* GET：安全方法，幂等方法，不可包含消息主体
* POST：非安全方法」，非幂等方法，可包含消息主体

P.S: 更正（2015-08-15），貌似「GET方法不可包含消息主体」的说法不靠谱！

### PUT方法

PUT方法请求服务器去把请求里的实体存储在请求URI（Request-URI）标识下。如果请求 URI（Request-URI）指定的的资源已经在源服务器上存在，那么此请求里的实体应该被当作是源服务器关于此URI所指定资源实体的最新修改版本。如果请求URI（Request-URI）指定的资源不存在，并且此URI被用户代理定义为一个新资源，那么源服务器就应该根据请求里的实体创建一个此URI所标识下的资源。如果一个新的资源被创建了，源服务器必须能向用户代理（user agent）发送201（已创建）响应。如果已存在的资源被改变了，那么源服务器应该发送200（Ok）或者204（无内容）响应。如果资源不能根据请求URI创建或者改变，一个合适的错误响应应该给出以反应问题的性质。实体的接收者不能忽略任何它不理解和不能实现的Content-*（如：Content-Range）头，并且必须返回501（没有被实现）响应。

P.S: 其他方法略过，很少用到。

<div class="imagediv" style="width: 394px; height: 148px">{% asset_img 20150728-01.png %}</div>

## HTTP和MIME

最开始HTTP协议是不允许在「消息实体」中挂载二进制文件的，后来逐渐扩展，开始支持MIME协议，这才允许在消息实体中挂载二进制文件，譬如音频、图片等。所以「实体头」Content-Type的很多值MIME-Type中所定义的值。

### multipleForm/form-data消息

经过不断的演化，HTTP协议借鉴并包容了MIME协议，使得HTTP协议能够服务器和客户端之间传输各种文件（文本文件譬如txt文件、二进制文件譬如png文件），具体的处理方式是将这些文件放在**消息主体**中，关于这个上文已有所述。

但是有一个问题：**消息主体中能够承载多个实体（entity）吗**？换句话说，**如果一次HTTP交互涉及多个文件的传输，该如何处理**？

如果只是基于上文所涉及的内容，是没办法做到在一次HTTP消息传输中传输多个文件的。而实现多个文件的传输就得涉及所谓的`multipleForm/form-data`消息了。

根据HTTP/1.1 RFC2616的协议规定，我们的请求方式只有OPTIONS、GET、HEAD、POST、PUT、DELETE、TRACE等，`multipart/form-data`是个什么东东呢？

对于`multipart/form-data`，似乎没有找到权威的定义，RFC2616对于它的描述只是出现在**Media Type部分**。根据我的理解，姑且认为`multipart/form-data`是一种类型的消息主体。

传输`multipart/form-data`消息主体的基础方法是POST；简单来说，当需要在一次**请求消息**中传输多个文件时，就将着多个文件揉成一个`multipart/form-data`消息主体，然后以POST形式传到服务器。

`multipart/form-data`消息有啥不同之处呢？

首先是Content-Type不同，其他的HTTP消息的Content-Type可能是`image/png`、`text/plain`之类的，但是`multipart/form-data`消息的Content-Type必须是`multipart/form-data`。

其次是`multipart/form-data`的消息主体内嵌了一个或多个其他的消息主体。

举个例子，假设有这么一段HTML代码：

``` HTML
<FORM ACTION="http://server.dom/cgi/handle" ENCTYPE="multipart/form-data" METHOD=POST>
    What is your name? <INPUT TYPE=TEXT NAME=submitter><br />
    What files are you sending? <INPUT TYPE=FILE NAME=pics>
</FORM>
```

对应的网页如下：

<div class="imagediv" style="width: 330px; height: 100px">{% asset_img 20150728-02.png %}</div>

当用户在姓名输入框中输入「张不坏」，并且选择了一个txt文档file1.txt时，客户端发送的HTTP消息体数据可能如下：

```
Content-type: multipart/form-data, boundary=AaB03x
    
--AaB03x
content-disposition: form-data; name="field1"
    
张不坏
--AaB03x
content-disposition: form-data; name="pics"; filename="file1.txt"
Content-Type: text/plain
  ... contents of file1.txt ...
--AaB03x--
```

可以看到，这个示例中涉及两个entity的传输：

* Content-Type为默认值的文本框输入值
* Content-Type为`text/plain`的纯文本文件

P.S: 这里若可以用HTTP协议分析工具分析一下最好了。

关于`multipart/form-data`消息体，还往往涉及boundary之类的更多概念，本文就不针对这个详述了。

还会经常有一种`Content-Type`值为`multipart/mixed`，笔者一度在`multipart/form-data`和`multipart/mixed`之间傻傻分不清楚。对于上述示例中“What files are you sending?”之后的文件选择按钮，当选择多个文件时（譬如keynote文件，事实上这个“文件是一个文件包”，它由多个文件组成），则该entity对应的Content-Type是`multipart/mixed`，表示该form entry对应多个文件，RFC1867的描述如下：
>If multiple files are to be returned as the result of a single form
entry, they can be returned as multipart/mixed embedded within the multipart/form-data.

但无论如何，`multipart/mixed`消息体都是嵌套在`multipart/form-data`内部的子消息体。

P.S: `multipleForm/form-data`消息这一部分内容的叙述不够详细，有时间还是使用HTTP报文分析工具进行更加详细的分析吧。

## 一些常见问题

上文是对HTTP协议的基本描述，这一部分列举一些常见Q/A，结合Q/A，会对HTTP协议有更深刻的理解。

Q: POST方法一般将参数放在消息体中，那么其Content-Type是什么？
A: 初步判断，一般是`application/json`。


## 本文参考

1. HTTP协议RFC2616文档（能够找到中文翻译版，我参考的是“孙超进”翻译版本，感谢他的贡献，但感觉翻译得不是很好）；
2. RFC2046，MIME协议part2，定义了Media Types；
3. RFC1867，Form-based File Upload in HTML；
4. 《[HTTP协议之multipart/form-data请求分析](http://blog.csdn.net/five3/article/details/7181521)》；