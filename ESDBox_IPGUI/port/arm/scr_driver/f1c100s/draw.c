


#include "emge.h"
//#include "sgl_draw.h"

#include "g_format.h"



extern QUEUE*  q_cmd;
void pro_draw_msg(DrawCmdMsg *);
static MSG msg_buf[16];              //用于接收拷贝过来的消息



REGION *    preg = 0;
Cmd_PEN     Pen;
Cmd_BRUSH   Brush;
//sgl_surf_t  surf;
Canvas_t    LcdCanvas = {

		    .depth =          BPP_32,                      //默认设置

		    .fmt  =          FORMAT_ARGB8888,
};

Canvas_t    Canvas;

void SetSurface(void)
{

	/*	surf.x = 0;
	surf.y = 0;
	surf.w = Canvas.w;
	surf.h = Canvas.h;
	surf.buffer = Canvas.buf;






	surf.x2 = Canvas.w-1;      //有问题
	surf.y2 = Canvas.h-1;
		surf.size = 1024*600;
	surf.dirty->x1 = 20;
	surf.dirty->y1 = 20;
	surf.dirty->x2 = 100;
	surf.dirty->x1 = 100;*/


}

void SetCanvas(Canvas_t * pc)
{
	Canvas.buf = pc->buf;
	Canvas.depth = pc->depth;
	Canvas.fmt = pc->fmt;
	Canvas.h = pc->h;
	Canvas.w = pc->w;

	SetSurface();
}



void Cmd_DrawRect(DrawRect_t * pdm);
void do_draw(void)    //专用绘图线程
{
	DrawCmdMsg *  pdm;
	MSG *  g;

	DrawRect_t  dr = {             //清屏,顺便测试一下
			.x = 0,
			.y = 0,
			.w = 1024,
			.h = 600,
			.color = 0xff,   //画笔颜色
			.f_color = 0xff0000,     //填充色
			.alpha = 100,
			.width = 16,

	};





	if(0 == hDeskTop) return;




	SetCanvas(&LcdCanvas);           //初始默认LCD为画布


	Cmd_DrawRect(&dr);          //清屏,顺便测试一下

	spl_prints("do_draw test !\r\n", 0);


	while(GetMessage(msg_buf, q_cmd, 0))    //frist uThrd线程
	{
		//spl_prints("desktop!\r\n", 0);

		g = GetMsgPtr(msg_buf);


		//从消息指针g这里读取绘图指令，进行绘图

		pdm = g->pbuf;


		pro_draw_msg(pdm);



	}




//	do_SysCursor();              //系统光标在这里绘制


//	do_timer();                  //WM_TIMER消息从这里post出去


}










void Cmd_DrawPoint(DrawPoint_t * pdm)
{



}


void Cmd_DrawLine(DrawLine_t * pdm)
{
	REGION *pr;
/*	sgl_draw_line_t desc;

	sgl_area_t coord;


	//SetSurface();

	desc.alpha = pdm->alpha;
	desc.color.full = LOGIC_2_RGB565(pdm->color);    //这里颜色要转换
	desc.width = pdm->width;
//	desc.x1 = pdm->x;
//	desc.y1 = pdm->y;
//	desc.x2 = pdm->x1;
//	desc.y2 = pdm->y1;

	coord.x1 = pdm->x;
	coord.y1 = pdm->y;
	coord.x2 = pdm->x1;
	coord.y2 = pdm->y1;
*/

	for(pr = preg; pr; pr = pr->next)
	{
		//sgl_draw_line(&surf, (sgl_area_t*)(&(pr->rect)), &desc);

//		sgl_draw_line(&surf, (sgl_area_t*)(&(pr->rect)), &coord, &desc);
	}

}
void Cmd_DrawRect(DrawRect_t * pdm)
{

	REGION *pr;

/*	sgl_rect_t rect;

	sgl_draw_rect_t desc;


	//SetSurface();

	rect.x1 = pdm->x;
	rect.y1 = pdm->y;
	rect.x2 = pdm->x + pdm->w -1;
	rect.y2 = pdm->y + pdm->h -1;

	desc.border_color.full = LOGIC_2_RGB565(pdm->color);
	desc.color.full = LOGIC_2_RGB565(pdm->f_color);
	desc.alpha = pdm->alpha;
	desc.radius = 0;
	desc.border = pdm->width;
//	desc.border_mask = 0;
	desc.pixmap = 0;
//	desc.border_alpha = desc.alpha;
*/

	for(pr = preg; pr; pr = pr->next)
	{

	//	sgl_draw_rect(&surf, (sgl_area_t*)(&(pr->rect)), &rect, &desc);

	}

}
void Cmd_DrawRoundRect(DrawRoundRect_t * pdm)
{
	REGION *pr;

/*	sgl_rect_t rect;

	sgl_draw_rect_t desc;


	//SetSurface();

	rect.x1 = pdm->x;
	rect.y1 = pdm->y;
	rect.x2 = pdm->x + pdm->w -1;
	rect.y2 = pdm->y + pdm->h -1;

	desc.border_color.full = LOGIC_2_RGB565(pdm->color);
	desc.color.full = LOGIC_2_RGB565(pdm->f_color);
	desc.alpha = pdm->alpha;
	desc.radius = pdm->r;;
	desc.border = pdm->width;
//	desc.border_mask = 0;
	desc.pixmap = 0;
//	desc.border_alpha = desc.alpha;
*/

	for(pr = preg; pr; pr = pr->next)
	{

//		sgl_draw_rect(&surf, (sgl_area_t*)(&(pr->rect)), &rect, &desc);

	}


}


