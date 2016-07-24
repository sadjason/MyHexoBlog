title: Git基础
date: 2015-12-05 11:04:04
tags:
- Git
- Tools
categories: Others
---

## Git起步

目前市场上主流的**版本控制系统**（Version Control Systems，简称VCS）有两种：**集中式版本控制系统**（Centralized Version Control Systems，简称CVCS）、分布式版本控制系统（Distributed Version Control System，简称DVCS）。其中CVCS包括CVS、Subversion以及Perforce。

Git是分布式版本控制系统（除了Git，DVCS还包括Mercurial、Bazaar以及Darcs等）。这一部分阐述Git的工作特点，并介绍一些基本概念。

**分布式 v.s 集中式**

分布式版本控制系统与集中式版本控制系统有何不同呢？

首先，不同于集中式版本控制系统，分布式版本控制系统没有所谓的「中央服务器」，每个人的电脑上都是一个完整的版本库，这样，你工作的时候，就不需要联网了，因为版本库就在你自己的电脑上。既然每个人电脑上都有一个完整的版本库，那多个人如何协作呢？比方说你在自己电脑上改了文件A，你的同事也在他的电脑上改了文件A，这时，你们俩之间只需把各自的修改推送给对方，就可以互相看到对方的修改了。

和集中式版本控制系统相比，分布式版本控制系统的安全性要高很多，因为每个人电脑里都有完整的版本库，某一个人的电脑坏掉了不要紧，随便从其他人那里复制一个就可以了。而集中式版本控制系统的中央服务器要是出了问题，所有人都没法干活了。

在实际使用分布式版本控制系统的时候，其实很少在两人之间的电脑上推送版本库的修改，因为可能你们俩不在一个局域网内，两台电脑互相访问不了，也可能今天你的同事病了，他的电脑压根没有开机。因此，分布式版本控制系统通常也有一台充当「中央服务器」的电脑，但这个服务器的作用仅仅是用来方便交换大家的修改，没有它大家也一样干活，只是交换修改不方便而已。

**直接记录快照，而非差异比较**

Git和其他版本控制系统不同，Git只关心文件数据的整体是否发生变化，而大多数其他系统则只关心文件内容的具体差异。这类系统（CVS、Subversion、Perforce等等）每次记录有哪些文件作了更新，以及都更新了哪些行的什么内容，如下图。

<div class="imagediv" style="width: 400px; height: 155px">{% asset_img deltas@2x.png %}</div>

Git并不保存这些前后变化的差异数据。实际上，Git更像是把变化的文件作快照后，记录在一个微型的文件系统中。每次提交更新时，它会纵览一遍所有文件的指纹信息（即根据文件信息，使用SHA-1算法计算得到的一个40位字串），并对文件作一快照，然后保存一个指向这次快照的索引。为提高性能，若文件没有变化，Git不会再次保存，而只对上次保存的快照作一链接。Git对待数据更像是一个快照流。Git的工作方式如下图所示。

<div class="imagediv" style="width: 400px; height: 150px">{% asset_img snapshots@2x.png %}</div>

**近乎所有操作都是本地执行**

CVCS的几乎所有操作都需要连接网络，譬如使用Subversion和CVS，你能修改文件，但不能向数据库提交修改（因为本地数据库离线了）。

而Git中的绝大多数操作都只需要访问本地文件和资源，不用联网。举个例子，要浏览项目的历史，Git不需外连到服务器去获取历史，然后再显示出来——它只需直接从本地数据库中读取。你能立即看到项目历史。如果你想查看当前版本与一个月前的版本之间引入的修改，Git会查找到一个月前的文件做一次本地的差异计算，而不是由远程服务器处理或从远程服务器拉回旧版本文件再来本地处理。

**时刻保持数据完整性**

在保存到Git之前，所有数据都要进行内容的校验和（checksum）计算，并将此结果作为数据的唯一标识和索引。换句话说，不可能在你修改了文件或目录之后Git一无所知。这项特性作为Git的设计哲学，建在整体架构的最底层。所以如果文件在传输时变得不完整，或者磁盘损坏导致文件数据缺失，Git都能立即察觉。Git使用SHA-1算法计算数据的校验和，通过对文件的内容或目录的结构计算出一个SHA-1哈希值，作为**指纹字符串**。该字串由40个十六进制字符（0-9及a-f）组成，看起来就像是：`24b9da6552252987aa493b52f8696cd6d3b00373`。

