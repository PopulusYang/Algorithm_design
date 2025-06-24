#include "mapbuild.h"
/*
@breif 
*/
class clue_finder {
public:
int clue_left=3; // ʣ����������
int size; // �Թ��ߴ�
int (*maze)[MAXSIZE]; // �Թ�����ָ��
int isvisited[MAXSIZE][MAXSIZE]; // ���ʱ������
std::pair<int, int> locker; // ��������
std::pair<int, int> current_position; // ��ǰ����λ��
clue_finder(int size, int (*maze)[MAXSIZE],std::pair<int,int> locker) {
    // ���캯�����Գ�ʼ��һЩ��Ҫ�Ĳ���
    this->size = size;
    this->maze = maze;      
    this->locker = locker; // ��������
}

void init_visited() {
    // ��ʼ�����ʱ������
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            isvisited[i][j] = 0; // δ���ʱ��Ϊ0
        }
    }
}

void find_clue(int x, int y) {
    //std::cout << "��ǰλ��: (" << x << ", " << y << ")" << std::endl;
    // �ݹ���������ĺ���
    if (x < 0 || x >= size || y < 0 || y >= size || isvisited[x][y] || maze[x][y] == static_cast<int>(MAZE::WALL)) {
        return ; // Խ����ѷ��ʻ���ǽ��
    }
    
    isvisited[x][y] = 1; // ���Ϊ�ѷ���

    // ��鵱ǰ�����Ƿ�������
    if (maze[x][y] == static_cast<int>(MAZE::CLUE)) {
        std::cout << "�ҵ�������λ��: (" << x << ", " << y << ")" << std::endl;
        clue_left--; // ����ʣ����������
        if (clue_left <= 0) {
            std::cout << "�����������ҵ���" << std::endl;
            current_position = std::make_pair(x, y);
            return ; // �ҵ����������󷵻�
        }
        //return; // �ҵ������󷵻�
    }

    // �ݹ�̽���ĸ�����
    find_clue(x + 1, y); // ��
    find_clue(x - 1, y); // ��
    find_clue(x, y + 1); // ��
    find_clue(x, y - 1); // ��
}

std::pair<int,int> getpos() {
    // ���ص�ǰ����λ��
    return current_position;
   
}

~clue_finder() {
    // �������������ͷ���Դ
}
};


int main() {
    int size = 15; // �Թ��ߴ�
    MazeGenerator generator(size); // ����������ʵ��
    generator.generate();          // �����Թ��ṹ
    generator.placeFeatures();     // ����������
    generator.print();             // ��ӡ�Թ�����ѡ��

    // ��ȡ�Թ�����ָ��
    int (*maze)[MAXSIZE] = generator.getmaze();
    int size_of_maze = generator.getsize();
    // �������ĳ�����ӵ�����
    clue_finder finder(size_of_maze, maze, generator.getLocker());
    finder.init_visited(); // ��ʼ�����ʱ������
    finder.find_clue(generator.getLocker().first, generator.getLocker().second); // �ӻ���λ��(��������λ��)��ʼ�������������ܽ�ң�������Щ��������棬��Ϊ���������ҵ�
    std::pair<int, int> pos = finder.getpos(); // ��ȡ������λ��
    std::cout<< "�����������������λ��: (" << pos.first << ", " << pos.second << ")" << std::endl;
    return 0;
}