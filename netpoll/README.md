
> go version go1.24.2 linux/arm64

# Level-triggered and edge-triggered
https://man7.org/linux/man-pages/man7/epoll.7.html

Go 原生的 `net` 是使用 `EPOLLET` (Edge Triggered) 的方式來處理事件通知。


## Golang TCP Server
![alt text](imgs/image-34.png)


## Golang 如何讓使用者感受到主塞

### Listen
![alt text](imgs/image.png)
![alt text](imgs/image-1.png)
![alt text](imgs/image-2.png)
![alt text](imgs/image-3.png)
![alt text](imgs/image-4.png)

#### 這邊可以看到 `SOCK_NONBLOCK`

![alt text](imgs/image-5.png)

![alt text](imgs/image-6.png)
![alt text](imgs/image-7.png)
![alt text](imgs/image-8.png)
![alt text](imgs/image-9.png)
![alt text](imgs/image-10.png)

#### `EPOLLET` - Edge Triggered

![alt text](imgs/image-11.png)

### Accept
![alt text](imgs/image-13.png)
![alt text](imgs/image-14.png)
![alt text](imgs/image-15.png)
![alt text](imgs/image-16.png)
![alt text](imgs/image-17.png)
![alt text](imgs/image-18.png)
#### 可以看到 wait 時會呼叫 `gopark`
![alt text](imgs/image-19.png)


### Listener
![alt text](imgs/image-12.png)

### Read
![alt text](imgs/image-20.png)
![alt text](imgs/image-21.png)
![alt text](imgs/image-22.png)
![alt text](imgs/image-23.png)

#### 這裡一樣使用 `pd.wait` 去呼叫 `gopark`
![alt text](imgs/image-24.png)


### 什麼時候喚醒 ?
![alt text](imgs/image-32.png)

![alt text](imgs/image-29.png)
![alt text](imgs/image-30.png)
![alt text](imgs/image-31.png)
![alt text](imgs/image-25.png)


#### 最後呼叫 `goready` 來喚醒 goroutine
![alt text](imgs/image-27.png)


## Summary

## Single Reactor

![alt text](imgs/image-35.png)

## [The edge-triggered misunderstanding](https://lwn.net/Articles/864947/)


# Multi-Reactors 異步網路模型

> 取自 gnet.host
![alt text](imgs/image-33.png)


![alt text](https://raw.githubusercontent.com/panjf2000/illustrations/master/go/multi-reactors.png)
![alt text](https://raw.githubusercontent.com/panjf2000/illustrations/master/go/multi-reactors-sequence-diagram.png)

![alt text](https://raw.githubusercontent.com/panjf2000/illustrations/master/go/multi-reactors%2Bthread-pool.png)

![alt text](https://raw.githubusercontent.com/panjf2000/illustrations/master/go/multi-reactors%2Bthread-pool-sequence-diagram.png)