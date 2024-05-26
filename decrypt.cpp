#include "box.h"

#include <algorithm>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

void init_str(vector<unsigned char>& str) {
    string in_str;

    cout << "(Строка должна быть не больше 16 символов): ";

    while (true) {
        getline(cin, in_str);

        if (in_str.length() > 16) {
            cout << "Строка больше чем 16 символов, попробуйте еще: ";
        }

        else {
            str.clear(); // Очищаем вектор перед добавлением новых данных
            str.reserve(in_str.size()); // Резервируем место для элементов
            for(char c : in_str) {
                str.push_back(static_cast<unsigned char>(c));
            }
            break;
        }
    }
}

void init_matrix(vector<unsigned char>& str, vector<vector<unsigned char>>& matrix) {
    matrix.clear();
    matrix.resize(4, vector<unsigned char>(4, 0)); // Инициализация матрицы 4x4 нулями

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int index = i * 4 + j;
            if (index < str.size()) {
                matrix[i][j] = str[index];
            }
        }
    }
}

void transpose_matrix(vector<vector<unsigned char>>& matrix) {
    int n = matrix.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            swap(matrix[i][j], matrix[j][i]);
        }
    }
}

vector<vector<unsigned char>> key_expansion(vector<vector<unsigned char>>& matrix, int t) {
    vector<vector<unsigned char>> exp_key;
    vector<vector<unsigned char>> state;

    for (const auto row : matrix) {
        exp_key.push_back(row);
        state.push_back(row);
    }

    rotate(state[3].begin(), state[3].begin() + 1, state[3].end());

    for (int i = 0; i < 4; i++) {
        state[3][i] = sBox[state[3][i] / 16][state[3][i] % 16];
        state[3][i] = state[3][i] ^ r_w[t][i];

        exp_key[0][i] = exp_key[0][i] ^ state[3][i];
        exp_key[1][i] = exp_key[0][i] ^ state[1][i];
        exp_key[2][i] = exp_key[1][i] ^ state[2][i];
        exp_key[3][i] = exp_key[2][i] ^ exp_key[3][i];

    }

    return exp_key;
}

vector<vector<unsigned char>> xor_matrices(const vector<vector<unsigned char>>& matrix1, const vector<vector<unsigned char>>& matrix2) {
    int n = matrix1.size();
    vector<vector<unsigned char>> result(n, vector<unsigned char>(n));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            result[i][j] = matrix1[i][j] ^ matrix2[i][j];
        }
    }

    return result;
}

void invSubBytes(vector<vector<unsigned char>>& matrix) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            matrix[i][j] = invsBox[matrix[i][j] / 16][matrix[i][j] % 16];
        }
    }
}

void invShiftRows(vector<vector<unsigned char>>& state) {
    for (int i = 1; i < 4; ++i) {
        rotate(state[i].rbegin(), state[i].rbegin() + i, state[i].rend());
    }
}

unsigned char gmul(unsigned char a, unsigned char b) {
    unsigned char p = 0;
    unsigned char counter;
    unsigned char hi_bit_set;
    for (counter = 0; counter < 8; counter++) {
        if (b & 1) {
            p ^= a;
        }
        hi_bit_set = (a & 0x80);
        a <<= 1;
        if (hi_bit_set) {
            a ^= 0x1b;
        }
        b >>= 1;
    }
    return p;
}

void invMixColumns(vector<vector<unsigned char>>& state) {
    for (int i = 0; i < 4; ++i) {
        unsigned char a[4];
        unsigned char b[4];

        for (int c = 0; c < 4; ++c) {
            a[c] = state[c][i];
        }

        b[0] = gmul(a[0], 0x0e) ^ gmul(a[1], 0x0b) ^ gmul(a[2], 0x0d) ^ gmul(a[3], 0x09);
        b[1] = gmul(a[0], 0x09) ^ gmul(a[1], 0x0e) ^ gmul(a[2], 0x0b) ^ gmul(a[3], 0x0d);
        b[2] = gmul(a[0], 0x0d) ^ gmul(a[1], 0x09) ^ gmul(a[2], 0x0e) ^ gmul(a[3], 0x0b);
        b[3] = gmul(a[0], 0x0b) ^ gmul(a[1], 0x0d) ^ gmul(a[2], 0x09) ^ gmul(a[3], 0x0e);

        for (int c = 0; c < 4; ++c) {
            state[c][i] = b[c];
        }
    }
}

void print_matrix_hex(const vector<vector<unsigned char>>& matrix) {
    for (const auto& row : matrix) {
        for (unsigned char value : row) {
            printf("%02X ", value);
        }
        cout << "\n";
    }

    cout << "\n\n";
}

void decrypt() {
    vector<unsigned char> key;
    vector<unsigned char> ciphertext;

    init_str(key);

    ifstream inputFile("ciphertext.txt", ios::binary);
    if (inputFile.is_open()) {
        unsigned char ch;
        while (inputFile.read(reinterpret_cast<char*>(&ch), sizeof(ch))) {
            ciphertext.push_back(ch);
        }
        inputFile.close();
    } else {
        cout << "Не удалось открыть файл для чтения" << endl;
        return;
    }

    vector<vector<unsigned char>> key_matrix;
    vector<vector<unsigned char>> ciphertext_matrix;

    init_matrix(key, key_matrix);
    init_matrix(ciphertext, ciphertext_matrix);

    print_matrix_hex(ciphertext_matrix);

    vector<vector<vector<unsigned char>>> round_key_exp;
    vector<vector<unsigned char>> firs_key_matrix = key_matrix;
    transpose_matrix(firs_key_matrix);

    round_key_exp.push_back(firs_key_matrix);
    transpose_matrix(firs_key_matrix);

    for(int i = 0; i < 10; i++) {
        vector<vector<unsigned char>> round = key_expansion(firs_key_matrix, i);
        transpose_matrix(round);
        round_key_exp.push_back(round);
        transpose_matrix(round);
        firs_key_matrix = round;
    }

    transpose_matrix(ciphertext_matrix);

    vector<vector<unsigned char>> result_matrix = xor_matrices(ciphertext_matrix, round_key_exp[10]);

    for(int i = 9; i > 0; i--) {
        invShiftRows(result_matrix);
        invSubBytes(result_matrix);
        result_matrix = xor_matrices(result_matrix, round_key_exp[i]);
        invMixColumns(result_matrix);
    }

    invShiftRows(result_matrix);
    invSubBytes(result_matrix);
    result_matrix = xor_matrices(result_matrix, round_key_exp[0]);
    transpose_matrix(result_matrix);

    print_matrix_hex(result_matrix);

    vector<unsigned char> plaintext;
    for(const auto& row :  result_matrix) {
        for(const unsigned char ch : row) {
            plaintext.push_back(ch);
        }
    }

    for(auto ch : plaintext){
        printf("%02X ", ch);
    }

    cout << 123 << endl;

    ofstream outputFile("decrypted_message.txt");
    if (outputFile.is_open()) {
        for (unsigned char ch : plaintext) {
            outputFile << ch;
        }
        outputFile.close();
        cout << "Расшифрованный текст успешно записан в файл decrypted_message.txt" << endl;
    } else {
        cout << "Не удалось открыть файл для записи" << endl;
    }
}

int main() {
    decrypt();

    return 0;
}