Git的工作完全依赖于这类指纹字串，所以你会经常看到这样的哈希值。实际上，所有保存在Git数据库中的东西都是用此哈希值来作索引的，而不是靠文件名。

**多数操作仅添加数据**

常用的Git操作大多仅仅是把数据添加到数据库。因为任何一种不可逆的操作，比如删除数据，都会使回退或重现历史版本变得困难重重。在别的VCS中，若还未提交更新，就有可能丢失或者混淆一些修改的内容，但在Git里，一旦提交快照之后就完全不用担心丢失数据，特别是在养成定期推送到其他仓库的习惯的情况下。

**工作区、版本库以及暂存区**

所谓工作区（working directory，又常称之为「工作目录」），就是Git目录（一般是`.git`目录）之外的区间。

>P.S: 每个项目都有一个**Git目录**（如果`git clone`出来的话，就是其中的`.git`目录；如果`git clone --bare`的话，新建的目录本身就是Git目录），它是Git用来保存元数据和对象数据库的地方。该目录非常重要，每次克隆镜像仓库的时候，实际拷贝的就是这个目录里面的数据。

所谓的暂存区只不过是个简单的文件，一般都放在Git目录中。有时候人们会把这个文件叫做index文件，不过标准说法还是叫暂存区。

Git目录里除了index文件（暂存区）之外，还有Git为我们自动创建的第一个分支`master`，以及指向`master`的一个指针`HEAD`。

<div class="imagediv" style="width: 429px; height: 300px">{% asset_img directories@2x.png %}</div>

**文件的三种状态**

被Git管理的任何一个文件，在Git内都有三种状态：

* 已提交（committed），表示数据已经安全的保存在本地数据库中；
* 已修改（modified），表示修改了文件，但还没保存到数据库中；
* 已暂存（staged），表示对一个已修改文件的当前版本做了标记，使之包含在下次提交的快照中；

由此可以看到Git管理项目时，文件流转的三个区域：

<div class="imagediv" style="width: 400px; height: 220px">{% asset_img areas@2x.png 工作区-暂存区-本地仓库 %}</div>

基本的Git工作流程如下：

1. 在工作区中修改某些文件；
2. 对修改后的文件进行快照，然后保存到暂存区；
3. 提交更新，将保存在暂存区的文件快照永久转储到版本库中。

## Git基础

这一部分将介绍几个最基本的，也是最常用的的Git命令；相对比较复杂的分支管理和远程操作，会在后续的博客中补充。

### 取得项目的Git仓库

有两种取得Git项目仓库的方法。第一种是在现存的目录下，通过导入所有文件来创建新的Git仓库。第二种是从已有的Git仓库克隆出一个新的镜像仓库来。

**从现有仓库克隆 -- git clone**

如果想对某个开源项目出一份力，可以先把该项目的Git仓库复制一份出来，这就需要用到`git clone`命令。注意，这里用的是`clone`，而不是像其他CVCS（譬如Subversion）里用到的`checkout`。这是个非常重要的差别，Git获取的是项目历史的所有数据（每个文件的每一个版本），服务器上有的数据经过`git clone`后，本地也都有了；换句话说，即使服务器的磁盘发生故障，任何一个克隆出来的镜像都可以重建服务器上的仓库，回到当初克隆时的状态。

克隆仓库的最基本命令格式为`git clone [url]`。比如，要克隆本博客仓库，可以用下面的命令：

```sh
$ git clone git://github.com/sadjason/sadjason.github.io.git
```

这会在当前目录下创建名为`sadjason.github.io`的目录，其中包含一个`.git`的目录，用于保存下载下来的所有版本记录，然后从中checkout最新版本的文件拷贝。如果进入这个新建的目录，会看到项目中的所有文件已经在里面了，准备好后续的开发和使用。

默认情况下，`git clone`还会完成「从Git目录checkout最新版本到根目录」的工作，若想略过这一步骤，加上`-n`（表示no checkout）选项，即：

```sh
$ git clone -n git://github.com/sadjasonsadjason.github.io.git
```

除了no checkout设定，结合我的实践体验，将`git clone`的其他用法也给列出来：

* 克隆到指定目录。格式为`git clone url target_dir`。
* 从本地克隆。格式为`git clone /path/to/repo`；还有一种更简单的方法，`Ctrl+C`->`Ctrl+V`或者`Cmd+C`->`Cmd+V`，也是醉了😂

