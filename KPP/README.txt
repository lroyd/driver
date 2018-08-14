按键驱动(利用系统自带的代码)
driver中有两个keypad，imx_keypad是imx6平台专用使用KPP寄存器实现，matrix_keypad是gpio模拟实现
A.gpio模拟实现
	1.选中对应的驱动
	2.在board中添加对应的platform_device及键盘相关keymap
	3注册col/row引脚（ex 4*4）
	#define SABRESD_COL_KEY4     IMX_GPIO_NR(4, 14)		//110

	#define SABRESD_COL_KEY5     IMX_GPIO_NR(4, 5)		//101

	#define SABRESD_COL_KEY6     IMX_GPIO_NR(1, 12)		//12
	#define SABRESD_COL_KEY7     IMX_GPIO_NR(1, 14)		//14

	#define SABRESD_ROW_KEY4     IMX_GPIO_NR(4, 15)		//111

	#define SABRESD_ROW_KEY5     IMX_GPIO_NR(1, 11)		//11

	#define SABRESD_ROW_KEY6     IMX_GPIO_NR(1, 13)		//13

	#define SABRESD_ROW_KEY7     IMX_GPIO_NR(1, 15)		//15	
	4映射键值
	static const uint32_t sabresd_keymap[] = {

		KEY(0, 0, KEY_ESC),
	KEY(0, 1, KEY_1),
    KEY(0, 2, KEY_2),
    KEY(0, 3, KEY_3),
   
		KEY(1, 0, KEY_4),
	KEY(1, 1, KEY_5),
    KEY(1, 2, KEY_6),
    KEY(1, 3, KEY_7),
    
		KEY(2, 0, KEY_8),
    KEY(2, 1, KEY_9),
    KEY(2, 2, KEY_0),
    KEY(2, 3, KEY_MINUS),
    
		KEY(3, 0, KEY_EQUAL),
    KEY(3, 1, KEY_BACKSPACE),
    KEY(3, 2, KEY_TAB),
    KEY(3, 3, KEY_Q),
	}；
	5添加platform_device
	6在board中注册
	
	参考http://www.cnblogs.com/helloworldtoyou/p/6236583.html
		
B.KPP寄存器实现
	1.在board中添加设备，同时需要将platform_imx_keypad.c（注释掉CONFIG_SOC_IMX6SL，这里IMX6SL使用KPP和IMX6Q是同一个地址）加进去
	2.映射键值
	注意使用KPP寄存器方式，需要在board.h中加上引脚功能定义配置成对应的KPP模式	
	映射的开始必须根据使用的KPPcol和row号定义
	3.在在board中初始化引脚功能并注册设备
	mxc_iomux_v3_setup_multiple_pads(mx6q_sabresd_kpp_pads, ARRAY_SIZE(mx6q_sabresd_kpp_pads));	
	imx6sl_add_imx_keypad(&sabresd_keymap_data);



知识点：
	A.想要添加对应platform下的device
		1.在对应的mach下（board.c同级目录）Kconfig 中添加 select IMX_HAVE_PLATFORM_XXXX
		2.完成1下在kernel/.config中的CONFIG_IMX_HAVE_PLATFORM_XXXX=y

	B.mkimage" command not found - U-Boot images will not be built错误
		sudo apt-get install uboot-mkimage 或者 sudo apt-get install u-boot-tools 或者用厂商提供的uboot编译出来的mkimage（方法没试过）




