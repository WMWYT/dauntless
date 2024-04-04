# dauntless

## 介绍

一个小型的支持mqtt V3.1.1版本的服务器。

## 编译&安装

编译：

``` bash
git clone https://github.com/WMWYT/dauntless.git
cd dauntless
mkdir build
cd build
cmake ..
make
```

安装：

``` bash
sudo make install
```

## 使用

设置之后就可以启动dauntless。

```bash
dauntless
```

默认端口为1883，启动前确保端口未被占用。

## 客户端链接

只要支持MQTT V3.1.1版本的客户端都可以链接此服务器。

## TODO

* dauntless_client
* 添加线程
* 打包
* ctest测试
