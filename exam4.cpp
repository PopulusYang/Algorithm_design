#include <bits/stdc++.h>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QDir>
using namespace std;

vector<int> posFixed(3, -1), posParity(3, -1);
bool needPrimeUnique = false;
vector<int> primes = {2, 3, 5, 7};
vector<int> cur;
vector<string> answers;
bool used[10] = {false};

class SHA256 {
private:
    // SHA-256�㷨�ĳ���
    const static uint32_t K[64];
    
    // ��ʼ��ϣֵ������SHA-256��׼��
    std::array<uint32_t, 8> H = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    
    // ���ߺ���������ת
    inline uint32_t rightRotate(uint32_t value, unsigned int count) {
        return (value >> count) | (value << (32 - count));
    }
    
    // ������512λ�����ݿ�
    void processBlock(const uint8_t* block) {
        // ������Ϣ���ȱ�
        uint32_t W[64];
        for (int t = 0; t < 16; t++) {
            W[t] = (block[t * 4] << 24) | (block[t * 4 + 1] << 16) |
                   (block[t * 4 + 2] << 8) | (block[t * 4 + 3]);
        }
        
        // ��չ��Ϣ���ȱ�
        for (int t = 16; t < 64; t++) {
            uint32_t s0 = rightRotate(W[t-15], 7) ^ rightRotate(W[t-15], 18) ^ (W[t-15] >> 3);
            uint32_t s1 = rightRotate(W[t-2], 17) ^ rightRotate(W[t-2], 19) ^ (W[t-2] >> 10);
            W[t] = W[t-16] + s0 + W[t-7] + s1;
        }
        
        // ��ʼ����������
        uint32_t a = H[0];
        uint32_t b = H[1];
        uint32_t c = H[2];
        uint32_t d = H[3];
        uint32_t e = H[4];
        uint32_t f = H[5];
        uint32_t g = H[6];
        uint32_t h = H[7];
        
        // ��ѭ��
        for (int t = 0; t < 64; t++) {
            uint32_t S1 = rightRotate(e, 6) ^ rightRotate(e, 11) ^ rightRotate(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = h + S1 + ch + K[t] + W[t];
            uint32_t S0 = rightRotate(a, 2) ^ rightRotate(a, 13) ^ rightRotate(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;
            
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        
        // ���¹�ϣֵ
        H[0] += a;
        H[1] += b;
        H[2] += c;
        H[3] += d;
        H[4] += e;
        H[5] += f;
        H[6] += g;
        H[7] += h;
    }
    
public:
    // �����������ݵ�SHA-256��ϣֵ
    std::vector<uint8_t> compute(const std::vector<uint8_t>& message) {
        // ��������ĳ��ȣ�ԭʼ���� + 1�ֽڵ�1 + ���0 + 8�ֽڵĳ��ȣ�
        uint64_t originalBitLength = message.size() * 8;
        uint64_t paddedLength = message.size() + 1 + 8; // ������Ҫ���9���ֽ�
        paddedLength = (paddedLength + 63) & ~63;       // ����Ϊ64�ֽڵı�����512λ��
        
        // �����������Ϣ
        std::vector<uint8_t> paddedMessage(paddedLength, 0);
        std::copy(message.begin(), message.end(), paddedMessage.begin());
        
        // ���һ��1λ����Ϊһ���ֽڵ�0x80��
        paddedMessage[message.size()] = 0x80;
        
        // �����Ϣ���ȣ���λΪ��λ�������
        for (int i = 0; i < 8; i++) {
            paddedMessage[paddedLength - 8 + i] = (originalBitLength >> ((7 - i) * 8)) & 0xFF;
        }
        
        // ���鴦����Ϣ
        for (size_t i = 0; i < paddedLength; i += 64) {
            processBlock(&paddedMessage[i]);
        }
        
        // �������չ�ϣֵ��32�ֽڣ�
        std::vector<uint8_t> hash(32);
        for (int i = 0; i < 8; i++) {
            hash[i * 4] = (H[i] >> 24) & 0xFF;
            hash[i * 4 + 1] = (H[i] >> 16) & 0xFF;
            hash[i * 4 + 2] = (H[i] >> 8) & 0xFF;
            hash[i * 4 + 3] = H[i] & 0xFF;
        }
        
        return hash;
    }
    
    // ���ù�ϣ״̬
    void reset() {
        H = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };
    }
};

// SHA-256�㷨�ĳ���������SHA-256��׼��
const uint32_t SHA256::K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

class PasswordLock {
private:
    // ��Python�汾�����saltֵ: b'\xb2S"e}\xdf\xb0\xfe\x9c\xde\xde\xfe\xf3\x1d\xdc>'
    std::vector<unsigned char> salt = {
        0xb2, 0x53, 0x22, 0x65, 0x7d, 0xdf, 0xb0, 0xfe, 
        0x9c, 0xde, 0xde, 0xfe, 0xf3, 0x1d, 0xdc, 0x3e
    };

    // �������������ֽ�����ת��Ϊʮ�������ַ���
    std::string bytesToHex(const std::vector<uint8_t>& bytes) {
        std::stringstream ss;
        for (const auto& byte : bytes) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return ss.str();
    }

public:
    std::string hashPassword(const std::string& password) {
        // ������ת��Ϊ�ֽ���
        std::vector<unsigned char> passwordBytes(password.begin(), password.end());
        
        std::vector<unsigned char> combined;
        combined.insert(combined.end(), salt.begin(), salt.end());
        combined.insert(combined.end(), passwordBytes.begin(), passwordBytes.end());
        
        SHA256 sha256;
        std::vector<uint8_t> hashBytes = sha256.compute(combined);
        
        return bytesToHex(hashBytes);
    }
    
    // ��֤����
    bool verifyPassword(const std::string& inputPassword, const std::string& storedHash) {
        // ʹ����ͬ����ֵ������������й�ϣ
        std::string calculatedHash = hashPassword(inputPassword);
        
        // �Ƚϼ�����Ĺ�ϣֵ��洢�Ĺ�ϣֵ
        return calculatedHash == storedHash;
    }
};


/**
 * @brief ������������Բ�������.
 * @param idx ��ǰ���ڴ��������λ�� (0-2).
 * @param targetHash Ҫƥ���Ŀ���ϣ�ַ���.
 * @param lock ���ڼ����ϣ��PasswordLockʵ��.
 * @param decrypt_count ���ã������ۼӽ��ܳ��Դ���.
 * @param foundPassword ���ã����ڴ洢�ҵ�������.
 * @return true ����ҵ����룬��������ֹ.
 * @return false ���δ�ҵ����룬���������.
 */
// �����������������������λ����
bool findPasswordDfs(int idx, const string& targetHash, PasswordLock& lock, int& decrypt_count, string& foundPassword) {
    // �����������һ����������λ�������ɺ�
    if (idx == 3) {
        string pwd = string(1, char('0' + cur[0])) +
                     string(1, char('0' + cur[1])) +
                     string(1, char('0' + cur[2]));
        
        decrypt_count++; // ���ӳ��Դ���

        // ��������ϣֵ
        if (lock.hashPassword(pwd) == targetHash) {
            foundPassword = pwd;
            return true; // �ҵ����룬����true����ֹ����
        }
        
        return false; // ��ϣ��ƥ�䣬��������
    }

    // �ݹ鲽��
    if (needPrimeUnique) {
        for (int d : primes) {
            if (used[d]) continue;
            if (posFixed[idx] != -1 && posFixed[idx] != d) continue;
            if (posParity[idx] != -1 && d % 2 != posParity[idx]) continue;
            
            used[d] = true;
            cur.push_back(d);
            if (findPasswordDfs(idx + 1, targetHash, lock, decrypt_count, foundPassword)) {
                return true; // ����������ҵ����룬��������
            }
            cur.pop_back();
            used[d] = false;
        }
    } else {
        for (int d = 0; d <= 9; ++d) {
            if (posFixed[idx] != -1 && posFixed[idx] != d) continue;
            if (posParity[idx] != -1 && d % 2 != posParity[idx]) continue;
            
            cur.push_back(d);
            if (findPasswordDfs(idx + 1, targetHash, lock, decrypt_count, foundPassword)) {
                return true; // ����������ҵ����룬��������
            }
            cur.pop_back();
        }
    }

    return false; // �������֧��û���ҵ�����
}

// --- MAIN FUNCTION (MODIFIED) ---
// int main(int argc, char *argv[]) {
//     QCoreApplication app(argc, argv);

//     QString dirPath = "../password_test";
//     QDir directory(dirPath);

//     if (!directory.exists()) {
//         cerr << "Directory not found: " << dirPath.toStdString() << endl;
//         return 1;
//     }
    
//     QStringList jsonFiles = directory.entryList(QStringList() << "*.json", QDir::Files);
//     long long total_decrypt_count = 0;

//     for (const QString &fileName : jsonFiles) {
//         QString jsonPath = directory.filePath(fileName);

//         // --- 1. ����ÿ���ļ���״̬ ---
//         posFixed.assign(3, -1);
//         posParity.assign(3, -1);
//         needPrimeUnique = false;
//         cur.clear();
//         fill(begin(used), end(used), false);
        
//         // --- 2. ��ȡ�ͽ���JSON�ļ� ---
//         QFile jsonFile(jsonPath);
//         if (!jsonFile.open(QIODevice::ReadOnly)) {
//             cerr << "Failed to open JSON file for reading: " << jsonPath.toStdString() << endl;
//             continue;
//         }
//         QByteArray jsonData = jsonFile.readAll();
//         jsonFile.close();

//         QJsonParseError parseError;
//         QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
//         if (parseError.error != QJsonParseError::NoError) {
//             cerr << "JSON parse error in " << jsonPath.toStdString() << ": " << parseError.errorString().toStdString() << endl;
//             continue;
//         }
//         QJsonObject rootObj = doc.object();

//         // --- 3. ��������"C"��������� ---
//         QJsonArray clues = rootObj.value("C").toArray();
//         for (const QJsonValue &val : clues) {
//             QJsonArray arr = val.toArray();
//             vector<int> v;
//             for (const QJsonValue &elem : arr) v.push_back(elem.toInt());
//             if (v.size() == 2) {
//                 if (v[0] == -1 && v[1] == -1) needPrimeUnique = true;
//                 else posParity[v[0] - 1] = v[1];
//             } else if (v.size() == 3) {
//                 for (int j = 0; j < 3; ++j) if (v[j] != -1) { posFixed[j] = v[j]; break; }
//             }
//         }
        
//         // --- 4. �������벢��ȷ���� ---
//         string inputHash = rootObj.value("L").toString().toStdString();
//         int decrypt_count = 0;
//         string foundPassword = "";
//         PasswordLock lock;

//         findPasswordDfs(0, inputHash, lock, decrypt_count, foundPassword);
        
//         total_decrypt_count += decrypt_count;

//         // --- 5. ���Ҳ������� ---
//         if (!foundPassword.empty()) {
//             cout << "File: " << left << setw(15) << fileName.toStdString() 
//                  << " -> Password found: " << foundPassword 
//                  << " (Attempts: " << decrypt_count << ")" << endl;

//             rootObj.insert("P", QString::fromStdString(foundPassword));
//             rootObj.insert("D", decrypt_count);

//             QJsonDocument newDoc(rootObj);
//             if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
//                 cerr << "  [ERROR] Failed to open file for writing: " << jsonPath.toStdString() << endl;
//             } else {
//                 jsonFile.write(newDoc.toJson(QJsonDocument::Indented));
//                 jsonFile.close();
//             }
//         } else {
//             cout << "File: " << left << setw(15) << fileName.toStdString() 
//                  << " -> Password NOT found. (Total attempts in space: " << decrypt_count << ")" << endl;
//         }
//     }

//     // --- 6. ��ӡ�������� ---
//     cout << "\n================================================" << endl;
//     cout << "  Total decryption attempts for all files: " << total_decrypt_count << endl;
//     cout << "================================================" << endl;

//     return 0;
// }