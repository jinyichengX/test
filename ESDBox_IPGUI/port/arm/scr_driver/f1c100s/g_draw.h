

#ifndef __G_DRAW_H

#define __G_DRAW_H


	#ifdef __cplusplus
	extern "C" {
	#endif






	// 指令类型枚举，唯一标识绘图操作
	typedef enum
	{
	    // 全局样式控制指令


	    CMD_SET_CLIP_RECT     ,
		CMD_SET_CLIP_REGION   ,
	    CMD_CLEAR_CLIP        ,

	    CMD_CLEAR_SCREEN      ,





		CMD_SET_PEN,
		CMD_SET_BRUSH,
		CMD_SET_FONT,
		CMD_SET_CANVAS,
		CMD_SET_REGION,

		CMD_SET_RECT,




	    // 基础几何图元指令
	    CMD_DRAW_POINT        ,
	    CMD_DRAW_LINE         ,
	    CMD_DRAW_RECT         ,
	    CMD_DRAW_ROUND_RECT   ,
	    CMD_DRAW_CIRCLE       ,
	    CMD_DRAW_ARC          ,
	    CMD_DRAW_SECTOR       ,
		CMD_DRAW_RING         ,
	    CMD_DRAW_ELLIPSE      ,
	    CMD_DRAW_PIE          ,

	    // 曲线、多边形
	    CMD_DRAW_BEZIER       ,
	    CMD_DRAW_POLYGON      ,

	    // 文字绘制分层指令
	    CMD_DRAW_CHAR         ,
	    CMD_DRAW_STRING       ,
	    CMD_DRAW_TEXT         ,

	    // 图像贴图
	    CMD_PASTE_IMG         ,
	    CMD_PASTE_IMG_ALPHA   ,



	} DrawCmdType;

	// 统一绘图消息载体，通过消息传给绘图线程
	#define DRAW_CMD_PARAM_MAX      1
	typedef struct
	{
		uint16_t cmd_id;               // 指令编号
	    int16_t param_len;                // 有效参数个数
	    int32_t params[DRAW_CMD_PARAM_MAX];// 通用参数缓存：坐标、半径、颜色、角度等
	 //   const uint8_t* img_buf;           // 贴图专用：图像缓冲区指针
	 //   uint16_t poly_point_num;          // 多边形专用：顶点数量
	} DrawCmdMsg;






	typedef struct tag_Cmd_PEN{


		uint16_t        style;                //样式
		uint16_t        width;

		uint32_t        color;


	}Cmd_PEN;

	typedef struct tag_Cmd_BRUSH{


		uint16_t        style;                //brush样式
		uint16_t        hstyle;               //hatch样式

		uint32_t        color;


	}Cmd_BRUSH;



/*


	extern void Cmd_DrawPoint();

	extern void Cmd_DrawLine();
	extern void Cmd_DrawRect();
	extern void Cmd_DrawRoundRect();

	extern void Cmd_DrawCircle();
	extern void Cmd_DrawEllipse();

	extern void Cmd_DrawArc();
	extern void Cmd_DrawSector();//extern void Cmd_DrawPie();//
	extern void Cmd_DrawRing();


//	extern void Cmd_DrawChar();
//	extern void Cmd_DrawString();

	extern void Cmd_DrawText();

	extern void Cmd_PasteImage();
	extern void Cmd_PasteImageAlpha();



	extern void Cmd_DrawBezier();

	extern void Cmd_DrawPolygon();



*/

	typedef struct
	{
		//Cmd_DrawPoint绘图协议：               x,y,color,alpha

		int16_t       x;
		int16_t       y;
		uint32_t      color;
		uint8_t       alpha;

	}DrawPoint_t;

	typedef struct
	{
		//Cmd_DrawLine绘图协议：                  x,y,x1,y1,color,alpha,pen_width,style(round/square/butt)

		int16_t       x;
		int16_t       y;
		int16_t       x1;
		int16_t       y1;
		uint32_t      color;
		uint8_t       alpha;
		uint8_t       width;
		uint8_t       style;

	}DrawLine_t;

	typedef struct
	{
		//Cmd_DrawRect绘图协议：                  x,y,w,h,color,alpha,pen_width,brush_color

		int16_t       x;
		int16_t       y;
		int16_t       w;
		int16_t       h;
		uint32_t      color;      //画笔颜色
		uint32_t      f_color;   //填充颜色
		uint8_t       alpha;
		uint8_t       width;


	}DrawRect_t;
	typedef struct
	{
		//Cmd_DrawRoundRect绘图协议：    x,y,w,h,r,color,alpha,pen_width,brush_color
		int16_t       x;
		int16_t       y;
		int16_t       w;
		int16_t       h;
		uint32_t      color;
		uint32_t      f_color;
		int16_t       r;
		uint8_t       alpha;
		uint8_t       width;


	}DrawRoundRect_t;
	typedef struct
	{
		//Cmd_DrawCircle绘图协议：            x,y,r,color,alpha,pen_width,brush_color

		int16_t       x;
		int16_t       y;
		uint32_t      color;
		uint32_t      f_color;
		int16_t       r;
		uint8_t       alpha;
		uint8_t       width;

	}DrawCircle_t;
	typedef struct
	{
		//Cmd_DrawEllipse绘图协议：         x,y,a,b,color,alpha,pen_width,brush_color

		int16_t       x;
		int16_t       y;
		int16_t       a;
		int16_t       b;
		uint32_t      color;
		uint32_t      f_color;
		uint8_t       alpha;
		uint8_t       width;

	}DrawEllipse_t;
	typedef struct
	{
		//Cmd_DrawArc绘图协议：                    x,y,r,color,alpha,pen_width,start,end,style(round/square/butt)

		int16_t       x;
		int16_t       y;
		uint32_t      color;
		int16_t       r;
		int16_t       start;
		int16_t       end;
		uint8_t       alpha;
		uint8_t       width;
		uint8_t       style;

	}DrawArc_t;
	typedef struct
	{
		//Cmd_DrawSector绘图协议：            x,y,r,color,alpha,pen_width,start,end,brush_color

		int16_t       x;
		int16_t       y;
		uint32_t      color;
		uint32_t      f_color;
		int16_t       r;
		int16_t       start;
		int16_t       end;
		uint8_t       alpha;
		uint8_t       width;

	}DrawSector_t;
	typedef struct
	{
		//Cmd_DrawRing绘图协议：                 x,y,r,r1,color,alpha,color1,start,end,style(round/square/butt)

		int16_t       x;
		int16_t       y;
		int16_t       r;
		int16_t       r1;
		uint32_t      color;
		uint32_t      color1;     //背景
		int16_t       start;
		int16_t       end;
		uint8_t       alpha;
		uint8_t       style;

	}DrawRing_t;


	typedef struct
	{
		void *        buf;   //缓冲区地址
		int16_t       w;
		int16_t       h;
		uint16_t      fmt;   //颜色格式
		uint16_t      depth;  //位深

	}Canvas_t;




	typedef struct
	{

		void *        buf;    //读好的字库数据(含头信息)或要查询的const字库数据头

		int16_t       x;      //显示位置
		int16_t       y;

		char *        str;       //要查询的字库中字符id序列
		int           nCount;    //要显示的字符个数


		uint32_t      color;
		uint32_t      BkColor;
		RECT          rect;         //裁剪
		uint8_t       alpha;
		uint8_t       Align;        //对齐方式
		int8_t        dLine;        //行间距             多行-   单行为0
		//uint8_t       ShowDir:4;      //显示方向
		uint8_t       code;         //编码（gbk，unicode）


		uint8_t       style;        //字体样式
		uint8_t       size;         //字号                    搜索可用字库
		uint8_t       AA;           //是否抗锯齿
		uint8_t       type;         //暂不使用

	//	uint8_t       lib;          //字库
	//	uint8_t       Mono;         //等宽


	}DrawFont_t;




	//需要查表的等宽，只在第一个字符前面放这；
	//不需要查表的非等宽，在每个字符前面放这
	typedef struct
	{
		union{
			char * cbuf;     //要查询的const字库数据        比如ucgui字库

			struct{                                  //人工制作的mono字库信息
				int16_t     rw;   //实际宽度
				int16_t     ls;   //左移量
				};
		};

		int16_t     dw;   //数据宽度
		int16_t     dh;   //数据高度

	}SizeFont_t;

	//放在buf的开头位置，在SizeFont_t的前面
	typedef struct
	{
		int32_t     length;   //总长度
		int32_t     count;    //总个数
		int16_t     x_spac;     //x方向间距
		int16_t     y_spac;     //y方向间距

	}HeaderFont_t;


	typedef struct tag_DataFont * pDataFont;


	typedef void (* cb_Read2Buffer)(pDataFont data,  int offset);
	typedef int (* cb_GetOffset)(uint32_t cd);       //这里要刷新字体高和宽


	typedef char * (* cb_GetPrev)(char * head, char * cur);
	typedef char * (* cb_GetNext)(char * cur);
	typedef uint32_t (* cb_ReadCode)(char * cur);



	typedef struct tag_DataFont
	{

		SizeFont_t  *       f_size;
		char *              data;


		cb_GetNext          GetNext;
		cb_ReadCode         ReadCode;

		cb_GetOffset        GetOffset;
		cb_Read2Buffer      Read2Buffer;

		void *              addr;
		int                 arg;


	}DataFont_t;


	void SelectFont(DrawFont_t * draw, DataFont_t * data);

	//void f_GetOffset(DrawFont_t * draw, DataFont_t * data);
	//void f_GetNext(DrawFont_t * draw, DataFont_t * data);
	//void f_ReadCode(DrawFont_t * draw, DataFont_t * data);
	//void f_Read2Buffer(DrawFont_t * draw, DataFont_t * data);

	void Write1Line2Canvas(DrawFont_t * draw);




	typedef struct
	{
		void *        buf;
		int16_t       w;
		int16_t       h;
		uint16_t      fmt;
		uint16_t      deep;

		int           offset;   //兼容部分缓冲
		int           size;     //像素个数

		int16_t       x;      //显示位置
		int16_t       y;

		RECT          rect;         //裁剪
		char *        mask;
		uint8_t       alpha;        //全局透明度
		uint8_t       Align;        //对齐方式
		uint8_t       stretch;      //伸缩
		uint8_t       Blend;        //混合方式

	}PasteImage_t;



	/*



                      设置画布:Cmd_SetCanvas协议:    buf（w、h），格式和位深（转换）


		Cmd_DrawPoint绘图协议：               x,y,color,alpha
		Cmd_DrawLine绘图协议：                  x,y,x1,y1,color,alpha,pen_width,style(round/square/butt)
		Cmd_DrawRect绘图协议：                  x,y,w,h,color,alpha,pen_width,brush_color
		Cmd_DrawRoundRect绘图协议：    x,y,w,h,r,color,alpha,pen_width,brush_color
		Cmd_DrawCircle绘图协议：            x,y,r,color,alpha,pen_width,brush_color
		Cmd_DrawEllipse绘图协议：         x,y,a,b,color,alpha,pen_width,brush_color
		Cmd_DrawArc绘图协议：                    x,y,r,color,alpha,pen_width,start,end,style(round/square/butt)
		Cmd_DrawSector绘图协议：            x,y,r,color,alpha,pen_width,start,end,brush_color
		Cmd_DrawRing绘图协议：                 x,y,r,r1,color,alpha,color1,start,end,style(round/square/butt)
		Cmd_DrawText绘图协议：                 字体（字库相关），字号（w、h），x，y，编码，横竖向、矩形内，多行，对齐方式，等宽,alpha
		Cmd_PasteImage绘图协议：            buf（w、h），格式和位深（转换），alpha（复制/混合），伸缩，矩形裁剪，mask，控制位
		Cmd_PasteImageAlpha绘图协议： buf（w、h），格式和位深（转换），伸缩，矩形裁剪，mask，控制位
		Cmd_DrawBezier绘图协议：
		Cmd_DrawPolygon绘图协议：          参照Cmd_DrawRect改成多顶点


                       控件：1、每条目颜色；2、选中条目颜色




                       布局协议：         隶属关系， id,type,位置，大小，样式，绘图颜色（画笔-线宽、画刷），字体/字号/字色，条目行高，其他属性



	 */






	
	
	#ifdef __cplusplus
	}
	#endif

#endif





