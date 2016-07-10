title: 使用hexo搭建静态博客
date: 2015-10-03 17:51:27
tags: Hexo
categories: Others
---

## 写在前面

[Hexo](https://hexo.io/)通常被用来搭建静态博客。使用hexo已经有近一年时间了，之前使用的是pacman主题，pacman主题架子非常不错，使用起来非常方便，但是不够漂亮，换个主题的心思由来已久；之前写了七八十篇博客，数量还不错，然而质量不高，大多数博客几乎只有自己才能看得懂；近期比较闲一点，决定对之前的博客进行整理，一来将博客面子拾掇得更漂亮一点，二来将确保博文的质量更高一些。

Hexo的基础使用门槛比较低，介绍hexo的博客非常多，hexo官网资料质量也不错（甚至还有中文版本）。但笔者在这第二次较详细了解hexo的过程中，仍然有一些问题，本文旨在将过程中遇到的问题记录下来。

## 安装hexo和相关插件

Hexo是基于Node.js的package，Node.js package一般使用npm工具管理（包括安装、更新、卸载），换句话说，在安装hexo之前，得安装npm。

除去Node.js和npm的安装之外，使用hexo创建blog site主要包括两个步骤：
1. 安装hexo-cli包；
2. 初始化blog site并安装一些package；

对于基于Node.js的package的安装，从安装路径来看，分为两种：安装到全局、安装到当前目录。

>P.S: 如何查看全局目录呢？输入指令`npm root -g`即可。

我猜「hexo-cli」是「hexo client」的缩写，安装它之后我们才可以使用`hexo`命令搭建blog site，因此一般把此package安装到全局：

```sh
# 参数-g指示安装到全局
npm install hexo-cli -g
```

安装hexo-cli之后便可使用hexo命令搭建blog site：

```sh
# 初始化blog site
hexo init blog_directory
```

该命令会在当前工作目录创建一个名为`blog_directory`的目录，并copy一些data到此目录中，其中包括package.json文件，该文件列举了一些blog site依赖的packages信息，可把它们认为是博客的插件，如下：

```json
{
  ...,
  "dependencies": {
    "hexo": "^3.1.0",
    "hexo-generator-archive": "^0.1.2",
    "hexo-generator-category": "^0.1.2",
    "hexo-generator-index": "^0.1.2",
    "hexo-generator-tag": "^0.1.1",
    "hexo-renderer-ejs": "^0.1.0",
    "hexo-renderer-stylus": "^0.3.0",
    "hexo-renderer-marked": "^0.2.4",
    "hexo-server": "^0.1.2"
  }
}
```

简单来说，安装这些插件后，博客才支持`hexo s`、`hexo g`等操作，才支持category、tag等。

``` sh
# 进入blog site目录
cd blog_directory
# 安装blog依赖的package
npm install
```

很容易想到这么一个问题：可以不可以把所有pacakge安装到全局呢？
当然可以！但何必这么干呢？

值得一提的是，这些插件只是默认提供的插件，日后还可以安装一些其他插件，譬如后文提到的「支持RSS的插件」「支持sitemap的插件」等。安装这些额外的插件只需执行：

```sh
# 进入blog site目录
cd blog_directory
# save参数是为了将other_package信息保存到package.json中
npm install other_package --save
```

OK，关于hexo及其插件的安装就介绍到这里了！

## Hexo主题

Hexo主题非常繁多，可参考：
* [github - hexo/themes](https://github.com/hexojs/hexo/wiki/Themes)
* [有那些好看的hexo主题？](http://www.zhihu.com/question/24422335)

笔者刚开始使用的主题是pacman，但该主题不够漂亮，查看了很多主题后，认为[hexo-theme-yilia](https://github.com/litten/hexo-theme-yilia)最能满足我的审美需求，于是决定选用这个作为我的新主题。

笔者的博客在yilia的基础上作了稍微的修改，yilia主题非常漂亮，感谢[@Litten](http://litten.github.io/)的分享！但对比我之前用过的pacman，yilia的组织结构并不是特别好，修改起来比较麻烦，二次定制比较不是很方便。

>Note: 个人经验，不要轻易更新hexo版本，尤其不要轻易替换hexo主题，二次修改太麻烦了，且都是脏活。

## Asset Folders

Asset Folders在hexo中应该早就存在了，只是之前没注意到它！

资源（Asset）代表source文件夹中除了文章以外的所有文件，例如图片、CSS、JS 文件等。Hexo提供了一种更方便管理Asset的设定：post_asset_folder。该设定可以在_comfig.yml中配置：

```json
post_asset_folder: true
```

将post_asset_folder属性设置为True的效果是：使用`hexo new article-demo`命令创建新文章时，除了在`source/_post/`目录中生成article-demo.md文件之外，还会创建一个同名目录`article-demo/`。

如果在article-demo.md中引用一些图片、js文件等，则可以将这些文件放在`article-demo/`中，其好处显而易见，文章和文章相关资源联系更紧密，逻辑性更强，也更方便管理。除此之外，在article-demo.md中访问`article-demo/`目录中的资源时，不必书写资源的全路径，直接标注其名字即可，当然，在这种情况下，访问资源的语法必须满足hexo所定义的，如下：

```
{% asset_path slug %}
{% asset_img slug [title] %}
{% asset_link slug [title] %}
```

譬如，在article-demo.md插入来自`article-demo/`的图片pic.png，可以这么写：

```
{% asset_img pic.png "title of the picture" %}
```

然而，美中不足的是，asset_img不支持自定义图片的宽度和高度（主要是为retina屏幕考虑）。如果hexo对retina屏幕有更完善的支持（譬如能够自动处理@2x.png图片的尺寸）该多好啊！

虽然Asset Folders有功能缺陷，但还是很想用它。只能想办法解决其**不能设置width和height**的缺陷了。

没get到太多前端技能，只能想到一种拙劣但简单的解决方案。

第一步，是在合适的css文件中，添加如下css代码：

```css
.imagediv {
  margin-left: auto; 
  margin-right: auto;
}
.imagediv img {
  margin: 0 auto;
  width: 100%;
  height: 100%;
}
```

>P.S: 以[yilia](https://github.com/litten/hexo-theme-yilia)主题为例，如上这部分代码添加在theme/yilia/source/_partial/article.styl的`.article-entry`块中。

第二步，在使用asset_img时套上一层class值为`"imagediv"`的div。

举个例子，写入如下内容：

```
<div class="imagediv" style="width: 135px; height: 135px">
  {% asset_img steve-jobs.png 史蒂夫·乔布斯 %}
</div>
```

这段代码能够以width=135、height=135的规格显示像素为270x270的图片，如下：

<div class="imagediv" style="width: 135px; height: 135px">{% asset_img steve-jobs.png 史蒂夫·乔布斯 %}</div>


## 添加RSS

Hexo提供了RSS的生成插件，只是默认情况下没有安装，需要手动安装和设置：

1. 安装RSS插件到本地，在博客根目录下执行`npm install hexo-generator-feed --save`；
2. 开启RSS功能，编辑博客根目录下的_config.yml文件，添加如下代码：

```
plugins:
  - hexo-generator-feed
```

如此这般，我们便可以在blog_url/atom.xml访问到RSS信息。

## 添加sitemap

同样，也需要手动安装sitemap插件：

1. 安装sitemap到本地，在博客根目录下执行`npm install hexo-generator-sitemap --save`；
2. 开启sitemap功能，编辑博客根目录下的_config.yml文件，添加如下代码：

```
plugins:
  - hexo-generator-sitemap
```

同样，我们便可以在blog_url/sitemap.xml访问到sitemap信息。和RSS不同，sitemap的初衷是给搜索引擎看的，目的是为了提高搜索引擎对博客站点的收录效果，最好手动到google和百度等搜索引擎提交sitemap.xml。

>P.S: 要想自己的博客被搜索到，还需要将sitemap信息提交到搜索引擎（百度和Google），以后在补充吧！

## 添加Disque评论

Hexo已经集成了Disque的配置，添加Disque比较简单。先在[Disque](https://disqus.com)中注册账户，填写一些基本信息，然后在博客的_config.yml中添加如下代码：

```
disqus_shortname: sadjason
```

>Note: 在博客的_config.yml中添加上述代码，而不是themes/yilia/_config.yml中添加哦！

## 本文参考

* [Hexo系列教程](http://zipperary.com/2013/05/30/hexo-guide-4/)
* [Hexo官方文档](https://hexo.io/zh-cn/docs/)
* [Github搭建hexo——更换主题、Disqus、RSS](http://blog.csdn.net/u010053344/article/details/50701191)