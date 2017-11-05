1.在mach-smdkv210.c的init中:
	首先调用i2c平台设备数据：
			s3c_i2c0_set_platdata(NULL);
			s3c_i2c1_set_platdata(NULL);
			s3c_i2c2_set_platdata(NULL);
	填充platform_device结构体,这里传的参数是NULL
	也就是使用默认的配置信息:	
		if (!pd)	pd = &default_i2c_data0;
		并且配置管脚的功能为i2c的SDA和SCL:npd->cfg_gpio = s3c_i2c0_cfg_gpio;
		最后的platform_device信息:
			struct platform_device s3c_device_i2c0 = {
				.name		  = "s3c2410-i2c",				//这3组(0,1,2)的名字都是这个.之后匹配的时候会用到.
				.num_resources	  = ARRAY_SIZE(s3c_i2c_resource),
				.resource	  = s3c_i2c_resource,
				.dev		=(struct s3c2410_platform_i2c)npd,//这里是i2c控制器的一些基本配置(比如管脚)
			};


	接下来调用注册板卡信息的函数:
		//每个i2c控制器可以挂接127个设备
		i2c_register_board_info(0, smdkv210_i2c_devs0,ARRAY_SIZE(smdkv210_i2c_devs0));
		i2c_register_board_info(1, smdkv210_i2c_devs1,ARRAY_SIZE(smdkv210_i2c_devs1));
		i2c_register_board_info(2, smdkv210_i2c_devs2,ARRAY_SIZE(smdkv210_i2c_devs2));
	这里表示注册这3组i2c总线上的从设备,smdkv210_i2c_devs0结构体中包含了名字和设备地址:
		{	
			{ I2C_BOARD_INFO("24c08", 0x50), },     /* Samsung S524AD0XD1 */
			{ I2C_BOARD_INFO("wm8580", 0x1b), },
		}
	这个函数会将这些设备注册到链表__i2c_board_list中去,(这里跟platform模型的链表没关系)
	最后调用platform_add_devices(...);批量注册平台设备,注册完device还要注册driver:

2.在i2c_s3c2410.c的init中:
	只有一个函数:platform_driver_register(&s3c24xx_i2c_driver);它注册了driver.
		参数:
			s3c24xx_i2c_driver={
				.probe		= s3c24xx_i2c_probe,
				.remove		= s3c24xx_i2c_remove,
				.id_table	= s3c24xx_driver_ids,	//匹配时的名字在这个id_table中.
				.driver		= {
					.owner	= THIS_MODULE,
					.name	= "s3c-i2c",		//这个名字显然跟上面的device的名字不一样.
					.pm	= S3C24XX_DEV_PM_OPS,
				},};
			这里只注册了一个driver,所以显然与device匹配上后,这个probe函数会被调用3次.
		现在看probe函数里:
				...
			i2c->adap.nr = pdata->bus_num;		//编号 0 1 2
			ret = i2c_add_numbered_adapter(&i2c->adap);
				...
		注册I2C控制器.因为调用3次probe,那么应该注册了3个adapter.这三个adapter靠adap.nr区分.
		
		在i2c_add_numbered_adapter(&i2c->adap)里,注册了i2c适配器:i2c_register_adapter(adap),
		注册的时候,又执行了i2c_scan_static_board_info(adap):
			也就是遍历之前在mach-smdkv210.c中注册板卡信息时,创建的那个链表__i2c_board_list,
		链表上有当前所有的从设备(名字)(地址)(所属总线):
				...
		if (devinfo->busnum == adapter->nr && !i2c_new_device(adapter,&devinfo->board_info))
				...
			//如果链表上的这个设备是我这组adapter上的,那么会创建一个client.
		在i2c_new_device(...)里,会将板卡信息赋给client,然后register:
			device_register(&client->dev);		//接下来的driver_register就是我们自己写的具体驱动
			所以说,从创建板卡信息链表,到注册i2c总线pdev,到注册i2c总线pdrv,然后调用probe(),probe
		再创建对应的adapter,adapter再根据之前创建的板卡信息链表,创建对应的client,并把它注册到总线
3.我所需要实现的:
	首先,我要实现的是client的driver层:
		1、我只需要注册一个驱动:
				driver_register(&at24_drv);
		2、那么只需要实现定义这个驱动结构体i2c_driver：
				struct i2c_driver at24_drv ={
					.probe = at24_i2c_probe,
					.remove	= at24_i2c_remove,
					.driver = {
						.name = "at24_e2prom_drv",
					}
					.id_table = at24_id_table,
				};
		3、只需要实现其中的probe、remove、另外,id_table可以写成一个数组.
			关于id_table:
			struct i2c_device_id {
				char name[I2C_NAME_SIZE];
				kernel_ulong_t driver_data	/* Data private to the driver */
						__attribute__((aligned(sizeof(kernel_ulong_t))));
			};
			这里的driver_data可以写我们自己需要的数据封装成的结构体:
			类似这样定义id_table数组:{"at24c02", (kernel_ulong_t)&at2402_private},
		4、用到的读写数据的函数:
			i2c_master_recv(...)/i2c_master_send(...)
			配合copy_to_user(...)和copy_from_user(...)
