#include "box.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
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

void sub_bytes(vector<vector<unsigned char>>& matrix) {
    for(int i = 0; i < 4; i++) {    // Заполнение матрицы
        for(int j = 0; j < 4; j++) {
            // Используем значение байта напрямую как индекс в sBox
            matrix[i][j] = sBox[matrix[i][j] / 16][matrix[i][j] % 16];
        }
    }
}

void shiftRows(vector<vector<unsigned char>>& state) {
    for (int i = 1; i < 4; ++i) {
        rotate(state[i].begin(), state[i].begin() + i, state[i].end());
    }
}

void mixColumns(vector<vector<unsigned char>>& state) {
    for (int i = 0; i < 4; ++i) {
        unsigned char a[4];
        unsigned char b[4];
        unsigned char h;

        for (int c = 0; c < 4; ++c) {
            a[c] = state[c][i];
            h = (unsigned char)((signed char)state[c][i] >> 7);
            b[c] = state[c][i] << 1;
            b[c] ^= 0x1B & h;
        }

        state[0][i] = b[0] ^ a[1] ^ b[1] ^ a[2] ^ a[3];
        state[1][i] = a[0] ^ b[1] ^ a[2] ^ b[2] ^ a[3];
        state[2][i] = a[0] ^ a[1] ^ b[2] ^ a[3] ^ b[3];
        state[3][i] = a[0] ^ b[0] ^ a[1] ^ a[2] ^ b[3];
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

void encrypted() {
    vector<unsigned char> key;
    vector<unsigned char> message;

    init_str(key);
    init_str(message);

    vector<vector<unsigned char>> key_matrix;
    vector<vector<unsigned char>> message_matrix;

    init_matrix(key, key_matrix);
    init_matrix(message, message_matrix);

    vector<vector<unsigned char>> key_exp = key_expansion(key_matrix, 0);

    transpose_matrix(key_exp);
    transpose_matrix(key_matrix);
    transpose_matrix(message_matrix);

    vector<vector<unsigned char>> result_matrix = xor_matrices(key_matrix, message_matrix);

    for(int i = 1; i < 10; i++) {
        sub_bytes(result_matrix);
        shiftRows(result_matrix);
        mixColumns(result_matrix);

        result_matrix = xor_matrices(result_matrix, key_exp);
        
        transpose_matrix(key_exp);

        key_exp = key_expansion(key_exp, i);

        transpose_matrix(key_exp);
    }

    sub_bytes(result_matrix);
    shiftRows(result_matrix);

    result_matrix = xor_matrices(result_matrix, key_exp);

    transpose_matrix(result_matrix);

    vector<unsigned char> ciphertext;

    for(const auto& row :  result_matrix) {
        for(const unsigned char ch : row) {
            ciphertext.push_back(ch);
        }
    }

    ofstream outputFile("ciphertext.txt");
    if (outputFile.is_open()) {
        for (unsigned char ch : ciphertext) {
            outputFile << ch;
        }

        outputFile.close();
        cout << "Зашифрованный текст успешно записан в файл ciphertext.txt" << endl;
    }
    
    else {
        cout << "Не удалось открыть файл для записи" << endl;
    }
}

int main() {
    encrypted();
}