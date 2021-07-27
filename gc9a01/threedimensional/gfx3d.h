// 3D Filled Vector Graphics
// (c) 2019 Pawel A. Hernik

/*
 Implemented features:
 - optimized rendering without local framebuffer, in STM32 case 1 to 32 lines buffer can be used
 - pattern based background
 - 3D starfield
 - no floating point arithmetic
 - no slow trigonometric functions
 - rotations around X and Y axes
 - simple outside screen culling
 - rasterizer working for all convex polygons
 - backface culling
 - visible faces sorting by Z axis
 - support for quads and triangles
 - optimized structures, saved some RAM and flash
 - added models
 - optimized stats displaying
 - fake light shading
*/

#include "gc9a01.h"

// #define swap(a, b) { int t = a; a = b; b = t; }

#define NUM_STARS 150

// 16 for ST7789 or 32 for ST7735
#define NLINES 16
uint16_t frBuf[GC9A01A_Width*NLINES];
int yFr=0;

// ------------------------------------------------
#define MAXSIN 255
const uint8_t sinTab[91] = {
0,4,8,13,17,22,26,31,35,39,44,48,53,57,61,65,70,74,78,83,87,91,95,99,103,107,111,115,119,123,
127,131,135,138,142,146,149,153,156,160,163,167,170,173,177,180,183,186,189,192,195,198,200,203,206,208,211,213,216,218,
220,223,225,227,229,231,232,234,236,238,239,241,242,243,245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
255
};

int fastSin(int i)
{
  while(i<0) i+=360;
  while(i>=360) i-=360;
  if(i<90)  return sinTab[i]; else
  if(i<180) return sinTab[180-i]; else
  if(i<270) return -(sinTab[i-180]); else
            return -(sinTab[360-i]);
}

int fastCos(int i)
{
  return fastSin(i+90);
}

// ------------------------------------------------

#define COL11 RGB565(0,250,250)  // CYAN
#define COL12 RGB565(0,180,180)
#define COL13 RGB565(0,210,210)

#define COL21 RGB565(250,0,250) // MAGENTA
#define COL22 RGB565(180,0,180)
#define COL23 RGB565(210,0,210)

#define COL31 RGB565(250,250,0) // YELLOW
#define COL32 RGB565(180,180,0)
#define COL33 RGB565(210,210,0)

#define COL41 RGB565(250,150,0) // ORANGE
#define COL42 RGB565(180,100,0)
#define COL43 RGB565(210,140,0)

#define COL51 RGB565(0,250,0) // GREEN
#define COL52 RGB565(0,180,0)
#define COL53 RGB565(0,210,0)

#define COL61 RGB565(250,250,250) // GREY
#define COL62 RGB565(180,180,180)
#define COL63 RGB565(210,210,210)

#define DRED     RGB565(150,0,0)
#define DGREEN   RGB565(0,150,0)
#define DBLUE    RGB565(0,0,150)
#define DCYAN    RGB565(0,150,150)
#define DYELLOW  RGB565(150,150,0)
#define DMAGENTA RGB565(150,0,150)

#include "models3d.h"


// ----------------------------------------------- 
// input arrays
int16_t numVerts;
int16_t *verts;
int16_t numPolys;
uint8_t *polys;
uint16_t *polyColors;

#define MAXVERTS 140
#define MAXPOLYS 240

// output arrays
int16_t transVerts[MAXVERTS*3];
int16_t projVerts[MAXVERTS*2];
uint16_t sortedPolys[MAXPOLYS];
uint16_t normZ[MAXPOLYS];
uint8_t color[3];

struct GC9A01_frame frame3D = {{0, 0}, {239, 239}};

int rot0 = 0, rot1 = 0;
int numVisible = 0;
int lightShade = 0;

// simple Amiga like blitter implementation
void rasterize(int x0, int y0, int x1, int y1, int16_t *line) 
{
  if((y0<yFr && y1<yFr) || (y0>=yFr+NLINES && y1>=yFr+NLINES)) return; // exit if line outside rasterized area
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int err2,err = dx-dy;
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  
  while(1)  {
    if(y0>=yFr && y0<yFr+NLINES) {
      if(x0<line[2*(y0-yFr)+0]) line[2*(y0-yFr)+0] = x0>0 ? x0 : 0;
      if(x0>line[2*(y0-yFr)+1]) line[2*(y0-yFr)+1] = x0<GC9A01A_Width ? x0 : GC9A01A_Width-1;
    }

    if(x0==x1 && y0==y1)  return;
    err2 = err+err;
    if(err2 > -dy) { err -= dy; x0 += sx; }
    if(err2 < dx)  { err += dx; y0 += sy; }
  }
}