void Cmd_DrawCircle(DrawCircle_t * pdm)
{
	REGION *pr;

/*	sgl_draw_circle_t desc;

	//SetSurface();


	desc.alpha = pdm->alpha;
	desc.border = pdm->width;
	desc.border_color.full = LOGIC_2_RGB565(pdm->color);
	desc.color.full = LOGIC_2_RGB565(pdm->f_color);
	desc.cx = pdm->x;
	desc.cy = pdm->y;
	desc.radius = pdm->r;
	desc.pixmap = 0;
*/

	for(pr = preg; pr; pr = pr->next)
	{

//		sgl_draw_circle(&surf, (sgl_area_t*)(&(pr->rect)), &desc);

	}


}
void Cmd_DrawEllipse(DrawEllipse_t * pdm)
{


}

void Cmd_DrawArc(DrawArc_t * pdm)
{
	REGION *pr;

/*	sgl_draw_arc_t desc;

	//SetSurface();


	desc.alpha = pdm->alpha;
	desc.color.full = LOGIC_2_RGB565(pdm->color);
	desc.cx = pdm->x;
	desc.cy = pdm->y;
	desc.start_angle = pdm->start;
	desc.end_angle = pdm->end;
	desc.radius_in = pdm->r - pdm->width;
	desc.radius_out = pdm->r;
	desc.bg_color.full = 0;               //这里注意
	desc.mode = pdm->style;          //这里注意

*/
	for(pr = preg; pr; pr = pr->next)
	{

//		sgl_draw_fill_arc(&surf, (sgl_area_t*)(&(pr->rect)), &desc);

	}


}
void Cmd_DrawSector(DrawSector_t * pdm)//extern void Cmd_DrawPie();//
{



}
void Cmd_DrawRing(DrawRing_t * pdm)
{
	REGION *pr;

/*	sgl_draw_arc_t desc;

	//SetSurface();


	desc.alpha = pdm->alpha;
	desc.color.full = LOGIC_2_RGB565(pdm->color);
	desc.cx = pdm->x;
	desc.cy = pdm->y;
	desc.start_angle = pdm->start;
	desc.end_angle = pdm->end;
	desc.radius_in = pdm->r;
	desc.radius_out = pdm->r1;
	desc.bg_color.full = LOGIC_2_RGB565(pdm->color1);
	desc.mode = pdm->style;          //这里注意

*/
	for(pr = preg; pr; pr = pr->next)
	{
		//sgl_draw_fill_ring(&surf, (sgl_area_t*)(&(pr->rect)), pdm->x, pdm->y,pdm->r,pdm->r1,pdm->color1,pdm->alpha);

	//	sgl_draw_fill_arc(&surf, (sgl_area_t*)(&(pr->rect)), &desc);

	}



}


void Cmd_DrawText(DrawFont_t * pdm)
{


}

void Cmd_PasteImage(PasteImage_t * pdm)
{


}
void Cmd_PasteImageAlpha(PasteImage_t * pdm)
{


}





void pro_draw_msg(DrawCmdMsg * pdm)
{

	REGION *pr;



	void * pv = pdm->params;;


//	sgl_draw_line_t desc;


	switch(pdm->cmd_id)
	{


		case CMD_SET_PEN:

			break;
		case CMD_SET_BRUSH:

			break;
		case CMD_SET_FONT:



			break;
		case CMD_SET_CANVAS:

			SetCanvas(pv);

			break;
		case CMD_SET_CLIP_REGION:



			m_free(preg);
			preg = pv;


			for(pr = preg; pr; pr = pr->next)   //矩形格式转换
			{
				pr->rect.w += (pr->rect.x - 1);    //转成x2
				pr->rect.h += (pr->rect.y - 1);    //转成y2
			}


			break;


		case CMD_DRAW_POINT:
			break;

		case CMD_DRAW_LINE:


		//	Cmd_DrawLine(pv);     //预计涉及除法,编译不过

			break;
		case CMD_DRAW_RECT:

			Cmd_DrawRect(pv);


			break;

		case CMD_DRAW_ROUND_RECT:
			Cmd_DrawRoundRect(pv);

			break;

		case CMD_DRAW_CIRCLE:
			Cmd_DrawCircle(pv);

			break;
		case CMD_DRAW_ARC:
			Cmd_DrawArc(pv);

			break;
		case CMD_DRAW_SECTOR:
			break;

		case CMD_DRAW_RING:

			Cmd_DrawRing(pv);
			break;
		case CMD_DRAW_ELLIPSE:
			break;
		case CMD_DRAW_TEXT:


			Write1Line2Canvas(pv);



			break;

		case CMD_PASTE_IMG:


			break;

		case CMD_PASTE_IMG_ALPHA:
			break;


		default:
			break;


	}


}


