# 文件云存储系统

>   图片没有显示的话，进入到docs目录下查看

## 介绍
基于Workflow + wfrest + MySQL和Restful风格实现的简易网页式网盘，实现了用户注册、登录、上传文件和下载文件功能。



## 项目架构

-   bin：存放Makefile，以及生成的可执行程序
-   conf：存放项目配置文件，保存项目中需要用到的配置，如MySQL地址
-   docs：存放项目介绍文档
-   include：存放项目头文件
-   log：存放项目执行中记录的日志
-   sql：存放构建项目所需数据库表的sql语句
-   src：存放项目源代码及实现文件
-   static：存放静态资源及前端页面
-   test：存放项目部分功能的测试文件



## 项目所使用到的第三方库

-   Workflow
-   wfrest
-   openssl
-   log4cpp



## 启动教程

1.  **构建项目所需数据库CloudDisk**

    ```sql
    -- 创建数据库
    CREATE DATABASE IF NOT EXISTS CloudDisk;
    
    -- 使用数据库
    USE CloudDisk;
    
    -- 创建文件表
    CREATE TABLE `tbl_file` (
        `id` int(11) NOT NULL AUTO_INCREMENT,
        `file_sha1` char(40) NOT NULL DEFAULT '' COMMENT '文件hash',
        `file_name` varchar(256) NOT NULL DEFAULT '' COMMENT '文件名',
        `file_size` bigint(20) DEFAULT '0' COMMENT '文件大小',
        `file_addr` varchar(1024) NOT NULL DEFAULT '' COMMENT '文件存储位置',
        `create_at` datetime default NOW() COMMENT '创建日期',
        `update_at` datetime default NOW() on update current_timestamp() COMMENT '更新日期',
        `status` int(11) NOT NULL DEFAULT '0' COMMENT '状态(可用/禁用/已删除等状态)',
        `ext1` int(11) DEFAULT '0' COMMENT '备用字段1',
        `ext2` text COMMENT '备用字段2',
        PRIMARY KEY (`id`),
        UNIQUE KEY `idx_file_hash` (`file_sha1`),
        KEY `idx_status` (`status`)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8;
    
    -- 创建用户表
    CREATE TABLE `tbl_user` (
        `id` int(11) NOT NULL AUTO_INCREMENT,
        `user_name` varchar(64) NOT NULL DEFAULT '' COMMENT '用户名',
        `salt` varchar(20) NOT NULL DEFAULT '' COMMENT '密码盐值',
        `user_pwd` varchar(256) NOT NULL DEFAULT '' COMMENT '用户encoded密码',
        `email` varchar(64) DEFAULT '' COMMENT '邮箱',
        `phone` varchar(128) DEFAULT '' COMMENT '手机号',
        `email_validated` tinyint(1) DEFAULT 0 COMMENT '邮箱是否已验证',
        `phone_validated` tinyint(1) DEFAULT 0 COMMENT '手机号是否已验证',
        `signup_at` datetime DEFAULT CURRENT_TIMESTAMP COMMENT '注册日期',
        `last_active` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '最后活跃时间戳',
        `profile` text COMMENT '用户属性',
        `status` int(11) NOT NULL DEFAULT '0' COMMENT '账户状态(启用/禁用/锁定/标记删除等)',
        PRIMARY KEY (`id`),
        UNIQUE KEY `idx_username` (`user_name`),
        KEY `idx_status` (`status`)
    ) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4;
    
    -- 创建用户token表
    CREATE TABLE `tbl_user_token` (
        `id` int(11) NOT NULL AUTO_INCREMENT,
        `user_name` varchar(64) NOT NULL DEFAULT '' COMMENT '用户名',
        `user_token` char(50) NOT NULL DEFAULT '' COMMENT '用户登录token',
        PRIMARY KEY (`id`),
        UNIQUE KEY `idx_username` (`user_name`)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    
    -- 创建用户文件表
    CREATE TABLE `tbl_user_file` (
        `id` int(11) NOT NULL PRIMARY KEY AUTO_INCREMENT,
        `user_name` varchar(64) NOT NULL,
        `file_sha1` varchar(64) NOT NULL DEFAULT '' COMMENT '文件hash',
        `file_size` bigint(20) DEFAULT '0' COMMENT '文件大小',
        `file_name` varchar(256) NOT NULL DEFAULT '' COMMENT '文件名',
        `upload_at` datetime DEFAULT CURRENT_TIMESTAMP COMMENT '上传时间',
        `last_update` datetime DEFAULT CURRENT_TIMESTAMP 
        ON UPDATE CURRENT_TIMESTAMP COMMENT '最后修改时间',
        `status` int(11) NOT NULL DEFAULT '0' COMMENT '文件状态(0正常1已删除2禁用)',
        UNIQUE KEY `idx_user_file` (`user_name`, `file_sha1`),
        KEY `idx_status` (`status`),
        KEY `idx_user_id` (`user_name`)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    ```
    
2.  进入到bin目录下，执行make命令

    ```shell
    $ make
    ```

    接着bin目录下会出现如下文件

    ```shell
    $ CloudDiskServer  Makefile  obj
    ```

    运行CloudDiskServer

    ```shell
    ./CloudDiskServer
    ```

    >   如果需要重新构建项目，可以执行
    >
    >   ```shell
    >   $ make rebuild
    >   ```
    >
    >   如果需要删除可执行程序以及.o文件，可以执行
    >
    >   ```shell
    >   $ make clean
    >   ```
    
3.  接着在客户端（浏览器）中输入对应URL即可看到对应页面以及服务器的相关信息

    -   用户注册：`/user/signup` —— `signup.html`
    -   用户登录：`/user/signin` —— `signin.html`
    -   个人中心：`/static/view/home.html` —— `home.html`
        -   加载用户信息：`/user/info?username=xxx&token=xxx`
        -   加载文件列表：`/file/query?username=xxx&token=xxx`
    -   上传文件：`/file/upload?usernaem=xxx&token=xxx` —— `index.html`
        -   上传成功：`/file/upload/success`——`upload_success.html`
    -   下载文件：在`home.html`中出现的文件旁有一个**下载按钮**



## 业务流程

首先进行注册，注册成功后会自动跳转到登录界面，登录成功后会自动跳转到个人中心页面，个人中心页面有一个上传按钮，点击然后跳转到上传文件页面，并且在个人中心页面的文件旁有一个下载按钮，点击下载文件



### 用户注册

客户端向服务器发送GET /user/signup请求，请求注册页面，服务器返回注册页面

接着用户在客户端输入用户名和密码，接着发送POST请求，将用户名和密码发送给服务器

接着服务器进行解析，解析出用户名和密码后，对用户密码进行加密，然后将用户信息（用户名、加密后的密码、salt）写入数据库，注册成功需要给客户端返回SUCCESS，失败则返回FAILED



#### 输入密码的是不直接明文显示

可以改进：输入密码的是不直接明文显示

![image-20231006163149254](README.assets/image-20231006163149254.png)

![image-20231006163157211](README.assets/image-20231006163157211.png)



#### 密码加密

可以改进：加密算法有很多，项目中（`EncryptPassword.h`）是随机生成salt，然后对密码进行加盐进行加密



