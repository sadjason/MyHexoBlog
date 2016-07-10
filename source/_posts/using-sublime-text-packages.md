title: 一些常用的Sublime Text插件
date: 2015-10-08 22:13:04
categories: Others
tags:
- Sublime Text
- Tools
---

本文旨在记录一些笔者曾经使用过的Sublime Text插件，记录插件的使用和配置，以免忘记。

* [**valign**](https://github.com/jarod2d/sublime_valign)

非常轻的一个插件，用于代码对齐，Xcode环境中也有一个类似的插件XAlign，使用非常简单：
1. 选中代码块；
2. 触发`⌘⇧X`（Cmd+Shift+X）；

但默认情况下，触发valign功能的快捷键是`ctrl+\\`，笔者已经习惯了XAlign的快捷键`⌘⇧X`，因此作了一些处理，即选中`Preferences` > `Key Bindings - User`，然后添加一条记录，如下：

``` json
[
    { "keys": ["command+shift+x"], "command": "valign" },
]
```

这便将触发valign的快捷键改为`⌘⇧X`。

* [**Soda Theme**](https://github.com/buymeasoda/soda-theme)

Soda Theme让Sublime Text的整体风格更符合Mac，但该theme只会变更整个程序的骨架风格，不会更改编辑区（譬如背景色及高亮显示）。编辑区的背景色及高亮显示是由color_scheme负责的。