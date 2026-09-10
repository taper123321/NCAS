#include "main.h"
#include "modules.h"
#include "console.h"
#include "textGUI.hpp"
#include <vector>
#include <string>
using namespace ustl;

int c_yshift=0;

void c_draw_rectangle(int x,int y,int w,int h,int c){
  freeze=true;
  y += c_yshift;
  draw_line(x,y,x+w,y,c);
  draw_line(x+w,y,x+w,y+h,c);
  draw_line(x,y+h,x+w,y+h,c);
  draw_line(x,y,x,y+h,c);
}
void c_draw_line(int x0,int y0,int x1,int y1,int c){
  freeze=true;
  y0 += c_yshift;
  y1 += c_yshift;
  draw_line(x0,y0,x1,y1,c);
}
void c_draw_circle(int xc,int yc,int r,int color,bool q1,bool q2,bool q3,bool q4){
  freeze=true;
  yc += c_yshift;
  draw_circle(xc,yc,r,color,q1,q2,q3,q4);
}
void c_draw_filled_circle(int xc,int yc,int r,int color,bool left,bool right){
  freeze=true;
  yc += c_yshift;
  draw_filled_circle(xc,yc,r,color,left,right);
}
void c_convert(int *x,int*y,vector< vector<int> > & v){
  for (int i=0;i<v.size();++i,++x,++y){
    v[i].push_back(*x);
    v[i].push_back(*y+c_yshift);
  }
}
void c_draw_polygon(int * x,int *y ,int n,int color){
  freeze=true;
  vector< vector<int> > v(n);
  c_convert(x,y,v);
  draw_polygon(v,color);
}
void c_draw_filled_polygon(int * x,int *y, int n,int xmin,int xmax,int ymin,int ymax,int color){
  freeze=true;
  vector< vector<int> > v(n);
  c_convert(x,y,v);
  draw_filled_polygon(v,xmin,xmax,ymin,ymax,color);
}
void c_draw_arc(int xc,int yc,int rx,int ry,int color,double theta1, double theta2){
  freeze=true;
  yc += c_yshift;
  draw_arc(xc,yc,rx,ry,color,theta1,theta2);
}
void c_draw_filled_arc(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int color,int xmin,int xmax,int ymin,int ymax,bool segment){
  freeze=true;
  y += c_yshift;
  draw_filled_arc(x,y,rx,ry,theta1_deg,theta2_deg,color,xmin,xmax,ymin,ymax,segment);
}
void c_set_pixel(int x,int y,int c){
  freeze=true;
  y += c_yshift;
  os_set_pixel(x,y,c);
}
void c_fill_rect(int x,int y,int w,int h,int c){
  y += c_yshift;
  freeze=true;
  if (w<0){
    w=-w;
    x -= w; 
  }
  if (h<0){
    h=-h;
    y -= h; 
  }
  if (x<0){ w+=x; x=0;}
  if (y<0){ h+=y; y=0;}
  drawRectangle(x,y,w,h,c);
}

int c_draw_string(int x,int y,int c,int bg,const char * s,bool fake){
  freeze=true;
  y += c_yshift;
  Printxy(x,y,s,c==_BLACK?0:1); // os_draw_string(x,y,c,bg,s,fake);
  return x+6*strlen(s);
}
int c_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake){
  freeze=true;
  y += c_yshift;
  if (c==0xffff){
    int r=bg>>11,g=(bg>>5)&0x3f,b=bg&0x1f;
    int minic=(r<8?1:0)*4+(g<16?1:0)*2+(b<8?1:0);
    PrintMiniMini(&x,&y,s,4,minic,0);
  }
  else {
    int r=c>>11,g=(c>>5)&0x3f,b=c&0x1f;
    int minic=(r>=8?1:0)*4+(g>=16?1:0)*2+(b>=8?1:0);
    PrintMiniMini(&x,&y,s,0,minic,0);
  }
  return x+4*strlen(s); // os_draw_string_small(x,y,c,bg,s,fake);
}
int c_draw_string_medium(int x,int y,int c,int bg,const char * s,bool fake){
  freeze=true;
  y += c_yshift;
  Printxy(x,y,s,c==_BLACK?0:1);
  return x+6*strlen(s);// os_draw_string_medium(x,y,c,bg,s,fake);
}

const char * caseval(const char * s){
  return s;
}

void c_sprint_double(char * s,double d){
  sprint_double(s,d);
}

/* TURTLE */
double deg2rad_d=M_PI/180;

void turtle_freeze(){
  freezeturtle=true;
}

logo_turtle * turtleptr=0;
  
logo_turtle & turtle(){
  if (!turtleptr)
    turtleptr=new logo_turtle;
  return * turtleptr;
}

vector<logo_turtle> & turtle_stack(){
  static vector<logo_turtle> * ans = 0;
  if (!ans){
    // initialize from python app storage
    ans=new vector<logo_turtle>(1,(*turtleptr));
    
  }
  return *ans;
}

