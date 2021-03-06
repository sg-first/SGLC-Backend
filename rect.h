#pragma once
#include "pos.h"
#include "QString.h"
#include "help.h"
#include <cmath>
#include <tuple>
using namespace std;

class rect
{
public:
    pos p1;
    pos p2;
    rect(pos p1, pos p2):p1(p1),p2(p2){}
    rect() : p1(pos(0,0)), p2(pos(0,0)) {} //洞

    bool isNull() const { return p1.x==0 && p1.y==0 && p2.x==0 && p2.y==0; }

    static rect getRect(qstring stri, float w=-1, float h=-1, bool isOrigin=false)
    {
        qstringList strList=help::splitSpace(stri);
        for(int i=0;i<strList.size();i++)
        {
            float x1,y1,x2,y2;
            if(strList[i]=="RECT")
            {
                x1=strList[i+1].toFloat();
                y1=strList[i+2].toFloat();
                x2=strList[i+3].toFloat();
                y2=strList[i+4].toFloat();
                pos p1(x1,y1,w,h,isOrigin);
                pos p2(x2,y2,w,h,isOrigin);
                rect r(p1,p2);
                return r;
            }
        }
    }

    void setToLayout(float setX, float setY, qstring dire)
    {
        this->p1.setDire(dire);
        this->p1.setToLayout(setX,setY);
        this->p2.setDire(dire);
        this->p2.setToLayout(setX,setY);
    }

    tuple<float,float> getMidPos() const
    {
        float r2_x_mid=(this->p1.x+this->p2.x)/2;
        float r2_y_mid=(this->p2.y+this->p2.y)/2;
        return make_tuple(r2_x_mid,r2_y_mid);
    }

    rect getOuterBorder(float spacing)
    {
        float expand=spacing;
        pos p1=pos(this->p1.x-expand,this->p1.y-expand);
        pos p2=pos(this->p2.x+expand,this->p2.y+expand);
        return rect(p1,p2);
    }

    bool isIntersect(const rect& r,float spacing,float width)
    {
        float r1_x_mid,r1_y_mid,r2_x_mid,r2_y_mid;
        tie(r1_x_mid,r1_y_mid)=r.getMidPos();
        tie(r2_x_mid,r2_y_mid)=this->getMidPos();

        float expand=(spacing-width)/2;

        float r1Width=r.p2.x-r.p1.x+expand;
        float r1Height=r.p2.y-r.p1.y+expand;
        float r2Width=this->p2.x-this->p1.x+expand;
        float r2Height=this->p2.y-this->p1.y+expand;

        if (
            fabs(r1_x_mid - r2_x_mid) < r1Width / 2.0 + r2Width / 2.0 //横向判断
            &&
            fabs(r1_y_mid - r2_y_mid) < r1Height / 2.0 + r2Height / 2.0 //纵向判断
            )
        {
            return true; //碰撞
        }
        else
            return false;
    }

    bool isLowerLeft(const rect &r)
    {
        if(this->isNull())
            return false;
        else if(r.isNull())
            return true;
        else
        {
            if(r.p1.x>=this->p1.x && r.p1.y>=this->p1.y)
                return true;
            else
                return false;
        }
    }

    void plusDbu(int dbu)
    {
        p1.x*=dbu;
        p1.y*=dbu;
        p2.x*=dbu;
        p2.y*=dbu;
    }
};


class pinRect : public rect
{
public:
    bool isOccupy=false;

    pinRect(pos _p1, pos _p2):rect(_p1,_p2){}
    pinRect() : rect() {} //洞
    pinRect(const rect &r) : rect(r.p1,r.p2) {}
};