#### log4cpp单例类和读取配置文件单例类

`Singleton.h Mylogger.h Configuration.h`



#### 注册失败

如果注册失败，即如果是因为服务器内部发生错误，除了可以直接`resp->String("FAILED");`，`wfrest`还提供了`resp->Error(error_code, error_msg)`函数，具体可以查看`HttpMsg.h`，error_code可以查看`ErrorCode.h`，返回的是一个JSON，封装了错误码，错误信息



#### 生成默认邮箱和手机号

一般来说可能手机号作为注册账号，然后邮箱是登录成功之后进行设置的，本项目中是直接生成随机的邮箱和手机号以供展示



### 用户登录

进入到登录界面后，用户在客户端（浏览器）中输入用户名和密码，然后浏览器发送POST /user/signin请求给服务器，服务器收到该POST请求之后，解析出用户名和密码，然后查询MySQL，获取用户名对应的salt和加密后的密码，对输入的密码进行加密，与MySQL中的加密后的密码进行比对，如果一致则代表密码正确，登录成功，此时服务器会根据用户名生成Token，然后封装响应JSON，将用户名、Token、以及个人中心页面的route返回给客户端

>   需要注意的是，最后返回的时候，不能直接`resp->Json(resp_json)`，而是需要`resp->String(resp_json.dump());`
>
>   应该是前端页面的解析方式导致的



#### token

可以改进：token生成的方式也有很多，项目中（`Token.h`）生成Token的方式是，首先将salt和用户名进行拼接，然后生成对应的MD5码，接着将MD5码和时间进行拼接，得到最终的Token

需要注意的是：`openssl/md5.h`中的的MD5函数已弃用，如果继续使用编译的时候会出现warning，但是可以正常运行，如果想要避免warning，可以使用`openssl/evp.h`中`EVP_MD_CTX_new()`及其相关函数



#### Redis存储token（改进）

使用缓存存储令牌的好处是提高了访问速度和性能。由于缓存是内存中的数据存储，读取和写入速度通常比数据库快得多。这对于频繁验证令牌的场景非常有用，如用户每次请求都需要验证令牌的有效性。

另外，将令牌存储在缓存中还可以更好地控制令牌的生命周期。您可以**设置令牌的过期时间**（项目中是设置了**30min**的过期时间），并在缓存中自动处理令牌的失效和续期。这样可以减轻数据库的负载，同时提供更好的灵活性和扩展性。

通常的做法是，**用户登录成功后，生成令牌并将其存储在缓存中，将令牌返回给客户端（如浏览器）作为身份验证凭证。客户端在后续请求中携带令牌，服务器通过查询缓存验证令牌的有效性。**

当然，具体的令牌存储方案还取决于应用程序的需求和架构。有些应用程序可能选择将令牌存储在数据库中，而不使用缓存。这取决于您的具体场景和系统设计。





### 个人中心

在用户登录成功之后，客户端收到服务器发过来的响应JSON，获取到JSON中的个人中心页面，发送GET请求给服务器，服务器返回给客户端个人中心页面

客户端获取到个人中心页面之后，需要加载用户信息，文件列表

**加载用户信息**时，客户端向服务器发送GET /user/info?username=xxx&token=xxx请求，包含查询词用户名和token（在前端页面的代码中没有看到，可以使用`wireshark`进行抓包，也可以直接在浏览器的网址输入框中看到），服务器收到该请求后，首先解析出username和token，然后查询Redis进行校验token，校验成功后，查询MySQL获取邮箱、手机号、用户注册时间，封装成JSON返回给客户端

客户端收到用户信息之后，会发送一个POST /file/info?username=xxx&token=xxx请求给服务器去加载文件列表

**加载文件列表**时，请求体是limit，默认为15，表示服务器返回的文件列表的文件个数不能超过15个，服务器首先去校验token，逻辑跟加载用户信息的时候一致，登录成功之后客户端的后续所有请求都需要携带token，校验完成之后查询MySQL，获取文件hash、文件大小、文件名、上传时间、最近修改时间，封装成JSON返回给客户端



#### 用户信息

前端页面中的用户信息一开始只有用户名和注册时间，但是数据库中的`tbl_user`表还是有挺多信息的，比如邮箱和手机号，所以可以对前端页面的代码进行修改，补充显示这些信息（已实现）

后续登录成功之后，可以改进成支持修改用户信息，比如发送一个POST /user/email 请求进行修改邮箱**==（未实现）==**





### 上传文件

用户在客户端的个人中心页面点击upload file按钮，然后向服务器发送一个GET /user/upload?username=xxx&token=xxx请求，服务器返回上传文件页面，接着客户端跳转到上传文件页面，选择要上传的文件之后，点击Upload按钮，此时向服务器发送POST /file/upload 请求，请求体的类型是multipart/form_data，包括文件名和文件内容，服务器获取到文件名和文件内容之后，根据用户名去创建一个用户根目录，用于存放各个用户的文件，然后读取文件内容，在用户目录下创建文件，创建成功后，给客户端重定向到一个上传成功页面

>   其实不应该重定向到上传成功页面，没什么用，应该重定向到个人中心页面，然后可以看到刚刚上传的文件就代表是上传成功了

>“根据用户名去创建一个用户根目录，用于存放各个用户的文件，然后读取文件内容，在用户目录下创建文件”
>
>参考之前的网盘项目，其实是不需要创建指定用户的根目录的，只需维护一个虚拟文件表，路径只存在于表中，即文件的路径只是逻辑存在，而不是真实在服务器中存在，每个用户维护一个当前的目录栈来找到当前用户所在的当前目录，

#### 生成文件hash值-sha1

SHA-1 和 Simhash 是两种不同的哈希算法，它们在不同的应用场景下具有不同的优势和特点。

SHA-1（Secure Hash Algorithm 1）是一个加密哈希函数，用于生成具有固定长度（160位）的哈希值。它被广泛应用于数据完整性验证和数字签名等领域。然而，由于 SHA-1 存在安全性漏洞，已经被广泛认为不再安全，并且不推荐在对安全性要求较高的场景中使用。

Simhash（Similarity Hash）是一种用于计算文本或数据相似性的哈希算法。它通过对文本或数据进行特征提取和加权处理，生成一个表示相似性的固定长度哈希值。Simhash 在信息检索、文本聚类、重复文本检测等领域具有广泛的应用。相比于传统哈希算法，Simhash 能够更好地保持相似的文本或数据之间的相似性，且对输入数据的微小变化不敏感。

选择 SHA-1 还是 Simhash 取决于您的具体需求和应用场景：

- 如果您需要进行**数据完整性验证**、数字签名等**安全性相关**的任务，建议使用更安全的哈希算法，如 **SHA-256**、**SHA-3** 等替代 SHA-1。

- 如果您需要**计算文本或数据之间的相似性**，并且对输入数据的微小变化比较敏感，Simhash 可能是一个更好的选择。

需要注意的是，哈希算法的选择还应考虑性能、哈希冲突的可能性和具体应用的要求。在实际使用时，建议综合考虑以上因素，选择适合您特定需求的哈希算法。



#### 分片上传（==重难点==）

需要客户端和服务器一起配合，没学过前端就没办法实现

