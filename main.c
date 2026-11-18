#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

PSP_MODULE_INFO("Street Mayhem PSP", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
#define PI 3.14159265f
#define BOT_COUNT 8
#define CITY_COUNT 18

static unsigned int __attribute__((aligned(16))) list[262144];
static SceCtrlData pad;
static int gameState=0, frame=0, selected=0;
static float px=0, pz=0, pyaw=0, speed=0;
static float camX=0, camY=8, camZ=12;
static int score=0, raceTime=0;

typedef struct { float x,z,yaw,speed; int color,mode; } Bot;
typedef struct { float x,z,w,d,h; } Building;
static Bot bots[BOT_COUNT];
static Building city[CITY_COUNT];

typedef struct { float x,y,z; unsigned int color; } Vertex;
static void cube(float x,float y,float z,float sx,float sy,float sz,unsigned int color) {
    ScePspFVector3 v[8]={
      {x-sx,y-sy,z-sz},{x+sx,y-sy,z-sz},{x+sx,y+sy,z-sz},{x-sx,y+sy,z-sz},
      {x-sx,y-sy,z+sz},{x+sx,y-sy,z+sz},{x+sx,y+sy,z+sz},{x-sx,y+sy,z+sz}};
    unsigned short idx[]={0,1,2,0,2,3, 4,6,5,4,7,6, 0,4,5,0,5,1,
      3,2,6,3,6,7, 1,5,6,1,6,2, 0,3,7,0,7,4};
    sceGuColor(color);
    sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_INDEX_16BIT|GU_TRANSFORM_3D,
                    36,idx,v);
}
static void ground(void) {
    sceGuColor(0xff303b32);
    ScePspFVector3 v[]={{-100,-.15f,-100},{100,-.15f,-100},{100,-.15f,100},{-100,-.15f,100}};
    unsigned short i[]={0,1,2,0,2,3};
    sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_INDEX_16BIT|GU_TRANSFORM_3D,6,i,v);
    for(int k=-5;k<=5;k++) {
      cube(k*12,.005f,0,.045f,.01f,100,0xffd8d0a0);
      cube(0,.006f,k*12,100,.01f,.045f,0xffd8d0a0);
    }
}
static void car(float x,float z,float yaw,unsigned int body,int player) {
    ScePspFVector3 p={x,.65f,z}; sceGumPushMatrix();
    sceGumTranslate(&p); sceGumRotateY(yaw);
    cube(0,0,0,1.05f,.38f,1.75f,body);
    cube(0,.42f,-.12f,.72f,.34f,.82f,0xff34485c);
    cube(0,.10f,1.05f,.72f,.13f,.10f,0xff202020);
    cube(0,.10f,-1.05f,.72f,.13f,.10f,0xff202020);
    cube(-.83f,-.18f,.95f,.18f,.27f,.34f,0xff151515);
    cube(.83f,-.18f,.95f,.18f,.27f,.34f,0xff151515);
    cube(-.83f,-.18f,-.95f,.18f,.27f,.34f,0xff151515);
    cube(.83f,-.18f,-.95f,.18f,.27f,.34f,0xff151515);
    sceGumPopMatrix();
}
static void initWorld(void) {
    for(int i=0;i<CITY_COUNT;i++) {
      city[i].x=((i%6)-3)*19.0f+7; city[i].z=((i/6)-1)*24.0f-12;
      city[i].w=4+(i%3)*1.5f; city[i].d=5+(i%4)*1.2f; city[i].h=5+(i%5)*3.0f;
    }
    for(int i=0;i<BOT_COUNT;i++) {
      bots[i].x=(i-4)*5.0f; bots[i].z=-25-(i%3)*10;
      bots[i].yaw=(i%2)?PI:0; bots[i].speed=0.12f+(i%3)*.035f;
      bots[i].color=(i%3==0)?0xffd04b31:((i%3==1)?0xff35a9b8:0xffd2a83d);
      bots[i].mode=i%2;
    }
}
static void update(void) {
    sceCtrlReadBufferPositive(&pad,1);
    if(gameState==0) {
      if(pad.Buttons & (PSP_CTRL_CROSS|PSP_CTRL_START)) { gameState=1; initWorld(); }
      return;
    }
    if(gameState==2) { if(pad.Buttons&PSP_CTRL_CROSS) gameState=0; return; }
    if(pad.Buttons&PSP_CTRL_SELECT) { gameState=2; return; }
    if(pad.Buttons&PSP_CTRL_UP) speed+=.018f;
    if(pad.Buttons&PSP_CTRL_DOWN) speed-=.025f;
    if(speed>0.42f)speed=.42f; if(speed<-.18f)speed=-.18f;
    if(!(pad.Buttons&(PSP_CTRL_UP|PSP_CTRL_DOWN))) speed*=.985f;
    if(pad.Buttons&PSP_CTRL_LEFT) pyaw+=.045f*(.5f+fabsf(speed));
    if(pad.Buttons&PSP_CTRL_RIGHT) pyaw-=.045f*(.5f+fabsf(speed));
    px+=sinf(pyaw)*speed*2.0f; pz+=cosf(pyaw)*speed*2.0f;
    for(int i=0;i<BOT_COUNT;i++) {
      Bot *b=&bots[i];
      float dx=px-b->x, dz=pz-b->z, dist=sqrtf(dx*dx+dz*dz);
      if(b->mode==0 && dist<45) {
        float target=atan2f(dx,dz);
        float diff=target-b->yaw;
        while(diff>PI)diff-=2*PI; while(diff<-PI)diff+=2*PI;
        b->yaw+=fmaxf(-.025f,fminf(.025f,diff));
      } else if((frame+i*31)%180==0) b->yaw+=(i%2?1:-1)*.8f;
      b->x+=sinf(b->yaw)*b->speed*1.8f;
      b->z+=cosf(b->yaw)*b->speed*1.8f;
      if(fabsf(b->x)>90||fabsf(b->z)>90)b->yaw+=PI*.7f;
      if(dist<2.6f) { b->yaw+=.18f; pyaw-=.05f; score+=1; }
    }
    frame++; if(frame%60==0)raceTime++;
}
static void drawTextHint(void) {
    // PSP GU scene is intentionally asset-free; UI text is supplied by the PSP debug console.
}
int main(void) {
    pspDebugScreenInit();
    sceCtrlSetSamplingCycle(0); sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    sceGuInit();
    sceGuStart(GU_DIRECT,list);
    sceGuDrawBuffer(GU_PSM_8888,(void*)0,512);
    sceGuDispBuffer(480,272,(void*)0x44000,512);
    sceGuDepthBuffer((void*)0x110000,512);
    sceGuOffset(2048-240,2048-136); sceGuViewport(2048,2048,480,272);
    sceGuDepthRange(65535,0); sceGuScissor(0,0,480,272); sceGuEnable(GU_SCISSOR_TEST);
    sceGuEnable(GU_DEPTH_TEST); sceGuDepthFunc(GU_LEQUAL);
    sceGuShadeModel(GU_SMOOTH); sceGuEnable(GU_CULL_FACE); sceGuFrontFace(GU_CW);
    sceGuEnable(GU_CLIP_PLANES); sceGuFinish(); sceGuSync(0,0);
    sceDisplayWaitVblankStart(); sceGuDisplay(GU_TRUE);
    while(1) {
      update();
      sceGuStart(GU_DIRECT,list);
      sceGuClearColor(0xff8bb7d5); sceGuClearDepth(0); sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
      sceGumMatrixMode(GU_PROJECTION); sceGumLoadIdentity(); sceGumPerspective(55.0f,16.0f/9.0f,.5f,250.0f);
      sceGumMatrixMode(GU_VIEW); sceGumLoadIdentity();
      float cx=px-sinf(pyaw)*10, cz=pz-cosf(pyaw)*10;
      ScePspFVector3 eye={cx,7.5f,cz}, at={px,0,pz}, up={0,1,0};
      sceGumLookAt(&eye,&at,&up);
      sceGumMatrixMode(GU_MODEL); sceGumLoadIdentity();
      ground();
      for(int i=0;i<CITY_COUNT;i++) {
        Building *b=&city[i];
        cube(b->x,b->h*.5f,b->z,b->w,b->h*.5f,b->d,0xff59616b);
        cube(b->x,b->h+.12f,b->z,b->w*.82f,.12f,b->d*.82f,0xff343c48);
      }
      for(int i=0;i<BOT_COUNT;i++) car(bots[i].x,bots[i].z,bots[i].yaw,bots[i].color,0);
      car(px,pz,pyaw,0xff2674d8,1);
      sceGuFinish(); sceGuSync(0,0); sceDisplayWaitVblankStart(); sceGuSwapBuffers();
      pspDebugScreenSetXY(0,0);
      if(gameState==0) {
        pspDebugScreenSetTextColor(0xffffffff);
        pspDebugScreenPrintf("       STREET MAYHEM - PSP\n\n");
        pspDebugScreenPrintf("  Original open-city driving sandbox\n\n");
        pspDebugScreenPrintf("  X / START : Start game\n");
        pspDebugScreenPrintf("  SELECT    : Pause / menu\n");
        pspDebugScreenPrintf("\n  Built for PSP / PPSSPP\n");
      } else if(gameState==2) {
        pspDebugScreenPrintf("PAUSED\n\nX: return to menu");
      } else {
        pspDebugScreenPrintf("STREET MAYHEM  |  SCORE %d  |  TIME %d",score,raceTime);
        pspDebugScreenSetXY(0,33);
        pspDebugScreenPrintf("D-PAD: steer / accelerate / brake   SELECT: pause");
      }
    }
    sceGuTerm(); sceKernelExitGame(); return 0;
}
