#ifndef HANDLER_H
#define HANDLER_H

#include <thread>
#include <memory>
#include <mutex>
#include <chrono>
#include <future>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <list>
#include <functional>
#include <array>
#include <string>
#include <algorithm>
#include "my_encrypt_iterator.h"
#include "my_queue.h"

using namespace std;

#define BUFFER_SIZE 10000 // размер одного блока информации, помещаемого в очередь

struct node;
struct mycompare;
struct buffer;

extern vector<char> BUF;                               // буфер, куда запишем текст из файла

class handler{
public:
    handler(string file_path);
    ~handler();
    void start();                                  // функция, которая запускает обработчик
private:
    string filePathForHandler_;
    string filePathForHaffman_;
    string filePathForEncrypt_;
    queue <shared_ptr<buffer>> queue_;                                          // очередь, куда будем помещать прочитанные блоки
    mutex mutex_;                                                               // мьютекс для защиты
    unordered_map<char, size_t> map_;                                      // ассоциативный массив, который хранит пары (символ, количество повторений в тексте)
    unordered_map<char, vector<bool>> table_;                              // ассоциативный массив, который хранит пары (символ, кодированное представление символа)
    bool indEnd_;                                                          // индикатор того, что читающий поток отработал
    promise<void> myPromise_;
    shared_future<void> myFuture_;
    vector<char> encryptBuf_;                                 // буфер для хранения зашифрованного текста
private:
    void buildTree();                                                      // функция, которая строит дерево Хаффмана
    void buildTable(node* _hnode, vector<bool> v);                         // функция, которая формирует кодированные представления символов
    void compress();                                                        // функция, которая сжимает информацию согласно выведенным кодам
    void isfOrder(node* _hnode);                                            // исфиксный обход дерева - для удаления указателей
private:
    void prepareFilepathAndFile();
    void archive();
    void writeEncryptText();
    /* Функция первого потока*/
    void createChunkStream1();                      // функция, которая добавляет в очередь блоки информации на обработку
    /* Функция второго потока*/
    void referenceToTheQueueStream2();            // функция, которая забирает из очереди блоки информации для обработки
    /* Функция третьего потока*/
    void referenceToTheQueueStream3();            // функция, которая забирает из очереди блоки информации для обработки
    void collectStatistic(shared_ptr<buffer> );   // функция, которая собирает статистику для построения дерева Хаффмана
    void encrypt(shared_ptr<buffer> );             // функция, которая пишет зашифрованный текст в файл
    void showCodeCombination() noexcept;         // функция, которая отображает кодированные представления символов
};

struct node{
    node* left_;
    node* right_;
    unsigned int ui_;                 // вероятность появления символа
    char ch_;                         // какой символ в узле
    node(unsigned int ui = 0, char ch = ' ') : ui_(ui), ch_(ch), left_(nullptr), right_(nullptr){}
    node(node* p1, node* p2, unsigned int ui) : left_(p1), right_(p2), ui_(ui){}
};

struct mycompare{
    bool operator ()(node* p1, node* p2){
        return p1->ui_ < p2->ui_;
    }
};

struct buffer{                                                          // класс, который представляет блоки данных, считанных из файла
    bool ind1_;                                                          // индикатор для первого потока (если поток работал с блоком, то true)
    bool ind2_;                                                          // индикатор для второго потока                                                    //здесь будет храниться сама информация
    bool lastChunk_;                                                    // индикатор крайнего блока
    pair<vector<char>::iterator, vector<char>::iterator> mypair_;        // границы буфера в общем буфере(BUF)
    buffer():ind1_(false), ind2_(false), lastChunk_(false){
        static unsigned int acc = 0;
        mypair_.first = BUF.begin() + acc;
        if(BUF.begin() + (acc + BUFFER_SIZE) >= BUF.end())
            mypair_.second = BUF.end();
        else
            mypair_.second = mypair_.first + BUFFER_SIZE;
        acc += BUFFER_SIZE;
        if(acc >= BUF.size())
            lastChunk_ = true;
    }
};
#endif // HANDLER_H