但是逻辑还是得知道（可以吹）

参看`大文件分片上传.md`







### 下载文件（未实现）

![image-20231007001843068](README.assets/image-20231007001843068.png)

服务器收到这个GET请求之后，首先先解析请求，获取到文件名、文件hash、用户名、token，然后去验证token，验证成功后返回给客户端一个下载链接，而客户端通过下载链接去下载文件则是由**Nginx**负责。

>   发送filehash其实没什么用，因为不是根据文件名和filehash去唯一指定一个文件
>
>   参考之前的网盘项目，有一个fileinfo表
>
>   ```mysql
>   CREATE TABLE fileInfo (
>       id INT PRIMARY KEY AUTO_INCREMENT, -- 文件ID，采用自增的方式赋予唯一标识符
>       filename VARCHAR(255), -- 文件名，用于存储文件的名称
>       type CHAR(1), -- 文件类型，用于指明文件的类型，d-目录，f-文件
>       pre_id INT, -- 所属目录ID，用于记录文件/目录所属的上一级目录（如果有，-1表示根目录）
>       username VARCHAR(255), -- 用户名，用于关联文件所属的用户
>       path VARCHAR(255), -- 文件路径，用于存储文件在文件系统中的路径信息
>       tomb INT DEFAULT 0, -- 删除标记，用于标记文件是否已删除，0表示未删除，1表示已删除
>       md5 VARCHAR(32), -- 文件MD5码，表示文件内容，NULL表示为目录
>       UNIQUE (username, path) -- 将username和path设置为唯一键
>   );
>   ```
>
>   ![image-20231007004438751](README.assets/image-20231007004438751.png)
>



# 优化

## 网盘架构1.0（本项目中的架构）

![image-20231007142759668](README.assets/image-20231007142759668.png)

目前的**问题**：

对于上传的文件，**没有备份**；一旦磁盘损坏之后，数据就会丢失

一般大公司有**多数据中心**

可选方案：

>   在公司中，遇到问题的时候，先想3种解决方案

1.  双磁盘备份 -> 磁盘阵列 RAID 0~5 （无法抵御自然灾害）
2.  多数据中心——备份容灾 （大企业）-> 有富裕能力 租给其他公司使用 云计算
3.  **OSS服务** 阿里云、百度云、腾讯云、华为云 （中小企业）

>   之前的网盘项目 - 0.9版本，客户端只能是命令行的形式，
>
>   多终端形式：命令行、浏览器、App、小程序等
>
>   协议不变就行了



## 网盘架构2.0

![image-20231007142831288](README.assets/image-20231007142831288.png)

**备份到云上**——阿里云OSS

使用阿里云的**SDK**做开发（https://help.aliyun.com/zh/oss/developer-reference/introduction-4?spm=a2c4g.11186623.0.i24#concept-hcq-cgl-ngb）

>   SDK：Software Development Kit 软件开发工具包

问题：

1.  上传到阿里云，需要流量（付费）、需要带宽、需要时间（大文件耗时较久）
2.  如果需要等备份到云上完成再返回客户端，用户体验就比较差（同步）

所以需要将备份到云上的操作进行解耦，改为只是发送一个指令，而不是执行完一整个备份操作



## 网盘架构3.0

![image-20231007142843323](README.assets/image-20231007142843323.png)

引入**消息队列**（**RabbitMQ**、RockerMQ、Kafka），部署到docker上去使用

方案：采用**异步**操作之后，将同步到OSS上的操作，单独拎出来，不再等待

备份到云上是一个异步操作，发送一个备份指令就行了

消息队列本质上就是生产者消费者问题

![image-20231007215006219](README.assets/image-20231007215006219.png)

生产者就是我们的Server，然后消息队列本身就是一个Server，负责消息的存储和转发，消费者是另一个进程，由这个进程去执行同步操作



v3.0存在的问题：

各种业务耦合在一个Server中

当一个业务出现问题时，可能会影响其他业务的处理

假如其中一个业务执行的时候导致服务器挂了，则其他用户无法继续执行业务



## 网盘架构4.0

![image-20231007144523638](README.assets/image-20231007144523638.png)

到3.0，都是一个**单体程序**（一个Server耦合了太多业务），4.0需要进行微服务体系改造

解决方案：**微服务**，拆分成微服务之后，**每一个业务都是一个单独的进程**

好处：

1.  分工明确，职责边界清晰
2.  单点故障，不会导致全局失效
3.  独立部署，独立更新
4.  细粒度
5.  异构系统，支持不同语言实现

之前的Server就改造成了一个API网关，负责转发请求给后端服务器（RPC协议），相当于是后端服务器的客户端，但又是外部Client的服务器（Http协议）

>   RPC、Remote Procedure Call 远程系统调用
>
>   -   企业内部使用，可以定制协议本身，私有协议，通信效率高
>
>   -   二进制协议，可读性差
>
>   -   使用起来比较方便，与函数调用类似
>
>   Http协议
>
>   -   统一标准，通用性更好
>
>   -   文本协议，可读性强，但是效率低
>
>   -   要比函数调用复杂一些

问题：

当企业内部的微服务数量太多时，沟通成本会增加



## 网盘架构5.0

![image-20231007145742726](README.assets/image-20231007145742726.png)

引入注册中心（类比考勤系统）、zookeeper、**consul**

部署到Docker中使用

API网关通过注册中心找到后端服务器的IP和port，从而定位到

注册中心通过发送心跳检测包，去检测后端服务器是否宕机



## 面试

手绘架构图，逐步解释：

1.  为什么要引入OSS服务？
2.  为什么要引入消息队列？
3.  为什么要引入微服务、RPC机制？
4.  为什么要引入注册中心？



1.  为什么要引入OSS服务？

    因为之前没有对数据做备份操作，一旦磁盘损坏，数据就会丢失，而备份数据主要有三种方案：一、双磁盘备份，二、大企业采用的数据中心，三、中小企业租用大企业的云服务，所以引入OSS服务来做数据的备份

    项目中使用的是**阿里云**的OSS服务

2.  为什么要引入消息队列？

    因为之前是一个同步操作，服务器需要等待备份完成之后再返回客户端，这对用户的体验较差，所以需要改成异步操作，服务器发送一个备份指令给消息队列，即可返回客户端，然后另一个进程就去消费，然后执行备份操作，从而实现一个解耦

    项目中使用的是**RabbitMQ**作为消息队列

3.  为什么要引入微服务、RPC机制？

    因为之前是一个单体程序，一个Server耦合了太多的业务，如果其中一个业务的操作导致服务器宕机了，则所有业务都无法执行，所以需要改成微服务，将每个业务都交给一个进程去执行。

    改成微服务的好处是

    1.  细粒度，分工明确，职责边界清晰，某一个业务单点故障不会导致全局失效

    2.  不同的业务可以独立部署，独立更新

    3.  支持异构系统，支持不同语言实现

    原来的单体Server此时成为一个API网关，通过RPC（远程过程调用）机制与后端服务器进行通信，API网关与外界Client还是使用Http协议进行通信，这是统一的标准，而在内部，则可以使用自己定制的、效率更高的RPC协议与后端服务器进行通信

    项目中的RPC机制使用的是**sRPC**（基于Workflow）

