#ifndef MY_ENCRYPT_ITERATOR_H
#define MY_ENCRYPT_ITERATOR_H

#include <iterator>
using namespace std;
template<typename Container> class myEncryptInserter : public iterator<output_iterator_tag, void, void, void, void>{
private:
    Container& container_;
    typedef typename Container::value_type type_of_value;
    bool registerValues_[32] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
                    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1};
    short links_[7] = {0, 24, 26, 28, 29, 30, 31};

    char generatorPsp(){
        static short counter = 0;
        static char buffer = 0;
        buffer = (counter == 0) ? 0 : buffer;
        short res = 0;
        char exitCode;
        for(int e = 0; e < 7; e++)
            res += registerValues_[links_[e]];
        for(int h = 31; h > 0; h--)
            registerValues_[h] = registerValues_[h - 1];
        exitCode = (res % 2) == true ? false : true;
        registerValues_[0] = exitCode;
        buffer |= short(exitCode);
        buffer <<= 1;
        counter++;
        if(counter == 7){
            counter = 0;
            return buffer;
        }
    }
public:
    myEncryptInserter(Container& container) : container_(container){}
    myEncryptInserter();
    myEncryptInserter<Container>& operator = (const type_of_value& value){
        container_.push_back(value ^ generatorPsp());
        return *this;
    }
    myEncryptInserter<Container>& operator * (){
        return *this;
    }
    myEncryptInserter<Container>& operator ++ (){
        return *this;
    }
};

#endif // MY_ENCRYPT_ITERATOR_H
