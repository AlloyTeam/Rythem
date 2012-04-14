# Rythem 

 <strong>fiddler like cross-platforms(MacOS/win/*linux*) tool using `Qt`</strong>


### 功能
 
* http代理服务 (https未实现)
* 规则替换

    <pre>
    匹配模式包括`wildcard`类型及全匹配两种
    以替换后内容区分有本地及远程两种。
    本地替换有三种：目录式，单个文件式，多文件合并成一文件
    远程替换暂时只支持一个文件对应一个远程路径
    </pre>

* host设置
* 替换规则远程及本地导入。
* 替换规则增删改。
* 颜色标记已被替换的请求
* 导入/导出 每条请求（兼容fiddler *.saz文件)
* 导出response body

### TODOs

* 规则管理(远程规则的更新机制）
* 各OS/浏览器版本下稳定性测试
* MacOS下自动设置代理
* 过滤显示请求


### 主要代码结构

* `RyProxyServer`: 代理server
* `RyConnection`: 每个socket对应一个实例，掌管每个请求的client socket及remote socket并处理相应的请求
* `RyPipeData`: 保存各个http请求的信息（包括request及response）
* `rule::RyRuleManager`: 做规则替换相关

### 以上几个类的关系：

1. ，当 `RyProxyServer` 检测到有新的client socket时，生成一个RyConnection实例，并将相应socket id传入。
2. `RyConnection` 对此 client socket传入数据分析，解包成若干http请求
    <pre>
    2.1. 每解析到一个http包生成一个`RyPipeData`实例，并压入缓冲队列pipeList 
    2.2. 生成pipeData后，如果当前有未完成的pipeData，结束，否则跳到2.3
    2.3 通过`RyRuleManger`检测是否有当前相应的替换规则
        如有匹配规则，跳到2.3
        如无匹配规则，跳到2.4
    2.3 如获取规则则照获修改`RyPipeData`，如修改后的pipeData已有内容（内容替换类规则）
         跳到2.5，否则跳到2.4
    2.4 向远程socket写入当前pipeData的http request 数据并监听返回
         当远程返回数据解包完成，跳到2.5
    2.5 获取下一个队列中的pipeData，如果为空，结束，否则跳到2.2步
    </pre>
3. 当`RyConnection`解析到新的请求包时，或解析到返回包时，发出相应的signal给具体UI