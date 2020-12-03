# 描述

- API接口

# 接口版本

|版本号|制定人|制定日期|版本描述|
|:---- |:---  |:-----  |-----   |
|1.0   |谢易成 |2020-11-03 |初始版本|

# ip地址
```
http://192.168.1.1
```

# API列表

|名字|描述|
|:----    |-----   |
|set_path |设置播放路径|
|set_volume |设置播放音量|
|switch_music |切换歌曲|

##1、set_path

### 请求参数说明

|参数名|类型|说明|
|:-----|:-----|-----|
|path|string|文件路径信息|
### 调用示例
```
ubus call mp3player set_path '{"path":"/usr/layout_test.mp3"}'
```

### 返回示例

```
{
        "path": "\/usr\/layout_test.mp3"
}
```

### 返回参数说明

|参数名|类型|说明|
|:-----|:-----|-----|
|path|string|文件路径信息|

### 错误码
|错误码|说明|
|:-----|:-----|

##2、set_volume

### 请求参数说明

|参数名|类型|说明|
|:-----|:-----|-----|
|volume|uint32|音量范围0-90|
### 调用示例
```
ubus call mp3player set_volume '{"volume":10}'
```
### 返回示例

```
{
        "volume": 10
}
```

### 返回参数说明

|参数名|类型|说明|
|:-----|:-----|-----|
|volume|uint32|音量值|

### 错误码
|错误码|说明|
|:-----|:-----|

##3、switch_music

### 请求参数说明

|参数名|类型|说明|
|:-----|:-----|-----|
|switch|string|选择上一曲"up"或者是下一曲"down"|

### 调用示例

```
ubus call mp3player switch_music '{"switch":"up"}'
ubus call mp3player switch_music '{"switch":"down"}'
```

### 返回示例

```
{
        "switch": "down",
        "index": 1,
        "music": "\/usr\/layout_test2.mp3"
}
```

### 返回参数说明

|参数名|类型|说明|
|:-----|:-----|-----|
|switch|string|选择上一曲或者是下一曲|
|index|uint32|当前歌曲所在目录位置|
|music|string|当前播放的歌曲路径|

### 错误码
|错误码|说明|
|:-----|:-----|





