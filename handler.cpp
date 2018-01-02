#include "handler.h"
vector<char> BUF;
handler::handler(string file_path){
    filePathForHandler_ = file_path;
    fstream stream1(filePathForHandler_);
    stream1.seekg (0, ios::end);
    unsigned int sizeOfFile = stream1.tellg();
    stream1.close();
    BUF.reserve(sizeOfFile);
    fstream stream2(filePathForHandler_);
    istreambuf_iterator<char> myiterator(stream2);
    istreambuf_iterator<char> myiteratorEOF;
    while(myiterator != myiteratorEOF){
        BUF.push_back(*myiterator);
        ++myiterator;
    }
    stream2.close();
    indEnd_ = false;
    myFuture_ = myPromise_.get_future();
    encryptBuf_.reserve(BUF.size());
}
handler::~handler(){}
void handler::start(){
    prepareFilepathAndFile();
    auto future1 = async(launch::async, &handler::createChunkStream1, this);
    auto future2 = async(launch::async, &handler::referenceToTheQueueStream2, this);
    auto future3 = async(launch::async, &handler::referenceToTheQueueStream3, this);
    future1.wait();
    future2.wait();
    future3.wait();
    archive();
    showCodeCombination();
}
void handler::writeEncryptText(){
    ofstream stream(filePathForEncrypt_);
    copy(encryptBuf_.begin(), encryptBuf_.end(),ostreambuf_iterator<char>(stream));
    stream.close();
}
void handler::createChunkStream1(){
    unique_lock<mutex> u_l(mutex_, defer_lock);                     // готовим захват мьютекса для совместной работы с очередью
    once_flag flag; bool indicator = false;
    while(!indicator){
        shared_ptr<buffer> sp(new buffer);                     // формируем очередной буфер
        u_l.lock();                                            // блокируем мьютекс
        if(sp->lastChunk_){
            indEnd_ = true; indicator = true;
        }
        queue_.push(move(sp));                                      // заносим буфер в очередь
        u_l.unlock();                                          // освобождаем очередь
        call_once(flag, [this](){myPromise_.set_value();});
    }
}
void handler::referenceToTheQueueStream2(){
    myFuture_.wait();                              // ожидаем, когда поток create_chunk отработает первый раз
    unique_lock<mutex> u_l(mutex_, defer_lock);
    while(true){
        u_l.lock();                               // блокируем мьютекс
        if(queue_.empty()){                            // если очередь пустая
            if(indEnd_ == true){                   // если поток create_chunk have already finished
                u_l.unlock();                       // освобождаем очередь, выходим из цикла
                break;
            }
            else{                                  // иначе, просто освобождаем очередь. Поток create_chunk еще не закончил работу
                u_l.unlock();
                continue;
            }
        }
        else{                                     // если очередь непустая
            if(queue_.peek()->ind1_ == false){           // проверяем, работали ли мы из этого потока с chunk, который первый на извлечение из очереди
                shared_ptr<buffer> sp;              // не работали, то работаем)
                if(queue_.peek()->ind2_ == true){          // если с этим блоком уже отработал поток reference_to_the_queue2, то смело его перемещаем и удаляем
                    sp = move(queue_.peek());
                    queue_.pop();
                }
                else{                                // если поток reference_to_the_queue2 не работал с блоком, то копируем
                    sp = queue_.peek();
                    queue_.peek()->ind1_ = true;           // ставим отметку, что из текущего потока мы с блоком отработали
                }
                u_l.unlock();                       // освобождаем очередь
                collectStatistic(move(sp));        // обрабатываем полученный блок
                continue;
            }
            if(queue_.peek()->ind1_ == true){           // если из текущего потока с блоком работали, то просто освобождаем очередь
                u_l.unlock();
            }
        }
    }
}
void handler::referenceToTheQueueStream3(){
    myFuture_.wait();
    unique_lock<mutex> u_l(mutex_, defer_lock);
    while(true){
        u_l.lock();
        if(queue_.empty()){
            if(indEnd_ == true){
                u_l.unlock();
                thread thr(&handler::writeEncryptText, this);  // если поток create_chunk отработал, то запускаем в отдельном потоке запись зашифрованного текста в файл
                thr.detach();
                break;
            }
            else{
                u_l.unlock();
                continue;
            }
        }
        else{
            if(queue_.peek()->ind2_ == false){
                shared_ptr<buffer> sp;
                if(queue_.peek()->ind1_ == true){
                    sp = move(queue_.peek());
                    queue_.pop();
                }
                else{
                    sp = queue_.peek();
                    queue_.peek()->ind2_ = true;
                }
                u_l.unlock();
                encrypt(move(sp));
                continue;
            }
            if(queue_.peek()->ind2_ == true){
                u_l.unlock();
            }
        }
    }
}
void handler::collectStatistic(shared_ptr<buffer> s){
    while(s->mypair_.first != s->mypair_.second){
        map_[*s->mypair_.first] += 1;
        ++s->mypair_.first;
    }
}
void handler::encrypt(shared_ptr<buffer> s){
    copy(s->mypair_.first, s->mypair_.second, myEncryptInserter<vector<char>>(encryptBuf_));
}
void handler::buildTree(){
    list<node*> l;
    for(auto it = map_.begin(); it != map_.end(); ++it){
        node* p = new node(it->second, it->first);
        l.push_back(p);
    }
    while(l.size() != 1){
        l.sort(mycompare());
        node* p1 = l.front();
        l.pop_front();
        node* p2 = l.front();
        l.pop_front();
        l.push_front(new node(p1, p2, p1->ui_ + p2->ui_));
    }
    node* hnode = l.front();
    l.pop_front();
    vector<bool> v;
    buildTable(hnode, v);
    isfOrder(hnode);
}
void handler::isfOrder(node* hnode){
    if(hnode->left_ != nullptr)
        isfOrder(hnode->left_);
    if(hnode->right_ != nullptr)
        isfOrder(hnode->right_);
    delete hnode;
}