4.  为什么要引入注册中心？

    系统中的微服务数量太多了，API网关可能不清楚后端的业务服务器是否还存在，这样就会导致沟通的成本增加，所以引入注册中心，后端业务服务器是正常上线的就去注册中心中注册，将自己的IP地址和端口号告知注册中心，然后API网关通过访问注册中心就可以知道后端有哪些业务是可以正常运行的，通过IP地址和端口号即可定位到业务，同时注册中心使用心跳检测去检查后端业务是否宕机

    项目中使用的是**Consul**



### 项目介绍

该项目是一个基于**HTTP**协议的文件管理系统，旨在为公司提供稳定可靠高效的文件上传和下载功能，并应用于云服务器环境

在项目中，为了处理高并发和可扩展性，我采用了**Proactor**模型，并结合开源框架**Workflow**构建了高性能的文件管理服务端。

基于**wfrest**框架，开发了一套RESTful API，支持用户注册、登录、个人中心、文件上传、分片上传和下载等功能

为了实现多用户独立的虚拟文件系统，我设计了一个虚拟文件表来存储用户的文件信息，实现了如文件存储和目录管理等主流文件系统的基本功能。

同时，我利用**Redis**存储用户登录Token以及用户文件的**MD5**码，实现用户登录功能与文件**秒传**功能

为了实现异步上传功能和提高系统的响应性，我集成了RabbitMQ消息队列。通过消息队列，可以将文件上传任务异步处理，提高系统的并发处理能力和可靠性。

为了确保文件的安全性和可靠性，我使用阿里云OSS作为系统的容灾备份方案。这样可以将文件备份到云端，确保文件的安全存储和可靠性。

为了实现微服务架构，我将不同的功能模块拆分成独立的服务，并使用RPC机制实现服务间的通信和协作。为了管理和发现这些微服务，我使用了注册中心Consul，确保微服务的高可用性和动态扩展能力。

综上所述，该项目是一个基于HTTP协议的文件管理系统，通过使用多种技术和组件，实现了稳定可靠的文件上传和下载功能，同时具备高并发、可扩展、安全可靠的特性。



# 阿里云OSS

相关概念：

1.  Bucket 文件夹
2.  Object 文件
3.  Endpoint 域名
4.  AccessKey 公钥和私钥（AccessKey secret）



安装阿里云OSS的C++SDK

https://help.aliyun.com/zh/oss/developer-reference/installation-12?spm=a2c4g.11186623.0.0.66714f30dKJFog

注意openssl的版本

### 补充备份阿里云OSS

#### 代码实现

```cpp
// OssUploader.h
#ifndef __OssUploader_H__
#define __OssUploader_H__
#include <alibabacloud/oss/OssClient.h>

#include <string>

using std::string;


struct OssInfo
{
    //定制化数据,需要进行修改
    string AccessKeyId = "YourAccessKeyId";
    string AccessKeySecret = "YourAccessKeySecret";
    string Endpoint = "oss-cn-guangzhou.aliyuncs.com";
    string BucketName = "tvux";
};

// 将本地文件备份到阿里云OSS
class OssUploader
{
public:
    OssUploader(const OssInfo & info = OssInfo())
    : _info(info)
    , _conf()
    , _ossClient(_info.Endpoint, _info.AccessKeyId, _info.AccessKeySecret, _conf)
    {   AlibabaCloud::OSS::InitializeSdk();    }

    // filename：本地文件路径
  	// objectName：阿里云OSS上的目的路径
    bool doUpload(const string & filename, const string & objectName)
    {
        auto outcome = _ossClient.PutObject(_info.BucketName, objectName, filename);
        
        bool ret = outcome.isSuccess();
        if(!ret) {
            std::cout << "PutObject fail" <<
            ",code:" << outcome.error().Code() <<
            ",message:" << outcome.error().Message() <<
            ",requestId:" << outcome.error().RequestId() << std::endl;
        }
        return ret;
    }

    ~OssUploader() {    AlibabaCloud::OSS::ShutdownSdk();  }

private:
    OssInfo _info;
    AlibabaCloud::OSS::ClientConfiguration _conf;
    AlibabaCloud::OSS::OssClient _ossClient;
};

#endif
```

测试

```cpp
// testOss.cc
#include "../include/OssUploader.h"
#include <iostream>
using namespace std;

void test() {
    OssUploader oss_uploader;
    oss_uploader.doUpload("./testUpload.txt", "Tvux/testUpload.txt");
}

int main(void)
{
    test();
    return 0;
}
```

封装阿里云OSS，并将该类OssUploader的对象加入到本项目的CloudDisk的数据成员，补充CloudDisk构造函数

![image-20231007193522043](README.assets/image-20231007193522043.png)

![image-20231007193621107](README.assets/image-20231007193621107.png)

修改文件上传对应的函数loadFileUploadModule，补充备份阿里云

![image-20231007194520094](README.assets/image-20231007194520094.png)



# Docker

## 安装Docker

```shell
$ curl -fsSL https://get.docker.com| bash -s docker --mirror Aliyun
```

### 安装Docker时出现如图报错：appstreamcli

解决方案：

```shell
$ sudo apt -f install libappstream4
```

![image-20231007194915685](README.assets/image-20231007194915685.png)

### 拉取镜像时出现报错

```
failed to register layer: exit status 22: unpigz: abort: zlib version less than 1.2.3
```

解决方案：

https://blog.csdn.net/weixin_42768584/article/details/132583562

#### 升级pigz

```shell
wget https://github.com/madler/pigz/archive/refs/tags/v2.8.tar.gz
```

http://zlib.net/pigz/


![image-20231007203254536](README.assets/image-20231007203254536.png)

```shell
$ cd pigz-2.8
$ make
```

#### 备份

```shell
$ which pigz
$ /usr/bin/pigz

$ which unpigz
$ /usr/bin/unpigz

$ mv /usr/bin/pigz /usr/bin/pigz.bak
$ mv /usr/bin/unpigz /usr/bin/unpigz.bak
```

#### 起用新版本

```shell
$ cd pigz-2.8
$ cp pigz /usr/bin/
$ cp unpigz /usr/bin/
```

#### 查看pigz版本

```shell
$ pigz -V
$ pigz 2.8 #正常显示版本号
```

#### 再次拉取镜像

```shell
$ docker pull hello-world # 能正常拉取
Using default tag: latest
latest: Pulling from library/hello-world
2db29710123e: Pull complete 
Digest: sha256:2498fce14358aa50ead0cc6c19990fc6ff866ce72aeb5546e1d59caac3d0d60f
Status: Downloaded newer image for hello-world:latest
docker.io/library/hello-world:latest

$ docker run hello-world # 正常运行

Hello from Docker!
This message shows that your installation appears to be working correctly.

To generate this message, Docker took the following steps:
 1. The Docker client contacted the Docker daemon.
 2. The Docker daemon pulled the "hello-world" image from the Docker Hub.
    (amd64)
 3. The Docker daemon created a new container from that image which runs the
    executable that produces the output you are currently reading.
 4. The Docker daemon streamed that output to the Docker client, which sent it
    to your terminal.

To try something more ambitious, you can run an Ubuntu container with:
 $ docker run -it ubuntu bash

Share images, automate workflows, and more with a free Docker ID:
 https://hub.docker.com/

For more examples and ideas, visit:
 https://docs.docker.com/get-started/

```







