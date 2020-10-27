#pragma once
#include "lefParser.h"
#include "defParser.h"

class layout
{
private:
    optional<rect> checkLine(line l)
    {
        for(LEF::cell &c : this->allCell) //fix:可以尝试只检测近邻的以剪枝
        {
            auto result=l.checkOBS(c);
            if(result.has_value())
                return result;
        }

        rect lr=l.toRect();
        for(line &li : this->allLine) //fix:剪枝同上
        {
            if(l.metal==li.metal)
            {
                rect lir=li.toRect();
                if(lr.isIntersect(lir,l.metal.spacing))
                    return lir;
            }
        }

        return optional<rect>();
    }

    void connect(LEF::pin &p1, LEF::pin &p2)
    {
        //避开目前有线位置和obs连接两个pin
        //1.查找二者最近的两个rect
        rect r1,r2;
        tie(r1,r2,ignore)=this->getPinDist(p1,p2);
        //2.尝试横/竖走线（注意横竖在不同层，连接横竖时需要打孔走孔）
        //2.1.生成两个line矩形 l1 l2、及连通它们的孔
        auto fixConnect=[this](line l)
        {
            //2.2.检测每条横/竖线中无法走线（与障碍矩形相交），此时要获取到整个障碍矩形 - 使用checkLine
            auto obsRect=this->checkLine(l);
            if(obsRect.has_value())
            {
                //3.在此处停止，进行局部修正（绕过障碍矩形/走到障碍矩形的一个距目标rect最近的顶点）
                //4.使用修正后的坐标继续横/竖走线（循环）、如果无法绕过，继续打孔？
            }
            else
                this->allLine.push_back(l);
        };

    }

    LEF::pin& findLEFPin(QString instName, QString pinName)
    {
        for(LEF::cell &c : this->allCell)
        {
            if(c.instName==instName)
            {
                for(LEF::pin &p : c.allPin)
                {
                    if(p.name==pinName)
                        return p;
                }
            }
        }
    }

    static tuple<rect,rect,float> getPinDist(LEF::pin &p1, LEF::pin &p2)
    {
        //rect中点
        float mid_p1_x ;
        float mid_p1_y ;
        float mid_p2_x ;
        float mid_p2_y ;
        //rect差值
        float div_x ;
        float div_y ;
        float div ;
        //上次计算结果
        float last;
        int p1_rect_cnt_last;
        int p2_rect_cnt_last;
        //循环比较
        for(int p1_rect_cnt=0;p1_rect_cnt<p1.allRect.size();p1_rect_cnt++)
        {
            for(int p2_rect_cnt=1;p2_rect_cnt<p2.allRect.size();p2_rect_cnt++)
            {
                if(p1_rect_cnt==p2_rect_cnt)
                    continue;

                tie(mid_p1_x,mid_p1_y)=p1.allRect[p1_rect_cnt].getMidPos();
                tie(mid_p2_x,mid_p2_y)=p2.allRect[p1_rect_cnt].getMidPos();

                div_x = mid_p1_x - mid_p2_x;
                div_y = mid_p1_y - mid_p2_y;

                div = abs(div_x) + abs(div_y);

                if((p1_rect_cnt == 0) && (p1_rect_cnt == 0))
                {
                    last = div ;
                    p1_rect_cnt_last = p1_rect_cnt;
                    p2_rect_cnt_last = p2_rect_cnt;
                }
                else if(div<last)
                {
                    last = div;
                    p1_rect_cnt_last = p1_rect_cnt;
                    p2_rect_cnt_last = p2_rect_cnt;
                }
            }
        }
        //最小的rect编号和最小x+y距离
        rect p1_rect = p1.allRect[p1_rect_cnt_last];
        rect p2_rect = p2.allRect[p2_rect_cnt_last];
        return make_tuple(p1_rect,p2_rect,last);
    }

    vector<LEF::pin> toLEFPinVec(const vector<DEF::pin> &allPin)
    {
        vector<LEF::pin> result;
        for(auto i : allPin)
            result.push_back(this->findLEFPin(i.instName,i.pinName));
        return result;
    }

    vector<LEF::pin> sortAllPin(const vector<DEF::pin> &allPin)
    {
        vector<LEF::pin> LEFallPin=this->toLEFPinVec(allPin);
        float distance = 0;
        float last_distance = 0;
        int min_num = 0;
        for (int i = 0; i < LEFallPin.size() - 2; i++)
        {
            for (int j = i + 1; j < LEFallPin.size() - 1; j++)
            {
                tie(ignore,ignore,distance) = this->getPinDist(LEFallPin[i], LEFallPin[j]);
                //在i后面找和i距离最小的
                if ((i == 0) && (j == 1)) {
                    last_distance = distance;
                    min_num = j;
                }
                else if (distance < last_distance) {
                    last_distance = distance;
                    min_num = j;
                }
            }
            LEF::pin temp = LEFallPin[min_num];
            LEFallPin[min_num] = LEFallPin[i+1];
            LEFallPin[i+1] = temp;
        }
        return LEFallPin;
    }

public:
    defParser dp;
    lefParser lp;
    vector<line> allLine; //将net转化为line
    vector<LEF::cell> allCell; //版图中放置的所有cell（转换为版图坐标系）

    layout(defParser dp, lefParser lp) : dp(dp), lp(lp)
    {
        for(DEF::component &c : dp.allComponent)
        {
            LEF::cell lefC=lp.getCell(c.cellName);
            lefC.instName=c.instName;
            lefC.setToLayout(c.x,c.y,c.dire);
            this->allCell.push_back(lefC);
        }
    }

    void connectAllNet()
    {
        for(DEF::net &n : dp.allNet)
        {
            auto LEFallPin=this->sortAllPin(n.allPin);
            for(int i=1;i<LEFallPin.size();i++)
            {
                this->connect(LEFallPin[i-1],LEFallPin[i]);
            }
        }
    }
};

