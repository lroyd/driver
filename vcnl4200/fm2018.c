#include <linux/init.h>
#include <linux/module.h>


#include <linux/fs.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/ioctl.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include <linux/delay.h>
#include <linux/gpio.h>
#include "fm2018.h"

#define FM2018_DEV_IOCTLID	0x15
#define IOC_MAXNR	2

#define IOCTL_GET_FM_PD_STATUS	_IO(FM2018_DEV_IOCTLID, 1)
#define IOCTL_SET_FM_PD_DEFAULT	_IO(FM2018_DEV_IOCTLID, 2)
#define FM2018_DRIVER_NAME "fm2018"
//#define FM2018_RESET 156


static const struct i2c_device_id fm2018_id[] = {
  {FM2018_DRIVER_NAME, 0x60,},
  {}
};

static struct i2c_driver fm2018_driver = {
  .probe = fm2018_probe,
  .remove = fm2018_remove,
  .id_table = fm2018_id,
  .driver = {
	     .name = FM2018_DRIVER_NAME,
	     },
};

static struct fm2018_data
{
  struct i2c_client *client;
  wait_queue_head_t wait;
} fm2018_data;

static const struct file_operations fm2018_fops = {
  .owner = THIS_MODULE,
  .open = fm2018_open,
  .release = fm2018_close,
  .unlocked_ioctl = fm2018_ioctl,
};

static struct miscdevice fm2018_dev = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = FM2018_DRIVER_NAME,
  .fops = &fm2018_fops,
};

static int
i2c_read (struct i2c_client *client, char *buf, int count)
{
  if (count != i2c_master_send (client, buf, count))
    {
      pr_err ("[FM2018] i2c_read --> Send reg. info error\n");
      return -1;
    }

  if (1 != i2c_master_recv (client, buf, 1))
    {
      pr_err ("[FM2018] i2c_read --> get response error\n");
      return -1;
    }
  return 0;
}

static int
i2c_write (struct i2c_client *client, char *buf, int count)
{
  if (count != i2c_master_send (client, buf, count))
    {
      pr_err ("[FM2018] i2c_write --> Send reg. info error count = %d,i2c_master_send = %d\n",count, i2c_master_send (client, buf, count));
      return -1;
    }
  return 0;
}

static int
fm2018_ioctl (struct inode *inode, struct file *file, unsigned int cmd,
	      unsigned long arg)
{
  int err = 0;

  struct i2c_client *client = fm2018_data.client;

  if (_IOC_TYPE (cmd) != FM2018_DEV_IOCTLID)
    {
      pr_err ("[FM2018] cmd magic type error\n");
      return -ENOTTY;
    }
  if (_IOC_NR (cmd) > IOC_MAXNR)
    {
      pr_err ("cmd number error\n");
      return -ENOTTY;
    }
  if (_IOC_DIR (cmd) & _IOC_READ)
    err = !access_ok (VERIFY_WRITE, (void __user *) arg, _IOC_SIZE (cmd));
  else if (_IOC_DIR (cmd) & _IOC_WRITE)
    err = !access_ok (VERIFY_READ, (void __user *) arg, _IOC_SIZE (cmd));
  if (err)
    {
      pr_err ("[FM2018] cmd access_ok error\n");
      return -EFAULT;
    }
  if (client == NULL)
    {
      pr_err ("[FM2018] I2C driver not install (fm2018_ioctl)\n");
      return -EFAULT;
    }

  switch (cmd)
    {
    case IOCTL_GET_FM_PD_STATUS:
	  printk(KERN_ERR "[FM2018] IOCTL_GET_FM_PD_STATUS procedure!!!!!\n");
      return fm2018_set_procedure (0);;

    case IOCTL_SET_FM_PD_DEFAULT:
      printk(KERN_ERR "[FM2018] IOCTL_SET_FM_PD_DEFAULT procedure!!!!!\n");
      return fm2018_set_procedure (1);

    default:
      printk(KERN_ERR "[FM2018]IOCTL: Command not found!\n");
      return -1;
    }
}

