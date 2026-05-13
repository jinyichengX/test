/* 4 bytes convert to float */
float byte2float(char * bytes)								
{								
	float ret;							
	int i;							
	char * b;											
	typedef union							
	{							
        float f;					
        unsigned char byte[4];										
	}un_t;

	un_t un;												
	b = bytes;							
	for(i = 0; i < 4; i ++)							
	{							
		un.byte[i] = *(b + 3 - i);					
	}							
	ret = (float)un.f;
								
	return ret;							
}								

int str_to_int(char * str)
{
	unsigned char flag;
	char * p;
	int ulInt;
	unsigned char i;
	unsigned char tmp;

	p = str;
	if (* p == '-')
	{
		flag = 1;	/* 负数 */
		p ++;
	}
	else
	{
		flag = 0;
	}

	ulInt = 0;
	for (i = 0; i < 15; i ++)
	{
		tmp = * p;
		if (tmp == '.')	/* 遇到小数点，自动跳过1个字节 */
		{
			p ++;
			tmp = * p;
		}
		if ((tmp >= '0') && (tmp <= '9'))
		{
			ulInt = ulInt * 10 + (tmp - '0');
			p ++;
		}
		else
		{
			break;
		}
	}

	if (flag == 1)
	{
		return -ulInt;
	}
	return ulInt;
}

void int_to_str(int number, char * to, unsigned char len)
{
	unsigned char i;
	int tmp;

	if (number < 0)	/* 负数 */
	{
		tmp = -number;	/* 转为正数 */
	}
	else
	{
		tmp = number;
	}

    while(len --)
        * to ++ = ' ';

	/* 将整数转换为ASCII字符串 */
	for (i = 0; i < len; i++)
	{
		to[len - 1 - i] = (tmp % 10) + '0';
		tmp = tmp / 10;
		if (tmp == 0)
		{
			break;
		}
	}
	to[len] = 0;

	if (number < 0)	/* 负数 */
	{
		for (i = 0; i < len; i++)
		{
			if ((to[i] == ' ') && (to[i + 1] != ' '))
			{
				to[i] = '-';
				break;
			}
		}
	}
}

/* bcd to char, example: 0x0A---->A */
char bcd2char(unsigned int bcd)
{
	if (bcd < 10)
	{
		return bcd + '0';
	}
	else if (bcd < 16)
	{
		return bcd + 'A';
	}
	else
	{
		return 0;
	}
}
/*
 * 将二进制数组转换为16进制格式的ASCII字符串。每个2个ASCII字符后保留1个空格。
 * 0x12 0x34 转化为 0x31 0x32 0x20 0x33 0x34 0x00  即 "1234"
 */
