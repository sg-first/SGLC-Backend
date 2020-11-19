#pragma once
#include "layout.h"

class codegen
{
private:
    //处理一个net，在后面添加line和via。返回处理完的行下标
    int genNet(int i,QString &result,vector<line> &allLine, vector<via> &allVia)
    {
        result+=this->l.dp.codeList[i]+"\n"; //把net那行加进去
        i++;

        for(;i<this->l.dp.codeList.length();i++)
        {
            QString stri=this->l.dp.codeList[i];
            result+=stri+"\n";
            //向下探测有没有到结尾
            QString stri2=this->l.dp.codeList[i+1];
            if(stri2.indexOf(";")!=-1) //fix:目前sample里暂时没有分号非单独一行的
            {
                //到结尾了
                if(!allLine.empty())
                {
                    result+="+ ROUTED ";
                    bool first=true;
                    //把allVia加上
                    for(via v : allVia)
                    {
                        if(!first)
                            result+="NEW ";
                        else
                            first=false;

                        result+=v.m.getName()+" "+v.getPos()+" "+v.getName()+"\n";
                    }
                    //把allLine加上
                    for(line l : allLine)
                    {
                        if(!first)
                            result+="NEW ";
                        else
                            first=false;

                        pos p1,p2;
                        tie(p1,p2)=l.getMidLine();

                        result+=l.metal.getName()+" "+p1.toStr()+" "+p2.toStr()+"\n";
                    }
                }
                break;
            }
        }
        result+=";\n";
        return i+1; //跳过分号那行到下一个net
    }

public:
    layout l;
    codegen(layout l) : l(l) {}

    QString doGen()
    {
        QString result;
        for(int i=0;i<this->l.dp.codeList.length();i++)
        {
            QString stri=this->l.dp.codeList[i];
            if(stri.indexOf("DESIGN ")!=-1)
                result+="DESIGN result ;\n";
            else if(stri.indexOf("NETS")!=-1)
                result+=this->genNETS(i);
            else
                result+=stri+"\n";
        }
        return result;
    }

    /*QString genNONDEFAULTRULES()
    {
        QString result="NONDEFAULTRULES 1 ;\n";
        result+="- DEFAULT_METAL1_580\n";
        for(LEF::metal &m : l.lp.allMetal)
        {
            result+="+ LAYER "+m.getName()+"\n";
            result+="WIDTH "+QString::number(int(m.width));
        }
        result+=";\n";
        result+="END NONDEFAULTRULES\n";
        return result;
    }*/

    QString genNETS(int &i)
    {
        QString result;
        //使用类似NETparser，每个net结束之后添加对应下标的line
        bool findNet=false;
        int netNum=0;
        for(;i<this->l.dp.codeList.length();i++)
        {
            QString stri=this->l.dp.codeList[i];
            if(!findNet && stri.indexOf("NETS")!=-1)
            {
                result+=stri+"\n";
                findNet=true;
            }

            if(findNet)
            {
                if(stri.indexOf("net")!=-1) //如果碰到一个net，转到genNet里去把这个处理完
                {
                    i=this->genNet(i,result,l.allNetLine[netNum],l.allNetVia[netNum]);
                    netNum++;
                }

                if(stri.indexOf("END NETS")!=-1)
                {
                    //END这句不会在前面添加到findNet里面去
                    result+=stri+"\n";
                    return result;
                }
            }
        }
        throw string("NET cannot found");
    }
};