static int
fm2018_read (int regH, int regL)
{
  uint8_t fm_wBuf[5];
  uint8_t fm_rBuf[4];
  int dataH, dataL, dataA;
  struct i2c_client *client = fm2018_data.client;

  fm_wBuf[0] = 0xFC;
  fm_wBuf[1] = 0xF3;
  fm_wBuf[2] = 0x37;
  fm_wBuf[3] = regH;
  fm_wBuf[4] = regL;
  i2c_write (client, fm_wBuf, 5);
  msleep (1);

  /* Get high byte */
  fm_rBuf[0] = 0xfc;
  fm_rBuf[1] = 0xf3;
  fm_rBuf[2] = 0x60;
  fm_rBuf[3] = 0x26;
  i2c_read (client, fm_rBuf, 4);
  dataH = fm_rBuf[0];

  /* Get low byte */
  fm_rBuf[0] = 0xfc;
  fm_rBuf[1] = 0xf3;
  fm_rBuf[2] = 0x60;
  fm_rBuf[3] = 0x25;
  i2c_read (client, fm_rBuf, 4);
  dataL = fm_rBuf[0];

  dataA = dataH;
  dataA = dataA << 8;
  dataA = dataA | dataL;

  return dataA;
}

static int
fm2018_write (int regH, int regL, int dataH, int dataL)
{
  uint8_t fm_wBuf[7];
  struct i2c_client *client = fm2018_data.client;

  fm_wBuf[0] = 0xFC;
  fm_wBuf[1] = 0xF3;
  fm_wBuf[2] = 0x3B;
  fm_wBuf[3] = regH;
  fm_wBuf[4] = regL;
  fm_wBuf[5] = dataH;
  fm_wBuf[6] = dataL;
  i2c_write (client, fm_wBuf, 7);
  msleep (1);

  return 0;
}

static int
fm2018_open (struct inode *inode, struct file *file)
{
  pr_debug ("[FM2018] has been opened\n");
  return 0;
}

static int
fm2018_close (struct inode *inode, struct file *file)
{
  pr_debug ("[FM2018] has been closed\n");
  return 0;
}