void hex2ascii(unsigned char * hex, char * to, int len)
{
	int i;
	
	if (len == 0)
	{
		to[0] = 0;
	}
	else
	{
		for (i = 0; i < len; i++)
		{
			to[3 * i] = bcd2char(hex[i] >> 4);
			to[3 * i + 1] = bcd2char(hex[i] & 0x0f);
			to[3 * i + 2] = ' ';
		}
		to[3 * (i - 1) + 2] = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: ascii2uint32
*	功能说明: 变长的 ASCII 字符转换为32位整数  ASCII 字符以空格或者0结束 。 支持16进制和10进制输入
*	形    参: *pAscii ：要转换的ASCII码
*	返 回 值: 转换得到的整数
*********************************************************************************************************
*/
unsigned int ascii2uint32(char * pAscii)
{
	char i;
	char bTemp;
	char bIsHex;
	char bLen;
	char bZeroLen;
	unsigned int lResult;
	unsigned int lBitValue;

	/* 判断是否是16进制数 */
	bIsHex = 0;
	if ((pAscii[0] == '0') && ((pAscii[1] == 'x') || (pAscii[1] == 'X')))
	{
		bIsHex=1;
	}

	lResult=0;
	// 最大数值为 4294967295, 10位+2字符"0x" //
	if (bIsHex == 0)
	{ // 十进制 //
		// 求长度 //
		lBitValue=1;

		/* 前导去0 */
		for (i = 0; i < 8; i ++)
		{
			bTemp = pAscii[i];
			if (bTemp != '0')
				break;
		}
		bZeroLen = i;

		for (i = 0; i < 10; i ++)
		{
			if ((pAscii[i] < '0') || (pAscii[i] > '9'))
				break;
			lBitValue = lBitValue * 10;
		}
		bLen = i;
		lBitValue = lBitValue / 10;
		if (lBitValue == 0)
			lBitValue=1;
		for (i = bZeroLen; i < bLen; i ++)
		{
			lResult += (pAscii[i] - '0') * lBitValue;
			lBitValue /= 10;
		}
	}
	else
	{	/* 16进制 */
		/* 求长度 */
		lBitValue=1;

		/* 前导去0 */
		for (i = 0; i < 8; i ++)
		{
			bTemp = pAscii[i + 2];
			if(bTemp!='0')
				break;
		}
		bZeroLen = i;
		for (; i < 8; i ++)
		{
			bTemp=pAscii[i+2];
			if (((bTemp >= 'A') && (bTemp <= 'F')) ||
				((bTemp>='a')&&(bTemp<='f')) ||
				((bTemp>='0')&&(bTemp<='9')) )
			{
				lBitValue=lBitValue * 16;
			}
			else
			{
				break;
			}
		}
		lBitValue = lBitValue / 16;
		if (lBitValue == 0)
			lBitValue = 1;
		bLen = i;
		for (i = bZeroLen; i < bLen; i ++)
		{
			bTemp = pAscii[i + 2];
			if ((bTemp >= 'A') && (bTemp <= 'F'))
			{
				bTemp -= 0x37;
			}
			else if ((bTemp >= 'a') && (bTemp <= 'f'))
			{
				bTemp -= 0x57;
			}
			else if ((bTemp >= '0') && (bTemp <= '9'))
			{
				bTemp -= '0';
			}
			lResult += bTemp * lBitValue;
			lBitValue /= 16;
		}
	}
	return lResult;
}


/* crc sum */
char crc_sum(char * buf, int len)
{
    char x = 0;
    while (len--)
    {
        x += *buf ++;
    }
    return x;
}


/* crc xor */
char crc_xor(char * buf, int Len)
{
  int i = 0;
  int x = 0;
  
  for(; i < Len; i++)
  {
    x = x ^ (*(buf + i));
  }
  
  return x;
}

/* 直接插入排序 */
/* 时间复杂度O(n^2),所以适合基本有序数列的排序 */
void insert_sort_demo(int a[], int l)
{
    int temp;
    int j;
    for(int i = 1; i < l; i ++)
    {
        if(a[i] < a[i-1])
        {
            temp = a[i];
            for(j = i - 1; j >= 0 && temp < a[j]; j --)
            {
                a[j + 1] = a[j];
            }
            a[j + 1] = temp;
        }

        /* 每排一次序就列出一次分布结果 */
        // for(int k = 0; k < l; k ++)
        // {
        //     printf("%d ",a[k]);
        // }
        // printf("\n");
    }

    /* print result */
    // printf("Straight Insertion Sort test result:\n");
    // for(int k = 0; k < l; k ++)
    // {
    //     printf("%d ",a[k]);
    // }
}

/* shell sort */
void shell_sort_demo(int arr[], int len)
{
    //增量每次都/2
    for (int step = len / 2; step > 0; step /= 2)
    {
        //从增量那组开始进行插入排序，直至完毕
        for (int i = step; i < len; i ++)
        {
            int j = i;
            int temp = arr[j];

            // j - step 就是代表与它同组隔壁的元素
            while (j - step >= 0 && arr[j - step] > temp) {
                arr[j] = arr[j - step];
                j = j - step;
            }
            arr[j] = temp;
        }
    }
    /* print result */
    // printf("Shell Sort test result:\n");
    // for(int k = 0;k < len; k ++)
    // {
    //     printf("%d ",arr[k]);
    // }
}

/* bubble sort */
void bubble_sort_demo(int arr[], int len)
{
    int i, j, temp;
    for (i = 0; i < len - 1; i ++)
    {
        for(j = 0; j < len - 1 - i; j ++)
        {
            if (arr[j] > arr[j + 1])
            {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
    /* print result */
    // printf("Bubble Sort test result:\n");
    // for(int k = 0; k < len; k ++)
    // {
    //     printf("%d ",arr[k]);
    // }
}

/* 直接选择排序，基本思想是选出所有元素中最小的插到最前面 */
void swap(int * a, int * b)
{
    int temp = * a;
    * a = * b;
    * b = temp;
}

void selection_sort(int arr[], int len)
{
    int i,j;

    for (i = 0 ; i < len - 1 ; i ++)
    {
        int min = i;
        for (j = i + 1; j < len; j ++)     //访问未排序的元素
        if (arr[j] < arr[min])    //找到目前最小值
                min = j;    //记录最小值
        swap(&arr[min], &arr[i]);    //交换
    }
    /* print result */
    // printf("Bubble Sort test result:\n");
    // for(int k = 0; k < len; k ++)
    // {
    //     printf("%d ",arr[k]);
    // }
}