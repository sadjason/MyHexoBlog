title: 使用Sublime Text
date: 2015-10-06 17:34:30
categories: Others
tags:
- Sublime Text
- Tools
---

Sublime Text是一款强大的代码编辑器，其强大体现在三个方面：
1. 丰富的快捷键组合
2. 实用的插件
3. 支持代码片段snippet

>P.S: 其提供的插件API基于Python。

为了叙述方便，下文将以「ST」代替「Sublime Text」。

接触Sublime Text由来已久，只是一直都是极轻量级使用（当成普通的文本编辑器），现在想挖掘其更深层次的功能，将一些东西记录下来，以免以后忘记了。本文所涉及的ST版本为Version 3，所参考的系统环境是Mac OS X。

## 基础配置

ST的配置主要是通过修改配置文件来实现的。

**配置文件**

其主要配置文件有两个：`Settings - Default`和`Settings - User`，如下图：

<div class="imagediv" style="width: 378px; height: 288px">{% asset_img sublime-text-configuration.png Sublime Text Configuration %}</div>

二者都以JSON格式记录配置信息，其中前者记录着ST的默认配置，禁止用户修改（强行修改也是可以的，在指定目录新建一个文件即可，但最好不要修改）；后者默认为空，允许用户修改。User配置文件的内容会覆盖Default的相应内容，所以只要修改User配置文件就好了。

配置文件的item有很多，每一项是什么含义，上网一搜就有不少解释，况且Default配置文件中的注释已经非常清晰了。如果你不知道写什么，可以从Default配置文件里拷贝相应的内容到User配置文件里，然后将参数的值修改为希望的样子。我的`Settings - User`配置如下：

``` json
{
    "theme": "Soda Dark 3.sublime-theme",
    "color_scheme": "Packages/Color Scheme - Default/Zenburnesque.tmTheme",
    "highlight_line": true,
    "save_on_focus_lost": true,
    "scroll_past_end": false,
    "tab_size": 4,
    "translate_tabs_to_spaces": true,
    "trim_trailing_white_space_on_save": true,
    "wrap_width": 90,
    "soda_folder_icons": true,
    "update_check": false,
}
```

值得一提的是，最后一项`"update_check":false`并不能在`Settings - Default`中找到，但确实有效，其作用是「disable每次启动ST后自动检查更新」。

**theme和color_scheme**