>P.S: 还可以设置`git clone`的选项，只克隆某个指定的分支，本文暂时不讨论分支相关问题，以后再说吧。

**在工作目录中初始化新仓库 -- git init**

要对现有的某个项目开始用Git管理，只需到此项目所在的根目录执行`git init`命令，此命令对该目录进行初始化。

初始化后，在当前目录会出现一个名为`.git`的目录，所有Git需要的数据和资源都存放在这个目录中，如下：

```sh
$ # 新建一个工程目录（简单的C工程）
$ mkdir A-Simple-C-Project
$ cd A-Simple-C-Project
$ git init
Initialized empty Git repository in ~/A-Simple-C-Project/.git/
$ ls -al
drwxr-xr-x   4 zhangbuhuai  staff  136  7  7 09:39 .
drwxr-xr-x   4 zhangbuhuai  staff  136  7  7 09:38 ..
drwxr-xr-x  10 zhangbuhuai  staff  340  7  7 09:39 .git
```

### 记录更新到仓库

工作区下面的所有文件都不外乎两种状态：**已跟踪**或**未跟踪**。已跟踪的文件是指被纳入版本控制管理的文件，在上次快照时有它们的记录，工作一段时间后，它们的状态可能是未更新、已修改（但未放入暂存区）或者已放入暂存区。而未跟踪文件，它们既没有上次更新时的快照，也不在当前暂存区域。

初次克隆某个仓库时，工作区中的所有文件都属于已跟踪文件，且状态为**未修改**。

在编辑过某些文件之后，Git将这些文件标为已修改。我们逐步把这些修改过的文件放到暂存区域，直到最后一次性提交所有这些暂存起来的文件，如此重复。使用Git时的文件状态变化周期如下图所示。

<div class="imagediv" style="width: 400px; height: 165px">{% asset_img lifecycle@2x.png 文件的状态变化周期 %}</div>

**检查当前文件状态 -- git status**

要确定哪些文件当前处于什么状态，可以用`git status`命令。如果在克隆仓库之后立即执行此命令，会看到类似这样的输出：

```sh
$ git status
On branch master
nothing to commit, working directory clean
```

这说明现在的工作区相当干净。换句话说，所有已跟踪文件在上次提交后都未被更改过。此外，上面的信息还表明，当前目录下没有出现任何处于未跟踪的新文件，否则Git会在这里列出来。该命令还显示了当前所在的分支是`master`。

现在在当前目录(~/A-Simple-C-Project/）下创建一个新文件main.c，再次使用`git status`会看到该文件会出现在未跟踪文件列表中，这一次加上`-s`选项,如下：

```sh
$ touch main.c
$ git status -s
?? main.c
```

加上`-s`的状态报告要简洁得多，其中`??`标记表示该文件是未跟踪文件。

>P.S: `git status -s`的状态报告中有几种可能的状态符：
* `??`：表示未跟踪；
* `A`：表示新添加到暂存区，但还没提交；
* `M_`（`M`在左侧）：表示已修改且添加（`git add`）到暂存区中，但还没提交（`git commit`)；
* `_M`（`M`在右侧）：表示已跟踪文件被修改了，但还没放入到暂存区中；
* `AM`：表示新增（新跟踪）文件，但后来又修改了，却没添加（`git add`）到暂存区中；
* `MM`：表示已跟踪文件被修改了，且添加（`git add`）到暂存区中，但后来又被修改了，却没有添加（`git add`）到暂存区；
* `_D`（`D`在右侧）：表示文件已经从工作区中删除（`rm`），但还没将更新添加到暂存区；
* `D_`（`D`在左侧）：表示文件已经从工作区中删除（`git rm`），且已将更新添加到暂存区，但还没提交（`git commit`)；
动手体验一下就知道了🙄。


或者通过`git status file_name1 file_name2 ...`查看某一个（或多个）文件的状态：

```sh
$ git status main.c -s
?? main.c
```

未跟踪的文件意味着Git在之前的快照中没有这些文件；Git不会自动将之纳入跟踪范围，除非明白告诉它「我需要跟踪该文件」，因此不用担心把临时文件的也归入到版本管理中。

**跟踪新文件 -- git add**

