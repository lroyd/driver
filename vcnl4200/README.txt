1. cat /sys/class/i2c-dev/i2c-1/device/1-0051/name  �鿴��board��ע����豸
	imx6q_add_imx_i2c(0, &mx6q_sabresd_i2c_data);	//ע������
	i2c_register_board_info(0, mxc_i2c0_board_info, ARRAY_SIZE(mxc_i2c0_board_info));		//ע���豸





2. IMX6 ƽ̨ ʹ�� module_i2c_driver(vcnl4200_driver); �����ã� �Ͱ汾�ں�û���������
	��ʵ���Ϻ���չ�����������
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

	
3.����ж��趨���������ڲ��ж��趨


4.֪ʶ�㣺gpio����
	1.���������ó�GPIOģʽ����ͬƽ̨��һ����IMX6��MX6Q_PAD_EIM_D19__GPIO_3_19��
	2.�������У���Ҫ#include <mach/gpio.h>����board��������ų�ʼ������Ҫ���ź�#define SABRESD_I2C_INT_VCNL	IMX_GPIO_NR(3, 28)
		gpio_request(SABRESD_VCNL_POW, "vcnl4200-power"); ��Ӧ gpio_free(SABRESD_VCNL_POW)
	3.�����������ģʽ��gpio_direction_output(SABRESD_VCNL_POW,0);	gpio_direction_input(SABRESD_I2C_INT_VCNL);	
	�ж����ã�
	4.�ȸ������ź��ҵ���Ӧ���жϺţ�gpio_to_irq()
	5.�����жϺ����룺request_threaded_irq �� request_irq  ��Ӧ free_irq()