vector<string> * ecrisptr=0;
vector<string> & ecristab(){
  if (!ecrisptr)
    ecrisptr=new vector<string>;
  return * ecrisptr;
}

static void c_turtle_move(int r,int theta2){
  double theta0;
  if ((*turtleptr).direct)
    theta0=(*turtleptr).theta-90;
  else {
    theta0=(*turtleptr).theta+90;
    theta2=-theta2;
  }
  (*turtleptr).x += r*(cos(M_PI/180*(theta2+theta0))-cos(M_PI/180*theta0));
  (*turtleptr).y += r*(sin(M_PI/180*(theta2+theta0))-sin(M_PI/180*theta0));
  (*turtleptr).theta = (*turtleptr).theta+theta2 ;
  if ((*turtleptr).theta<0)
    (*turtleptr).theta += 360;
  if ((*turtleptr).theta>360)
    (*turtleptr).theta -= 360;
}

static void c_update_turtle_state(bool clrstring){
#if defined NUMWORKS && defined DEVICE
  if (!ck_turtle_size()){
    ctrl_c=true; interrupted=true;
    return;
  }
#endif
  if (clrstring)
    (*turtleptr).s=-1;
  (*turtleptr).theta = (*turtleptr).theta - floor((*turtleptr).theta/360)*360;
  if (!turtle_stack().empty()){
    logo_turtle & t=turtle_stack().back();
    if (t.equal_except_nomark(*turtleptr)){
      t.theta=turtleptr->theta;
      t.mark=turtleptr->mark;
      t.visible=turtleptr->visible;
      t.color=turtleptr->color;
    }
    else
      turtle_stack().push_back((*turtleptr));
  }
  else
    turtle_stack().push_back((*turtleptr));    
}

void c_turtle_clear(int clrpos){
  turtle_stack().clear();
  if (clrpos) (*turtleptr) = logo_turtle();
  c_update_turtle_state(true);
}

