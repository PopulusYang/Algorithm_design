#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <algorithm>
#include <utility>
#include "mapbuild.h"
// Ϊ�˷�����ĿҪ�󣬶���һ���̶���MAXSIZE�����ڲ�ʹ�ö�̬��std::vector<std::string>
// ���������������C++�����ʵ����


int call_mapbuild_example() {
    int size;
    std::cout << "�������Թ��ĳߴ� (�Ƽ�����, ��СΪ7): ";
    std::cin >> size;

    std::cout << "\n�������� " << size << "x" << size << " (�������ĳߴ�) ���Թ�...\n" << std::endl;

    // ����������ʵ��
    MazeGenerator generator(size);

    // �����Թ��ṹ
    generator.generate();
    
    // ������ø������
    generator.placeFeatures();

    //��ȡ�Թ�����,��ӡmaze[0][0]
    std::cout<<generator.getMaze()[0][0];

    // ��ӡ���յ��Թ�
    std::cout << "�Թ��������:" << std::endl;
    std::cout << "S: ���, E: �յ�, #: ǽ��, G: ���, T: ����, L: ����, B: BOSS" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    generator.print();
    std::cout << "-------------------------------------------------" << std::endl;
    generator.print_num();

    return 0;
}