#include "collect_coin.h"

bool coin_collecter::ifsourvaild(point currentpoint)//判断周围视野中是否有资源
{
    int x=currentpoint.x;
    int y=currentpoint.y;
    oneview(0,point(x-1,y-1));
    oneview(1,point(x-1,y));
    oneview(2,point(x-1,y+1));
    oneview(3,point(x,y-1));
    oneview(4,point(x,y+1));
    oneview(5,point(x+1,y-1));
    oneview(6,point(x+1,y));
    oneview(7,point(x+1,y+1));
    for(int i=0;i<=7;i++)
    {
        if(view[i].type==MAZE::SOURCE)
        {
            return true;
        }
    }
    return false;
}
point coin_collecter::findway(point currentpoint)//在这个函数里找到下一步的位置
{
    if(!ifsourvaild(currentpoint))return currentpoint;//如果周围没有资源，那么返回玩家现在的位置
    int maxindex;
    int maxsource=-100;
    for(int i=0;i<=7;i++)
    {
        if(i==0||i==2||i==5||i==7)
        {
            view[i].valueff=view[i].value/2;
        }
        else view[i].valueff=view[i].value;
        if(view[i].valueff>maxsource)
        {
            maxsource=view[i].valueff;
            maxindex=i;
        }
    }
    if(maxindex==1||maxindex==3||maxindex==4||maxindex==6)return view[maxindex].coinpoint;//距离为一格的情况
    if(maxindex==0)//距离为两格的情况
    {
        if(view[3].accessible==false&&view[1].accessible==false)return currentpoint;
        if(view[3].value>view[1].value)return view[3].coinpoint;
        return view[1].coinpoint;
    }
    else if(maxindex==2)
    {
        if(view[4].accessible==false&&view[1].accessible==false)return currentpoint;
        if(view[4].value>view[1].value)return view[4].coinpoint;
        return view[1].coinpoint;
    }
    else if(maxindex==5)
    {
        if(view[3].accessible==false&&view[6].accessible==false)return currentpoint;
        if(view[6].value>view[3].value)return view[6].coinpoint;
        return view[3].coinpoint;
    }
    else
    {
        if(view[4].accessible==false&&view[6].accessible==false)return currentpoint;
        if(view[6].value>view[4].value)return view[6].coinpoint;
        return view[4].coinpoint;
    }
}

void coin_collecter::oneview(int index,point viewpoint)
{
    view[index].coinpoint=viewpoint;
    int x=viewpoint.x;
    int y=viewpoint.y;
    if(x>=0&&x<informations->mazesize&&y>=0&&y<informations->mazesize)
    {
        if (informations->maze[x][y] == static_cast<int>(MAZE::SOURCE)) // 资源
        {
            view[index].accessible=true;
            view[index].type=MAZE::SOURCE;
            view[index].value=sourse_value[point(x,y)];
        }
        else if (informations->maze[x][y] == static_cast<int>(MAZE::WAY)) // 通路
        {
            view[index].accessible=true;
            view[index].type=MAZE::WAY;
            view[index].value=0;

        }
        else if (informations->maze[x][y] == static_cast<int>(MAZE::LOCKER)) // 陷阱
        {
            view[index].accessible=true;
            view[index].type=MAZE::LOCKER;
            view[index].value=-1;

        }
        else if (informations->maze[x][y] = static_cast<int>(MAZE::TRAP))
        {
            view[index].accessible=true;
            view[index].type=MAZE::TRAP;
            view[index].value=-2;
        }
        else if (informations->maze[x][y] == static_cast<int>(MAZE::WALL))
        {
            view[index].accessible=false;
            view[index].type=MAZE::WALL;
            view[index].value=-3;
        }
        view[index].coinpoint=viewpoint;
    }
    else
    {
        view[index].accessible=false;
        view[index].value=-10;
        view[index].coinpoint=viewpoint;
    }
}
