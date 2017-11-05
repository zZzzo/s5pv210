1.��mach-smdkv210.c��init��:
	���ȵ���i2cƽ̨�豸���ݣ�
			s3c_i2c0_set_platdata(NULL);
			s3c_i2c1_set_platdata(NULL);
			s3c_i2c2_set_platdata(NULL);
	���platform_device�ṹ��,���ﴫ�Ĳ�����NULL
	Ҳ����ʹ��Ĭ�ϵ�������Ϣ:	
		if (!pd)	pd = &default_i2c_data0;
		�������ùܽŵĹ���Ϊi2c��SDA��SCL:npd->cfg_gpio = s3c_i2c0_cfg_gpio;
		����platform_device��Ϣ:
			struct platform_device s3c_device_i2c0 = {
				.name		  = "s3c2410-i2c",				//��3��(0,1,2)�����ֶ������.֮��ƥ���ʱ����õ�.
				.num_resources	  = ARRAY_SIZE(s3c_i2c_resource),
				.resource	  = s3c_i2c_resource,
				.dev		=(struct s3c2410_platform_i2c)npd,//������i2c��������һЩ��������(����ܽ�)
			};


	����������ע��忨��Ϣ�ĺ���:
		//ÿ��i2c���������Թҽ�127���豸
		i2c_register_board_info(0, smdkv210_i2c_devs0,ARRAY_SIZE(smdkv210_i2c_devs0));
		i2c_register_board_info(1, smdkv210_i2c_devs1,ARRAY_SIZE(smdkv210_i2c_devs1));
		i2c_register_board_info(2, smdkv210_i2c_devs2,ARRAY_SIZE(smdkv210_i2c_devs2));
	�����ʾע����3��i2c�����ϵĴ��豸,smdkv210_i2c_devs0�ṹ���а��������ֺ��豸��ַ:
		{	
			{ I2C_BOARD_INFO("24c08", 0x50), },     /* Samsung S524AD0XD1 */
			{ I2C_BOARD_INFO("wm8580", 0x1b), },
		}
	��������Ὣ��Щ�豸ע�ᵽ����__i2c_board_list��ȥ,(�����platformģ�͵�����û��ϵ)
	������platform_add_devices(...);����ע��ƽ̨�豸,ע����device��Ҫע��driver:

2.��i2c_s3c2410.c��init��:
	ֻ��һ������:platform_driver_register(&s3c24xx_i2c_driver);��ע����driver.
		����:
			s3c24xx_i2c_driver={
				.probe		= s3c24xx_i2c_probe,
				.remove		= s3c24xx_i2c_remove,
				.id_table	= s3c24xx_driver_ids,	//ƥ��ʱ�����������id_table��.
				.driver		= {
					.owner	= THIS_MODULE,
					.name	= "s3c-i2c",		//���������Ȼ�������device�����ֲ�һ��.
					.pm	= S3C24XX_DEV_PM_OPS,
				},};
			����ֻע����һ��driver,������Ȼ��deviceƥ���Ϻ�,���probe�����ᱻ����3��.
		���ڿ�probe������:
				...
			i2c->adap.nr = pdata->bus_num;		//��� 0 1 2
			ret = i2c_add_numbered_adapter(&i2c->adap);
				...
		ע��I2C������.��Ϊ����3��probe,��ôӦ��ע����3��adapter.������adapter��adap.nr����.
		
		��i2c_add_numbered_adapter(&i2c->adap)��,ע����i2c������:i2c_register_adapter(adap),
		ע���ʱ��,��ִ����i2c_scan_static_board_info(adap):
			Ҳ���Ǳ���֮ǰ��mach-smdkv210.c��ע��忨��Ϣʱ,�������Ǹ�����__i2c_board_list,
		�������е�ǰ���еĴ��豸(����)(��ַ)(��������):
				...
		if (devinfo->busnum == adapter->nr && !i2c_new_device(adapter,&devinfo->board_info))
				...
			//��������ϵ�����豸��������adapter�ϵ�,��ô�ᴴ��һ��client.
		��i2c_new_device(...)��,�Ὣ�忨��Ϣ����client,Ȼ��register:
			device_register(&client->dev);		//��������driver_register���������Լ�д�ľ�������
			����˵,�Ӵ����忨��Ϣ����,��ע��i2c����pdev,��ע��i2c����pdrv,Ȼ�����probe(),probe
		�ٴ�����Ӧ��adapter,adapter�ٸ���֮ǰ�����İ忨��Ϣ����,������Ӧ��client,������ע�ᵽ����
3.������Ҫʵ�ֵ�:
	����,��Ҫʵ�ֵ���client��driver��:
		1����ֻ��Ҫע��һ������:
				driver_register(&at24_drv);
		2����ôֻ��Ҫʵ�ֶ�����������ṹ��i2c_driver��
				struct i2c_driver at24_drv ={
					.probe = at24_i2c_probe,
					.remove	= at24_i2c_remove,
					.driver = {
						.name = "at24_e2prom_drv",
					}
					.id_table = at24_id_table,
				};
		3��ֻ��Ҫʵ�����е�probe��remove������,id_table����д��һ������.
			����id_table:
			struct i2c_device_id {
				char name[I2C_NAME_SIZE];
				kernel_ulong_t driver_data	/* Data private to the driver */
						__attribute__((aligned(sizeof(kernel_ulong_t))));
			};
			�����driver_data����д�����Լ���Ҫ�����ݷ�װ�ɵĽṹ��:
			������������id_table����:{"at24c02", (kernel_ulong_t)&at2402_private},
		4���õ��Ķ�д���ݵĺ���:
			i2c_master_recv(...)/i2c_master_send(...)
			���copy_to_user(...)��copy_from_user(...)