使用`git add`开始跟踪一个新文件。`git add file1 file2`命令使得file1和file2这两个文件被纳入到Git管理；`git add`还可以让某个目录被追踪，即`git add some_dir`，值得一提的是，该命令会让some_dir下的所有文件被追踪，而不仅仅是some_dir目录本身。

**暂存已修改文件 -- git add**

对于工作区的文件，哪怕该文件已经被追踪了，当该文件被修改时，也应该在提交到版本库之前把它添加到暂存区，此操作仍然也由`git add`完成。

>Note: `git add`是个多功能命名，根据目标文件的状态不同，此命令的作用效果不同，其一是它可以跟踪新文件，其二是把已跟踪文件放到暂存区中，其三是在合并时把冲突的文件标记为已解决状态。

**忽略某些文件**

我们常常会有些文件无需纳入到Git管理中，譬如工程项目源码、配置之外的文件（尤其是二进制文件等）。我们可以创建一个`.gitignore`文件，列出需要忽略的文件模式。来看一个实际的例子：

```sh
$ cat .gitignore
*.[oa]
*~
```

第一行告诉Git忽略所有已`.o`或`.a`结尾的文件，一般这类对象文件和存档文件都是编译过程中出现的，用不着跟踪它们的版本信息；第二行告诉Git忽略所有以波浪符（`~`）结尾的文件，许多文本编译软件（比如Emacs）都用这样的文件名保存副本。此外，还可能需要忽略`log`、`tmp`或者`pid`目录，以及自动生成的文档等等。

要养成一开始就设置好`.gitignore`文件的习惯，以免将来误提交这类无用的文件。

`.gitignore`的规格规范如下：

* 所有空行或者以注释（`#`）开头的行都会被Git忽略
* 可以使用标准的glob模式匹配
* 匹配模式最后跟反斜杠（`/`）说明要忽略的是目录
* 要忽略指定模式以外的文件或目录，可以在模式前加上惊叹号（`!`）取反

P.S: 所谓的glob模式是指shell所使用的简化了的正则表达式。

