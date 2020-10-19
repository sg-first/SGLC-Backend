#pragma once
#include "lefStru.h"
#include "help.h"
#include <QStringList>

class lefParser
{
private:
    QStringList codeList;

    int getPin(int i, LEF::cell &c)
    {
        LEF::pin p;
        //get pinName
        p.name=help::getLastElm(this->codeList[i],"PIN");
        i++;
        //实际读内容
        for(int j=i;j<this->codeList.length();j++)
        {
            QString stri=this->codeList[j];
            if(stri.indexOf("LAYER")!=-1)
                p.layer=help::getLastElm(stri,"LAYER");
            else if(stri.indexOf("RECT ")!=-1)
            {
                rect r=rect::getRect(stri,c.sizeA1,c.sizeA2);
                p.allRect.push_back(r);

            }
            else if(stri.indexOf("END "+p.name)!=-1)
            {
                c.allPin.push_back(p);
                return j;
            }
        }
        return -1;
    }

    int getOBS(int i, LEF::cell &c)
    {
        QString layerName;
        for(int j=i;i<this->codeList.length();j++)
        {
            QString stri=this->codeList[j];
            if(stri.indexOf("LAYER")!=-1)
                layerName=help::getLastElm(stri,"LAYER");
            else
            {
                if(layerName!="")
                {
                    if(stri.indexOf("RECT")!=-1)
                    {
                        rect r=rect::getRect(stri,c.sizeA1,c.sizeA2);
                        c.o[layerName].push_back(r);
                    }
                }
                if(stri.indexOf("END")!=-1)
                    return j;
            }
        }
        return -1;
    }

public:
    lefParser(QString code)
    {
        codeList=code.split("\n");
    }

    LEF::cell getCell(QString cellName)
    {
        LEF::cell c;
        bool findCell=false;
        for(int i=0;i<codeList.length();i++)
        {
            QString stri=codeList[i];
            if(findCell==false && stri=="MACRO "+cellName) //找到CELL位置，进状态
            {
                c.name=cellName;
                findCell=true;
            }
            else if(findCell)
            {
                if(stri.indexOf("SIZE")!=-1)
                {
                    QStringList sizeList=help::splitSpace(stri);
                    for(int j=0;j<sizeList.size();j++)
                    {
                        if(sizeList[j]=="SIZE")
                        {
                            c.sizeA1=sizeList[j+1].toFloat();
                            c.sizeA2=sizeList[j+3].toFloat();
                            break;
                        }
                    }
                }
                else if(stri.indexOf("PIN")!=-1)
                    i=this->getPin(i,c);
                else if(stri.indexOf("OBS")!=-1)
                    i=this->getOBS(i+1,c);
                else if(stri=="END "+cellName)
                    break;
            }
        }
        return c;
    }
};