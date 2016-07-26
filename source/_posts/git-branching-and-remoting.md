title: Git分支与远程操作
date: 2015-12-06 13:23:23
tags:
- Git
- Tools
categories: Others
---

之前在《[Git基础](/git-basics/)》中介绍了一些Git的重要概念和基本操作，本文在此基础上补充Git分支与远程操作相关的内容。

## 分支

分支是Git中的重要概念，其意义和重要性都被讲烂了，这里就不再过多阐述了。简单来说，每次commit都会产生一个快照，为了更好管理，Git把这些快照串成一条时间线，每条时间线就是一个分支，每个不少于一次提交的Git仓库都至少有一个分支；每个commit都有一个commit-id，而分支更高级一些，每个分支都有一个名字，譬如`master`。

《[Git基础](/git-basics/)》中曾说过「在Git中，使用`HEAD`表示当前版本」，这种说法其实不够准确，因为`HEAD`指针并不是指向commit，而是指向当前分支，当前分支才指向commit。譬如在下图中，`HEAD`指向`some branch`（只是举个例子，Git不允许这样命名），`some branch`指向到commit。

<div class="imagediv" style="width: 280px; height: 140px">{% asset_img head-some-branch@2x.png %}</div>

如果新创建一个分支`other branch`，再把`HEAD`指向到它，就表示当前分支在`other branch`上：

<div class="imagediv" style="width: 280px; height: 190px">{% asset_img head-other-branch@2x.png %}</div>

默认情况下，当前分支的名字叫`master`，也就是经常所说的`master`分支。

Note: `master`分支并不是一个特殊分支，它跟其他分支完全没有区别。之所以几乎每个Git仓库都有master分支，是因为`git init`命令默认创建它，并且大多数人都懒得去改动它。

Git的分支操作涉及的命令包括`git branch`、`git checkout`、`git merge`，通过该命令可以创建、删除以及列举分支，具体使用如下文所述。

**创建分支**

创建新分支很简单，其本质是创建一个可以移动的新指针。比如，创建一个名为`testing`的分支，使用`git branch <branch-name>`即可：

```bash
$ git branch test-branch  # 创建一个名为test-branch的分支
```

这条命令会在当前commit上创建一个指针。

**列举分支**

使用`git branch`或者`git branch --list`可以将所有分支给显示出来：

```bash
$ git branch --list
* master
  test-branch
```

`master`前的星号（`*`）标记了当前分支，如上意味着当前分支是`master`。

使用`git log`命令并辅以`--decorate`选项可以查看各个分支所指向的commit：

```bash
$ git log --decorate --pretty=oneline
0eb9dceaaf8f86bacd76ee7109edc1fec256b56e (HEAD -> master, test-branch) version 3
82be09ef9e1ddb51b41d9b2b36039218673e5a6e version 2
87e61d64ce34693ad62a23f7ee448ed3d9278f56 version 1
```

正如所见，当前`master`和`test-branch`分支均指向version 3所对应的commit。

P.S: 如何在创建分支时指定commit？

**切换分支**

使用`git checkout <branch-name>`命令切换分支。

切换分支的命令非常简单，似乎也没有太多让人纠结的选项；但仍然有一个重大疑问：切换分支时，会对工作区和暂存区有影响吗？影响如何？

~~我们的期待是什么样子呢？我所期待的是，在branch a捣鼓了一通后，因为某种原因切换到branch b捣鼓一番，再切回到branch a时，之前在branch a中捣鼓的东西仍然有保留，无论是工作区还是暂存区，都是原来的味道；再切换到branch b也是同样的结果。换句话说，我们希望在逻辑上，branch a和branch b的工作区和暂存区也保持独立，互不影响。~~

~~事实是我所期待的那样的吗？~~