int
fm2018_set_procedure (int commad)
{
  uint16_t fmdata;

  switch (commad)
    {
    case GET_FM_PD_STATUS:
      fmdata = fm2018_read (0x1E, 0x3A);

      return fmdata;

    case SET_FM_PD_DEFAULT:
      fm2018_write (0x1E, 0x30, 0x02, 0x31);
      fm2018_write (0x1E, 0x34, 0x00, 0x6B);
      fm2018_write (0x1E, 0x3D, 0x02, 0x00);
      fm2018_write (0x1E, 0x3E, 0x01, 0x01);
      fm2018_write (0x1E, 0x41, 0x01, 0x01);
      fm2018_write (0x1E, 0x44, 0x00, 0x81);
      fm2018_write (0x1E, 0x45, 0x03, 0xCF);
      fm2018_write (0x1E, 0x46, 0x00, 0x11);
      fm2018_write (0x1E, 0x47, 0x21, 0x00);
      fm2018_write (0x1E, 0x48, 0x10, 0x00);
      fm2018_write (0x1E, 0x49, 0x08, 0x80);
      fm2018_write (0x1E, 0x4D, 0x03, 0x00);
      fm2018_write (0x1E, 0x52, 0x00, 0x13);
      fm2018_write (0x1E, 0x60, 0x00, 0x01);
      fm2018_write (0x1E, 0x63, 0x00, 0x03);
      fm2018_write (0x1E, 0x70, 0x05, 0xC0);
      fm2018_write (0x1E, 0x86, 0x00, 0x07);
      fm2018_write (0x1E, 0x87, 0x00, 0x03);
      fm2018_write (0x1E, 0x88, 0x38, 0x00);
      fm2018_write (0x1E, 0x89, 0x00, 0x01);
      fm2018_write (0x1E, 0x8B, 0x00, 0x80);
      fm2018_write (0x1E, 0x8C, 0x00, 0x10);
      fm2018_write (0x1E, 0x92, 0x78, 0x00);
      fm2018_write (0x1E, 0xA0, 0x10, 0x00);
      fm2018_write (0x1E, 0xA1, 0x33, 0x00);
      fm2018_write (0x1E, 0xA2, 0x32, 0x00);
      fm2018_write (0x1E, 0xBC, 0x68, 0x00);
      fm2018_write (0x1E, 0xBD, 0x01, 0x00);
      fm2018_write (0x1E, 0xBF, 0x70, 0x00);
      fm2018_write (0x1E, 0xC0, 0x26, 0x80);
      fm2018_write (0x1E, 0xC1, 0x10, 0x80);
      fm2018_write (0x1E, 0xC5, 0x2B, 0x06);
      fm2018_write (0x1E, 0xC6, 0x0C, 0x1F);
      fm2018_write (0x1E, 0xC8, 0x28, 0x79);
      fm2018_write (0x1E, 0xC9, 0x65, 0xAB);
      fm2018_write (0x1E, 0xCA, 0x40, 0x26);
      fm2018_write (0x1E, 0xCB, 0x7F, 0xFF);
      fm2018_write (0x1E, 0xCC, 0x7F, 0xFE);
      fm2018_write (0x1E, 0xF8, 0x04, 0x00);
      fm2018_write (0x1E, 0xF9, 0x01, 0x00);
      fm2018_write (0x1E, 0xFF, 0x4B, 0x00);
      fm2018_write (0x1F, 0x00, 0x7F, 0xFF);
      fm2018_write (0x1F, 0x0A, 0x0A, 0x00);
      fm2018_write (0x1F, 0x0C, 0x01, 0x00);
      fm2018_write (0x1F, 0x0D, 0x78, 0x00);
      fm2018_write (0x1E, 0x3A, 0x00, 0x00);

      return 0;

    case SET_FM_PD_SW_BYPASS:
      fm2018_write (0x1E, 0x70, 0x05, 0xC0);	//1E70 05C0
      fm2018_write (0x1E, 0x34, 0x00, 0x22);	//1E34 0022
      fm2018_write (0x1E, 0x3D, 0x01, 0x00);	//1E3D 0100
      fm2018_write (0x1E, 0x45, 0x00, 0x2D);	//1E45 002D
      fm2018_write (0x1E, 0xDA, 0x40, 0x00);	//1EDA 4000
      fm2018_write (0x1E, 0xF9, 0x03, 0x00);	//1EF9 0300
      fm2018_write (0x1F, 0x00, 0x2D, 0xC8);	//1F00 2DC8
      fm2018_write (0x1F, 0x01, 0x2F, 0x00);	//1F01 2F00
      fm2018_write (0x1F, 0x0C, 0x03, 0x80);	//1F0C 0380
      fm2018_write (0x1E, 0xFF, 0x28, 0x00);	//1EFF 2800
      fm2018_write (0x1E, 0xA0, 0x20, 0x00);	//1EA1 2000
      fm2018_write (0x1E, 0xA2, 0x20, 0x00);	//1EA2 2000
      fm2018_write (0x1E, 0x44, 0x28, 0x89);	//1E44 2889
      fm2018_write (0x1E, 0x4F, 0x00, 0x10);	//1E4F 0010
      fm2018_write (0x1E, 0x51, 0xC0, 0x00);	/* no need to reload parameter */
      fm2018_write (0x1E, 0x3A, 0x00, 0x00);
      return 0;

    case SET_FM_PD_ENV1:
      /*
         #ENVIRONMENT
         MicSet 18
         Mic0InLevel 0
         Mic1InLevel 0
         Mic0EchoLevel 0
         Mic1EchoLevel 0
         LineOutLevel 900
         LineInLevel 400
         VoltagDivider 1
         MicDiff 2.00
         MicNominalDiff 0.00
         TuningStep NULL
       */

      fm2018_write (0x1E, 0x34, 0x00, 0xC9);	//1E34 00C9
      fm2018_write (0x1E, 0x45, 0x00, 0x1E);	//1E45 001E
      fm2018_write (0x1E, 0x47, 0x19, 0x00);	//1E47 1900
      fm2018_write (0x1E, 0x52, 0x00, 0x13);	//1E52 0013
      fm2018_write (0x1E, 0x58, 0x00, 0x13);	//1E58 0013
      fm2018_write (0x1E, 0x60, 0x00, 0x00);	//1E60 0000
      fm2018_write (0x1E, 0x70, 0x05, 0xC0);	//1E70 05C0
      fm2018_write (0x1E, 0xA1, 0x21, 0x9A);	//1EA1 219A
      fm2018_write (0x1F, 0x00, 0x2A, 0x62);	//1F00 2A62
      fm2018_write (0x1F, 0x01, 0x28, 0x00);	//1F01 2800
      fm2018_write (0x1F, 0x0D, 0x0F, 0x00);	//1F0D 0F00
      fm2018_write (0x1F, 0x10, 0x7F, 0xFF);	//1F10 7FFF
      fm2018_write (0x1F, 0x11, 0x60, 0x50);	//1F11 6050
      fm2018_write (0x1F, 0x12, 0x4C, 0x9F);	//1F12 4C9F
      fm2018_write (0x1F, 0x13, 0x3F, 0x82);	//1F13 3F82
      fm2018_write (0x1F, 0x14, 0x36, 0x3E);	//1F14 363E
      fm2018_write (0x1F, 0x15, 0x2E, 0xB2);	//1F15 2EB2
      fm2018_write (0x1F, 0x16, 0x27, 0x11);	//1F16 2711
      fm2018_write (0x1F, 0x17, 0x1E, 0xF8);	//1F17 1EF8
      fm2018_write (0x1F, 0x18, 0x17, 0xCB);	//1F18 17CB
      fm2018_write (0x1F, 0x19, 0x12, 0xF0);	//1F19 12F0
      fm2018_write (0x1E, 0x51, 0xC0, 0x00);	/* no need to reload parameter */
      fm2018_write (0x1E, 0x3A, 0x00, 0x00);
      return 0;

    default:
      printk(KERN_ERR "[FM2018]IOCTL: Command not found!\n");
      return -1;
    }
}


