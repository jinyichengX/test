/*
 * 全控件渲染演示 v2
 * 全部使用框架圆角/线条/三角形 API 绘制，拒绝填矩形
 * 黄金比例布局 (φ = 2.618)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
static FILE *fopen_utf8(const char *path, const char *mode)
{ wchar_t wp[MAX_PATH],wm[16]; MultiByteToWideChar(CP_UTF8,0,path,-1,wp,MAX_PATH);
  MultiByteToWideChar(CP_ACP,0,mode,-1,wm,16); return _wfopen(wp,wm); }
#else
#define fopen_utf8(path,mode) fopen(path,mode)
#endif

#include "ipgui_utils.h"
#include "ipgui_core.h"
#include "ipgui_color.h"
#include "ipgui_prim.h"
#include "ipgui_blend.h"
#include "ipgui_blend_color.h"
#include "ipgui_blend_gradient_color.h"
#include "ipgui_blend_image.h"
#include "ipgui_gradient_color.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_draw_box_shadow.h"
#include "ipgui_draw_image.h"
#include "ipgui_draw_image_api.h"
#include "ipgui_draw_pixel.h"
#include "ipgui_lcd_pix_fmt.h"

void ipgui_printk(char*fmt,...){}
void ipgui_memset(void*s,int c,unsigned int n){memset(s,c,n);}
void*ipgui_memcpy(void*d,const void*s,unsigned long long n){return memcpy(d,s,(size_t)n);}
void*ipgui_mem_alloc_def(unsigned long long s){return calloc(1,(size_t)s);}
void*ipgui_mem_alloc(unsigned long long s,void*p){(void)p;return calloc(1,(size_t)s);}
void ipgui_mem_free_def(void*p){free(p);}
void ipgui_mem_free_pool(void*p,void*pl){(void)pl;free(p);}
void*ipgui_mem_realoc(void*p,unsigned long long s,void*pl){(void)pl;return realloc(p,(size_t)s);}

#define CW 1280
#define CH 1320
#define ODIR "ESDBox_IPGUI/examples/widget_demo"

static int ppm(const ipgui_surf_t*s,const char*path){
 FILE*fp;int x,y,w=s->surf.end.x-s->surf.start.x+1,h=s->surf.end.y-s->surf.start.y+1;
 if(!s||!s->color||!path)return-1;fp=fopen_utf8(path,"wb");if(!fp)return-1;
 fprintf(fp,"P6\n%d %d\n255\n",w,h);
 for(y=0;y<h;y++){unsigned char*row=s->color+(size_t)y*s->stride;
  for(x=0;x<w;x++){unsigned char rgb[3],*px=row+(size_t)x*s->pix_size;
   rgb[0]=px[1];rgb[1]=px[2];rgb[2]=px[3];fwrite(rgb,1,3,fp);}}fclose(fp);return 0;}

static void clr(ipgui_surf_t*s,u32_t v){ipgui_color_t c;IPGUI_COLOR_SET(c,255,v);
 ipgui_blend_color(s,0,&s->surf,c,255,0,0,IPGUI_BLEND_NORMAL);}

static void rad(ipgui_box_style_t*s,int r){*s=(ipgui_box_style_t){0};
 s->left_top_radius=s->right_top_radius=s->left_bottom_radius=s->right_bottom_radius=r;}

static void rad_tl(ipgui_box_style_t*s,int tl,int tr,int bl,int br){*s=(ipgui_box_style_t){0};
 s->left_top_radius=tl;s->right_top_radius=tr;s->left_bottom_radius=bl;s->right_bottom_radius=br;}

static void bg(ipgui_box_bg_style_t*b,u32_t v){b->paint.type=IPGUI_PAINT_COLOR;
 IPGUI_COLOR_SET(b->paint.src.color,255,v);b->opacity=255;b->blend_mode=IPGUI_BLEND_NORMAL;}

static void bgg(ipgui_box_bg_style_t*b,ipgui_grad_src_t*g){b->paint.type=IPGUI_PAINT_GRADIENT;
 b->paint.src.grad_src=*g;b->opacity=255;b->blend_mode=IPGUI_BLEND_NORMAL;}

static void bo(ipgui_box_border_style_t*b,int w,u32_t v){b->paint.type=IPGUI_PAINT_COLOR;
 IPGUI_COLOR_SET(b->paint.src.color,255,v);b->opacity=255;b->width=w;b->blend_mode=IPGUI_BLEND_NORMAL;}

static void sha(ipgui_box_shadow_style_t*s,u32_t v,u8_t a,int bl,int sp,int ox,int oy){
 IPGUI_COLOR_SET(s->color,255,v);s->opacity=a;s->blur=bl;s->spread=sp;s->offset_x=ox;s->offset_y=oy;}

static void grad_ver(ipgui_grad_src_t*g,ipgui_color_t t,ipgui_color_t b,int x1,int y1,int x2,int y2){
 ipgui_aabb_t a={{x1,y1},{x2,y2}};g->grad_type=IPGUI_GRADIENT_TYPE_LINEAR;
 ipgui_liner_gradient_init(&g->grad.liner_grad,0.0f,0.0f,0.0f,1.0f);
 ipgui_gradient_color_stop_t s0={.color=t,.pos=0},s1={.color=b,.pos=255};
 ipgui_liner_gradient_add_stop(&g->grad.liner_grad,&s0);
 ipgui_liner_gradient_add_stop(&g->grad.liner_grad,&s1);
 ipgui_liner_gradient_apply_to_aabb(&g->grad.liner_grad,&a);}

static void grad_hor(ipgui_grad_src_t*g,ipgui_color_t l,ipgui_color_t r,int x1,int y1,int x2,int y2){
 ipgui_aabb_t a={{x1,y1},{x2,y2}};g->grad_type=IPGUI_GRADIENT_TYPE_LINEAR;
 ipgui_liner_gradient_init(&g->grad.liner_grad,0.0f,0.0f,1.0f,0.0f);
 ipgui_gradient_color_stop_t s0={.color=l,.pos=0},s1={.color=r,.pos=255};
 ipgui_liner_gradient_add_stop(&g->grad.liner_grad,&s0);
 ipgui_liner_gradient_add_stop(&g->grad.liner_grad,&s1);
 ipgui_liner_gradient_apply_to_aabb(&g->grad.liner_grad,&a);}

/* ---- 便捷绘制 ---- */
static void box_draw(ipgui_surf_t*sf,int x,int y,int w,int h,ipgui_box_style_t*sh,
 ipgui_box_bg_style_t*bgsty,ipgui_box_border_style_t*bos,ipgui_box_shadow_style_t*sd){
 ipgui_aabb_t bx={{x,y},{x+w-1,y+h-1}};
 if(sd)ipgui_draw_box_shadow(sf,0,&bx,sh,sd);
 if(bgsty)ipgui_draw_box_background(sf,0,&bx,sh,bgsty);
 if(bos)ipgui_draw_box_border(sf,0,&bx,sh,bos);}