## 容器镜像加速服务

https://cr.console.aliyun.com/cn-hangzhou/instances/mirrors

注意：需要切换root用户

![image-20231007195144035](README.assets/image-20231007195144035.png)



## Docker相关命令

注意：需要在root用户下执行

### 下载镜像 / 查看镜像

```shell
# 下载镜像
docker pull hello-world

# 查看本地已经下载好的镜像
docker images
```

![image-20231007200617036](README.assets/image-20231007200617036.png)

注意：docker是一个分层结构，如果上层产品依赖于底层的某些库，是可以共享的

### 启动容器

就是将某一个镜像放到容器中去运行，**镜像 ≠ 容器**，一个镜像可以放到很多个容器去运行

```shell
# 启动
docker run hello-world
```

![image-20231007200542403](README.assets/image-20231007200542403.png)

### 以detach的方式启动容器

![image-20231007201200069](README.assets/image-20231007201200069.png)

### 以交互方式进入容器内部

![image-20231007201245238](README.assets/image-20231007201245238.png)

### 查看容器的状态

```shell
# 只能查看正在运行的容器
docker ps 

# 查看所有的容器（包括停止的和正在运行的）
docker ps -a
```



![image-20231007200728828](README.assets/image-20231007200728828.png)



### 启动一个停止运行的容器


```shell
# 启动一个停止运行的容器 xxx为容器ID
docker start xxx
```

![image-20231007200911743](README.assets/image-20231007200911743.png)



### 停止容器的运行

```shell
# 停止容器的运行 xxx为容器ID
docker stop xxx
```

![image-20231007201112889](README.assets/image-20231007201112889.png)



### 删除容器


```shell
# 删除容器 xxx为容器ID
docker rm xxx
```

![image-20231007200955907](README.assets/image-20231007200955907.png)

### 端口号

![image-20231007201023491](README.assets/image-20231007201023491.png)



### 建立宿主机到容器的端口映射

![image-20231007205116944](README.assets/image-20231007205116944.png)



### 在容器内部退出的操作

![image-20231007201453581](README.assets/image-20231007201453581.png)

### 当容器运行时，重新进入容器内部

![image-20231007201551133](README.assets/image-20231007201551133.png)

### 指定容器的名字

如果不指定，就是一个hash值

![image-20231007221015452](README.assets/image-20231007221015452.png)

### 进入到容器内部修改文件

![image-20231007211315671](README.assets/image-20231007211315671.png)

在容器修改的文件，只针对某一个容器会起作用，当容器被删除之后，那么所做的任何修改都不会保存下来

如果要达到上述目标，**需要将修改的信息写入到镜像中才行**

因此需要对容器整体进行打包，打包成一个镜像进行保存，下次再运行该镜像

### 把容器打包成镜像

![image-20231007211620028](README.assets/image-20231007211620028.png)

![image-20231007211630050](README.assets/image-20231007211630050.png)

### **将容器中的文件直接挂载到宿主机**ubuntu上

除了端口可以映射之外，文件目录也可以进行映射

![image-20231007211707950](README.assets/image-20231007211707950.png)

![image-20231007211747417](README.assets/image-20231007211747417.png)

这里将本地的`/tmp/html`与nginx中的`/usr/share/nginx/html`目录进行映射，两边在目录中的操作，都会同步更新到两边





# 消息队列RabbitMQ

## 下载

```shell
$ docker pull rabbitmq:management # management，带网页管理的版本
```



## 项目优化

网盘架构3.0

### 如何设计这个消息队列

![image-20231007215327855](README.assets/image-20231007215327855.png)

问题：

1.  网络连接。因为消息队列是一个独立的Server，与生产者和消费者之间进行通信就涉及到了网络连接的问题
2.  消息模式：推模式和拉模式。（观察者模式）**推模式**就是消息主动推送给用户，而**拉模式**是由用户选择是否去拉取消息
3.  限流。消息达到处理的上限时需要丢弃消息
4.  路由选择，队列有多条时，应该如何选择将消息交给哪一个队列。



### AMQP协议

![image-20231007220451237](README.assets/image-20231007220451237.png)

数据经过交换机（Exchange）处理之后，由Exchange进行路由选择（路由绑定规则），选择不同的队列

### 交换机

![image-20231007220719059](README.assets/image-20231007220719059.png)

### Docker部署RabbitMQ

![image-20231007221101805](README.assets/image-20231007221101805.png)

```shell
docker run -d --hostname rabbit_server --name rabbit -p 5672:5672 -p 15672:15672 -p 25672:25672 rabbitmq:management
```

>   第一对映射的端口是AMQP的，第二对是HTTP的，第三对是集群的

管理员页面（IP+15672 http://192.168.10.128:15672/）的用户名和密码默认都是guest

![image-20231007221746522](README.assets/image-20231007221746522.png)

登录成功之后进入到如下页面

![image-20231007221857525](README.assets/image-20231007221857525.png)

创建三个队列

![image-20231007222105862](README.assets/image-20231007222105862.png)

![image-20231007222138016](README.assets/image-20231007222138016.png)

创建交换机

![image-20231007222259810](README.assets/image-20231007222259810.png)

绑定路由

![image-20231007222435888](README.assets/image-20231007222435888.png)

![image-20231007222421554](README.assets/image-20231007222421554.png)

![image-20231007222516279](README.assets/image-20231007222516279.png)

发布消息

![image-20231007222638401](README.assets/image-20231007222638401.png)

提示消息已发送

![image-20231007222700649](README.assets/image-20231007222700649.png)

查看队列，通过key1发送的，只有queue1和queue2收到消息

![image-20231007222718080](README.assets/image-20231007222718080.png)

查看具体某个队列

![image-20231007222831755](README.assets/image-20231007222831755.png)

可以查看到收到的消息内容

![image-20231007222921666](README.assets/image-20231007222921666.png)

使用key2也发送一次消息

![image-20231007223109654](README.assets/image-20231007223109654.png)

![image-20231007223124483](README.assets/image-20231007223124483.png)

一次查看2条消息

![image-20231007223207466](README.assets/image-20231007223207466.png)

### 安装第三方库进行开发

```shell
# 先安装C的客户端，再安装C++的客户端
$ tar zxvf rabbitmq-c-0.11.0.tar.gz
$ cd rabbitmq-c-0.11.0
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install

$ tar zxvf SimpleAmqpClient-2.5.1.tar.gz
$ cd SimpleAmqpClient-2.5.1
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```

安装`SimpleAmqpClient-2.5.1`时，执行`cmake ..`报错：

缺少Boost库

