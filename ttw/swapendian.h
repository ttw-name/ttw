#ifndef __TTW_SWAPENDIAN_H
#define __TTW_SWAPENDIAN_H

#include <stdint.h>
#include <endian.h>
namespace ttw {


    template <typename T>
    T swapEndian(T num){
        T result = 0;
        for (size_t i = 0; i < sizeof(T); ++i)
        {
            result |= ((num >> (i * 8)) & 0xFF) << ((sizeof(T) - 1 - i) * 8);
        }
        return result;
    }


#if BYTE_ORDER == BIG_ENDIAN
    template <typename T>
    T bytesOnBigSwapEndin(T t){
        return t;
    }
#else
    template <typename T>
    T bytesOnBigSwapEndin(T t){
        return swapEndian(t);
    }
#endif
    
}



#endif