static void chkdraw(ipgui_surf_t*sf,int x,int y,int w,int h,u8_t ck,u32_t co){
 ipgui_box_style_t s;ipgui_box_bg_style_t b;ipgui_box_border_style_t bo2;
 rad(&s,4);bg(&b,ck?co:0xFFFFFF);bo(&bo2,2,ck?0x3070EF:0xBBBBBB);
 box_draw(sf,x,y,w,h,&s,&b,&bo2,0);
 if(ck){
  int sw=IPGUI_MAX(2,w/9);
  for(int i=0;i<sw+3;i++){
   int dy=i*sw/3;
   int x1=x+w/4+i-dy/2, y1=y+h/2+dy-i/2;
   ipgui_aabb_t rr={{x1,y1},{x1+1,y1+1}};rad(&s,1);bg(&b,0xFFFFFF);
   ipgui_draw_box_background(sf,0,&rr,&s,&b);
   int x2=x+w/2+i+dy/2, y2=y+h*3/4-dy+i/2;
   ipgui_aabb_t r2={{x2,y2},{x2+1,y2+1}};rad(&s,1);bg(&b,0xFFFFFF);
   ipgui_draw_box_background(sf,0,&r2,&s,&b);
   int x3=x+w/2+i, y3=y+h/2+i/2;
   ipgui_aabb_t r3={{x3,y3},{x3+1,y3+1}};rad(&s,1);bg(&b,0xFFFFFF);
   ipgui_draw_box_background(sf,0,&r3,&s,&b);}
}}

static void radiodraw(ipgui_surf_t*sf,int x,int y,int w,int h,u8_t sel,u32_t co){
 ipgui_box_style_t s;ipgui_box_bg_style_t b;ipgui_box_border_style_t bo2;
 rad(&s,w/2);bg(&b,0xFFFFFF);bo(&bo2,2,sel?co:0xBBBBBB);
 box_draw(sf,x,y,w,h,&s,&b,&bo2,0);
 if(sel){int c=w/2,r2=w/4;ipgui_box_style_t ds;ipgui_box_bg_style_t db;
  rad(&ds,r2);bg(&db,co);
  ipgui_aabb_t dot={{x+c-r2,y+c-r2},{x+c+r2-1,y+c+r2-1}};
  ipgui_draw_box_background(sf,0,&dot,&ds,&db);}
}

static void circle_dot(ipgui_surf_t*sf,int cx,int cy,int r,u32_t co){
 ipgui_box_style_t s;rad(&s,r);ipgui_box_bg_style_t b;bg(&b,co);
 ipgui_aabb_t d={{cx-r,cy-r},{cx+r-1,cy+r-1}};
 ipgui_draw_box_background(sf,0,&d,&s,&b);}

