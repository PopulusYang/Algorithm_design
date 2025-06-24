#include "collect_coin.h"

coin_collecter::coin_collecter(int**map,int size)
    :map(map),
    size(size)
{
    map=new int*[size];
}
coin_collecter::~coin_collecter()
{

}
