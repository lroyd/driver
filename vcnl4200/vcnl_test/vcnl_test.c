/*******************************************************************************
	> File Name: test for vcnl4200
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#include <stdio.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <sys/epoll.h> 

#define TEST_MODE_EPOLL	(1)

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
int main(int argc ,char *argv[])
{  
    int fd, ret;  
    unsigned short distance = 0;  

 
    fd = open("/dev/vcnl4200",O_RDWR);  
    if (fd < 0)  
    {  
        printf("open error\n");  
		return 0;  
    }  
	
	/* 靠近或离开均触发中断，读到的值 大于 阈值 表示离开，小于 阈值表示接近 */
	//ioctl(fd, 0, 0x000A); //阈值0x000A 大概0.8m左右
	ioctl(fd, 0, 0x0030);	//测试使用
	

	int epoll_fd, nfd, ev_count;
 	struct epoll_event ev;
	struct epoll_event events[8];	

	epoll_fd = epoll_create(8);
	
	ev.data.fd = fd;
	ev.events = (EPOLLIN);
	
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		printf("fd epoll_ctl fail");
		return -1;
	}

    while(1)  
    {  
#if TEST_MODE_EPOLL  
		if((ev_count = epoll_wait(epoll_fd, events, 8, 5000)) == -1)
		{
			printf("epoll_wait error");
			continue;
		}
		
		for(nfd = 0; nfd < ev_count; nfd++) 
		{
			if(events[nfd].events & EPOLLIN) 
			{
				if(fd == events[nfd].data.fd) 
				{
					read(fd,&distance,1);  
					printf("distance = [0x%04x]\n",distance);  
				}
			} 
			
		}	
		printf("epoll time out\r\n");
#else	
		read(fd,&distance,1);  
		printf("distance = [0x%04x]\n",distance); 		
		usleep(500*1000);
#endif		
    }  
    return 0;  
}  