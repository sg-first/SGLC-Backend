#include <iostream>
#include "codegen.h"
#include <fstream>

using namespace std;

string ReadTXT(qstring path)
{
    ifstream ifs(path);

    if (ifs.fail()) {
        cout << "文件不能打开" << endl;
    }
    string ss((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());

    ifs.close();
    return ss;
}

void WriteTXT(string path, string text)
{
    fstream op;
    op.open(path, ios::out);
    op<<text;
    op.close();
}

int main(int argc, char* argv[])
{
    /*qstring lef=ReadTXT(argv[2]);
    qstring def=ReadTXT(argv[4]);*/
    qstring def=ReadTXT("D:/sample4.def");
    qstring lef=ReadTXT("D:/sample4.lef");
    defParser p1(def);
    lefParser p2(lef,1,8);
    /*auto c=p2.getCell("CELL2");
    auto v=p2.getVia(6);
    auto m=p2.getMetal(6);*/
    router l(p1,p2);
    codegen cg(l);
    qstring result=cg.doGen();
    WriteTXT("D:/result.def",result);
    return 0;
}
