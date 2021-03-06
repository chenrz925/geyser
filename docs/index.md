# 欢迎

![GitHub](https://img.shields.io/github/license/chenrz925/geyser?style=for-the-badge)
![PyPI](https://img.shields.io/pypi/v/geyser?style=for-the-badge)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/chenrz925/geyser?style=for-the-badge)
![PyPI - Status](https://img.shields.io/pypi/status/geyser?style=for-the-badge)

Geyser是一个低代码任务编排与运行框架，适用于小任务之间的解耦与参数化定义。

特点：

- 能够支持多种配置文件格式，包括JSON、YAML、TOML、HOCON以及Apple pList。
- 有向无环图（DAG）并行化任务，提高任务执行效率
- 低代码运行，任务参数与代码解耦

## 快速使用

首先，安装`geyser`与`geyser-lava`。

```bash
pip install geyser geyser-lava
```

执行：

```bash
geyser -d hello/hello.yaml
```

安装成功能够输出以下信息：

```
(2021-09-30 09:52:51,010)[DEBUG][2328932][140688370164544][geyser_lava.task.hello.Hello]: Hello, level debug.
(2021-09-30 09:52:51,010)[INFO][2328932][140688370164544][geyser_lava.task.hello.Hello]: Hello, level info.
(2021-09-30 09:52:51,010)[WARNING][2328932][140688370164544][geyser_lava.task.hello.Hello]: Hello, level warning
(2021-09-30 09:52:51,010)[ERROR][2328932][140688370164544][geyser_lava.task.hello.Hello]: Hello, level error.
(2021-09-30 09:52:51,010)[CRITICAL][2328932][140688370164544][geyser_lava.task.hello.Hello]: Hello, level critical.
```

### 核心安装

如果你希望完全自己实现所有任务，可以只安装`geyser`，即：

```bash
pip install geyser
```

### 扩展安装

#### 支持PyTorch

依照[PyTorch](https://pytorch.org/)的安装方式，安装`pytorch`、`torchvision`与`torchaudio`，并依照[PyTorch Ignite](https://pytorch.org/ignite/)的安装方式，安装`pytorch-ignite`。

验证安装，在CIFAR-10数据集训练ResNet-50模型：

```bash
geyser torch/train_resnet_cifar10.yaml
```