#ifndef _TTW_INSTANTIATION_H
#define _TTW_INSTANTIATION_H

#include<memory>
#include<map>
namespace ttw{

    template <class T>
    class Instantiation{
    public:
        static std::shared_ptr<T> GetInstantion(){
            
            
            std::shared_ptr<T> ret(new T); 
            return ret;
        }
    };

    
}
#endif