static int
fm2018_probe (struct i2c_client *client, const struct i2c_device_id *id)
{
  int rc;
  unsigned int tt;
  int err = 0;
  fm2018_data.client = client;
  printk("[FM2018] Probe!!\n");

/*
  rc = gpio_request (FM2018_RESET, "De-noise MIC");
  if (rc)
    {
      pr_err ("GPIO request for De-noise MIC clock failed!\n");
      return rc;
    }

  gpio_direction_output (FM2018_RESET, 1);
  msleep (130);
  gpio_set_value (FM2018_RESET, 0);
  msleep (330);
  gpio_set_value (FM2018_RESET, 1);
  msleep (130);
*/
  if (!i2c_check_functionality (client->adapter, I2C_FUNC_I2C))

    {
      pr_err ("[FM2018] i2c_check_functionality error!\n");
      return -ENOTSUPP;
    }

  strlcpy (client->name, FM2018_DRIVER_NAME, I2C_NAME_SIZE);
  i2c_set_clientdata (client, &fm2018_data);

  init_waitqueue_head (&fm2018_data.wait);

  err = misc_register (&fm2018_dev);
  if (err)
    {
      pr_err ("fm2018_probe: fm2018_dev register failed\n");
      goto error_fm2018_dev;
    }

  fm2018_set_procedure (SET_FM_PD_DEFAULT);
  
  printk(KERN_ERR "[FM2018] probe done\n");
  return 0;

error_fm2018_dev:
  pr_err ("[FM2018] probe error\n");
  return err;
}

static int
fm2018_remove (struct i2c_client *client)
{
	printk(KERN_ERR "%s[%d] do nothing\n",__func__,__LINE__);
  return 0;
}

static int fm2018_init (void)
{
  int res = 0;
  res = i2c_add_driver (&fm2018_driver);
  if (res)
    {
      pr_err ("[FM2018]i2c_add_driver failed! \n");
      return res;
    }
  printk("[FM2018] fm2018-380 device init ok!\n");
  return 0;
}

static void __exit fm2018_exit (void)
{
  i2c_del_driver (&fm2018_driver);
}

module_init (fm2018_init);
module_exit (fm2018_exit);
MODULE_LICENSE ("Dual BSD/GPL");
MODULE_AUTHOR ("Andyl Liu <Andyl_Liu@acer.com.tw>");
MODULE_DESCRIPTION ("FM2018-380 driver");
