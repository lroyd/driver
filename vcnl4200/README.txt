1. cat /sys/class/i2c-dev/i2c-1/device/1-0051/name  查看在board中注册的设备
	imx6q_add_imx_i2c(0, &mx6q_sabresd_i2c_data);	//注册总线
	i2c_register_board_info(0, mxc_i2c0_board_info, ARRAY_SIZE(mxc_i2c0_board_info));		//注册设备





2. IMX6 平台 使用 module_i2c_driver(vcnl4200_driver); 不能用？ 低版本内核没有这个函数
	其实如上函数展开后就是下面
	static int vcnl4200_init (void)
	{ 
		i2c_add_driver (&vcnl4200_driver);
		return 0;
	}
	static void __exit vcnl4200_exit (void)
	{
		i2c_del_driver (&vcnl4200_driver);
	}
	module_init (vcnl4200_init);
	module_exit (vcnl4200_exit);

	
3.外层中断设定还是驱动内部中断设定


4.知识点：gpio操作
	1.将引脚设置成GPIO模式（不同平台不一样，IMX6：MX6Q_PAD_EIM_D19__GPIO_3_19）
	2.在驱动中（需要#include <mach/gpio.h>）或board中添加引脚初始化：需要引脚号#define SABRESD_I2C_INT_VCNL	IMX_GPIO_NR(3, 28)
		gpio_request(SABRESD_VCNL_POW, "vcnl4200-power"); 对应 gpio_free(SABRESD_VCNL_POW)
	3.配置输入输出模式：gpio_direction_output(SABRESD_VCNL_POW,0);	gpio_direction_input(SABRESD_I2C_INT_VCNL);	
	中断设置：
	4.先根据引脚号找到对应的中断号：gpio_to_irq()
	5.根据中断号申请：request_threaded_irq 或 request_irq  对应 free_irq()