```shell
$ cmake ..
-- The CXX compiler identification is GNU 7.5.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
CMake Error at /usr/local/share/cmake-3.21/Modules/FindPackageHandleStandardArgs.cmake:230 (message):
  Could NOT find Boost (missing: Boost_INCLUDE_DIR chrono system) (Required
  is at least version "1.47.0")
Call Stack (most recent call first):
  /usr/local/share/cmake-3.21/Modules/FindPackageHandleStandardArgs.cmake:594 (_FPHSA_FAILURE_MESSAGE)
  /usr/local/share/cmake-3.21/Modules/FindBoost.cmake:2345 (find_package_handle_standard_args)
  CMakeLists.txt:51 (find_package)


-- Configuring incomplete, errors occurred!
See also "/home/hjh/SimpleAmqpClient-2.5.1/build/CMakeFiles/CMakeOutput.log".
```

解决方案：

```shell
$ sudo apt install libboost-all-dev
```



测试

```cpp
// testRabbitMQ.cc
#include <SimpleAmqpClient/SimpleAmqpClient.h> // -lSimpleAmqpClient

#include <iostream>
#include <string>
using namespace std;

struct AmqpInfo
{
    const string amqp_url = "amqp://guest:guest@localhost:5672";
    const string Exchange = "mytest";
    const string Queue = "queue1";
    const string Routing_key = "key1";
};

void createPublisher() {
    using namespace AmqpClient;
    AmqpInfo amqp_info;
    // ptr_t: shared_ptr
    Channel::ptr_t channel = Channel::Create();

    BasicMessage::ptr_t message = BasicMessage::Create("this is testRabbitMQ");
    channel->BasicPublish(amqp_info.Exchange, amqp_info.Routing_key, message);
}

void createConsumer() {
    using namespace AmqpClient;
    AmqpInfo amqp_info;
    Channel::ptr_t channel = Channel::Create();

    // 指定从哪个队列中获取数据
    channel->BasicConsume(amqp_info.Queue);

    // 定义一个信封
    Envelope::ptr_t envelope;
    while (1) {
        // 超时时间设置为3s
        bool ret = channel->BasicConsumeMessage(envelope, 3000);
        if (ret) {
            string msg = envelope->Message()->Body();
            cout << "consume msg: " << msg << endl;
        }
        else {
            cout << "get message timeout" << endl;
            break;
        }
    }
}

int main(void)
{
    /* createPublisher(); */
    createConsumer();
    return 0;
}
```

```shell
# 注释函数创建生产者和消费者
g++ testRabbitMQ.cc -o publisher -lSimpleAmqpClient
g++ testRabbitMQ.cc -o consumer -lSimpleAmqpClient
```

执行生产者程序之前

![image-20231007232641330](README.assets/image-20231007232641330.png)

执行生产者程序之后

![image-20231007232711065](README.assets/image-20231007232711065.png)

网页后台多了一个channel的状态显示

![image-20231008001826041](README.assets/image-20231008001826041.png)

执行消费者程序

通过一个循环，循环读取queue1中的消息，直到队列中没有消息，然后等待3s的超时时间后退出

![image-20231007232804075](README.assets/image-20231007232804075.png)

![image-20231007232901025](README.assets/image-20231007232901025.png)

#### SimpleAmqpClient源码阅读

##### Channel类

![image-20231008001320820](README.assets/image-20231008001320820.png)

![image-20231008001355034](README.assets/image-20231008001355034.png)

BasicConsumeMessage

![image-20231007233308804](README.assets/image-20231007233308804.png)

##### BasicMessage类

![image-20231008001433659](README.assets/image-20231008001433659.png)

### 代码

修改`loadFileUploadModule`函数

![image-20231007235401001](README.assets/image-20231007235401001.png)

实际去执行备份操作的程序`ConsumerUploader.cc`

![image-20231008000256683](README.assets/image-20231008000256683.png)

编译

```shell
make # 创建服务器程序
make Consumer # 创建备份消费者程序
```

### 流程图

![image-20231008001448026](README.assets/image-20231008001448026.png)

![image-20231008001914880](README.assets/image-20231008001914880.png)

这里发送同步指令在项目中代码实现就是往消息队列写入一个JSON，JSON中封装的是要备份文件的本地路径和在OSS上的目的路径，然后就可以直接返回

然后实际去执行文件备份操作的是`ConsumerUploader`，通过一个`while(1)`循环不断去消费消息队列`ossqueue`，获取到消息（JSON），就解析出要备份文件的本地路径和在OSS上的目的路径，然后调用`OssUploader::doUpload`函数去备份到阿里云OSS

需要注意的是，备份文件消费者程序需要和Server在同一个主机，因为消息中的本地文件路径使用的是相对路径





# 微服务

## 如何设计一个RPC的框架

1.  网络连接。因为API网关需要跟每一个后端业务服务器进行交互，本质上是一个进程间通信的问题
2.  序列化与反序列化机制（xml、JSON、**protobuf**）。
3.  代理模式。client-stub和server-stub
4.  注册中心（发现服务）。

### 示意图

![image-20231008014236928](README.assets/image-20231008014236928.png)

### 框架对比

![image-20231008014313022](README.assets/image-20231008014313022.png)

## probobuf

### 特点

1.  专为RPC框架来设计的
2.  二进制，传输时数据量是最小的
3.  向前（forward）、向后（backward）兼容
4.  跨语言（IDL文件）

>   跨语言，JSON也是跨语言的，支持不同的语言去封装/解析JSON
>
>   probobuf实现跨语言是通过IDL文件，IDL（Interface description language，接口描述语言）



**向前向后兼容**

![image-20231008031701417](README.assets/image-20231008031701417.png)



### 安装

```shell
$ tar zxvf protobuf-3.20.1.tar.gz
$ cd protobuf-3.20.1/
$ ./autogen.sh
```

此时出现如下报错：

![image-20231008020102959](README.assets/image-20231008020102959.png)

这个命令表示通过运行 `autogen.sh` 脚本来生成构建系统所需的文件。然而，脚本报告找不到 `autoreconf` 命令。

`autoreconf` 是 GNU Autotools 工具集中的一部分，用于自动生成构建系统所需的文件，如 `configure` 脚本。这个错误通常说明系统上未安装 Autotools 工具。

解决方案：

```shell
sudo apt-get update
sudo apt-get install autoconf automake libtool
```

接着继续执行

```shell
$ ./configure
$ make -j4
$ sudo make install
$ sudo ldconfig
$ protoc --version # 查看版本号
libprotoc 3.20.1
```

>   protoc是一个命令行工具，也称为是probobuf的编译器



### IDL文件

>   注意：required和optional已在proto3弃用了

```protobuf
syntax = "proto3"; // 指定使用proto3语法，默认proto2语法

// 定义一个消息
message ReqSignup
{
    // required - 必须出现（默认）
    // optional - 可以出现，也可以不出校
    // repeated - 可以重复出现，表示数组
    // 类型 字段名   编号
    string username = 1; // 编号是用来编码的，不是值
    string password = 2;
    // repeated string address = 3;
}
```

### 文档

https://protobuf.dev/getting-started/cpptutorial/

![image-20231008023124251](README.assets/image-20231008023124251.png)

### 生成C++相关的文件

```shell
$ protoc --cpp_out=./ user.proto
# 根据写好的proto文件生成头文件和实现文件
user.pb.cc  user.pb.h
```

可以看到通过proto文件，即可生成不同语言的代码文件，从而表明protobuf是支持跨语言的

![image-20231008021513084](README.assets/image-20231008021513084.png)