void drawQuad( int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint16_t c) 
{
  int x,y;
  int16_t line[NLINES*2];
  for(y=0;y<NLINES;y++) { line[2*y+0] = GC9A01A_Width+1; line[2*y+1] = -1; }

  rasterize( x0, y0, x1, y1, line );
  rasterize( x1, y1, x2, y2, line );
  rasterize( x2, y2, x3, y3, line );
  rasterize( x3, y3, x0, y0, line );

  for(y=0;y<NLINES;y++)
    if(line[2*y+1]>line[2*y+0]) for(x=line[2*y+0]; x<=line[2*y+1]; x++) frBuf[GC9A01A_Width*y+x]=c;
    //if(line[2*y+1]>line[2*y]) for(x=line[2*y]; x<=line[2*y+1]; x++) if(c==COL13) frBuf[GC9A01A_Width*y+x]=pat7[((y+yFr)&0x1f)*32 + ((x-line[2*y])&0x1f)]; else frBuf[GC9A01A_Width*y+x]=c;
}

void drawTri( int x0, int y0, int x1, int y1, int x2, int y2, uint16_t c) 
{
  int x,y;
  int16_t line[NLINES*2];
  for(y=0;y<NLINES;y++) { line[2*y+0] = GC9A01A_Width+1; line[2*y+1] = -1; }

  rasterize( x0, y0, x1, y1, line );
  rasterize( x1, y1, x2, y2, line );
  rasterize( x2, y2, x0, y0, line );

  for(y=0;y<NLINES;y++)
    if(line[2*y+1]>line[2*y+0]) for(x=line[2*y+0]; x<=line[2*y+1]; x++) frBuf[GC9A01A_Width*y+x]=c;
}

void cullQuads(int16_t *v) 
{
  // backface culling
  numVisible=0;
  int x1,y1,x2,y2,z;
  for(int i=0;i<numPolys;i++) {
    if(bfCull) {
      x1 = v[3*polys[4*i+0]+0]-v[3*polys[4*i+1]+0];
      y1 = v[3*polys[4*i+0]+1]-v[3*polys[4*i+1]+1];
      x2 = v[3*polys[4*i+2]+0]-v[3*polys[4*i+1]+0];
      y2 = v[3*polys[4*i+2]+1]-v[3*polys[4*i+1]+1];
      z = x1*y2-y1*x2;
      normZ[i] = z<0? -z : z;
      if((!orient && z<0) || (orient && z>0)) sortedPolys[numVisible++] = i;
    } else sortedPolys[numVisible++] = i;
    //char txt[30];
    //snprintf(txt,30,"%d z=%6d  dr=%2d r0=%d",i,z,sortedQuads[i],rot[0]);
    //Serial.println(txt);
  }
  
  int i,j,zPoly[numVisible];
  // average Z of the polygon
  for(i=0;i<numVisible;++i) {
    zPoly[i] = 0.0;
    for(j=0;j<4;++j) zPoly[i] += v[3*polys[4*sortedPolys[i]+j]+2];
  }

  // sort by Z
  for(i=0;i<numVisible-1;++i) {
    for(j=i;j<numVisible;++j) {
      if(zPoly[i]<zPoly[j]) {
        swap(zPoly[j],zPoly[i]);
        swap(sortedPolys[j],sortedPolys[i]);
      }
    }
  }
}

void cullTris(int16_t *v) 
{
  // backface culling
  numVisible=0;
  int x1,y1,x2,y2,z;
  for(int i=0;i<numPolys;i++) {
    if(bfCull) {
      x1 = v[3*polys[3*i+0]+0]-v[3*polys[3*i+1]+0];
      y1 = v[3*polys[3*i+0]+1]-v[3*polys[3*i+1]+1];
      x2 = v[3*polys[3*i+2]+0]-v[3*polys[3*i+1]+0];
      y2 = v[3*polys[3*i+2]+1]-v[3*polys[3*i+1]+1];
      z = x1*y2-y1*x2;
      normZ[i] = z<0? -z : z;
      if((!orient && z<0) || (orient && z>0)) sortedPolys[numVisible++] = i;
    } else sortedPolys[numVisible++] = i;
  }

  int i,j,zPoly[numVisible];
  // average Z of the polygon
  for(i=0;i<numVisible;++i) {
    zPoly[i] = 0.0;
    for(j=0;j<3;++j) zPoly[i] += v[3*polys[3*sortedPolys[i]+j]+2];
  }

  // sort by Z
  for(i=0;i<numVisible-1;++i) {
    for(j=i;j<numVisible;++j) {
      if(zPoly[i]<zPoly[j]) {
        swap(zPoly[j],zPoly[i]);
        swap(sortedPolys[j],sortedPolys[i]);
      }
    }
  }
}

