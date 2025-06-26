#ifndef BACKTRACK_FIND_CLUE_H
#define BACKTRACK_FIND_CLUE_H
#include "mapbuild.h" 
/*
*@brief ����·�������������ʹ�û��ݷ����ɴ���ʼ���굽����������·��
*@param �Թ���Ϣ
*@param ��ʼ���꣨��������꿪ʼѰ������������
*@param ��������
*@return ����ʼ���굽����������·������ʼ��->��һ������->�ڶ�������->.....��ÿһ�ηֱ𱣴���vector������
*/
class clue_finder {
public:
    int clue_left_total; // �Թ�������������
    int size;             // �Թ��ߴ�
    int (*maze)[MAXSIZE]; // �Թ�����ָ��
    std::pair<int, int> locker;     // ��ʼ��������

private:
    int isvisited[MAXSIZE][MAXSIZE]; // ���ʱ������

    // �ڲ���������ʼ�����ʱ��
    void init_visited() {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                isvisited[i][j] = 0;
            }
        }
    }

    // ���Ļ��ݺ�������Ϊ˽�У���Ϊ�ڲ�ʵ��ϸ�ڣ�
    bool find_clue_recursive(int x, int y, std::vector<std::pair<int, int>>& path) {
        if (x < 0 || x >= size || y < 0 || y >= size || isvisited[x][y] || maze[x][y] == static_cast<int>(MAZE::WALL)) {
            return false;
        }

        isvisited[x][y] = 1;
        path.push_back({x, y});

        if (maze[x][y] == static_cast<int>(MAZE::CLUE)) {
            return true;
        }

        if (find_clue_recursive(x + 1, y, path)) return true;
        if (find_clue_recursive(x - 1, y, path)) return true;
        if (find_clue_recursive(x, y + 1, path)) return true;
        if (find_clue_recursive(x, y - 1, path)) return true;

        path.pop_back();
        return false;
    }

public:
    // ���캯��
    clue_finder(int size, int (*maze)[MAXSIZE], std::pair<int, int> locker, int total_clues) {
        this->size = size;
        this->maze = maze;
        this->locker = locker;
        this->clue_left_total = total_clues;
    }

    // --- �µĹ����ӿں��� ---
    // ���ô˺�����Ѱ����������������·��
    // ����ֵ: һ������������ÿ��Ԫ������һ����һ���㵽��һ��������·��
    std::vector<std::vector<std::pair<int, int>>> find_all_clue_paths() {
        std::vector<std::vector<std::pair<int, int>>> all_paths;
        std::pair<int, int> current_start_pos = this->locker;
        //std::cout<<"player current position: ("<<current_start_pos.first<<","<<current_start_pos.second<<")"<<std::endl;
        int clues_to_find = this->clue_left_total;

        while (clues_to_find > 0) {
            init_visited(); // Ϊ��һ���������÷�������
            std::vector<std::pair<int, int>> path_segment; // �洢��ǰ·����
            
            // �����ڲ����ݺ���Ѱ�Ҵӵ�ǰ��㵽��һ��������·��
            if (find_clue_recursive(current_start_pos.first, current_start_pos.second, path_segment)) {
                // �ҵ���һ������
                all_paths.push_back(path_segment); // ���ҵ���·���δ�����·������

                // ������һ�����������Ϊ�ո��ҵ�������λ��
                current_start_pos = path_segment.back();
                
                // �����ҵ����������Թ��б��Ϊ��ͨ·���������ظ�����
                // ע�⣺����޸Ĵ�����Թ�����
                maze[current_start_pos.first][current_start_pos.second] = static_cast<int>(MAZE::WAY);
                
                clues_to_find--;
            } else {
                // ����Ҳ�������������˵��ʣ���������ɴ�˳�ѭ��
                std::cout << "����: �޷��ҵ����� " << clue_left_total << " �������������в��ֱ����롣" << std::endl;
                break;
            }
        }
        
        // �ָ����޸ĵ��������Ա��ⲿ�����ٴ�ʹ��ԭʼ�Թ�״̬����ѡ��
        for(const auto& path_seg : all_paths) {
            if (!path_seg.empty()) {
                const auto& clue_pos = path_seg.back();
                maze[clue_pos.first][clue_pos.second] = static_cast<int>(MAZE::CLUE);
            }
        }

        return all_paths;
    }

    ~clue_finder() {}
};

int call_finder_example() {
    int size = 15;
    MazeGenerator generator(size);
    generator.generate();
    generator.placeFeatures();
    std::cout << "--- ��ʼ�Թ� ---" << std::endl;
    generator.print();

    int (*maze)[MAXSIZE] = generator.getmaze();
    int size_of_maze = generator.getsize();
    int total_clues = 3; // ������3������

    // ���� finder ʵ��
    clue_finder finder(size_of_maze, maze, generator.getLocker(), total_clues);//�ڴ˴����뿪ʼ���꣬��ʼѰ����������

    // --- ֻ�����һ���������ɻ�ȡ����·�� ---
    std::cout << "\n--- ��ʼѰ������������·�� ---" << std::endl;
    auto all_clue_paths = finder.find_all_clue_paths();//���ص��Ǵ�����������꣬;������������·��

    // --- ��ӡ��� ---
    if (all_clue_paths.empty()) {
        std::cout << "δ���ҵ��κδӻ��ص�������·����" << std::endl;
    } else {
        std::cout << "�ɹ��ҵ� " << all_clue_paths.size() << " ��·���Σ�" << std::endl;
        for (size_t i = 0; i < all_clue_paths.size(); ++i) {
            std::cout << "\n--- ·���� " << i + 1 << " ---" << std::endl;
            const auto& path = all_clue_paths[i];
            for (size_t j = 0; j < path.size(); ++j) {
                std::cout << "(" << path[j].first << ", " << path[j].second << ")";
                if (j < path.size() - 1) {
                    std::cout << " -> ";
                }
            }
            std::cout << " (�ҵ�����!)" << std::endl;
        }
    }

    std::cout << "\n--- ���ҽ��� ---" << std::endl;

    return 0;
}

#endif