![image-20231008022809321](README.assets/image-20231008022809321.png)

```cpp
// testUser.cc
#include "user.pb.h"
// g++ testUser.cc user.pb.cc -lprotobuf

#include <iostream>
#include <string>
using namespace std;

void test() {
    ReqSignup req;
    req.set_username("Tvux");
    req.set_password("123");

    // 序列化
    string serialize_str;
    // 序列化为一个字符串
    req.SerializeToString(&serialize_str);
    // 序列化之后是一个二进制的形式
    for (size_t i = 0; i < serialize_str.size(); ++i) {
        // 以十六进制的形式输出serialize_str的每一个字节
        printf("%02x", serialize_str[i]);
    }
    printf("\n");
}

int main(void)
{
    test();
    return 0;
}
```

生成的序列化后的字符串的内容：0a04547675781203313233

![image-20231008031133658](README.assets/image-20231008031133658.png)

所以JSON占据的空间更大，protobuf占据的空间更小



## srpc

中文介绍：https://github.com/sogou/srpc/blob/master/README_cn.md

架构介绍：https://github.com/sogou/srpc/blob/master/docs/wiki.md

### 安装

```shell
$ tar zxvf srpc-0.10.0.tar.gz
$ cd srpc-0.10.0/
$ mkdir build
$ cd build/
$ cmake ..
$ make
$ sudo make install
$ sudo ldconfig
```

#### 报错一

```shell
-- Looking for lz4.h
-- Looking for lz4.h - not found
CMake Error at CMakeLists.txt:24 (message):
  Lz4 is neither installed not in third_party!
```

解决方案：

```shell
# https://github.com/lz4/lz4/releases/tag/v1.9.4
$ tar zxvf lz4-1.9.4.tar.gz
$ cd lz4-1.9.4/
$ make
$ sudo make install
```

#### 报错二

```shell
-- Looking for C++ include snappy.h
-- Looking for C++ include snappy.h - not found
CMake Error at CMakeLists.txt:41 (message):
  Snappy is neither installed not in third_party!
```

解决方案

```shell
$ sudo apt install libsnappy-dev
```



### 使用

srpc的使用先从一个IDL文件开始

```protobuf
// user.proto
...

// 定义一个响应消息
message RespSignup
{
    int32 code = 1;
    string msg = 2;
}

// 服务
// UserService 会变成C++文件中的命名空间
service UserService
{
    // 类似于函数的用法
    // Signup 类似函数名
    // ReqSignup 类似传入参数
    // RespSignup 类似返回值
    rpc Signup(ReqSignup) returns (RespSignup) {}
}
```

```shell
$ protoc --cpp_out=./ user.proto
user.pb.cc  user.pb.h  user.proto
```

srpc安装好之后，可以使用命令行工具`srpc-generator`

```shell
$ srpc_generator
Usage:
    srpc_generator [protobuf|thrift] <idl_file> <output_dir>

Available options(linux):
    -f, --idl_file      : IDL file name. If multiple files are imported, specify the top one. Will parse recursively.
    -o, --output_dir    : Output directory.
    -i, --input_dir     : Specify the directory in which to search for imports.
    -s, --skip_skeleton : Skip generating skeleton file. (default: generate)
    -v, --version       : Show version.
    -h, --help          : Show usage.
    
$ srpc_generator protobuf user.proto ./
client.pb_skeleton.cc server.pb_skeleton.cc  user.pb.cc  user.pb.h  user.proto  user.srpc.h
```

生成了客户端和服务器的框架代码以及头文件

接着就在框架代码中填写自己的业务逻辑

`client.pb_skeleton.cc`

![image-20231008135115551](README.assets/image-20231008135115551.png)

`server.pb_skeleton.cc`

![image-20231008135650157](README.assets/image-20231008135650157.png)

编译运行

```makefile
all:server client

server:
	g++ server.pb_skeleton.cc user.pb.cc -o $@ -lsrpc -lprotobuf -lworkflow

client:
	g++ client.pb_skeleton.cc user.pb.cc -o $@ -lsrpc -lprotobuf -lworkflow

clean:
	rm -rf server client
```

```shell
$ make server
$ make client
# 也可以直接make 默认会编译all

$ ./server
$ netstat -ntlp | grep 1412 # 可以看到服务器已经开始listen了
（并非所有进程都能被检测到，所有非本用户的进程信息将不会显示，如果想看到所有信息，则必须切换到 root 用户）
tcp        0      0 0.0.0.0:1412            0.0.0.0:*               LISTEN      5685/./server  

$ ./client
```

![image-20231008140041936](README.assets/image-20231008140041936.png)



### SRPC小工具

上面这样用太麻烦了，SRPC官方提供了一个小工具

https://github.com/sogou/srpc/blob/master/tools/README_cn.md

可以快速生成SRPC项目



## 项目优化

这里只改了注册业务，其他业务同理

第一步：将`user.pb.h`和`user.srpc.h`移动到`include`目录下

第二步：修改`loadSignupModule`函数，不是直接在`CloudDiskServer`中直接去进行注册，`CloudDiskServer`变成一个API网关，浏览器发送Http请求过来，API网关就创建一个RPC任务，将解析出的用户名和密码转发到后端服务器`SignupServer`，由`SignupServer`去处理注册的实际业务逻辑（解析请求获取用户名和密码、生成salt，密码加密、将数据写入MySQL），执行完之后通过RPC返回`RespSignup`给`CloudDiskServer`，然后`CloudDiskServer`再返回响应给浏览器

```cpp
// 注册
void CloudDiskServer::loadSignUpModule() {
    http_server.POST("/user/signup", [](const HttpReq* req, HttpResp* resp, SeriesWork* series) {
        // 怎么知道这个返回的是一个x-www-urlencoded
        // 直接使用wireshark抓包
        // APPLICATION_URLENCODED 这个宏定义在 HttpDef.h
        if (req->content_type() == APPLICATION_URLENCODED) {
            // 1. 解析请求
            map<string, string>& form_kv = req->form_kv();
            string username = form_kv["username"];
            string password = form_kv["password"];

            // 2. 转发给后端SignupServer，发送RPC请求
            
	        GOOGLE_PROTOBUF_VERIFY_VERSION;
            // v4.0的问题：后端业务服务器的IP和PORT是写死的
            // 所以v5.0进行改进 - 注册中心
	        const char *ip = "127.0.0.1";
	        unsigned short port = 1412;

            // UserService 变成了一个命名空间
	        UserService::SRPCClient client(ip, port);

	        // example for RPC method call
	        ReqSignup signup_req;
	        //signup_req.set_message("Hello, srpc!");
            
            // 组装一个ReqSignup消息
            signup_req.set_username(username);
            signup_req.set_password(password);

            // 客户端发起一个RPC请求，类似于函数调用
	        /* client.Signup(&signup_req, signup_done); */

            // 3. 创建RPC任务，并将其加入到序列中运行
            auto rpc_task = client.create_Signup_task([resp](RespSignup* response, srpc::RPCContext* ctx){
                if (ctx->success() && response->code() == 0) {
                    // 5. 响应
                    // 成功必须返回SUCCESS，前端页面已经写好了
                    resp->String("SUCCESS");
                }
                else {
                    resp->String("FAILED");
                }
            });

            // 序列化消息
            rpc_task->serialize_input(&signup_req);
            // 交给序列运行
            series->push_back(rpc_task);

        }
        else {
            resp->String("FAILED");
        }
    });
}
```

