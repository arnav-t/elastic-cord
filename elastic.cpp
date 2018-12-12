#include <SDL2/SDL.h>
#include <iostream>
#include <math.h>

const int n = 64;
const int scr_w = 512;
const int scr_h = 512;
const float dt = 0.001;
float m = 0.01/n;
float l = scr_h/(4*n);
const float k = -5;
const float g = 117.6;
const float damp = -25;

const SDL_Color red={255,0,0,255};
const SDL_Color green={0,255,0,255};
const SDL_Color blue={0,0,255,255};
const SDL_Color white={255,255,255,255};
const SDL_Color black={0,0,0,255};
SDL_Color Tcol(float h,float v)
{
    double c=v*255;
    h*=6;
    //h=60-h;
    double x=c*(1-abs(fmod(h,2) - 1))*255;
    SDL_Color col;
    if(h<1)
        col={c,x,0};
    else if(h>=1&&h<2)
        col={x,c,0};
    else if(h>=2&&h<3)
        col={0,c,x};
    else if(h>=3&&h<4)
        col={0,x,c};
    else if(h>=4&&h<5)
        col={x,0,c};
    else if(h>=5&&h<6)
        col={c,0,x};
    else
    col={0,0,255,255};
    return(col);
}



inline int sgn(float x) {return(x > 0) ? 1 : ((x < 0) ? -1 : 0);}

struct Vector
{
    float x,y;
    Vector()
    {
        x=0;y=0;
    }
};
Vector operator+(Vector a,Vector b)
{
    Vector c;
    c.x=a.x + b.x;
    c.y=a.y + b.y;
    return c;
}
Vector operator-(Vector a,Vector b)
{
    Vector c;
    c.x=a.x - b.x;
    c.y=a.y - b.y;
    return c;
}
Vector operator*(Vector a,float k)
{
    Vector c;
    c.x=a.x*k;
    c.y=a.y*k;
    return c;
}
inline float mag(Vector z)
{
    return sqrt(z.x*z.x+z.y*z.y);
}

Vector spa(Vector a,Vector b, Vector va, Vector vb)
{
    Vector d = a-b;
    Vector zero;
    float spd = abs(mag(d)-l);
    /*if(spd<=0)
        return zero;
    else*/
        return d*(k*spd/(mag(d)*m)) + (va-vb)*damp;
}

class Display
{
    public:
        Display(int w,int h)
        {
            win=SDL_CreateWindow("String",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,w,h,SDL_WINDOW_SHOWN);
            ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);
        }
        ~Display()
        {
            SDL_DestroyTexture(tex);
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
        }
        void drawLine(Vector a,Vector b,SDL_Color c)
        {
            SDL_SetRenderDrawColor(ren,c.r,c.g,c.b,c.a);
            SDL_RenderDrawLine(ren,a.x,a.y,b.x,b.y);
        }
        void drawBox(SDL_Rect& rect,SDL_Color c)
        {
            SDL_SetRenderDrawColor(ren,c.r,c.g,c.b,c.a);
            SDL_RenderFillRect(ren,&rect);
        }
        void drawCircle(Vector point,double r,SDL_Color c)
        {
            SDL_SetRenderDrawColor(ren,c.r,c.g,c.b,c.a);
            for(double angle=0;angle<6.283185;angle+=0.1)
            {
                SDL_RenderDrawPoint(ren,point.x+r*cos(angle),point.y+r*sin(angle));
            }
            SDL_Rect re={point.x-r/2,point.y-r/2,r,r};
            SDL_RenderFillRect(ren,&re);
        }
        void renderTexture(int x,int y)
        {
            SDL_Rect paste={x-texSize.x/2,y-texSize.y/2,texSize.x,texSize.y};
            SDL_RenderCopy(ren,tex,NULL,&paste);
        }
        void clearScreen(SDL_Color c)
        {
            SDL_SetRenderDrawColor(ren,c.r,c.g,c.b,c.a);
            SDL_RenderFillRect(ren,NULL);
        }
        void updateScreen()
        {
            SDL_RenderPresent(ren);
        }
    private:
        SDL_Window* win=NULL;
        SDL_Renderer* ren=NULL;
        SDL_Texture* tex=NULL;
        SDL_Point texSize;
};


class String
{
private:
    Vector nodes[n];
    Vector v[n];
    void calcF(Vector P, bool apply)
    {
        for(int i=0;i<n;++i)
        {
            float t;
            if(i==0)
            {
                Vector o;
                o.x=scr_w/2;
                o.y=scr_h/2;
                Vector zero;
                v[i] = v[i] + spa(nodes[i],o,v[i],zero)*dt;
            }
            else
            {
                v[i] = v[i] + spa(nodes[i],nodes[i-1],v[i],v[i-1])*dt;
            }
            if(i==(n-1)&&apply)
            {
                Vector zero;
                v[i] = v[i] + spa(nodes[i],P,v[i],zero)*(dt/64);
            }
            else if(i!=(n-1))
            {
                v[i] = v[i] + spa(nodes[i],nodes[i+1],v[i],v[i+1])*dt;
            }
            v[i].y+=g*dt;
        }
    }
    void calcNode()
    {
        for(int i=0;i<n;++i)
        {
            nodes[i].x+=v[i].x*dt;
            nodes[i].y+=v[i].y*dt;
        }
    }
    void draw(Display &d,bool apply,Vector P)
    {
        Vector c;
        c.x=scr_w/2;
        c.y=scr_h/2;
        float q=1-(mag(nodes[0]-c)/(3*l));
        //std::cout<<q<<std::endl;
        d.drawLine(c,nodes[0],Tcol(q,1));
        for(int i=0;i<n-1;++i)
        {
            q=1-(mag(nodes[i]-nodes[i+1])/(3*l));
            d.drawLine(nodes[i],nodes[i+1],Tcol(q,1));
        }
        if(apply)
            d.drawLine(nodes[n-1],P,white);
    }
public:
    String()
    {
        for(int k=0;k<n;++k)
        {
            nodes[k].x = scr_w/2;
            nodes[k].y = scr_h/2 + (k+1)*l;
        }
    }
    void process(Vector P, bool apply,Display &d)
    {
        calcF(P,apply);
        calcNode();
        draw(d,apply,P);
    }
};


int main(int argc,char *argv[])
{
    Display display(scr_w,scr_h);
    String s;
    SDL_Event e;
    display.clearScreen(black);
    display.updateScreen();
    bool quit=false;
    bool app = false;
    Vector mouse;
    int x,y;
    while(!quit)
    {
        while(SDL_PollEvent(&e)!=0)
        {
            if(e.type==SDL_QUIT)
            {
                quit=true;
                break;
            }
            else if(e.type==SDL_MOUSEBUTTONDOWN||e.type==SDL_MOUSEMOTION)
            {
                SDL_GetMouseState(&x,&y);
                mouse.x=x;
                mouse.y=y;
                if(e.button.button==SDL_BUTTON_LEFT&&e.type==SDL_MOUSEBUTTONDOWN)
                    app = true;
            }
            else if(e.type==SDL_MOUSEBUTTONUP)
            {
                if(e.button.button==SDL_BUTTON_LEFT)
                    app = false;

            }
        }
        display.clearScreen(black);
        s.process(mouse,app,display);
        display.updateScreen();
        SDL_Delay(dt*1000);
    }
    return 0;
}