我在`Settings - User`中将theme设置为「Soda Dark」，[Soda Theme](https://github.com/buymeasoda/soda-theme)让ST的整体风格更符合Mac，但该theme只会变更整个程序的骨架风格，不会更改编辑区（譬如背景色及高亮显示）。编辑区的背景色及高亮显示是由color_scheme负责的。

theme在ST中也以插件的形式存在，简单来说，设置theme的步骤是：
1. 下载安装theme package；
2. 配置`Settings - User`，配置theme这个item；

那么color_scheme是如何设置的呢？

和theme不同，ST的color scheme不是以插件的形式安装的，支持ST的color scheme一般都对应一个.tmTheme文件，github中能找到很多此类color theme配色方案文件，弄一个心仪的下来添加到ST中即可。问题是拷贝到哪里呢？在网上看到很多博客的说法是「只需要将相应的文件放在Packages/Color Scheme - Default/目录下就行」。但事实上，在Mac环境下我找不到所谓的`Color Scheme - Default/`目录，对于ST Version 3而言，倒是能够在与`Packages/`同级目录`Cache/`中找到名为`Color Scheme - Default/`的目录...

P.S：如何快速找到`Packages/`目录呢？点击`Preferences` > `Browse Packages…`菜单，所进入的目录即`Packages/`目录。

总之，最后，我的处理策略是：
1. 在`Packages/`目录下新建一个`Color Scheme - User/`目录；
2. 将下载的.tmTheme文件拷贝到该目录中（以Flatland Dark.tmTheme为例）；
3. 然后修改`Settings - User`配置文件中的color_scheme，设置其值为"Packages/Color Scheme - User/Flatland Dark.tmTheme"；

立马生效，且在`Preferences` > `Color Scheme`中看到多了一个`Color Scheme - User`选项，如下：

<div class="imagediv" style="width: 741px; height: 326px">{% asset_img sublime-text-color-scheme.png Sublime Text Color Scheme %}</div>

如此这般，我们便可以通过UI选项卡选择color scheme了，当然，也可以在`Settings - User`中配置color_scheme。

**设置Tab**

代码迁移中最频繁的问题是处理Tab和Spacing；比较好的习惯是在编辑代码时，使用数个空格代替Tab。

上文在`Settings - User`中设置了两个与Tab相关的item：translate_tabs_to_spaces和tab_size。前者设置为True是为了让编辑器自动将Tab输入处理成空格，后者用于指示「一个Tab等于多少个Spacing」。

当然，除了配置`Settings - User`之外，也可以通过选项卡设置，如下：

<div class="imagediv" style="width: 518px; height: 557px">{% asset_img sublime-text-indentation.png Sublime Text Indentation %}</div>


`Settings - User`可配置的内容有很多，上文只是列出了其中几个于我而言比较通用的配置。

## 使用Package Control管理插件

ST中所谓的「插件」，其专业说法是「Package」。

ST之所以强大的一个重要原因是其具备强大的插件系统，像很多其他的集成环境或开发平台一样，ST中也有一个管理插件的插件，名为「Package Control」，Package Control的[官方](https://packagecontrol.io/)描述如下：
>The Sublime Text package manager that makes it exceedingly simple to find, install and keep packages up-to-date.

**安装Package Control**

对于Package Control的安装，官网中有比较详细的安装[说明](https://packagecontrol.io/installation)，自动安装常常出错，我比较倾向手动安装，手动安装Package Control步骤如下：
1. 点击`Preferences` > `Browse Packages…`菜单；
2. 跳入上一级目录，进入`Installed Packages/`目录；
3. 下载[Package Control.sublime-package](https://packagecontrol.io/Package%20Control.sublime-package)，将之拷贝到`Installed Packages/`目录；
4. 重启Sublime Text；

P.S：这里提到的`Installed Packages/`目录和上文的`Packages/`目录是平级关系，手动安装的插件一般都放到该目录中。

在初步使用Package Control的过程中，笔者自然而然产生了一些疑虑：
* 如何更新Package Control？
* 如何查看通过Package Control下载的Package？
* 如何remove通过Package Control？
* 如何更新通过Package Control安装的Package？

以上文提到的Soda Theme为例，通过Package Control安装Soda Theme的步骤如下：
1. 触发`Cmd+Shift+P`，激活命令框，输入「Package Control: Install Package」（或者干脆「pcinstall」）命令；
2. 等待片刻（加载Package List，ST的底部状态栏可看到加载状态）后，输入SoDa过滤得到Soda Theme Package，点击后即会自动安装（ST的底部状态栏可看到安装状态）；

安装完成后可在`Installed Packages/`目录下看到`Theme - Soda.sublime-package`文件，即安装的package文件。

到此为止，「更新插件」「移除插件」等操作就比较容易理解了。

ST的插件实在太丰富，Package Control官网的[流行插件页面](https://packagecontrol.io/browse/popular)能够看到更详细的插件列表。插件相关的使用以后单辟博客介绍。

## 快捷键

熟练使用ST有两大关键，除了熟练使用合适的插件之外，还得灵活使用ST快捷键。

ST原生自带的快捷键也能很大程度上方便开发，这里以Mac为主，Windows多数与其相似。下文中的会用一些特殊字符表示Mac环境下的一些按键，对应关系如下：
* `⌘`Command key
* `⌃`Control key
* `⌥`Option key
* `⇧`Shift Key

为了方便记忆，将快捷键分成了8个类型，分别为：
* Edit（编辑）
* Selection（光标选中）
* Find（查找）
* View（视图）
* Go to（跳转）
* Project（工程）
* General（通用）
* Tabs（标签）

### Edit快捷键

* `⌘[`向左缩进
* `⌘]`向右缩进
* `⌘⌃↑`与上一行互换
* `⌘⌃↓`与下一行互换
* `⌘⇧D`复制粘贴当前选中内容（如果没有选中内容，则复制粘贴当前行）
* `⌘J`拼接当前行和下一行
* `⌘←`去往行的开头
* `⌘→`去往行的结尾
* `⌃K`从光标开始的地方删除到行尾
* `⌃⇧K`删除一整行
* `⌘⇧⏎`向光标前插入一行
* `⌘⏎`向光标后插入一行
* `⌘⌥T`插入特殊字符
* `⌃D`向后删除

* `⌘D`选中相同的词
* `⌘L`选中一行（没按一下选一行）
* `⌃⇧↑`一行一行向上选中
* `⌃⇧↓`一行一行向下选中

### Find快捷键

* `⌘F`普通查找
* `⌘G`查找下一个
* `⌘⇧F`在文件夹中查找

### View快捷键

* 推荐使用Origami插件，可以随意对sublime进行分割

### Go to快捷键

* `⌘P`跳转文件
* `⌘R`定位文件中的方法（对于markdown，可以定义「标题」）
* `^G`定位文件中的行号
* `⌘⌥→`下一个打开的文件

### Project快捷键

* `⌘⌃P`在保存过的工程中切换

### General快捷键

* `⌘⇧P`打开命令行
* `⌘K`/`⌘B`隐藏/打开侧边栏

### Tab快捷键

* `⌘⇧T`打开最后一次关闭的文件
* `^Tab`循环遍历Tab
* `^⇧Tab`反方向循环遍历Tab

## ST的高级配置

**在终端中使用ST打开/创建文件**

很简单，使用ln命令创建一个软链接即可：
``` sh
ln -sf /Applications/Sublime\ Text.app/Contents/SharedSupport/bin/subl /usr/local/bin/subl
```

之后便可在终端中基于subl命令，使用ST创建或打开文件。

**修改ST图标**

ST的图标太丑了，网上有许多方法修改ST图标的博客，譬如[这篇](http://davidwalsh.name/mac-app-icons)，但是都无效。希望以后能解决这个问题。

**ST和Alfred配合使**
引入alfred workflow即可，详见[这里](https://github.com/franzheidl/alfred-workflows/tree/master/open-with-sublime-text)。

## 本文参考

* [知乎：Sublime Text 有哪些使用技巧？](http://www.zhihu.com/question/24896283)
* [知乎：Sublime Text 3插件推荐？](http://www.zhihu.com/question/24736400)