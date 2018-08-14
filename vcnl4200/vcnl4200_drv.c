/*******************************************************************************
	> File Name: driver for vcnl4200
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <mach/gpio.h>

/*******************************************************************************
*		MACRO	define
********************************************************************************/
#define DEVICE_NAME		"vcnl4200"
#define DEVICE_ADDR		(0x51)
#define DRV_VERSION		"2.0.0"
#define DRV_MODE_POLL	(1)		
/*******************************************************************************
*		GPIO PIN define
********************************************************************************/
#define VCNL_POW_PIN		IMX_GPIO_NR(3, 29)
#define VCNL_INT_PIN		IMX_GPIO_NR(3, 28)

/*******************************************************************************
*		STRUCT	define
********************************************************************************/
enum {
	FALSE,
	TRUE,
};

typedef enum _tag
{
	ALS_CONFIG	= 0x00,
	ALS_THDH	= 0x01,
	ALS_THDL	= 0x02,
	PS_CONFIG	= 0x03,
	PS_MS		= 0x04,
	PS_CANC		= 0x05,
	PS_THDL		= 0x06,
	PS_THDH		= 0x07,
	PS_DATA		= 0x08,
	ALS_DATA	= 0x09,
	INT_FLAG 	= 0x0D,
}VCNL_CTR_REG;

static dev_t dev_major;
static struct class *dev_class;
static struct device *dev_node;
static struct cdev cdev;

static DECLARE_WAIT_QUEUE_HEAD(wait_q_head);
static int wait_flag = 0;
static short action = 0;

static char opened = FALSE;

typedef struct _tag_vcnl4200_t 
{
	struct i2c_client *client;
} vcnl4200_t;

static vcnl4200_t vcnl4200 = {0};