const uint8_t  bit_tab[8] = {1,2,4,8,16,32,64,128};
void drawc16(Canvas_t * pcan,int16_t cx, int16_t cy, uint16_t color, RECT * pr, int16_t bytes, char * font)
{
	uint8_t * mark;
	uint8_t n;
	int16_t i,j;
	uint16_t* p;  //  注意

	for(i = pr->y; pr->h --; i ++)
	{
		mark = font + (i * bytes) + (pr->x >> 3);
		p = (pcan->buf) + (pcan->w * (pr->y+i) + (pr->x + cx)) ;
		n = 8 - (pr->x & 7);
		for(j = pr->w; j--;   )    //逐行打点，高位在前
		{

			n--;
			if((*mark) & (bit_tab[n]))//if((*mark) & (1 << n))  //这样可以直接读取字库
			{
				*p = color;
			}
			p ++;
			if(0 == n) {n = 8;mark ++;}
		}
	}
}




void Write1Line2Canvas(DrawFont_t * pdraw)   //注意对齐问题
{

	HeaderFont_t *   hdr;
	SizeFont_t *     psize;

	int32_t cnt;

	int16_t   w;
	int16_t cx, cy;

	RECT rt;


	char *  pchar;    //类型转换及移位用

	char *  p;    //类型转换及移位用


	hdr = pdraw->buf;    //装的上述数据

	pchar = (void*)(hdr + 1);
	psize = (void*)pchar;

	cx = pdraw->x;  cy = pdraw->y;   //原始开始坐标


	if(0 == pdraw->str)    //已按顺序读到缓冲区方式
	{
		//   HeaderFont_t + SizeFont_t + FontBitMap + SizeFont_t + FontBitMap + SizeFont_t + FontBitMap + ..........

		for(cnt = hdr->count; cnt --;  )
		{


			w = (psize->dw+7) >> 3;    //每行所占bytes数

			rt.x = psize->ls;  rt.y =  0;   rt.w =  psize->rw;  rt.h = psize->dh;    //字体内裁剪

			cx -= psize->ls;


			drawc16(&Canvas,  cx,   cy, pdraw->color, &rt, w, pchar + sizeof(SizeFont_t));  //注意rect与pdraw->rect交集

			cx += (psize->ls + psize->rw);   //右移一个字符位置

			//	cx += hdr->x_spac;          //考虑字间距


			pchar+= (sizeof(SizeFont_t) + (psize->dh* (w)));//,psize = (void*)pchar

			p = (void*)psize;
			*p = *pchar;*(p+1) = *(pchar+1);*(p+2) = *(pchar+2);*(p+3) = *(pchar+3);
			*(p+4) = *(pchar+4);*(p+5) = *(pchar+5);*(p+6) = *(pchar+6);*(p+7) = *(pchar+7);

		}


	}else{    //用id查询方式

		//   HeaderFont_t + SizeFont_t
		//   w + h + FontBitMap +  FontBitMap +  FontBitMap + ..........
		pchar = psize->cbuf;    //先装宽和高,后面是有效数据


		//psize->dw =  *((int16_t*)pchar);  pchar += 2;
		//psize->dh =  *((int16_t*)pchar);  pchar += 2;

		p = (void*)psize;
		*(p+4) = *pchar++;*(p+5) = *pchar++;*(p+6) = *pchar++;*(p+7) = *pchar++;



		w = (psize->dw+7) >> 3;    //每行所占bytes数


		psize->cbuf = (void*)pchar;


		pchar = pdraw->str;     //id序列

		rt.x = 0;  rt.y =  0;   rt.w =  psize->dw;  rt.h = psize->dh;    //字体内裁剪

		for(cnt = hdr->count; cnt --;  pchar ++)
		{


			drawc16(&Canvas,  cx,   cy, pdraw->color, &rt, w, psize->cbuf + ((*pchar) *  (psize->dh* (w))));

			cx += psize->dw;   //右移一个字符位置

		//	cx += hdr->x_spac;          //考虑字间距


		}





	}


}






void FontBlend(int16_t x0, int16_t y0, char * buf, SizeFont_t * info, RECT * pr)
{


	int16_t X,W,Y,H;



	int16_t r = 0, d = 0;


	X = pr->x,Y= pr->y,W = pr->w,H = pr->h;


	//Canvas.

	while(r <= X)
	{
		r += info->rw;       //实际显示宽

		d += info->dw;       //数据宽
		info ++;
	}
	info --;
	r -= info->rw;         d -= info->dw;

	r = X - r;

	//d += (r + info->ls);

	//buf += (d * (info->dh +7) >> 3);




}


















