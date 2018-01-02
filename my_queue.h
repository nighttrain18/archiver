#ifndef MY_QUEUE_H
#define MY_QUEUE_H

template <typename type> struct listnode
{
  listnode<type>* previous_; //указатель на предыдущий узел
  type value_; //содержимое узла
};
template<typename type> class queue
{
    listnode<type>* pFirst_; //указатель на последний элемент очереди
    listnode<type>* pLast_; //указатель на первый элемент очереди
    int size_;
public:
    queue(){pFirst_ = 0; pLast_ = 0; size_ = 0;}
    void push(const type v); //добавляет элемент в конец списка
    type pop(); //удаляет и возвращает элемент из начала списка
    type& peek(); //возвращает элемент из начала списка
    bool empty();
    int getSize(){
        return size_;
    }
};
template<typename type> bool queue<type>::empty(){
    if(pFirst_ == 0 && pLast_ == 0)
        return true;
    return false;
}
template <typename type> void queue<type>::push(const type v){
    listnode<type>* node = new listnode<type>;
    node->value_ = v;
    if(pFirst_ == 0){
        pFirst_ = node;
        pLast_ = node;
        size_++;
        return;
    }
    pFirst_->previous_ = node;
    node->previous_ = 0;
    pFirst_ = node;
    size_++;
}
template <typename type> type queue<type>::pop(){
    if(pFirst_ == 0)
        return 0;
    type var;
    if(pFirst_ == pLast_){
        var = pFirst_->value_;
        delete pFirst_;
        pFirst_ = 0;
        pLast_ = 0;
        size_--;
        return var;
    }
    listnode<type>* temp;
    temp = pLast_;
    var = temp->value_;
    pLast_ = pLast_->previous_;
    delete temp;
    size_--;
    return var;
}
template <typename type> type& queue<type>::peek(){
    if(pLast_ != 0)
        return pLast_->value_;
}

#endif // MY_QUEUE_H