void drawQuads(int16_t *v2d) 
{
  int q,v0,v1,v2,v3,c,i;
  for(i=0;i<numVisible;i++) {
    q = sortedPolys[i];
    v0 = polys[4*q+0];
    v1 = polys[4*q+1];
    v2 = polys[4*q+2];
    v3 = polys[4*q+3];
    if(lightShade>0) {
      c = normZ[q]*255/lightShade;
      if(c>255) c=255;
      drawQuad(v2d[2*v0+0],v2d[2*v0+1],  v2d[2*v1+0],v2d[2*v1+1], v2d[2*v2+0],v2d[2*v2+1], v2d[2*v3+0],v2d[2*v3+1], RGB565(c,c,c/2));
    } else
      drawQuad(v2d[2*v0+0],v2d[2*v0+1],  v2d[2*v1+0],v2d[2*v1+1], v2d[2*v2+0],v2d[2*v2+1], v2d[2*v3+0],v2d[2*v3+1], polyColors[q]);
  }
} 

void drawTris(int16_t *v2d) 
{
  int q,v0,v1,v2,c,i; //v3 variable unused
  for(i=0;i<numVisible;i++) {
    q = sortedPolys[i];
    v0 = polys[3*q+0];
    v1 = polys[3*q+1];
    v2 = polys[3*q+2];
    if(lightShade>0) {
      c = normZ[q]*255/18000;
      if(c>255) c=255;
      drawTri(v2d[2*v0+0],v2d[2*v0+1],  v2d[2*v1+0],v2d[2*v1+1], v2d[2*v2+0],v2d[2*v2+1], RGB565(c,c,c/2));
    } else
      drawTri(v2d[2*v0+0],v2d[2*v0+1],  v2d[2*v1+0],v2d[2*v1+1], v2d[2*v2+0],v2d[2*v2+1], polyColors[q]);
  }
} 

// ------------------------------------------


void backgroundStars(void) {
  int i;
  for(i=0; i<NLINES*240; i++) frBuf[i] = BLACK;
}


int t=0;

// mode=0 for quads, mode=1 for tris
void render3D(int mode) {
  int cos0,sin0,cos1,sin1;
  int i,x0,y0,z0,fac,distToObj;
  int camZ = 200;
  int scaleFactor = GC9A01A_Height/4;
  int near = 300;

  if(t++>360) t-=360;
  distToObj = 150 + 300*fastSin(3*t)/MAXSIN;
  cos0 = fastCos(rot0);
  sin0 = fastSin(rot0);
  cos1 = fastCos(rot1);
  sin1 = fastSin(rot1);

  for(i=0;i<numVerts;i++) {
    x0 = verts[3*i+0];
    y0 = verts[3*i+1];
    z0 = verts[3*i+2];
    transVerts[3*i+0] = (cos0*x0 + sin0*z0)/MAXSIN;
    transVerts[3*i+1] = (cos1*y0 + (cos0*sin1*z0-sin0*sin1*x0)/MAXSIN)/MAXSIN;
    transVerts[3*i+2] = camZ + ((cos0*cos1*z0-sin0*cos1*x0)/MAXSIN - sin1*y0)/MAXSIN;

    fac = scaleFactor * near / (transVerts[3*i+2]+near+distToObj);

    projVerts[2*i+0] = (100*GC9A01A_Width/2 + fac*transVerts[3*i+0] + 100/2)/100;
    projVerts[2*i+1] = (100*GC9A01A_Height/2 + fac*transVerts[3*i+1] + 100/2)/100;
  }

  // if(bgMode==3) updateStars();
  mode ? cullTris(transVerts) : cullQuads(transVerts);

  frame3D.start.X = 0;
  frame3D.end.X = 240;
  frame3D.start.Y = GC9A01A_Width - 1;
  frame3D.end.Y = yFr + NLINES - 1;
  GC9A01_set_frame(frame3D);

  for(i=0;i<GC9A01A_Height-1;i+=NLINES-1) {
    yFr = i;
    backgroundStars();
    mode ? drawTris(projVerts) : drawQuads(projVerts);
    frame3D.start.X = 0;
    frame3D.end.X = GC9A01A_Width-1;
    frame3D.start.Y = yFr;
    frame3D.end.Y = yFr+NLINES-1;
    GC9A01_set_frame(frame3D);

    for (size_t j = 0; j < GC9A01A_Width*NLINES; j++) {
      color[2] = (uint8_t)((frBuf[j] & 0x1F) << 3);   // blue
      color[1] = (uint8_t)((frBuf[j] & 0x7E0) >> 3);  // green
      color[0] = (uint8_t)((frBuf[j] & 0xF800) >> 8); // red
      if (j == 0) {
        GC9A01_write(color, sizeof(color));
      }
      else {
        GC9A01_write_continue(color, sizeof(color));
      }
    }

    
  }

  rot0 += 2;
  rot1 += 4;
  if(rot0>360) rot0-=360;
  if(rot1>360) rot1-=360;
} 