根据[Git Pro](https://git-scm.com/book/en/v2/Git-Branching-Branches-in-a-Nutshell)的说法，`git checkout <branch-name>`这条命令做了两件事情：一是使`HEAD`指向`<branch-name>`分支，二是将工作区恢复成`<branch-name>`分支所指向的快照内容。

P.S: 对上面这段说明，表示不怎么理解，有待更详细的分析。

最后，`git checkout <branch-name>`加上选项`-b`更强大，它的效果是，创建一个名为`<branch-name>`的分支并将切换到该分支。

P.S: 关于`git checkout <branch-name>`对工作区及暂存区的影响，现在仍然没有完全搞清楚，以后有了更深刻理解再补充吧。

**删除分支**

使用`git branch -d <branch-name>`删除分支，比较简单，只是需要注意的是，Git不允许删除当前分支。

**重命名分支**

使用`git branch -m <current-name> <new-name>`修改分支的名字，也比较简单。

**无冲突合并分支**

`git merge`命令用于合并指定分支到当前分支。分支合并的复杂地方在于冲突处理。阅读了一些资料，总结了常见的无冲突合并，假设在`master`中合并`test`，则无冲突合并包括：

* 在`test`中新增文件；
* 在`test`中删除某文件，`master`中的该文件是其ancestor版本；
* 在`test`中修改某文件，`master`中的该文件使其ancestor版本；

P.S: 「解决分支冲突」是个大主题，在以后的博客中补充吧！

P.S: `git merge`的操作是事务性的吗？会存在某个文件合并成功但是别的文件合并失败的情况吗？

P.S: 对`git branch`这几个命令的使用不是很理（zan）解（tong），如果是我设计，`git branch`的作用与`git branch list`等同，都是用于列举所有分支；使用`git branch delete`代替`git branch -d`；使用`git branch rename`代替`git branch -m`。

## 远程仓库

为了能在任意Git项目上协作，需要知道如何管理自己的远程仓库。远程仓库是指托管在因特网或其他网络中的项目的版本库。你可以有好几个远程仓库，通常有些仓库对你只读，有些则可以读写。与他人协作涉及管理远程仓库以及根据需要推送或拉取数据。

除了之前提到的`git clone`，还有4个命令与远程仓库息息相关：`git remote`、`git fetch`、`git pull`、`git push`。有一张非常常见且直观的图对这些命令的功能进行了描述，如下：

<div class="imagediv" style="width: 400px; height: 113px">{% asset_img git-remote@2x.jpg %}</div>

**远程仓库的查看、添加、移除以及重命名 -- git remote**

为了便于管理，Git要求每个远程主机都有一个主机名，简单来说，`git remote`命令就是用于管理Git远程主机名的。

不带选项的时候，`git remote`命令列出所有远程主机：

```sh
$ git remote
origin 
```

也可以指定选项`-v`，会显示更多信息：

```sh
git remote -v  
origin  git://github.com/sadjason/sadjason.github.io.git (fetch)
origin  git://github.com/sadjason/sadjason.github.io.git (push)
```

>P.S: 克隆（`git clone`）版本库的时候，所使用的远程主机被Git自动命名为`origin`，如果想使用其他的主机名，需要使用`-o`选项指定，譬如`git clone -o example git://example.com/example.git`。

`git remote`是一个比较复杂的命令，旗下还包括一些子命令：

* `git remote add`，用于**添加远程仓库**，使用起来非常简单，运行`git remote add <name> <url>`即可添加一个新的远程Git仓库，同时指定一个名称（任意）。
* `git remote show`，用于**查看指定远程仓库的更多信息**，基本使用格式是`git remote show <name>`。
* `git remote remove`，用于**移除远程仓库**，如果因为一些原因想要移除一个远程仓库，可以使用`git remote remove <name>`命令，`git remote rm`与之等价。
* `git remote rename`，用于**远程仓库重命名**，基本使用格式是`git remote rename <old_name> <new_name>`。

>P.S: 如上所列的几个`git remote`命令，只有`git remote show`需要访问网络，其余几个都是本地操作。以`git remote add`为例，其本质无非是在本地为远程仓库的URL创建一个字符串映射（毕竟操作remote repository时输入URL并不是一个特别好的体验）。这个字符串映射并没有任何实际意义，可以简单看成是远程仓库URL的一个alias。stackoverflow中的[解释](https://stackoverflow.com/questions/5617211/what-is-git-remote-add-and-git-push-origin-master/)更棒一些。既然`git remote add`的本质是为远程仓库的url创建字符串映射，那么这个映射存放在哪里呢？答案是`.git/config`。

**关于git remote的一些补充**

我在学习过程中对这个命令产生了极大的误解，我以为能够通过`git remote`管理远程（服务器）repository，譬如远程在服务器中创建一个repository。

甚至做了如下的测试：

```sh
$ mkdir MyLocalRepository
$ cd MyLocalRepository
$ git init
$ echo "hello, github" >> README.md
$ git add README.md
$ git commit -m "add readme file"
$ git remote add MyLocalRepository https://github.com/xxoo/MyLocalRepository.git
```

其中最后一步，我的目的是在github中创建一个名为MyLocalRepository的仓库，上述命令执行完后没有任何错误信息（UNIX的哲学是没有消息就是好消息）。如果这个命令执行成功，我能够在github中看到一个新的仓库被创建（不是通过[Create a New Repository](https://github.com/new)这个接口创建哦）。但我登录github后，啥都没发生。

这个测试结果说明我对`git remote`的理解有问题，于是去stackoverflow中寻找「可否在本地远程在github上创建repository」问题答案，还真找到了，即[Is it possible to create a remote repository on github from client](http://stackoverflow.com/questions/2423777/is-it-possible-to-create-a-remote-repo-on-github-from-the-cli-without-ssh)。显然，stackoverflow给出的答案是：可以，但是需要通过github的api，这是另外一个故事了，本文不涉入。

**从远程仓库抓取数据 -- git fetch**

从远程仓库中获取数据，可以执行`git fetch <remote-repository-name>`命令，该命令会访问远程仓库，从中拉取所有本地还没有的数据，执行完成后，本地将会拥有那个远程仓库中的所有分支的引用。

默认情况下，`git fetch`取回所有分支的更新，但若只想取回特定分支的更新，可以指定分支名：

```sh
$ git fetch <remote-repository-name> <branch-name>
```

使用`git fetch`的一些说明：

* 如果不指定`<remote-repository-name>`，则默认执行`git fetch origin`，执行成功的前提是`origin`这个远程仓库确实存在；
* `git fetch`可以fetch多个repository，指定选项`--all`即可；
* `git fetch`仅仅是将指定的远程仓库的所有分支内容全部下载到本地，除此之外不做其他的事情（譬如分支合并）；

**git pull**

`git pull`命令的作用是，取回远程主机某个分支的更新，再与本地的指定分支合并。其完整格式如下：

```bash
git pull <remote-repository-name> <remote-branch-name>:<local-branch-name>
```

比如，取回`origin`远程仓库的`next`分支，与本地的`master`分支合并，需要写成下面这样：

```bash
$ git pull origin next:master
```

如果远程分支是与当前分支合并，则`:<local-branch-name>`可以去掉，譬如：

```bash
$ git pull origin next  
```

P.S: `git pull`是事务性操作吗？

**推送数据到远程仓库 -- git push**

`git push`命令用于将本地分支的更新，推送到远程主机，使用格式是：

```sh
$ git push <remote-repository-name> <local-branch-name>:<remote-branch-name>
```

**git clone v.s git fetch v.s git pull**

`git clone`、`git fetch`、`git pull`这三个命令似乎都是用于从远程拉取数据，三者有啥区别呢？

本地没有repository时，`git clone`将远程repository整个下载下来；`git fetch`将远程新的commit数据（如果有的话）下载过来；`git pull`相当于`git fetch`和`git merge`，在`git fetch`的基础上，进行分支合并。

## 本文参考

* [Pro Git](http://git-scm.com/book/)
* [Pro Git中文版](https://git-scm.com/book/zh/v2/)
* [Git远程操作详解](http://www.ruanyifeng.com/blog/2014/06/git_remote.html)
* [What is git-remote-add and git-push-origin-master?](https://stackoverflow.com/questions/5617211/what-is-git-remote-add-and-git-push-origin-master/)
* [Is it possible to create a remote repo on GitHub from the CLI without opening browser?](http://stackoverflow.com/questions/2423777/is-it-possible-to-create-a-remote-repo-on-github-from-the-cli-without-opening-br)
* [GitHub秘籍](https://snowdream86.gitbooks.io/github-cheat-sheet/content/zh/index.html)