/* 环形进度 — 用一圈小圆点近似 */
static void ring_progress(ipgui_surf_t*sf,int cx,int cy,int r,int lw,int pc,u32_t co,u32_t tc){
 int n=r*2,deg=(pc*360+50)/100,i;
 ipgui_box_style_t s;ipgui_box_bg_style_t b;
 for(i=0;i<360;i+=IPGUI_MAX(2,360/n)){
  float ang=-90+i*(3.1415926f/180);
  int tx=(int)(cx+cosf(ang)*r),ty=(int)(cy+sinf(ang)*r);
  ipgui_aabb_t d={{tx-lw/2,ty-lw/2},{tx+lw/2-1,ty+lw/2-1}};
  rad(&s,lw/2);bg(&b,i<=deg?co:tc);
  ipgui_draw_box_background(sf,0,&d,&s,&b);}
}

static void arrow_dn(ipgui_surf_t*sf,int ox,int oy,int sz,u32_t co){
 ipgui_box_style_t s;ipgui_box_bg_style_t b;bg(&b,co);
 int h=sz*2/3;if(h<3)h=3;
 for(int row=0;row<h;row++){
  int rw=sz-(row*sz/h);if(rw<2)rw=2;
  int lx=ox+(sz-rw)/2;
  ipgui_aabb_t rr={{lx,oy+row},{lx+rw-1,oy+row}};
  rad(&s,1);ipgui_draw_box_background(sf,0,&rr,&s,&b);}
}

static ipgui_image_data_t*mkimg(void){
 static ipgui_image_data_t m;static u8_t d[64*64*4];m.w=64;m.h=64;m.stride=256;m.pixmap=d;
 m.fmt=IPGUI_IMG_FMT_ARGB8888;m.px_size=4;
 for(int y=0;y<64;y++){u8_t*rw=d+y*256;
  for(int x=0;x<64;x++){u8_t R=(u8_t)(x*4),G=(u8_t)(y*4),B=(u8_t)((x^y)&16?128:200);
   rw[x*4+0]=B;rw[x*4+1]=G;rw[x*4+2]=R;rw[x*4+3]=255;}}return&m;}

int main(void){
 u32_t ps=4,st=CW*ps;
 u8_t*buf=(u8_t*)calloc(1,(size_t)CH*st);if(!buf)return 1;
 ipgui_surf_t sf={.surf={{0,0},{CW-1,CH-1}},.color=buf,.stride=st,.pix_fmt=PIX_FMT_ARGB8888,.pix_size=ps};
 clr(&sf,0xF0F2F5);
 ipgui_image_data_t*img=mkimg();

 ipgui_box_style_t sh;ipgui_box_bg_style_t b1,b2;ipgui_box_border_style_t bo1,bo2;
 ipgui_box_shadow_style_t sd;int x,y,W,H;

/* ============================================================
 * ROW 1 : Button / Switch / Checkbox / Radio / Dropdown
 * ============================================================ */
 x=40;y=30;

 /* -- Button (gradient blue + shadow) -- */
 W=180;H=44;
 {ipgui_grad_src_t g;ipgui_color_t t={64,128,255,255},b={32,96,221,255};
  rad(&sh,8);grad_ver(&g,t,b,x,y,x+W-1,y+H-1);bgg(&b1,&g);bo(&bo1,1,0x3070EF);
  sha(&sd,0x4080FF,50,6,0,0,5);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);}

 /* -- Switch ON -- */
 x=40;y=100;W=52;H=28;
 {rad(&sh,H/2);bg(&b1,0x34C759);box_draw(&sf,x,y,W,H,&sh,&b1,0,0);
  ipgui_box_style_t ks;rad(&ks,12);bg(&b1,0xFFFFFF);
  ipgui_aabb_t kn={{x+W-H+2,y+2},{x+W-4,y+H-3}};
  ipgui_draw_box_background(&sf,0,&kn,&ks,&b1);}

 /* -- Switch OFF -- */
 x=40;y=148;W=52;H=28;
 {rad(&sh,H/2);bg(&b1,0xCCCCCC);box_draw(&sf,x,y,W,H,&sh,&b1,0,0);
  ipgui_box_style_t ks;rad(&ks,12);bg(&b1,0xFFFFFF);
  ipgui_aabb_t kn={{x+2,y+2},{x+H-4,y+H-3}};
  ipgui_draw_box_background(&sf,0,&kn,&ks,&b1);}

 /* -- Checkbox unchecked + checked -- */
 x=40;y=205;W=26;H=26;chkdraw(&sf,x,y,W,H,0,0);
 x=100;chkdraw(&sf,x,y,W,H,1,0x4080FF);

 /* -- Radio unselected + selected -- */
 x=40;y=255;W=26;H=26;radiodraw(&sf,x,y,W,H,0,0x4080FF);
 x=100;radiodraw(&sf,x,y,W,H,1,0x4080FF);

 /* -- Dropdown -- */
 x=40;y=310;W=200;H=36;
 {rad(&sh,6);bg(&b1,0xFFFFFF);bo(&bo1,1,0xCCCCCC);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);
  arrow_dn(&sf,x+W-22,y+H/2-5,10,0x999999);}