第三步：设置`SignupServer`，创建目录`signup_server`，修改`Signup`函数

```cpp
class UserServiceServiceImpl : public UserService::Service
{
public:

	void Signup(ReqSignup *request, RespSignup *response, srpc::RPCContext *ctx) override
	{
		// TODO: fill server logic here
        
        // 代理模式要实现该Signup函数
        // 当该函数执行时，服务器已经收到了ReqSignup数据了
        // 处理逻辑...
        // 接下来只需要填充RespSignup就可以了，由框架发回给客户端
        
        // 1. 解析请求
        std::string username = request->username();
        std::string password = request->password();

        // 2. 密码加密
        string salt = generateSalt();
        string encrypted_pwd = encryptPassword(password, salt);

        // 3. 生成随机手机号和邮箱
        string phone = generatePhone();
        string email = phone + "@qq.com";

        // 4，写入MySQL
        string sql = "INSERT INTO cloud_disk.tbl_user(user_name, salt, user_pwd, email, phone) VALUES('";
        sql += username + "', '" + salt + "', '" + encrypted_pwd + "', '" + email + "', '" + phone + "')";
        LogInfo(sql.c_str());

        const string mysql_url = conf["mysql_url"];
        auto mysql_task = WFTaskFactory::create_mysql_task(mysql_url, 1, [response, ctx](WFMySQLTask* mysql_task){
            // 对MySQL任务进行状态检测
            int state = mysql_task->get_state();
            int error = mysql_task->get_error();
            if (state != WFT_STATE_SUCCESS) {
                string error_str = WFGlobal::get_error_string(state, error);
                LogError("%s", error_str.c_str());

                response->set_code(-1);
                response->set_msg("Signup Failed");
                return;
            }

            // 对SQL进行语法检测
            protocol::MySQLResponse* mysql_resp = mysql_task->get_resp();
            if (mysql_resp->get_packet_type() == MYSQL_PACKET_ERROR) {
                int error_code = mysql_resp->get_error_code();
                string error_msg = mysql_resp->get_error_msg();
                LogError("ERROR %d: %s", error_code, error_msg.c_str());

                response->set_code(-1);
                response->set_msg("Signup Failed");
                return;
            }

            protocol::MySQLResultCursor cursor(mysql_resp);
            if (cursor.get_cursor_status() == MYSQL_STATUS_OK) {
                // 写操作正常执行
                int affected_rows = cursor.get_affected_rows();
                string suffix = affected_rows > 1 ? " rows affected" : " row affected";
                string res = std::to_string(affected_rows) + suffix;
                LogInfo("Query OK, %s", res.c_str());

                response->set_code(0);
                response->set_msg("OK");
            }
            else {
                // 简单返回错误信息
                response->set_code(-1);
                response->set_msg("Signup Failed");
            }
        });

        mysql_task->get_req()->set_query(sql);
        ctx->get_series()->push_back(mysql_task);
    }
};
```

在`signup_server`下编译生成`SignupServer`，在bin目录编译生成`CloudServer`，然后启动



### v4.0注册业务流程

客户端向服务器发送GET /user/signup请求，请求注册页面，服务器返回注册页面

接着用户在客户端输入用户名和密码，接着发送POST请求，将用户名和密码发送给服务器

接着服务器进行解析，解析出用户名和密码后，获取后端注册服务器的IP和PORT（v4.0是写死的，v5.0通过注册中心获取），创建一个`SRPCClient`，封装RPC消息，创建一个RPC任务，并交给序列去运行，在RPC任务的回调函数中，根据返回的消息类型去返回客户端

后端注册服务器`SignupServer`，获取到消息之后，解析请求，获取到用户名和密码，然后生成salt，对密码进行加密，将数据插入MySQL，如果写操作正常执行，返回成功的消息给`CloudDiskServer`，否则返回失败的消息



其他业务同理

-   用户登录`SigninServer`
-   个人中心
    -   加载用户信息`UserinfoServer`
    -   加载文件列表`FilelistServer`
-   文件上传`FileUploadServer`
-   文件下载`FileDownloadServer`

原来的`CloudDiskServer`变成API网关，所有业务的实际操作交给后端业务服务器去完成



# 注册中心

https://book-consul-guide.vnzmi.com/01_what_is_consul.html

## consul安装

```shell
$ docker pull consul
```

## consul部署

### 1. 启动consul服务器

```shell
# 映射的端口号含义：http、lan、wan、dns
$ docker run --hostname consul_server_1 --name consul_node_1 -d -p 8500:8500 -p 8301:8301 -p 8302:8302 -p 8600:8600 consul agent -server -bootstrap-expect 2 -ui -bind=0.0.0.0 -client=0.0.0.0
```

### 2. 查看分配的IP地址

```shell
$ docker inspect --format '{{.NetworkSettings.IPAddress}}' consul_node_1
172.17.0.2
```

### 3. 启动另外两个节点，加入集群

```shell
$ docker run --hostname consul_server_2 --name consul_node_2 -d -p 8501:8500 consul agent -server -ui -bind=0.0.0.0 -client=0.0.0.0 -join 172.17.0.2

$ docker run --hostname consul_server_3 --name consul_node_3 -d -p 8502:8500 consul agent -server -ui -bind=0.0.0.0 -client=0.0.0.0 -join 
```

>   集群的结点数设计成奇数（质数）

### 4. 查看集群状态

```shell
$ docker exec -t consul_node_1 consul members
```

![image-20231008152810687](README.assets/image-20231008152810687.png)

### 网页查看consul

http://192.168.10.128:8500

![image-20231008154414481](README.assets/image-20231008154414481.png)



## ppconsul

### 安装

```shell
$ tar zxvf ppconsul-0.2.3.tar.gz
$ cd ppconsul-0.2.3/
$ mdkir build
$ cd build
$ cmake ..
$ make
$ sudo make install
$ sudo ldconfig
```

### 官网的例子

```cpp
#include "ppconsul/agent.h"

using ppconsul::Consul;
using namespace ppconsul::agent;

// Create a consul client that uses default local endpoint `http://127.0.0.1:8500` and default data center
Consul consul;
// We need the 'agent' endpoint for a service registration
Agent agent(consul);

// Register a service with associated HTTP check:
agent.registerService(
    kw::name = "my-service",
    kw::port = 9876,
    kw::tags = {"tcp", "super_server"},
    kw::check = HttpCheck{"http://localhost:80/", std::chrono::seconds(2)}
);

...

// Unregister service
agent.deregisterService("my-service");

...

// Register a service with TTL
agent.registerService(
    kw::name = "my-service",
    kw::port = 9876,
    kw::id = "my-service-1",
    kw::check = TtlCheck{std::chrono::seconds(5)}
);

// 心跳检测 - 服务器需要给consul去发送心跳包
// Report service is OK
agent.servicePass("my-service-1");

// Report service is failed
agent.serviceFail("my-service-1", "Disk is full");
```