void handler::buildTable(node* hnode, vector<bool> v){
    if(hnode->right_ != nullptr){
        v.push_back(0);
        buildTable(hnode->right_, v);
        v.pop_back();
    }
    if(hnode->left_ != nullptr){
        v.push_back(1);
        buildTable(hnode->left_, v);
    }
    if(hnode->left_ == nullptr && hnode->right_ == nullptr)
        table_[hnode->ch_] = v;
}
void handler::compress(){
    ofstream stream(filePathForHaffman_);
    ostreambuf_iterator<char> myiterator(stream);
    int count = 0;
    char buffer = 0;
    for(size_t i = 0; i < BUF.size(); i++){
        auto tableVector = table_[BUF[i]];
        for(int j = 0; j < tableVector.size(); j++){
            if(count == 7){
                buffer |= tableVector[j];
                *myiterator = buffer;
                ++myiterator;
                buffer = 0;
                count = 0;
                continue;
            }
            else{
                buffer |= tableVector[j];
                buffer <<= 1;
                count++;
            }
        }
    }
    buffer <<= (7 - count);
    *myiterator = buffer;
    stream.close();
}
void handler::archive(){
    buildTree();          // строим дерево Хаффмана
    compress();            // кодируем символы согласно построенному дереву
    cout << " - Compression completed " << endl;
}
void handler::showCodeCombination() noexcept{
    cout << " - Code combination for symbol " << endl;
    for(auto it = table_.begin(); it != table_.end(); ++it){
        cout << it->first << " = ";
        for(size_t i = 0; i < it->second.size(); i++)
            cout << it->second[i];
        cout << endl;
    }
}
void handler::prepareFilepathAndFile(){
    string::reverse_iterator rpos = find(filePathForHandler_.rbegin()++, filePathForHandler_.rend(), '/');
    auto pos = rpos.base();
    copy(filePathForHandler_.begin(), pos--, back_inserter(filePathForHaffman_));
    filePathForEncrypt_ = filePathForHaffman_;
    filePathForEncrypt_ += "additional_scrembling_text.txt";
    filePathForHaffman_ += "haffman_compress_text.txt";
}
