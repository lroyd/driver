��������(����ϵͳ�Դ��Ĵ���)
driver��������keypad��imx_keypad��imx6ƽ̨ר��ʹ��KPP�Ĵ���ʵ�֣�matrix_keypad��gpioģ��ʵ��
A.gpioģ��ʵ��
	1.ѡ�ж�Ӧ������
	2.��board����Ӷ�Ӧ��platform_device���������keymap
	3ע��col/row���ţ�ex 4*4��
	#define SABRESD_COL_KEY4     IMX_GPIO_NR(4, 14)		//110

	#define SABRESD_COL_KEY5     IMX_GPIO_NR(4, 5)		//101

	#define SABRESD_COL_KEY6     IMX_GPIO_NR(1, 12)		//12
	#define SABRESD_COL_KEY7     IMX_GPIO_NR(1, 14)		//14

	#define SABRESD_ROW_KEY4     IMX_GPIO_NR(4, 15)		//111

	#define SABRESD_ROW_KEY5     IMX_GPIO_NR(1, 11)		//11

	#define SABRESD_ROW_KEY6     IMX_GPIO_NR(1, 13)		//13

	#define SABRESD_ROW_KEY7     IMX_GPIO_NR(1, 15)		//15	
	4ӳ���ֵ
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
	}��
	5���platform_device
	6��board��ע��
	
	�ο�http://www.cnblogs.com/helloworldtoyou/p/6236583.html
		
B.KPP�Ĵ���ʵ��
	1.��board������豸��ͬʱ��Ҫ��platform_imx_keypad.c��ע�͵�CONFIG_SOC_IMX6SL������IMX6SLʹ��KPP��IMX6Q��ͬһ����ַ���ӽ�ȥ
	2.ӳ���ֵ
	ע��ʹ��KPP�Ĵ�����ʽ����Ҫ��board.h�м������Ź��ܶ������óɶ�Ӧ��KPPģʽ	
	ӳ��Ŀ�ʼ�������ʹ�õ�KPPcol��row�Ŷ���
	3.����board�г�ʼ�����Ź��ܲ�ע���豸
	mxc_iomux_v3_setup_multiple_pads(mx6q_sabresd_kpp_pads, ARRAY_SIZE(mx6q_sabresd_kpp_pads));	
	imx6sl_add_imx_keypad(&sabresd_keymap_data);



֪ʶ�㣺
	A.��Ҫ��Ӷ�Ӧplatform�µ�device
		1.�ڶ�Ӧ��mach�£�board.cͬ��Ŀ¼��Kconfig ����� select IMX_HAVE_PLATFORM_XXXX
		2.���1����kernel/.config�е�CONFIG_IMX_HAVE_PLATFORM_XXXX=y

	B.mkimage" command not found - U-Boot images will not be built����
		sudo apt-get install uboot-mkimage ���� sudo apt-get install u-boot-tools �����ó����ṩ��uboot���������mkimage������û�Թ���




