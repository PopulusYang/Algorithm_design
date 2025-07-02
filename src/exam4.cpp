#include "heads/exam4.h"
#include "heads/exam4.h"
vector<int> posFixed(3, -1), posParity(3, -1);
bool needPrimeUnique = false;
vector<int> primes = {2, 3, 5, 7};
vector<int> cur;
vector<string> answers;
bool used[10] = {false};
std::pair<int, string> crack_result = {0, "NAN"}; // 用于存储破解结果 <解密次数, 密码>
uint32_t SHA256::rightRotate(uint32_t value, unsigned int count)
{
    return (value >> count) | (value << (32 - count));
}
void SHA256::processBlock(const uint8_t *block)
    
{
    // 鍒涘缓娑堟伅璋冨害琛�
    uint32_t W[64];
    for (int t = 0; t < 16; t++) {
        W[t] = (block[t * 4] << 24) | (block[t * 4 + 1] << 16) |
               (block[t * 4 + 2] << 8) | (block[t * 4 + 3]);
    }
    // 鎵╁睍娑堟伅璋冨害琛�
    for (int t = 16; t < 64; t++) {
        uint32_t s0 = rightRotate(W[t-15], 7) ^ rightRotate(W[t-15], 18) ^ (W[t-15] >> 3);
        uint32_t s1 = rightRotate(W[t-2], 17) ^ rightRotate(W[t-2], 19) ^ (W[t-2] >> 10);
        W[t] = W[t-16] + s0 + W[t-7] + s1;
    }
    
    // 鍒濆鍖栧伐浣滃彉閲�
    uint32_t a = H[0];
    uint32_t b = H[1];
    uint32_t c = H[2];
    uint32_t d = H[3];
    uint32_t e = H[4];
    uint32_t f = H[5];
    uint32_t g = H[6];
    uint32_t h = H[7];
    
    // 涓诲惊鐜�
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
    
    // 鏇存柊鍝堝笇鍊�
    H[0] += a;
    H[1] += b;
    H[2] += c;
    H[3] += d;
    H[4] += e;
    H[5] += f;
    H[6] += g;
    H[7] += h;
}

std::vector<uint8_t> SHA256::compute(const std::vector<uint8_t>& message) {
    // 璁＄畻濉厖鍚庣殑闀垮害锛堝師濮嬮暱搴� + 1瀛楄妭鐨�1 + 濉厖0 + 8瀛楄妭鐨勯暱搴︼級
    uint64_t originalBitLength = message.size() * 8;
    uint64_t paddedLength = message.size() + 1 + 8; // 鑷冲皯闇€瑕佹坊鍔�9涓瓧鑺�
    paddedLength = (paddedLength + 63) & ~63;       // 璋冩暣涓�64瀛楄妭鐨勫€嶆暟锛�512浣嶏級
    
    // 鍒涘缓濉厖鍚庣殑娑堟伅
    std::vector<uint8_t> paddedMessage(paddedLength, 0);
    std::copy(message.begin(), message.end(), paddedMessage.begin());
    
    // 娣诲姞涓€涓�1浣嶏紙浣滀负涓€涓瓧鑺傜殑0x80锛�
    paddedMessage[message.size()] = 0x80;
    
    // 娣诲姞娑堟伅闀垮害锛堜互浣嶄负鍗曚綅锛屽ぇ绔簭锛�
    for (int i = 0; i < 8; i++) {
        paddedMessage[paddedLength - 8 + i] = (originalBitLength >> ((7 - i) * 8)) & 0xFF;
    }
    
    // 鎸夊潡澶勭悊娑堟伅
    for (size_t i = 0; i < paddedLength; i += 64) {
        processBlock(&paddedMessage[i]);
    }
    
    // 鐢熸垚鏈€缁堝搱甯屽€硷紙32瀛楄妭锛�
    std::vector<uint8_t> hash(32);
    for (int i = 0; i < 8; i++) {
        hash[i * 4] = (H[i] >> 24) & 0xFF;
        hash[i * 4 + 1] = (H[i] >> 16) & 0xFF;
        hash[i * 4 + 2] = (H[i] >> 8) & 0xFF;
        hash[i * 4 + 3] = H[i] & 0xFF;
    }
    
    return hash;
}

void SHA256::reset() {
    H = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
}

// SHA-256绠楁硶鐨勫父閲忥紙鏉ヨ嚜SHA-256鏍囧噯锛�
const uint32_t SHA256::K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

std::string PasswordLock::bytesToHex(const std::vector<uint8_t> &bytes)
{
    std::stringstream ss;
    for (const auto &byte : bytes)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return ss.str();
}

std::string PasswordLock::hashPassword(const std::string &password)
{
    // 灏嗗瘑鐮佽浆鎹负瀛楄妭娴�
    std::vector<unsigned char> passwordBytes(password.begin(), password.end());
    
    std::vector<unsigned char> combined;
    combined.insert(combined.end(), salt.begin(), salt.end());
    combined.insert(combined.end(), passwordBytes.begin(), passwordBytes.end());
    
    SHA256 sha256;
    std::vector<uint8_t> hashBytes = sha256.compute(combined);
    
    return bytesToHex(hashBytes);
}


// 楠岃瘉瀵嗙爜
bool PasswordLock::verifyPassword(const std::string &inputPassword, const std::string &storedHash)
{
    // 浣跨敤鐩稿悓鐨勭洂鍊煎杈撳叆瀵嗙爜杩涜鍝堝笇
    std::string calculatedHash = hashPassword(inputPassword);

    // 姣旇緝璁＄畻鍑虹殑鍝堝笇鍊间笌瀛樺偍鐨勫搱甯屽€�
    return calculatedHash == storedHash;
}

bool findPasswordDfs(int idx, const string &targetHash, PasswordLock &lock, int &decrypt_count, string &foundPassword)
{
    // 鍩烘湰鎯呭喌锛氬綋涓€涓畬鏁寸殑涓変綅瀵嗙爜鐢熸垚鍚�
    if (idx == 3)
    {
        string pwd = string(1, char('0' + cur[0])) +
                     string(1, char('0' + cur[1])) +
                     string(1, char('0' + cur[2]));

        decrypt_count++; // 澧炲姞灏濊瘯娆℃暟

        // 绔嬪嵆妫€鏌ュ搱甯屽€�
        if (lock.hashPassword(pwd) == targetHash)
        {
            foundPassword = pwd;
            return true; // 鎵惧埌瀵嗙爜锛岃繑鍥瀟rue浠ョ粓姝㈡悳绱�
        }

        return false; // 鍝堝笇涓嶅尮閰嶏紝缁х画鎼滅储
    }

    // 閫掑綊姝ラ
    if (needPrimeUnique)
    {
        for (int d : primes)
        {
            if (used[d])
                continue;
            if (posFixed[idx] != -1 && posFixed[idx] != d)
                continue;
            if (posParity[idx] != -1 && d % 2 != posParity[idx])
                continue;

            used[d] = true;
            cur.push_back(d);
            if (findPasswordDfs(idx + 1, targetHash, lock, decrypt_count, foundPassword))
            {
                return true; // 濡傛灉瀛愭悳绱㈡壘鍒板瘑鐮侊紝绔嬪嵆杩斿洖
            }
            cur.pop_back();
            used[d] = false;
        }
    }
    else
    {
        for (int d = 0; d <= 9; ++d)
        {
            if (posFixed[idx] != -1 && posFixed[idx] != d)
                continue;
            if (posParity[idx] != -1 && d % 2 != posParity[idx])
                continue;

            cur.push_back(d);
            if (findPasswordDfs(idx + 1, targetHash, lock, decrypt_count, foundPassword))
            {
                return true; // 濡傛灉瀛愭悳绱㈡壘鍒板瘑鐮侊紝绔嬪嵆杩斿洖
            }
            cur.pop_back();
        }
    }

    return false; // 鍦ㄨ繖涓垎鏀笅娌℃湁鎵惧埌瀵嗙爜
}

std::pair<int, string> get_crack_info(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    string password; // 密码
    QString dirPath = "./password_test";
    // QString dirPath = "../vscode_algo";
    QDir directory(dirPath);

    if (!directory.exists())
    {
        cerr << "Directory not found: " << dirPath.toStdString() << endl;
        return crack_result;
    }

    QStringList jsonFiles = directory.entryList(QStringList() << "*.json", QDir::Files);
    long long total_decrypt_count = 0;

    for (const QString &fileName : jsonFiles)
    {
        QString jsonPath = directory.filePath(fileName);

        // --- 1. 重置每个文件的状态 ---
        posFixed.assign(3, -1);
        posParity.assign(3, -1);
        needPrimeUnique = false;
        cur.clear();
        fill(begin(used), end(used), false);

        // --- 2. 读取和解析JSON文件 ---
        QFile jsonFile(jsonPath);
        if (!jsonFile.open(QIODevice::ReadOnly))
        {
            cerr << "Failed to open JSON file for reading: " << jsonPath.toStdString() << endl;
            continue;
        }
        QByteArray jsonData = jsonFile.readAll();
        jsonFile.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
        if (parseError.error != QJsonParseError::NoError)
        {
            cerr << "JSON parse error in " << jsonPath.toStdString() << ": " << parseError.errorString().toStdString() << endl;
            continue;
        }
        QJsonObject rootObj = doc.object();

        // --- 3. 处理来自"C"数组的线索 ---
        QJsonArray clues = rootObj.value("C").toArray();
        for (const QJsonValue &val : clues)
        {
            QJsonArray arr = val.toArray();
            vector<int> v;
            for (const QJsonValue &elem : arr)
                v.push_back(elem.toInt());
            if (v.size() == 2)
            {
                if (v[0] == -1 && v[1] == -1)
                    needPrimeUnique = true;
                else
                    posParity[v[0] - 1] = v[1];
            }
            else if (v.size() == 3)
            {
                for (int j = 0; j < 3; ++j)
                    if (v[j] != -1)
                    {
                        posFixed[j] = v[j];
                        break;
                    }
            }
        }

        // --- 4. 搜索密码并精确计数 ---
        string inputHash = rootObj.value("L").toString().toStdString();
        int decrypt_count = 0;
        string foundPassword = "";
        PasswordLock lock;

        findPasswordDfs(0, inputHash, lock, decrypt_count, foundPassword);

        total_decrypt_count += decrypt_count;

        // --- 5. 查找并保存结果 ---
        if (!foundPassword.empty())
        {
            std::cout << "File: " << left << setw(15) << fileName.toStdString()
                      << " -> Password found: " << foundPassword
                      << " (Attempts: " << decrypt_count << ")" << endl;

            rootObj.insert("P", QString::fromStdString(foundPassword));
            rootObj.insert("D", decrypt_count);
            password = foundPassword;
            QJsonDocument newDoc(rootObj);
            if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                cerr << "  [ERROR] Failed to open file for writing: " << jsonPath.toStdString() << endl;
            }
            else
            {
                jsonFile.write(newDoc.toJson(QJsonDocument::Indented));
                jsonFile.close();
            }
        }
        else
        {
            cout << "File: " << left << setw(15) << fileName.toStdString()
                 << " -> Password NOT found. (Total attempts in space: " << decrypt_count << ")" << endl;
        }
    }

    // --- 6. 打印最终总数 ---
    cout << "\n================================================" << endl;
    cout << "  Total decryption attempts for all files: " << total_decrypt_count << endl;
    cout << "================================================" << endl;
    crack_result = {total_decrypt_count, password};
    return crack_result;
}