void c_turtle_forward(double d){
  (*turtleptr).x += d * cos((*turtleptr).theta*deg2rad_d);
  (*turtleptr).y += d * sin((*turtleptr).theta*deg2rad_d) ;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

void c_turtle_left(double d){
  (*turtleptr).theta += d;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

void c_turtle_up(int i){
  if (i)
    (*turtleptr).mark = false;
  else
    (*turtleptr).mark = true;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

void c_turtle_goto(double x,double y){
  (*turtleptr).x=x;
  (*turtleptr).y=y;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

void c_turtle_cap(double x){
  (*turtleptr).theta=x;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

int c_turtle_getcap(){
  return (*turtleptr).theta;
}

int c_turtle_crayon(int i){
  if (i==-128)
    return (*turtleptr).turtle_length;
  if (i<0)
    (*turtleptr).turtle_length=-i;
  else
    (*turtleptr).color=i;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
  return 0;
}

int c_find_radius(int & r,int & t1,int & t2,int &direct){
  direct=r>=0;
  if (r<0) r=-r;
  if (r>512) r=512;
  return r | (t1 << 9) | (t2 << 18 );
}

void c_turtle_rond(int r,int t1,int t2){
  int direct;
  int radius=c_find_radius(r,t1,t2,direct);
  (*turtleptr).radius=radius;
  (*turtleptr).direct=direct;
  while (t1<0)
    t1 += 360;
  while (t2<0)
    t2 += 360;
  c_turtle_move(r,t2);
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

void c_turtle_disque(int r,int t1,int t2,int centre){
  int direct,radius=c_find_radius(r,t1,t2,direct);
  if (centre){
    // saute(r); tourne_gauche(direct?90:-90)
  }
  (*turtleptr).radius=radius;
  (*turtleptr).direct=direct;
  c_turtle_move(r,t2);
  (*turtleptr).radius += 1 << 27;
  c_update_turtle_state(true);
  if (centre){
    // _tourne_droite(direct?90:-90,contextptr); _saute(-r,contextptr);
  }
  py_ck_ctrl_c();
}
int turtle_fillbegin=-1,turtle_fillcolor=_BLACK;


void c_turtle_fill(int i){
  if (i==1){
    turtle_fillbegin=turtle_stack().size();
    return;
  }
  int c=turtleptr->color;
  c_turtle_crayon(turtle_fillcolor);
  int n=turtle_stack().size()- turtle_fillbegin;
  turtle_fillbegin=-1;
  turtleptr->radius=-absint(n);
  c_update_turtle_state(true);
  if (turtle_fillcolor>=0){
    turtleptr->radius=0;
    c_turtle_crayon(c);
  }
  py_ck_ctrl_c();
}

int rgb(int r,int g,int b){
  if (r<0) r=0; if(r>255) r=255;
  if (g<0) g=0; if(g>255) g=255;
  if (b<0) b=0; if(b>255) b=255;
  return (((r*32)/256)<<11) | (((g*64)/256)<<5) | (b*32/256);

}
void c_turtle_fillcolor(double r,double g,double b,int entier){
  if (entier)
    turtle_fillcolor=rgb(int(r),int(g),int(b));
  else
    turtle_fillcolor=rgb(int(r*256),int(g*256),int(b*256));
  py_ck_ctrl_c();
}

void c_turtle_getposition(double * x,double * y){
  *x=turtleptr->x;
  *y=turtleptr->y;
}

void c_turtle_show(int visible){
  (*turtleptr).visible=visible;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);
}

void c_turtle_towards(double x,double y){
  double x0=turtleptr->x,y0=turtleptr->y;
  double t=atan2(x-x0,y-y0);
  c_turtle_cap(t*180/M_PI);
}

int c_turtle_getcolor(){
  return turtleptr->color;
}

void c_turtle_color(int c){
  turtleptr->color=c;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);  
}

void c_turtle_fillcolor1(int c){
  turtle_fillcolor=c;
}

void displaylogo(){
  Turtle t={&turtle_stack(),0,0,1,1};
  freeze=true; // avoid clearscreen
  while (1){
    int save_ymin=clip_ymin;
    clip_ymin=0;
    t.draw();
    clip_ymin=save_ymin;
    unsigned int key;
    ck_getkey((int*)&key);
    if (key==KEY_CTRL_EXIT || key==KEY_CTRL_AC || key==KEY_CTRL_MENU || key==KEY_CTRL_EXE || key==KEY_CTRL_VARS)
      break;
    if (key==KEY_CTRL_UP){ t.turtley += 10; }
    if (key==KEY_CTRL_PAGEUP) { t.turtley += 100; }
    if (key==KEY_CTRL_DOWN) { t.turtley -= 10; }
    if (key==KEY_CTRL_PAGEDOWN) { t.turtley -= 100;}
    if (key==KEY_CTRL_LEFT) { t.turtlex -= 10; }
    if (key==KEY_CTRL_RIGHT) { t.turtlex += 10; }
    if (key==KEY_CHAR_PLUS) { t.turtlezoom *= 2;}
    if (key==KEY_CHAR_MINUS){ t.turtlezoom /= 2;  }
  }
  freeze=false; 
}

void fl_arc(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int c=_BLACK){
  rx/=2;
  ry/=2;
  // *logptr(contextptr) << "theta " << theta1_deg << " " << theta2_deg << endl;
  if (ry==rx){
    if (theta2_deg-theta1_deg==360){
      draw_circle(x+rx,y+rx,rx,c);
      return;
    }
    if (theta1_deg==0 && theta2_deg==180){
      draw_circle(x+rx,y+rx,rx,c,true,true,false,false);
      return;
    }
    if (theta1_deg==180 && theta2_deg==360){
      draw_circle(x+rx,y+rx,rx,c,false,false,true,true);
      return;
    }
  }
  // *logptr(contextptr) << "draw_arc" << theta1_deg*M_PI/180. << " " << theta2_deg*M_PI/180. << endl;
  draw_arc(x+rx,y+ry,rx,ry,c,theta1_deg*M_PI/180.,theta2_deg*M_PI/180.);
}

void fl_pie(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int c=_BLACK,bool segment=false){
  //cout << "fl_pie " << theta1_deg << " " << theta2_deg << " " << c << endl;
  if (!segment && ry==rx){
    if (theta2_deg-theta1_deg>=360){
      rx/=2;
      draw_filled_circle(x+rx,y+rx,rx,c);
      return;
    }
    if (theta1_deg==-90 && theta2_deg==90){
      rx/=2;
      draw_filled_circle(x+rx,y+rx,rx,c,false,true);
      return;
    }
    if (theta1_deg==90 && theta2_deg==270){
      rx/=2;
      draw_filled_circle(x+rx,y+rx,rx,c,true,false);
      return;
    }
  }
  // approximation by a filled polygon
  // points: (x,y), (x+rx*cos(theta)/2,y+ry*sin(theta)/2) theta=theta1..theta2
  while (theta2_deg<theta1_deg)
    theta2_deg+=360;
  if (theta2_deg-theta1_deg>=360){
    theta1_deg=0;
    theta2_deg=360;
  }
  int N0=theta2_deg-theta1_deg+1;
  // reduce N if rx or ry is small
  double red=double(rx)/LCD_WIDTH_PX*double(ry)/LCD_HEIGHT_PX;
  if (red>1) red=1;
  if (red<0.1) red=0.1;
  int N=red*N0;
  if (N<5)
    N=N0>5?5:N0;
  if (N<2)
    N=2;
  vector< vector<int> > v(segment?N+1:N+2,vector<int>(2));
  x += rx/2;
  y += ry/2;
  int i=0;
  if (!segment){
    v[0][0]=x;
    v[0][1]=y;
    ++i;
  }
  double theta=theta1_deg*M_PI/180;
  double thetastep=(theta2_deg-theta1_deg)*M_PI/(180*(N-1));
  for (;i<v.size()-1;++i){
    v[i][0]=int(x+rx*cos(theta)/2+.5);
    v[i][1]=int(y-ry*sin(theta)/2+.5); // y is inverted
    theta += thetastep;
  }
  v.back()=v.front();
  draw_filled_polygon(v,0,LCD_WIDTH_PX,0,LCD_HEIGHT_PX,c);
}

unsigned short motif[8]={0xffff,0xfff0,0xff00,0xf0f0,0xe38e,0xcccc,0xaaaa,0xfc0a};
inline void fl_line(int x0,int y0,int x1,int y1,int c){
  draw_line(x0,y0,x1,y1,c,motif[c%8]);
}

inline void fl_polygon(int x0,int y0,int x1,int y1,int x2,int y2,int c){
  draw_line(x0,y0,x1,y1,c);
  draw_line(x1,y1,x2,y2,c);
}

#ifdef FX
void text_print(int fontsize,const char * s,int x,int y,int c=_BLACK,int bg=_WHITE,int mode=0){
  // *logptr(contextptr) << x << " " << y << " " << fontsize << " " << s << endl; return;
  c=(unsigned short) c;
  if (x>LCD_WIDTH_PX) return;
  int ss=strlen(s);
  if (ss==1 && s[0]==0x1e){ // arrow for limit
    if (mode)
      c=bg;
    draw_line(x,y-4,x+fontsize/2,y-4,c);
    draw_line(x,y-3,x+fontsize/2,y-3,c);
    draw_line(x+fontsize/2-4,y,x+fontsize/2,y-4,c);
    draw_line(x+fontsize/2-3,y,x+fontsize/2+1,y-4,c);
    draw_line(x+fontsize/2-4,y-7,x+fontsize/2,y-3,c);   
    draw_line(x+fontsize/2-3,y-7,x+fontsize/2+1,y-3,c);   
    return;
  }
  if (ss==2 && strcmp(s,"pi")==0){
    if (mode){
      drawRectangle(x,y-fontsize,fontsize,fontsize,c);
      c=bg;
    }
    draw_line(x+fontsize/3-1,y-2,x+fontsize/3,y+2-fontsize,c);
    //draw_line(x+fontsize/3-2,y-2,x+fontsize/3-1,y+2-fontsize,c);
    //draw_line(x+2*fontsize/3,y-2,x+2*fontsize/3,y+2-fontsize,c);
    draw_line(x+2*fontsize/3+1,y-2,x+2*fontsize/3+1,y+2-fontsize,c);
    //draw_line(x+1,y+2-fontsize,x+fontsize,y+2-fontsize,c);
    draw_line(x+1,y+1-fontsize,x+fontsize,y+1-fontsize,c);
    return;
  }
  if (fontsize>=6 && ss==2 && s[0]==char(0xe5) && (s[1]==char(0xea) || s[1]==char(0xeb))) // special handling for increasing and decreasing in tabvar output
    fontsize=8;
  if (fontsize>=8){
    while (*s && x<0){
      x+=6; ++s; --ss;
    }
    if (x+ss*6>LCD_WIDTH_PX){
      // clip string to print: ss=(LCD_WIDTH_PX-x)/4
      char buf[ss+1];
      strcpy(buf,s);
      buf[(LCD_WIDTH_PX-x)/6]=0;
      Printxy(x,y-fontsize, buf, mode?MINI_REV:0);      
    } else
      Printxy(x,y-fontsize,s,mode?1:0);
    return;
  }
  while (*s && x<0){
    x+=4; ++s; --ss;
  }
  if (x+ss*4>LCD_WIDTH_PX){
    // clip string to print: ss=(LCD_WIDTH_PX-x)/4
    char buf[ss+1];
    strcpy(buf,s);
    buf[(LCD_WIDTH_PX-x)/4]=0;
    Printmini( x, y-fontsize, buf, mode?MINI_REV:0);      
  } else
    Printmini( x, y-fontsize, s, mode?MINI_REV:0);
}

void Turtle::draw(){
  vector<logo_turtle> * turtleptr=(vector<logo_turtle> *) turtleptr_;
  const int deltax=0,deltay=0;
  drawRectangle(deltax, deltay, LCD_WIDTH_PX, LCD_HEIGHT_PX,_WHITE);
  if (turtleptr &&
#ifdef TURTLETAB
      turtle_stack_size
#else
      !turtleptr->empty()
#endif
      ){
    if (turtlezoom>8)
      turtlezoom=8;
    if (turtlezoom<0.125)
      turtlezoom=0.125;
    // check that position is not out of screen
#ifdef TURTLETAB
    logo_turtle t=turtleptr[turtle_stack_size-1];
#else
    logo_turtle t=turtleptr->back();
#endif
    double x=turtlezoom*(t.x-turtlex);
    if (x<0)
      turtlex += int(x/turtlezoom);
    if (x>=LCD_WIDTH_PX-10)
      turtlex += int((x-LCD_WIDTH_PX+10)/turtlezoom);
    double y=turtlezoom*(t.y-turtley);
    if (y<0)
      turtley += int(y/turtlezoom);
    if (y>LCD_HEIGHT_PX-10)
      turtley += int((y-LCD_HEIGHT_PX+10)/turtlezoom);
  }
  // Show turtle position/cap
  if (turtleptr &&
#ifdef TURTLETAB
      turtle_stack_size &&
#else
      !turtleptr->empty() &&
#endif
      !(maillage & 0x4)){
#ifdef TURTLETAB
    logo_turtle turtle=turtleptr[turtle_stack_size-1];
#else
    logo_turtle turtle=turtleptr->back();
#endif
    //drawRectangle(deltax+horizontal_pixels,deltay,LCD_WIDTH_PX-horizontal_pixels,2*COORD_SIZE,COLOR_YELLOW);
    // drawRectangle(deltax, deltay, LCD_WIDTH_PX, LCD_HEIGHT_PX,_BLACK);
    string tmp("x ");
    tmp += printint(int(turtle.x+.5));
    Printmini(64,0,tmp.c_str(),_BLACK);
    tmp=string("y ");
    tmp += printint(int(turtle.y+.5));
    Printmini(85,0,tmp.c_str(),_BLACK);
    tmp=string("t ");
    tmp += printint(int(turtle.theta+.5));
    Printmini(106,0,tmp.c_str(),_BLACK);
  }
  // draw turtle Logo
  if (turtleptr){
#ifdef TURTLETAB
    int l=turtle_stack_size;
#else
    int l=turtleptr->size();
#endif
    if (l>0){
#ifdef TURTLETAB
      logo_turtle prec =turtleptr[0];
#else
      logo_turtle prec =(*turtleptr)[0];
#endif
      for (int k=1;k<l;++k){
#ifdef TURTLETAB
	logo_turtle current =(turtleptr)[k];
#else
	logo_turtle current =(*turtleptr)[k];
#endif
	if (current.s>=0){ // Write a string
	  //cout << current.radius << " " << current.s << endl;
	  if (current.s<ecristab().size())
	    text_print(current.radius,ecristab()[current.s].c_str(),int(deltax+turtlezoom*(current.x-turtlex)),int(deltay+LCD_HEIGHT_PX-turtlezoom*(current.y-turtley)),current.color);
	}
	else {
	  if (current.radius>0){
	    int r=current.radius & 0x1ff; // bit 0-8
	    double theta1,theta2;
	    if (current.direct){
	      theta1=prec.theta+double((current.radius >> 9) & 0x1ff); // bit 9-17
	      theta2=prec.theta+double((current.radius >> 18) & 0x1ff); // bit 18-26
	    }
	    else {
	      theta1=prec.theta-double((current.radius >> 9) & 0x1ff); // bit 9-17
	      theta2=prec.theta-double((current.radius >> 18) & 0x1ff); // bit 18-26
	    }
	    bool rempli=(current.radius >> 27) & 0x1;
	    bool seg=(current.radius >> 28) & 0x1;
	    double angle;
	    int x,y,R;
	    R=int(2*turtlezoom*r+.5);
	    angle = M_PI/180*(theta2-90);
	    if (current.direct){
	      x=int(turtlezoom*(current.x-turtlex-r*cos(angle) - r)+.5);
	      y=int(turtlezoom*(current.y-turtley-r*sin(angle) + r)+.5);
	    }
	    else {
	      x=int(turtlezoom*(current.x-turtlex+r*cos(angle) -r)+.5);
	      y=int(turtlezoom*(current.y-turtley+r*sin(angle) +r)+.5);
	    }
	    if (current.direct){
	      if (rempli)
		fl_pie(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,theta1-90,theta2-90,current.color,seg);
	      else
		fl_arc(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,theta1-90,theta2-90,current.color);
	    }
	    else {
	      if (rempli)
		fl_pie(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,90+theta2,90+theta1,current.color,seg);
	      else
		fl_arc(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,90+theta2,90+theta1,current.color);
	    }
	  } // end radius>0
	  else {
	    if (prec.mark){
	      fl_line(deltax+int(turtlezoom*(prec.x-turtlex)+.5),deltay+int(LCD_HEIGHT_PX+turtlezoom*(turtley-prec.y)+.5),deltax+int(turtlezoom*(current.x-turtlex)+.5),deltay+int(LCD_HEIGHT_PX+turtlezoom*(turtley-current.y)+.5),prec.color);
	    }
	  }
	  if (current.radius<-1 && k+current.radius>=0){
	    // poly-line from (*turtleptr)[k+current.radius] to (*turtleptr)[k]
	    vector< vector<int> > vi(1-current.radius,vector<int>(2));
	    for (int i=0;i>=current.radius;--i){
#ifdef TURTLETAB
	      logo_turtle & t=(turtleptr)[k+i];
#else
	      logo_turtle & t=(*turtleptr)[k+i];
#endif
	      vi[-i][0]=deltax+turtlezoom*(t.x-turtlex);
	      vi[-i][1]=deltay+LCD_HEIGHT_PX+turtlezoom*(turtley-t.y);
	      //*logptr(contextptr) << i << " " << vi[-i][0] << " " << vi[-i][1] << endl;
	    }
	    //vi.back()=vi.front();
	    draw_filled_polygon(vi,0,LCD_WIDTH_PX,0,LCD_HEIGHT_PX,current.color);
	  }
	} // end else (non-string turtle record)
	prec=current;
      } // end for (all turtle records)
#ifdef TURTLETAB
      logo_turtle & t = (turtleptr)[l-1];
#else
      logo_turtle & t = (*turtleptr)[l-1];
#endif
      int x=int(turtlezoom*(t.x-turtlex)+.5);
      int y=int(turtlezoom*(t.y-turtley)+.5);
      double cost=cos(t.theta*deg2rad_d);
      double sint=sin(t.theta*deg2rad_d);
      int Dx=int(turtlezoom*t.turtle_length*cost/2+.5);
      int Dy=int(turtlezoom*t.turtle_length*sint/2+.5);
      if (t.visible){
	fl_line(deltax+x+Dy,deltay+LCD_HEIGHT_PX-(y-Dx),deltax+x-Dy,deltay+LCD_HEIGHT_PX-(y+Dx),t.color);
	int c=t.color;
	if (!t.mark) c=t.color ^ 0x7777;
	fl_line(deltax+x+Dy,deltay+LCD_HEIGHT_PX-(y-Dx),deltax+x+3*Dx,deltay+LCD_HEIGHT_PX-(y+3*Dy),0); // always display turtle
	fl_line(deltax+x-Dy,deltay+LCD_HEIGHT_PX-(y+Dx),deltax+x+3*Dx,deltay+LCD_HEIGHT_PX-(y+3*Dy),0);
      }
    }
    return;
  } // End logo mode
}
#else
  void text_print(int fontsize,const char * s,int x,int y,int c=COLOR_BLACK,int bg=COLOR_WHITE,int mode=0){
    // *logptr(contextptr) << x << " " << y << " " << fontsize << " " << s << endl; return;
    c=(unsigned short) c;
    if (x>LCD_WIDTH_PX) return;
    int ss=strlen(s);
    if (ss==1 && s[0]==0x1e){ // arrow for limit
      if (mode==4)
	c=bg;
      draw_line(x,y-4,x+fontsize/2,y-4,c);
      draw_line(x,y-3,x+fontsize/2,y-3,c);
      draw_line(x+fontsize/2-4,y,x+fontsize/2,y-4,c);
      draw_line(x+fontsize/2-3,y,x+fontsize/2+1,y-4,c);
      draw_line(x+fontsize/2-4,y-7,x+fontsize/2,y-3,c);   
      draw_line(x+fontsize/2-3,y-7,x+fontsize/2+1,y-3,c);   
      return;
    }
    if (ss==2 && strcmp(s,"pi")==0){
      if (mode==4){
	drawRectangle(x,y+2-fontsize,fontsize,fontsize,c);
	c=bg;
      }
      draw_line(x+fontsize/3-1,y+1,x+fontsize/3,y+6-fontsize,c);
      draw_line(x+fontsize/3-2,y+1,x+fontsize/3-1,y+6-fontsize,c);
      draw_line(x+2*fontsize/3,y+1,x+2*fontsize/3,y+6-fontsize,c);
      draw_line(x+2*fontsize/3+1,y+1,x+2*fontsize/3+1,y+6-fontsize,c);
      draw_line(x+2,y+6-fontsize,x+fontsize,y+6-fontsize,c);
      draw_line(x+2,y+5-fontsize,x+fontsize,y+5-fontsize,c);
      return;
    }
    if (fontsize>=16 && ss==2 && s[0]==char(0xe5) && (s[1]==char(0xea) || s[1]==char(0xeb))) // special handling for increasing and decreasing in tabvar output
      fontsize=18;
    if (fontsize>=18){
      y -= 40;//36; // status area shift
      PrintMini(&x,&y,(unsigned char *)s,mode,0xffffffff,0,0,c,bg,1,0);
      return;
    }
    if (fontsize<16){
      y -= 36;//32;
      PrintMiniMini( &x, &y, (unsigned char *)s, mode,c, 0 );
      return;
    }
    y -= 38;//34;
    //PrintCXY(x,y,s,TEXT_MODE_NORMAL,-1,COLOR_BLACK,COLOR_WHITE,1,0);
    Bdisp_MMPrint(x,y,s,mode,0xffffffff,0,0,c,bg,1,0);
  }

void Turtle::draw(){
  vector<logo_turtle> * turtleptr=(vector<logo_turtle> *) turtleptr_;
  int fl_line_width=1,save_width=fl_line_width;
  const int deltax=0,deltay=24,COORD_SIZE=30;
  int horizontal_pixels=LCD_WIDTH_PX-2*COORD_SIZE;
  // Check for fast redraw
  // Then redraw the background
  drawRectangle(deltax, deltay, LCD_WIDTH_PX, LCD_HEIGHT_PX-24,COLOR_WHITE);
#ifdef TURTLETAB
  if (turtleptr && turtle_stack_size==0){
    turtleptr[0]=logo_turtle();
    ++turtle_stack_size;
  }
#endif
  if (turtleptr &&
#ifdef TURTLETAB
      turtle_stack_size
#else
      !turtleptr->empty()
#endif
      ){
    if (turtlezoom>8)
      turtlezoom=8;
    if (turtlezoom<0.125)
      turtlezoom=0.125;
    // check that position is not out of screen
#ifdef TURTLETAB
    logo_turtle t=turtleptr[turtle_stack_size-1];
#else
    logo_turtle t=turtleptr->back();
#endif
    double x=turtlezoom*(t.x-turtlex);
    double y=turtlezoom*(t.y-turtley);
#if 0
    if (x<0)
      turtlex += int(x/turtlezoom);
    if (x>=LCD_WIDTH_PX-10)
      turtlex += int((x-LCD_WIDTH_PX+10)/turtlezoom);
    if (y<0)
      turtley += int(y/turtlezoom);
    if (y>LCD_HEIGHT_PX-10)
      turtley += int((y-LCD_HEIGHT_PX+10)/turtlezoom);
#endif
  }
#if 0
  if (maillage & 0x3){
    fl_color(FL_BLACK);
    double xdecal=floor(turtlex/10.0)*10;
    double ydecal=floor(turtley/10.0)*10;
    if ( (maillage & 0x3)==1){
      for (double i=xdecal;i<LCD_WIDTH_PX+xdecal;i+=10){
	for (double j=ydecal;j<LCD_HEIGHT_PX+ydecal;j+=10){
	  fl_point(deltax+int((i-turtlex)*turtlezoom+.5),deltay+LCD_HEIGHT_PX-int((j-turtley)*turtlezoom+.5));
	}
      }
    }
    else {
      double dj=std::sqrt(3.0)*10,i0=xdecal;
      for (double j=ydecal;j<LCD_HEIGHT_PX+ydecal;j+=dj){
	int J=deltay+int(LCD_HEIGHT_PX-(j-turtley)*turtlezoom);
	for (double i=i0;i<LCD_WIDTH_PX+xdecal;i+=10){
	  fl_point(deltax+int((i-turtlex)*turtlezoom+.5),J);
	}
	i0 += dj;
	while (i0>=10)
	  i0 -= 10;
      }
    }
  }
#endif
  // Show turtle position/cap
  if (turtleptr &&
#ifdef TURTLETAB
      turtle_stack_size &&
#else
      !turtleptr->empty() &&
#endif
      !(maillage & 0x4)
      ){
#ifdef TURTLETAB
    logo_turtle turtle=turtleptr[turtle_stack_size-1];
#else
    logo_turtle turtle=turtleptr->back();
#endif
    drawRectangle(deltax+horizontal_pixels,deltay,LCD_WIDTH_PX-horizontal_pixels,2*COORD_SIZE,COLOR_YELLOW);
    // drawRectangle(deltax, deltay, LCD_WIDTH_PX, LCD_HEIGHT_PX,COLOR_BLACK);
    char buf[32];
    sprintf(buf,"x %i   ",int(turtle.x+.5));
    text_print(18,buf,deltax+horizontal_pixels,deltay+(2*COORD_SIZE)/3-2,COLOR_BLACK,COLOR_YELLOW);
    sprintf(buf,"y %i   ",int(turtle.y+.5));
    text_print(18,buf,deltax+horizontal_pixels,deltay+(4*COORD_SIZE)/3-3,COLOR_BLACK,COLOR_YELLOW);
    sprintf(buf,"t %i   ",int(turtle.theta+.5));
    text_print(18,buf,deltax+horizontal_pixels,deltay+2*COORD_SIZE-4,COLOR_BLACK,COLOR_YELLOW);
  }
  // draw turtle Logo
  if (turtleptr){
#ifdef TURTLETAB
    int l=turtle_stack_size;
#else
    int l=turtleptr->size();
#endif
    if (l>0){
#ifdef TURTLETAB
      logo_turtle prec =turtleptr[0];
#else
      logo_turtle prec =(*turtleptr)[0];
#endif
      //cout << "turtle length=" << l << '\n';
      for (int k=1;k<l;++k){
#ifdef TURTLETAB
	logo_turtle current =(turtleptr)[k];
#else
	logo_turtle current =(*turtleptr)[k];
#endif
	if (current.s>=0){ // Write a string
	  //cout << current.radius << " " << current.s << endl;
	  if (current.s<ecristab().size())
	    text_print(current.radius,ecristab()[current.s].c_str(),int(deltax+turtlezoom*(current.x-turtlex)),int(deltay+LCD_HEIGHT_PX-turtlezoom*(current.y-turtley)),current.color);
	}
	else {
	  // cout <<"k=" << k << "r=" << current.radius << '\n';
	  //fl_line_width=width;
	  int width=current.turtle_length & 0x1f;
	  if (current.radius>0){
	    int r=current.radius & 0x1ff; // bit 0-8
	    double theta1,theta2;
	    if (current.direct){
	      theta1=prec.theta+double((current.radius >> 9) & 0x1ff); // bit 9-17
	      theta2=prec.theta+double((current.radius >> 18) & 0x1ff); // bit 18-26
	    }
	    else {
	      theta1=prec.theta-double((current.radius >> 9) & 0x1ff); // bit 9-17
	      theta2=prec.theta-double((current.radius >> 18) & 0x1ff); // bit 18-26
	    }
	    bool rempli=(current.radius >> 27) & 0x1;
	    bool seg=(current.radius >> 28) & 0x1;
	    double angle;
	    int x,y,R;
	    R=int(2*turtlezoom*r+.5);
	    angle = M_PI/180*(theta2-90);
	    if (current.direct){
	      x=int(turtlezoom*(current.x-turtlex-r*cos(angle) - r)+.5);
	      y=int(turtlezoom*(current.y-turtley-r*sin(angle) + r)+.5);
	    }
	    else {
	      x=int(turtlezoom*(current.x-turtlex+r*cos(angle) -r)+.5);
	      y=int(turtlezoom*(current.y-turtley+r*sin(angle) +r)+.5);
	    }
	    if (current.direct){
	      if (rempli)
		fl_pie(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,theta1-90,theta2-90,current.color,seg);
	      else
		fl_arc(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,theta1-90,theta2-90,current.color);
	    }
	    else {
	      if (rempli)
		fl_pie(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,90+theta2,90+theta1,current.color,seg);
	      else
		fl_arc(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,90+theta2,90+theta1,current.color);
	    }
	  } // end radius>0
	  else {
	    if (prec.mark){
	      fl_line(deltax+int(turtlezoom*(prec.x-turtlex)+.5),deltay+int(LCD_HEIGHT_PX+turtlezoom*(turtley-prec.y)+.5),deltax+int(turtlezoom*(current.x-turtlex)+.5),deltay+int(LCD_HEIGHT_PX+turtlezoom*(turtley-current.y)+.5),prec.color);
	    }
	  }
	  if (current.radius<-1 && k+current.radius>=0){
	    // poly-line from (*turtleptr)[k+current.radius] to (*turtleptr)[k]
	    vector< vector<int> > vi(1-current.radius,vector<int>(2));
	    for (int i=0;i>=current.radius;--i){
#ifdef TURTLETAB
	      logo_turtle & t=(turtleptr)[k+i];
#else
	      logo_turtle & t=(*turtleptr)[k+i];
#endif
	      //cout << i << " " << t.radius << " " << t.color << " " << current.color << '\n';
	      if (t.radius>0){
		int r=t.radius & 0x1ff; // bit 0-8
		int x,y,R;
		R=int(2*turtlezoom*r+.5);
		double angle = M_PI/180*(current.theta-90);
		if (t.direct){
		  x=int(turtlezoom*(t.x-turtlex-r*cos(angle) - r)+.5);
		  y=int(turtlezoom*(t.y-turtley-r*sin(angle) + r)+.5);
		}
		else {
		  x=int(turtlezoom*(t.x-turtlex+r*cos(angle) -r)+.5);
		  y=int(turtlezoom*(t.y-turtley+r*sin(angle) +r)+.5);
		}
		fl_pie(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,0,360,current.color,false);
	      }
	      vi[-i][0]=deltax+turtlezoom*(t.x-turtlex);
	      vi[-i][1]=deltay+LCD_HEIGHT_PX+turtlezoom*(turtley-t.y);
	      //*logptr(contextptr) << i << " " << vi[-i][0] << " " << vi[-i][1] << endl;
	    }
	    //vi.back()=vi.front();
	    draw_filled_polygon(vi,0,LCD_WIDTH_PX,24,LCD_HEIGHT_PX,current.color);
	  }
	} // end else (non-string turtle record)
	prec=current;
      } // end for (all turtle records)
#ifdef TURTLETAB
      logo_turtle & t = (turtleptr)[l-1];
#else
      logo_turtle & t = (*turtleptr)[l-1];
#endif
      int x=int(turtlezoom*(t.x-turtlex)+.5);
      int y=int(turtlezoom*(t.y-turtley)+.5);
      double cost=cos(t.theta*deg2rad_d);
      double sint=sin(t.theta*deg2rad_d);
      int tl=t.turtle_length+10;
      int Dx=int(turtlezoom*tl*cost/2+.5);
      int Dy=int(turtlezoom*tl*sint/2+.5);
      if (t.visible){
	fl_line(deltax+x+Dy,deltay+LCD_HEIGHT_PX-(y-Dx),deltax+x-Dy,deltay+LCD_HEIGHT_PX-(y+Dx),t.color);
	int c=t.color;
	if (!t.mark)
	  c=t.color ^ 0x7777;
	fl_line(deltax+x+Dy,deltay+LCD_HEIGHT_PX-(y-Dx),deltax+x+3*Dx,deltay+LCD_HEIGHT_PX-(y+3*Dy),c);
	fl_line(deltax+x-Dy,deltay+LCD_HEIGHT_PX-(y+Dx),deltax+x+3*Dx,deltay+LCD_HEIGHT_PX-(y+3*Dy),c);
      }
    }
    fl_line_width=save_width;
    return;
  } // End logo mode
}  
#endif