/* ============================================================
 * ROW 2 : Slider / ProgressBar / TextBox
 * ============================================================ */
 x=40;y=390;

 /* -- Slider -- */
 W=280;H=32;
 {int th=8,ty=y+(H-th)/2;rad(&sh,th/2);
  ipgui_aabb_t trk={{x,ty},{x+W-1,ty+th-1}};bg(&b1,0xE8E8E8);
  ipgui_draw_box_background(&sf,0,&trk,&sh,&b1);
  ipgui_aabb_t fl={{x,ty},{x+W*35/100-1,ty+th-1}};bg(&b1,0x4080FF);
  ipgui_draw_box_background(&sf,0,&fl,&sh,&b1);
  ipgui_box_style_t ks;rad(&ks,H/2);ipgui_box_bg_style_t kb;bg(&kb,0xFFFFFF);
  ipgui_box_border_style_t kbo;bo(&kbo,2,0x4080FF);
  int kx=W*35/100-H/2;ipgui_aabb_t kn={{x+kx,y},{x+kx+H-1,y+H-1}};
  ipgui_draw_box_background(&sf,0,&kn,&ks,&kb);
  ipgui_draw_box_border(&sf,0,&kn,&ks,&kbo);}

 /* -- ProgressBar 40% -- */
 x=40;y=450;W=280;H=26;
 {rad(&sh,H/2);bg(&b1,0xEBEBEB);bo(&bo1,1,0xD5D5D5);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);
  ipgui_aabb_t f={{x,y},{x+W*40/100-1,y+H-1}};bg(&b1,0x4080FF);
  ipgui_draw_box_background(&sf,0,&f,&sh,&b1);}

 /* -- ProgressBar 80% -- */
 x=40;y=500;W=280;H=26;
 {rad(&sh,H/2);bg(&b1,0xEBEBEB);bo(&bo1,1,0xD5D5D5);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);
  ipgui_aabb_t f={{x,y},{x+W*80/100-1,y+H-1}};bg(&b1,0x34C759);
  ipgui_draw_box_background(&sf,0,&f,&sh,&b1);}

 /* -- ProgressBar 100% -- */
 x=40;y=550;W=280;H=26;
 {rad(&sh,H/2);bg(&b1,0xEBEBEB);bo(&bo1,1,0xD5D5D5);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);
  ipgui_aabb_t f={{x,y},{x+W-1,y+H-1}};bg(&b1,0xFF3B30);
  ipgui_draw_box_background(&sf,0,&f,&sh,&b1);}

 /* -- TextBox (placeholder) -- */
 x=40;y=610;W=280;H=42;
 {rad(&sh,8);bg(&b1,0xFFFFFF);bo(&bo1,1,0xD0D0D0);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);
  ipgui_aabb_t ph={{x+12,y+H/2-3},{x+W-80,y+H/2+3}};
  rad(&sh,2);bg(&b1,0xCCCCCC);b1.opacity=100;
  ipgui_draw_box_background(&sf,0,&ph,&sh,&b1);}

 /* -- TextBox with text -- */
 x=40;y=675;W=280;H=42;
 {rad(&sh,8);bg(&b1,0xFFFFFF);bo(&bo1,1,0xD0D0D0);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);
  ipgui_aabb_t tb={{x+12,y+H/2-4},{x+W-90,y+H/2+4}};
  rad(&sh,2);bg(&b1,0x333333);b1.opacity=160;
  ipgui_draw_box_background(&sf,0,&tb,&sh,&b1);}

 /* ============================================================
  * COLUMN 2 : Panel / ListBox
  * ============================================================ */
 x=370;y=30;

 /* -- Panel -- */
 W=290;H=130;
 {rad(&sh,8);bg(&b1,0xF5F5F5);bo(&bo1,1,0xE0E0E0);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);
  ipgui_box_style_t ts;rad_tl(&ts,7,7,0,0);
  bg(&b2,0xE8E8E8);
  ipgui_aabb_t tb={{x+1,y+1},{x+W-2,y+32}};
  ipgui_draw_box_background(&sf,0,&tb,&ts,&b2);}

 /* -- Panel with shadow -- */
 x=370;y=190;W=290;H=130;
 {rad(&sh,8);bg(&b1,0xF5F5F5);bo(&bo1,1,0xE0E0E0);
  sha(&sd,0x000000,35,8,0,2,6);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);
  ipgui_box_style_t ts;rad_tl(&ts,7,7,0,0);
  bg(&b2,0xE8E8E8);
  ipgui_aabb_t tb={{x+1,y+1},{x+W-2,y+32}};
  ipgui_draw_box_background(&sf,0,&tb,&ts,&b2);}

 /* -- ListBox -- */
 x=370;y=350;W=290;H=240;
 {rad(&sh,8);bg(&b1,0xFFFFFF);bo(&bo1,1,0xD5D5D5);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);
  for(int i=0;i<8;i++){int iy=y+4+i*28;
   if(i==3){bg(&b2,0x4080FF);b2.paint.src.color.a=30;
    ipgui_aabb_t ib={{x+3,iy},{x+W-4,iy+26}};rad(&sh,4);
    ipgui_draw_box_background(&sf,0,&ib,&sh,&b2);}}}

 /* -- Image STRETCH -- */
 x=370;y=620;W=140;H=105;
 {ipgui_image_draw_style_t ds;ds.opacity=255;ds.blend_mode=IPGUI_BLEND_NORMAL;
  ipgui_aabb_t bx={{x,y},{x+W-1,y+H-1}};
  rad(&sh,8);bo(&bo1,1,0xDDDDDD);
  ipgui_draw_box_border(&sf,0,&bx,&sh,&bo1);
  ipgui_draw_image_in_rect(&sf,img,&bx,IPGUI_IMG_ALIGN_CENTER,IPGUI_IMG_FIT_STRETCH,&ds);}

 /* -- Image FIT -- */
 x=370+160;y=620;W=140;H=105;
 {ipgui_image_draw_style_t ds;ds.opacity=255;ds.blend_mode=IPGUI_BLEND_NORMAL;
  ipgui_aabb_t bx={{x,y},{x+W-1,y+H-1}};
  rad(&sh,8);bo(&bo1,1,0xDDDDDD);
  ipgui_draw_box_border(&sf,0,&bx,&sh,&bo1);
  ipgui_draw_image_in_rect(&sf,img,&bx,IPGUI_IMG_ALIGN_CENTER,IPGUI_IMG_FIT_FIT,&ds);}

 /* ============================================================
  * COLUMN 3 : Gradient Buttons
  * ============================================================ */
 x=710;y=30;

 /* -- Grad Ver Blue -- */
 W=250;H=50;
 {rad(&sh,8);ipgui_grad_src_t g;ipgui_color_t t={64,128,255,255},b={32,96,221,255};
  grad_ver(&g,t,b,x,y,x+W-1,y+H-1);bgg(&b1,&g);bo(&bo1,1,0x3070EF);
  sha(&sd,0x4080FF,50,8,0,0,6);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);}

 /* -- Grad Hor Green -- */
 x=710;y=110;W=250;H=50;
 {rad(&sh,8);ipgui_grad_src_t g;ipgui_color_t l={52,199,89,255},r={30,180,60,255};
  grad_hor(&g,l,r,x,y,x+W-1,y+H-1);bgg(&b1,&g);bo(&bo1,1,0x28C840);
  sha(&sd,0x34C759,50,8,0,0,6);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);}

 /* -- Solid Red -- */
 x=710;y=190;W=250;H=50;
 {rad(&sh,8);bg(&b1,0xFF3B30);bo(&bo1,1,0xE0352A);
  sha(&sd,0xFF3B30,50,8,0,0,6);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);}

 /* -- Solid Orange -- */
 x=710;y=270;W=250;H=50;
 {rad(&sh,8);bg(&b1,0xFF9500);bo(&bo1,1,0xE08500);
  sha(&sd,0xFF9500,50,8,0,0,6);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);}

 /* -- Dark XL -- */
 x=710;y=350;W=250;H=60;
 {rad(&sh,10);ipgui_grad_src_t g;ipgui_color_t t={46,49,54,255},b={29,33,41,255};
  grad_ver(&g,t,b,x,y,x+W-1,y+H-1);bgg(&b1,&g);
  bo(&bo1,1,0x1A1D24);sha(&sd,0x000000,80,12,2,0,8);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);}

 /* -- Outline Button -- */
 x=710;y=440;W=250;H=50;
 {rad(&sh,8);bg(&b1,0xFFFFFF);bo(&bo1,2,0x4080FF);
  sha(&sd,0x4080FF,30,6,0,0,4);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);}

 /* -- Purple solid -- */
 x=710;y=520;W=250;H=50;
 {rad(&sh,8);ipgui_grad_src_t g;ipgui_color_t t={175,82,222,255},b={140,50,190,255};
  grad_ver(&g,t,b,x,y,x+W-1,y+H-1);bgg(&b1,&g);
  bo(&bo1,1,0x8C30B0);sha(&sd,0xAF52DE,50,8,0,0,6);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);}

 /* -- Super Round (pill) -- */
 x=710;y=600;W=250;H=50;
 {rad(&sh,H/2);ipgui_grad_src_t g;ipgui_color_t l={0,122,255,255},r={90,200,250,255};
  grad_hor(&g,l,r,x,y,x+W-1,y+H-1);bgg(&b1,&g);
  bo(&bo1,1,0x0070E0);sha(&sd,0x007AFF,50,8,0,0,6);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);}

 /* ============================================================
  * BOTTOM ROW : Status labels
  * ============================================================ */
 x=40;y=750;W=220;H=48;
 {rad(&sh,10);bg(&b1,0xE8F4FD);bo(&bo1,2,0x91CAFF);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);}

 x=280;y=750;W=320;H=48;
 {rad(&sh,10);bg(&b1,0xFFF3E8);bo(&bo1,2,0xFFB37F);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);}

 x=620;y=750;W=200;H=48;
 {rad(&sh,10);bg(&b1,0xE8F8E8);bo(&bo1,2,0x91D891);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);}

 /* ============================================================
  * SECTION 2 : Ten New Widgets
  * ============================================================ */

 /* -- Spinner Ring (60% progress) -- */
 {int cx=120,cy=850,r=36,lw=8;
  ring_progress(&sf,cx,cy,r,lw,60,0x4080FF,0xE0E0E0);}

 /* -- Spinner Ring (90% progress) -- */
 {int cx=240,cy=850,r=36,lw=8;
  ring_progress(&sf,cx,cy,r,lw,90,0x34C759,0xE0E0E0);}

 /* -- Spinner indeterminate (full ring) -- */
 {int cx=360,cy=850,r=36,lw=8;
  ring_progress(&sf,cx,cy,r,lw,100,0xFF9500,0xE0E0E0);}

 /* -- Separator Horizontal -- */
 x=480;y=830;W=200;H=16;
 {ipgui_box_style_t ss={0};ipgui_box_bg_style_t sb;
   sb.paint.type=IPGUI_PAINT_COLOR;sb.paint.src.color.r=0xDD;sb.paint.src.color.g=0xDD;
  sb.paint.src.color.b=0xDD;sb.paint.src.color.a=255;sb.opacity=255;sb.blend_mode=IPGUI_BLEND_NORMAL;
  ipgui_aabb_t rr={{x,y+H/2-1},{x+W-1,y+H/2}};
  ipgui_draw_box_background(&sf,0,&rr,&ss,&sb);}

 /* -- Badge (Dot) -- */
 x=720;y=830;circle_dot(&sf,x,y,10,0xFF3B30);

 /* -- Badge (Ring) -- */
 x=760;y=830;
 {circle_dot(&sf,x,y,12,0xFF3B30);circle_dot(&sf,x,y,8,0xFFFFFF);}

 /* -- Badge (Green online) -- */
 x=800;y=830;circle_dot(&sf,x,y,10,0x34C759);

 /* -- Avatar with border -- */
 x=860;y=810;W=56;H=56;
 {ipgui_box_style_t avs;rad(&avs,W/2);ipgui_box_bg_style_t avb;
  ipgui_box_border_style_t avbo;
  bg(&avb,0xD0D8E8);bo(&avbo,3,0xFFFFFF);
  box_draw(&sf,x,y,W,H,&avs,&avb,&avbo,0);
  /* online dot */
  circle_dot(&sf,x+W-9,y+H-9,7,0x34C759);
  ipgui_box_style_t ds;rad(&ds,7);ipgui_box_border_style_t db;
  bo(&db,2,0xFFFFFF);ipgui_aabb_t dd={{x+W-16,y+H-16},{x+W-2,y+H-2}};
  ipgui_draw_box_border(&sf,0,&dd,&ds,&db);}

 /* -- Avatar offline -- */
 x=940;y=810;W=56;H=56;
 {ipgui_box_style_t avs;rad(&avs,W/2);ipgui_box_bg_style_t avb;
  ipgui_box_border_style_t avbo;
  bg(&avb,0xC8D0E0);bo(&avbo,3,0xFFFFFF);
  box_draw(&sf,x,y,W,H,&avs,&avb,&avbo,0);
  circle_dot(&sf,x+W-9,y+H-9,7,0xAAAAAA);
  ipgui_box_border_style_t db;bo(&db,2,0xFFFFFF);
  ipgui_box_style_t ds2;rad(&ds2,7);
  ipgui_aabb_t dd2={{x+W-16,y+H-16},{x+W-2,y+H-2}};
  ipgui_draw_box_border(&sf,0,&dd2,&ds2,&db);}

 /* -- Color Swatches -- */
 {u32_t cols[]={0xFF3B30,0xFF9500,0xFFCC00,0x34C759,0x007AFF,0x5856D6,0xAF52DE,0x8E8E93};
  for(int i=0;i<8;i++){
   x=60+i*64;y=930;W=48;H=48;
   ipgui_box_style_t sws;rad(&sws,10);ipgui_box_bg_style_t swb;bg(&swb,cols[i]);
   ipgui_box_border_style_t swbo;bo(&swbo,1,0xDDDDDD);
   box_draw(&sf,x,y,W,H,&sws,&swb,&swbo,0);
   if(i==2){ /* selected */
    ipgui_box_border_style_t sel;bo(&sel,3,0x007AFF);
    ipgui_aabb_t sbx={{x-2,y-2},{x+W+1,y+H+1}};
    ipgui_draw_box_border(&sf,0,&sbx,&sws,&sel);}
  }}

 /* -- Segmented Control (3 segments, sel=0) -- */
 x=620;y=920;W=320;H=40;
 {rad(&sh,8);bg(&b1,0xF0F0F0);bo(&bo1,1,0xCCCCCC);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);
  int n=3,sw=W/n;
  /* sel */
  {ipgui_box_style_t ss;rad(&ss,7);bg(&b2,0xFFFFFF);
   ipgui_aabb_t sbx={{x+2,y+2},{x+sw-3,y+H-3}};
   ipgui_draw_box_background(&sf,0,&sbx,&ss,&b2);}
  /* dividers */
  for(int i=1;i<n;i++){
   ipgui_aabb_t dv={{x+i*sw-1,y+H/5},{x+i*sw,y+H*4/5}};
   ipgui_box_style_t ds={0};ipgui_box_bg_style_t db;
   db.paint.type=IPGUI_PAINT_COLOR;
   db.paint.src.color.r=0xCC;db.paint.src.color.g=0xCC;db.paint.src.color.b=0xCC;db.paint.src.color.a=80;
   db.opacity=255;db.blend_mode=IPGUI_BLEND_NORMAL;
   ipgui_draw_box_background(&sf,0,&dv,&ds,&db);}}

 /* -- Rating Stars (3/5) -- */
 x=620;y=980;
 {int sz=22,gap=4;
  for(int i=0;i<5;i++){
   u32_t co=(i<3)?0xFF9500:0xE0E0E0;
   int ox=x+i*(sz+gap),oy=y;
   ipgui_box_bg_style_t rb;rb.paint.type=IPGUI_PAINT_COLOR;
   IPGUI_COLOR_SET(rb.paint.src.color,255,co);rb.opacity=255;rb.blend_mode=IPGUI_BLEND_NORMAL;
   /* star = cross + diagonals */
   int cx2=ox+sz/2,cy2=oy+sz/2,a=sz/2,aw=IPGUI_MAX(2,sz/8);
   ipgui_box_style_t ps;rad(&ps,1);
   ipgui_aabb_t v={{cx2-aw/2,cy2-a},{cx2+aw/2-1,cy2+a-1}};ipgui_draw_box_background(&sf,0,&v,&ps,&rb);
   ipgui_aabb_t h={{cx2-a,cy2-aw/2},{cx2+a-1,cy2+aw/2-1}};ipgui_draw_box_background(&sf,0,&h,&ps,&rb);
   ipgui_aabb_t d1={{cx2-a/2-aw/2,cy2+a*2/5},{cx2+a/2+aw/2-1,cy2-a*2/5}};
   ipgui_draw_box_background(&sf,0,&d1,&ps,&rb);
  }}

 /* -- Rating Dots (4/5) -- */
 x=620;y=1020;
 {int sz=18,gap=6;
  for(int i=0;i<5;i++){
   u32_t co=(i<4)?0x007AFF:0xE0E0E0;
   circle_dot(&sf,x+i*(sz+gap)+sz/2,y+sz/2,sz/2,co);}}

 /* -- Scrollbar Vertical -- */
 x=980;y=810;W=14;H=180;
 {ipgui_box_style_t ss;rad(&ss,W/2);ipgui_box_bg_style_t tb;
  IPGUI_COLOR_SET(tb.paint.src.color,255,0xEEEEEE);tb.paint.type=IPGUI_PAINT_COLOR;
  tb.opacity=255;tb.blend_mode=IPGUI_BLEND_NORMAL;
  ipgui_aabb_t trk={{x,y},{x+W-1,y+H-1}};
  ipgui_draw_box_background(&sf,0,&trk,&ss,&tb);
  /* thumb at 30% */
  int th=H*30/100,ty=y+H*30/100;
  ipgui_box_bg_style_t mb;mb.paint.type=IPGUI_PAINT_COLOR;
  IPGUI_COLOR_SET(mb.paint.src.color,255,0xBBBBBB);mb.opacity=255;mb.blend_mode=IPGUI_BLEND_NORMAL;
  ipgui_aabb_t thb={{x+1,ty},{x+W-2,ty+th-1}};
  ss.left_top_radius=ss.right_top_radius=ss.left_bottom_radius=ss.right_bottom_radius=(W-2)/2;
  ipgui_draw_box_background(&sf,0,&thb,&ss,&mb);}

 /* -- Scrollbar Horizontal -- */
 x=980;y=1030;W=240;H=14;
 {ipgui_box_style_t ss;rad(&ss,H/2);ipgui_box_bg_style_t tb;
  IPGUI_COLOR_SET(tb.paint.src.color,255,0xEEEEEE);tb.paint.type=IPGUI_PAINT_COLOR;
  tb.opacity=255;tb.blend_mode=IPGUI_BLEND_NORMAL;
  ipgui_aabb_t trk={{x,y},{x+W-1,y+H-1}};
  ipgui_draw_box_background(&sf,0,&trk,&ss,&tb);
  int tw=W*35/100,tx=x+W*20/100;
  ipgui_box_bg_style_t mb;mb.paint.type=IPGUI_PAINT_COLOR;
  IPGUI_COLOR_SET(mb.paint.src.color,255,0xBBBBBB);mb.opacity=255;mb.blend_mode=IPGUI_BLEND_NORMAL;
  ipgui_aabb_t thb={{tx,y+1},{tx+tw-1,y+H-2}};
  ss.left_top_radius=ss.right_top_radius=ss.left_bottom_radius=ss.right_bottom_radius=(H-2)/2;
  ipgui_draw_box_background(&sf,0,&thb,&ss,&mb);}

 /* -- Stepper -- */
 x=40;y=920;W=180;H=44;
 {rad(&sh,8);bg(&b1,0xFFFFFF);bo(&bo1,1,0xCCCCCC);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,0);
  /* minus btn */
  int bw=H-4;bg(&b2,0xF5F5F5);
  ipgui_aabb_t mb={{x+2,y+2},{x+bw-1,y+H-3}};ipgui_draw_box_background(&sf,0,&mb,&sh,&b2);
  ipgui_aabb_t mbar={{x+bw/4,y+H/2-1},{x+bw*3/4,y+H/2+1}};
  bg(&b2,0x333333);ipgui_draw_box_background(&sf,0,&mbar,&sh,&b2);
  /* plus btn */
  ipgui_aabb_t pb={{x+W-bw-2,y+2},{x+W-3,y+H-3}};bg(&b2,0xF5F5F5);
  ipgui_draw_box_background(&sf,0,&pb,&sh,&b2);
  ipgui_aabb_t ph={{x+W-bw*3/4,y+H/2-1},{x+W-bw/4,y+H/2+1}};
  ipgui_aabb_t pv={{x+W-bw/2-1,y+H/4},{x+W-bw/2,y+H*3/4}};
  bg(&b2,0x333333);ipgui_draw_box_background(&sf,0,&ph,&sh,&b2);
  ipgui_draw_box_background(&sf,0,&pv,&sh,&b2);}

 /* -- Card -- */
 x=260;y=920;W=240;H=150;
 {rad(&sh,10);bg(&b1,0xFFFFFF);bo(&bo1,1,0xE0E0E0);
  sha(&sd,0x000000,35,8,0,0,4);
  box_draw(&sf,x,y,W,H,&sh,&b1,&bo1,&sd);
  /* image area */
  ipgui_box_style_t is;rad_tl(&is,9,9,0,0);
  bg(&b2,0xF0F0F0);ipgui_aabb_t ia={{x+1,y+1},{x+W-2,y+70}};
  ipgui_draw_box_background(&sf,0,&ia,&is,&b2);
  /* divider */
  ipgui_aabb_t dv={{x,y+70},{x+W-1,y+71}};
  ipgui_box_style_t ds={0};ipgui_box_bg_style_t db;
  db.paint.type=IPGUI_PAINT_COLOR;
  IPGUI_COLOR_SET(db.paint.src.color,255,0xE0E0E0);db.paint.src.color.a=80;
  db.opacity=255;db.blend_mode=IPGUI_BLEND_NORMAL;
  ipgui_draw_box_background(&sf,0,&dv,&ds,&db);
  /* action area placeholder */
  ipgui_aabb_t aa={{x+16,y+90},{x+W/2-10,y+110}};rad(&sh,6);bg(&b2,0x4080FF);
  ipgui_draw_box_background(&sf,0,&aa,&sh,&b2);}

 /* ---- Output ---- */
 const char*fn=ODIR"/widgets.ppm";
 int r=ppm(&sf,fn);printf("%s: %s (%dx%d)\n",r?"FAIL":"OK",fn,CW,CH);
 free(buf);return r;
}
