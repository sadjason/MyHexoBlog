title: reactive-cocoa-part-4
date: 2016-07-29 10:27:43
tags:
---

* 信号高阶操作
* 冷信号与热信号

什么叫高阶信号？

* 信号的值还是信号

高阶信号有什么用？

降阶操作，为什么要降阶？

升阶操作，为什么要升阶？

**降阶操作 -- Switch To Latest**

非常重要，经常会用到

**降阶操作 -- Flatten**

`flatten`有点像`merge`。

`flatten:1`其实就是`concat`

三个降阶操作：

* switchToLatest
* flatten
* concat（即flatten:1）