GitHub有一个十分详细的针对数十种项目及语言的`.gitignore`文件[列表](https://github.com/github/gitignore/)。

**查看工作区相对于暂存区、暂存区相对于版本库的更新 -- git diff**

`git status`命令可以让我们时刻掌握仓库当前的状态，但它所显示的讯息还是比较简单，`git diff`可以用来查看difference，会使用文件补丁的格式显示具体添加和删除的行。`git diff`的用法比较丰富。

不加参数的`git diff`比较的是工作区和暂存区之间的差异，也就是修改之后还没有暂存起来的变化内容。上文的main.c是空文件，现在编辑内容如下：

```c
#include <stdio.h>

int main(void) {
    return 0;
}
```

保存，使用`git diff`查看工作区相对于暂存区的变化：

```sh
index e69de29..cd9ca7d 100644
--- a/main.c
+++ b/main.c
@@ -0,0 +1,5 @@
+#include <stdio.h>
+
+int main(void) {
+    return 0;
+}
```

使用`git add main.c`命令将main.c的变化放入到暂存区，然后再使用`git diff`查看，可以看到报告内容为空。

`git diff`加上`--cached`选项，能够查看暂存区相对于版本库（默认是HEAD指向的分支）的变化，譬如：

```
$ git diff --cached
diff --git a/main.c b/main.c
new file mode 100644
index 0000000..cd9ca7d
--- /dev/null
+++ b/main.c
@@ -0,0 +1,5 @@
+#include <stdio.h>
+
+int main(void) {
+    return 0;
+}
```

P.S: `--staged`选项和`--cached`作用相同，且表达意思更准确一些，只是1.6.1及更高版本才能使用。

**提交更新 -- git commit**

如果所有文件新增、文件修改都放入了暂存区，那么意味着已经准备妥当，可以提交以记录成版本了。如果不确定，可以使用`git status`或者`git diff`查看一下。提交操作命令是`git commit`，常用的格式是`git commit -m "some commit messages"`。

正常情况下，修改一个文件的流程是：在工作区修改->`git add ...`->`git commit ...`。有时候会觉得`git add`太麻烦了，就可以直接跳过使用暂时区，help文档描述如下：
>By using the `-a` switch with the commit command to automatically "add" changes from all known files (i.e. all files that are already listed in the index) and to automatically "rm" files in the index that have been removed from the working tree, and then perform the actual commit.

**移除文件 -- git rm**

要从Git中移除某个文件，就必须要从已跟踪文件清单中移除（确切地说，是从暂存区移除），然后提交。可以用`git rm`命令完成此项工作，并连带从工作区中删除指定的文件，这样以后就不会出现在未跟踪文件清单中了。

如果只是简单的从工作区中手工删除文件，运行`git status`时会看到「Changes not staged for commit」信息，此时还得补上`git add`命令才行。

还有一种常见情况是，我们想把文件从Git仓库中删除（亦即从暂存区中移除），但仍然希望保留在当前工作区中。换句话说，想让文件保存在磁盘，但并不想让Git继续跟踪该文件。当忘记添加`.gitignore`文件，不小心把一个很大的日志文件或一堆`.a`这样的编译生成文件添加到暂存区时，这一做法尤其有用，为达到这一目的，需要加上`--cached`选项。

**移动文件 -- git mv**

不像其他VCS，Git并不显式跟踪文件移动操作（包括重命名）。可使用`git mv`命令处理文件移动处理，简单来说，运行`git mv file_from file_to`相当于运行了下面三条命令：

```sh
$ mv file_from file_to
$ git rm file_from
$ git add file_to
```

### 查看提交历史

在提交了若干更新，又或者克隆了某个项目之后，也许想回顾下提交历史. 完成这个任务最简单而又有效的工具是`git log`命令。

注意，`git log`命令并不是查看所有git操作记录，而只是查看`git commit`的记录。

默认不用任何参数的话，`git log`会按提交时间列出所有的commits，最近的更新排在最上面，如下：

```sh
$ git log
commit 75c19156d70f1617cba0091bd43494f840c91ea2
Author: sadjason <sadjason@qq.com>
Date:   Sun Jul 10 13:44:23 2016 +0800

    modify main.c

commit 007069d0f1b81bd34823dec48d62b959cc665ec0
Author: sadjason <sadjason@qq.com>
Date:   Sun Jul 10 13:41:54 2016 +0800

    add a.out

commit c589bac19d121c56cbc078f67ab7bd9a91764815
Author: sadjason <sadjason@qq.com>
Date:   Sun Jul 10 13:40:59 2016 +0800

    add two files

...
```

可以看到，这个命令会列出每个commit的SHA-1校验和、作者的姓名和电子邮件、提交时间、提交说明。

`git log`有许多选项，下面介绍一些常用的。

一个常用的选项是`-p`，用来显示每次提交的内容差异；也可以加上`-2`来显示最近两次提交。

也可以为`git log`附带一系列的总结性选项。比如说，如果想看到每次提交的简略的统计信息，可以使用`--stat`选项。

关于更多`git log`的选项配置，参考[Viewing the Commit History](https://git-scm.com/book/en/v2/Git-Basics-Viewing-the-Commit-History)。

### 撤销与版本回退

在任何一个阶段，都有可能想要撤销某些操作。这里，将学习几个撤销所作修改的基本工具。除了撤销，这一部分还介绍版本回退。

值得一提的是，有些撤销操作是不可逆的。

**撤销最后一次提交**

有时候提交完了，才发现漏掉了几个文件没有添加，或者提交信息写错了。此时，可以运行带有`--amend`选项（amend译作「修改」）的命令尝试重新提交：

```sh
$ git commit --amend -m "some message"
```

这个命令会将暂存区中的文件提交，并把当前分支的最新commit给覆盖掉。

```sh
$ git commit -m "last commit"  # mark 1
$ # some modifies
$ git commit --amend -m "new commit"  # mark 2
# 作用效果是：mark 1标记的commit被mark 2标记的commit给覆盖掉
```

如果自上次提交以来还未做任何修改（例如，在上次提交后马上执行了此命令），那么快照会保持不变，而修改的只是commit message：

```sh
$ git commit -m "last commit"  # 上一次commit
$ git commit --amend -m "new commit"  # 更新commit message
```

**撤销已经暂存的文件 -- git reset**

`git add`会影响暂存区里的文件状态，有时需要撤销。例如，已经修改了两个文件并且想要将它们作为两次独立的修改提交，但是却意外地输入了`git add *`暂存了它们俩。如何只取消暂存两个中的一个呢？`git status`命令其实有提示：

```sh
$ touch file_step_1 file_step_2  # 创造两个新文件
$ git add *
$ git status
On branch master
Changes to be committed:
  (use "git reset HEAD <file>..." to unstage)

	new file:   file_step_1
	new file:   file_step_2

```
在「Changes to be committed」正下方，提示使用「git reset HEAD <file>...」来取消暂存。

所以，我们可以这样来暂存取消file_step_2文件：

```sh
$ git reset HEAD file_step_2
$ git status
On branch master
Changes to be committed:
  (use "git reset HEAD <file>..." to unstage)

	new file:   file_step_1

Untracked files:
  (use "git add <file>..." to include in what will be committed)

	file_step_2

```

P.S: `git reset HEAD`命令会撤销所有暂存。

**撤销对文件的修改 -- git checkout**

可能经常会有这样的场景，在工作区对某个文件进行修改，但因为某种原因想放弃对它的修改，想把它还原到上次提交时的样子（或者刚克隆完的样子，或者刚把它放入工作区的样子），怎么办？其实`git status`也告诉了应该如何做，比如，在提交了所有更新后再次修改main.c文件，调用`git status`命令可以看到如下信息：

```sh
$ # 修改main.c文件
$ git status
On branch master
Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git checkout -- <file>..." to discard changes in working directory)

	modified:   main.c

no changes added to commit (use "git add" and/or "git commit -a")
```

「use "git checkout -- <file>..." to discard changes in working directory」提示告诉了我们该怎么干，照做就是：

```sh
$ git checkout -- main.c  # 注意`--`和`main.c`之间得用空格隔开
$ git status
On branch master
nothing to commit, working directory clean
```

值得注意的是，`git checkout -- <file>`是一个危险的命令。对文件做的任何修改都会消失 -- 它只是拷贝了另一个文件来覆盖它。因此，除非确实清楚不想要那个文件了，否则不要使用这个命令。此外，该命令能执行成功的前提是仓库中已有该文件。

~~P.S: 如果修改了某个文件，并且把它提交到暂存区，但现在想放弃该文件的所有修改，该如何处理呢？也简单，先`git reset HEAD <file>`，然后再`git checkout -- <file>`即可。~~

补充说明：`git checkout -- <file>`的意思是把`<file>`文件在工作区的修改全部撤销，这里有两种情况：一种是`<file>`自修改后还没有被放到暂存区，该命令作用后，`<file>`回到和版本库一模一样的状态；另一种是`<file>`已经被添加到暂存区，且之后又作了修改，即该命令对应的状态符是`MM`，该命令作用后，`<file>`回到添加到暂存区后的状态。

P.S: `git reset --hard HEAD`命令也能使得工作区的所有文件状态恢复到与当前版本库一致。

**版本回退 -- git reset**

很多时候遇到这样抓狂的事情：在某个节点之后修改工程，做了一番修改后，工程无法再次通过编译，或者无法达到想要的目的，寻找问题却始终不得结果，无奈之下，只能回到最后一次正常的节点推倒重来。如果项目使用Git进行管理，那么「回到最后一次正常的节点」应该就是常说的版本回退。

版本回退的意义应该不用赘述了。如何处理呢？首先确定工具（命令），`git reset`是也！

下面阐述版本回退的内容比较长...

这里先简要介绍两个概念：commit-id和`HEAD`指针。每个版本（每次commit）都有一个id，这个id是Git自动生成的，即所谓的commit-id；`HEAD`指针的概念相对复杂一些，它与分支也有关系，当前还没有涉及到分支，目前所需要知道的是：在Git中，使用`HEAD`表示当前版本，`HEAD^`表示上一个版本，`HEAD^^`表示上上个版本，依次类推，当然，也可以使用`HEAD~n`表示`HEAD`的前`n`个版本（`HEAD^`等价于`HEAD~1`）。

P.S: 貌似在比较高的版本中将`HEAD^`这种语义给剔除掉了，我所使用的版本是2.7.4。

举个例子，创建一个Git仓库，提交3次，提交的message分别写为「version 1」「version 2」「version 3」，通过`git log`命令可以看到这3次提交记录：

```bash
$ # 创建一个demo，提交3次，通过git log查看3个版本记录
$ git log --pretty=oneline  # --pretty=oneline选项会让显示结果更简洁一些
0eb9dceaaf8f86bacd76ee7109edc1fec256b56e version 3
82be09ef9e1ddb51b41d9b2b36039218673e5a6e version 2
87e61d64ce34693ad62a23f7ee448ed3d9278f56 version 1
```

下图是具象化的说明：

<div class="imagediv" style="width: 299px; height: 149px;">{% asset_img version1-version2-version3@2x.png %}</div>

如果想看当前的版本库信息，即`HEAD`指针指向的commit，可以通过`git show HEAD`查看，同样，通过`git show HEAD~<n>`可以查看往前的第`n`个版本信息：

```bash
$ git show HEAD  # 查看当前版本库信息
commit 0eb9dceaaf8f86bacd76ee7109edc1fec256b56e
Author: zhangwei72 <zhangwei72@meituan.com>
Date:   Sat Jul 23 19:08:05 2016 +0800

    version 3
    ...
$ git show HEAD~1  # 查看往前的版本库信息
commit 82be09ef9e1ddb51b41d9b2b36039218673e5a6e
Author: zhangwei72 <zhangwei72@meituan.com>
Date:   Sat Jul 23 19:07:38 2016 +0800

    version 2
    ...
```

讲了这么多`HEAD`指针，其实是为了给「通过`git reset`回退版本」作铺垫。

所谓版本回退，根据我的理解，其本质是设置`HEAD`指针的指向，譬如回退到version 2，其实所需要做的是重定向`HEAD`指针到version 2的快照：

<div class="imagediv" style="width: 300px; height: 150px;">{% asset_img set-HEAD-to-version2@2x.png %}</div>

设置`HEAD`指针有两种方式，一种是`git reset <commit-id>`，另一种是通过`git reset HEAD~<n>`。假如当前`HEAD`指针指向version 3，`git reset 82be09e`（82be09e是version 2对应的commit-id的前6位）会将`HEAD`指向到version 2；`git reset HEAD~1`也会有同样的作用。

本文将`HEAD`指针指向到version 2：

```sh
$ git reset HEAD~1  # 将HEAD指针指向到前一个版本（version 2）
$ git show HEAD
commit 82be09ef9e1ddb51b41d9b2b36039218673e5a6e
Author: zhangwei72 <zhangwei72@meituan.com>
Date:   Sat Jul 23 19:07:38 2016 +0800

    version 2
    ...
```

现在确定当前版本（`HEAD`指针）切换到version 2了，再用`git log`查看commits信息会发现version 3不见了：

```bash
$ git log --pretty=oneline
82be09ef9e1ddb51b41d9b2b36039218673e5a6e version 2
87e61d64ce34693ad62a23f7ee448ed3d9278f56 version 1
```

问题来了：version 3快照会被删除吗？

答案是：不会！version 3之所以没有在`git log`报告中显示出来，是因为`git log`的报告结果依赖于`HEAD`指针，它只会将比`HEAD`所指向的commit以及更旧的commits给查询出来。

那么还有机会将当前版本切换到version 3吗？在Git中，总有后悔药可以吃的。version 3没有被删除，只要知道其commit-id，就可以通过`git reset <commit-id>`切回到version 3。

这就说到另外一个命令：`git reflog`。该命令可以查看所有分支的所有操作记录（包括commit和reset的操作）。简单一句话，可以通过`git reflog`找到version 3的commit-id，有了commit-id，后面的事情就好办了...

使用`git reset`重置`HEAD`时，还会涉及几个选项：

* `--mixed`，此为默认选项，除了设置`HEAD`，还额外的将暂存区给清空；
* `--soft`，此选项除了设置`HEAD`指针，其他的啥都不干；
* `--hard`，此选项在`--mixed`的基础上还会重置工作区，使得工作区的文件状态和`HEAD`所指向的版本完全一致；
* `--merge`和`--keep`，这俩选项似乎很少用到，略过不讲；

P.S: 关于`git reset`的使用，似乎还有很多内容没有说到，以后再另外补充吧！

还有一个问题：重定向`HEAD`后，如何将`HEAD`之后的commits给永远删掉呢？

接下来的Git博客：[Git分支与远程操作](/git-branching-and-remoting/)。

## 本文参考

* [Pro Git](http://git-scm.com/book/)
* [Pro Git中文版](https://git-scm.com/book/zh/v2/)
* [廖雪峰Git教程之撤销修改](http://www.liaoxuefeng.com/wiki/0013739516305929606dd18361248578c67b8067c8c017b000/001374831943254ee90db11b13d4ba9a73b9047f4fb968d000)

