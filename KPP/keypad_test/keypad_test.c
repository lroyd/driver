#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <linux/input.h>
#include <unistd.h>



int main (int argc, char *argv[])
{
	struct input_event ev;
	int fd, rd;
	int i,size;

	if ((fd = open ("/dev/input/event5", O_RDONLY)) == -1)
	{
		printf ("not a vaild device.\n");
		return -1;
	}
	

	while (1)
	{
		memset((void*)&ev, 0, sizeof(ev));
		rd = read (fd, (void*)&ev, sizeof(ev));

		if (rd <= 0)
		{
			printf ("rd: %d\n", rd);
		}
		
		switch(ev.type)
		{
			case EV_KEY:
				switch(ev.value)
				{
					case 0:
						printf("key release[0x%02x]\r\n", ev.code);
						break;
					
					case 1:
						printf("key press  [0x%02x]\r\n", ev.code);
						break;
					
					case 2:
						printf("key hold   [0x%02x]\r\n", ev.code);
						break;
					
					default: 
						printf("undifined value %d\n", ev.value);
				}				
				break;
			
			default:
				printf("undifined input evt type %d\n", ev.type);
				break;
		}

	}

	return 0;
}