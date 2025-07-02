#ifndef exam4_h
#define exam4_h
#include <bits/stdc++.h>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QDir>
using namespace std;


class SHA256 {
private:
    // SHA-256绠楁硶鐨勫父閲�
    const static uint32_t K[64];
    
    // 鍒濆鍝堝笇鍊硷紙鏉ヨ嚜SHA-256鏍囧噯锛�
    std::array<uint32_t, 8> H = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    
    // 宸ュ叿鍑芥暟锛氬彸鏃嬭浆
    uint32_t rightRotate(uint32_t value, unsigned int count);

    // 澶勭悊鍗曚釜512浣嶇殑鏁版嵁鍧�
    void processBlock(const uint8_t *block);

public:
    // 璁＄畻杈撳叆鏁版嵁鐨凷HA-256鍝堝笇鍊�
    std::vector<uint8_t> compute(const std::vector<uint8_t> &message);

    // 閲嶇疆鍝堝笇鐘舵€�
    void reset();
};



class PasswordLock {
private:
    // 涓嶱ython鐗堟湰瀵归綈鐨剆alt鍊�: b'\xb2S"e}\xdf\xb0\xfe\x9c\xde\xde\xfe\xf3\x1d\xdc>'
    std::vector<unsigned char> salt = {
        0xb2, 0x53, 0x22, 0x65, 0x7d, 0xdf, 0xb0, 0xfe, 
        0x9c, 0xde, 0xde, 0xfe, 0xf3, 0x1d, 0xdc, 0x3e
    };

    // 杈呭姪鍑芥暟锛氬皢瀛楄妭鏁扮粍杞崲涓哄崄鍏繘鍒跺瓧绗︿覆
    std::string bytesToHex(const std::vector<uint8_t> &bytes);

public:
    std::string hashPassword(const std::string &password);

    // 楠岃瘉瀵嗙爜
    bool verifyPassword(const std::string &inputPassword, const std::string &storedHash);
};


/**
 * @brief 娣卞害浼樺厛鎼滅储浠ユ煡鎵惧瘑鐮�.
 * @param idx 褰撳墠姝ｅ湪澶勭悊鐨勫瘑鐮佷綅缃� (0-2).
 * @param targetHash 瑕佸尮閰嶇殑鐩爣鍝堝笇瀛楃涓�.
 * @param lock 鐢ㄤ簬璁＄畻鍝堝笇鐨凱asswordLock瀹炰緥.
 * @param decrypt_count 寮曠敤锛岀敤浜庣疮鍔犺В瀵嗗皾璇曟鏁�.
 * @param foundPassword 寮曠敤锛岀敤浜庡瓨鍌ㄦ壘鍒扮殑瀵嗙爜.
 * @return true 濡傛灉鎵惧埌瀵嗙爜锛屽垯鎼滅储缁堟.
 * @return false 濡傛灉鏈壘鍒板瘑鐮侊紝鍒欑户缁悳绱�.
 */
// 娣卞害浼樺厛鐢熸垚婊¤冻鏉′欢鐨勪笁浣嶅瘑鐮�
bool findPasswordDfs(int idx, const string &targetHash, PasswordLock &lock, int &decrypt_count, string &foundPassword);


/**
 * @brief 获取破解信息，包括解密次数和密码.
 * @param argc 命令行参数数量.
 * @param argv 命令行参数数组.
 * @return 包含解密次数和密码的pair.
 */
std::pair<int, string> get_crack_info(int argc, char *argv[]);

#endif

