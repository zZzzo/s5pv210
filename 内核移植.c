

timer_func()
{
	取到dev_id赋给的那个全局变量
	判断按键是按下了还是释放了
	判断完保存按键值
}
isr()
{
	确定哪个按键触发的中断,(dev_id赋给一个全局变量(中断服务程序和进程共享的全局变量要volatile))
	启动定时器。
}


xxx_open
{
	注册中断
}

xxx_read
{
	判断是否要阻塞
	copy_to_user
}

struct file_operations xx={};

init
{
	注册设备号
					file_operations实现
	初始化cdev
	向内核添加cdev
	
	设备节点
	
	初始化定时器并添加到内核
}