/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static irqreturn_t irq_threadhandler(int irq, void *dev_id) 
{
	struct i2c_client *client = vcnl4200.client;
	int int_data;

	if(TRUE != opened)
	{
		printk(KERN_ERR "irq_threadhandler device not open\n");
		return IRQ_HANDLED;
	}
	
	int_data = i2c_smbus_read_word_data(client, INT_FLAG);
	action = i2c_smbus_read_word_data(client, PS_DATA);
	
	wait_flag = 1;
	wake_up_interruptible(&wait_q_head);
	
	return IRQ_HANDLED;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static int fops_open(struct inode *inode, struct file *filp) 
{
	int ret = 0;
	struct i2c_client *client = vcnl4200.client;
	if(TRUE == opened) 
	{
		return -EBUSY;
	}	
	opened = TRUE;

	gpio_request(VCNL_POW_PIN, "vcnl4200-power");
	gpio_direction_output(VCNL_POW_PIN,0);
	
	
	gpio_request(VCNL_INT_PIN, "vcnl4200-int");
	gpio_direction_input(VCNL_INT_PIN);
	
	
	ret |= i2c_smbus_write_word_data(client, ALS_CONFIG, 0x0001);	//ALS disable
	ret |= i2c_smbus_write_word_data(client, PS_CONFIG , 0x0B3A);	//H[7-4:x]	[3:out 12/16]	[2:x]	[1-0:int conf] 
	ret |= i2c_smbus_write_word_data(client, PS_MS     , 0x0040);	//0x0040  长距离
	/* Setting the default distance value */
	ret |= i2c_smbus_write_word_data(client, PS_THDH     , 0x000A);
	ret |= i2c_smbus_write_word_data(client, PS_THDL     , 0x000A);	
	if (ret) 
	{
		printk(KERN_ERR "sensor-vcnl4200 configure error !!!\n");
		return -1;
	}
	
	i2c_smbus_read_word_data(client, INT_FLAG);
	
	wait_flag = 0;
	action = 0;	
	
	ret = request_threaded_irq(client->irq, NULL, irq_threadhandler, IRQ_TYPE_EDGE_FALLING, NULL, NULL); //IRQ_TYPE_EDGE_BOTH IRQ_TYPE_EDGE_FALLING
	if (ret < 0) 
	{
		dev_err(&client->dev, "cannot request_threaded_irq irq %d!\n", client->irq);
		return -ENODEV;
	}	
	
	printk(KERN_DEBUG "sensor-vcnl4200 fops_open\n");
	return 0;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static int fops_release(struct inode *inode, struct file *filp)
{	
	struct i2c_client *client = vcnl4200.client;	
	
	if(TRUE != opened) 
	{
		return 0;
	}
	opened = FALSE;
	free_irq(client->irq, NULL);
	i2c_smbus_write_word_data(client, PS_CONFIG , 0x0001);	//PS disable
	
	gpio_free(VCNL_INT_PIN);
	gpio_free(VCNL_POW_PIN);
	
	printk(KERN_DEBUG "sensor-vcnl4200 fops_release\n");
    return 0;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static ssize_t fops_read(struct file *file, char __user *buf, size_t len, loff_t *off) 
{
#if DRV_MODE_POLL 	
	if(wait_event_interruptible(wait_q_head, wait_flag == 1))
	{
        return -ERESTARTSYS;
    }

	if(0 != copy_to_user(buf, &action, sizeof(char))) 
	{
		return -1;
	}

	action = 0;
	wait_flag = 0;	
#else
	struct i2c_client *client = vcnl4200.client;
	short int_data = 0;

	int_data = i2c_smbus_read_word_data(client, INT_FLAG);
	action = i2c_smbus_read_word_data(client, PS_DATA);
	
	printk("test: int flags = 0x%02x, data = 0x%02x\r\n",int_data, action);
#endif
    return sizeof(short);
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static ssize_t fops_write(struct file *file, const char __user *buf, size_t len, loff_t *off) 
{
	
	return 0;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static unsigned int fops_poll(struct file *file, struct poll_table_struct *table) 
{
	unsigned int mask = 0;
	poll_wait(file, &wait_q_head, table);
	
	if (action)
	{
		mask |= POLL_IN|POLLRDNORM;
	}
    return mask;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static long fops_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
	struct i2c_client *client = vcnl4200.client;
	int ret = 0;
	switch(cmd) 
	{
		case 0: 	//distance value
		{
			unsigned short value = (unsigned short)arg;
			ret |= i2c_smbus_write_word_data(client, PS_THDH     , value);
			ret |= i2c_smbus_write_word_data(client, PS_THDL     , value);	
			if(ret)
			{
				printk(KERN_ERR "sensor-vcnl4200 set PS distance value error!!!\n");
				return -1;				
			}
			break;
		}
	}
	return 0;
}


static struct file_operations vcnl4200_fops = 
{
    .owner 		= THIS_MODULE,
    .open 		= fops_open,
    .release 	= fops_release,
    .poll   	= fops_poll,
    .read   	= fops_read,
    .write   	= fops_write,
	.unlocked_ioctl= fops_ioctl,	
};

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static int vcnl4200_probe(struct i2c_client *client,const struct i2c_device_id *id) 
{
	int ret;
	vcnl4200.client = client;

	ret = alloc_chrdev_region(&dev_major, 0, 1, DEVICE_NAME);
	if (ret < 0) 
	{
		printk(KERN_ERR "cannot alloc_chrdev_region!\n");
		return ret;
	}
	cdev_init(&cdev, &vcnl4200_fops);
	cdev.owner = THIS_MODULE;
	ret = cdev_add(&cdev, dev_major, 1);
	if (ret < 0) 
	{
		printk(KERN_ERR "cannot add cdev device!\n");
		return ret;
	}
	dev_class = class_create(THIS_MODULE, "vcnl4200_class");
	if(IS_ERR(dev_class))
	{
		printk(KERN_ERR "cannot create class\n");
	} else 
	{
		dev_node = device_create(dev_class, NULL, MKDEV(MAJOR(dev_major), 0), NULL, DEVICE_NAME);
	}
	printk(KERN_DEBUG "vcnl4200_probe major %d, name %s, irq %d\n", MAJOR(dev_major), DEVICE_NAME, client->irq);	
	return 0;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static int vcnl4200_remove(struct i2c_client *client) 
{
	unregister_chrdev_region(dev_major, 1);
	device_destroy(dev_class, MKDEV(MAJOR(dev_major), 0));
	cdev_del(&cdev);
	class_destroy(dev_class);
	printk(KERN_DEBUG "vcnl4200_remove major %d, name %s, irq %d\n", MAJOR(dev_major), DEVICE_NAME, client->irq);
	return 0;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static const struct i2c_device_id vcnl4200_id[] = 
{
	{ "vcnl4200", 0 },
	{ }
};
//MODULE_DEVICE_TABLE(i2c, vcnl4200_id);

static struct i2c_driver vcnl4200_driver = 
{
	.driver		= {
		.name	= "vcnl4200",
	},
	.probe		= vcnl4200_probe,
	.remove		= vcnl4200_remove,
	.id_table	= vcnl4200_id,
};

//module_i2c_driver(vcnl4200_driver);

static int vcnl4200_init (void)
{
	int res = 0;
	res = i2c_add_driver (&vcnl4200_driver);
	if (res)
	{
		printk(KERN_ERR "vcnl4200 i2c_add_driver failed! \n");
		return res;
	}
	return 0;
}

static void __exit vcnl4200_exit (void)
{
	i2c_del_driver (&vcnl4200_driver);
}

module_init (vcnl4200_init);
module_exit (vcnl4200_exit);


MODULE_AUTHOR("zhanght <lroyd-zhang@systechn.com>");
MODULE_DESCRIPTION("vcnl4200driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);