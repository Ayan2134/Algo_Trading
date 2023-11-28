#include <vector>
#include <iostream>
#include <string>

#define map_size 100000

template <typename T>
class map {
    public :
    
    int murmurhash2(const char* buf, int len, int seed){
        const int m = 1540483477;
        int hash = seed ^ len ;
        while ( len >= 4) { // Mix 4 bytes at a time into the hash .
        int k = *(( const int *) buf );
        k *= m ; k ^= k >> 24; k *= m ;
        hash *= m ; hash ^= k ;
        buf += 4; len -= 4;
        }
        int k ;
        switch ( len ) { // Handle the last few bytes of the input array .
        case 3: k = buf [2]; hash ^= k << 16;
        case 2: k = buf [1]; hash ^= k << 8;
        case 1: k = buf [0]; hash ^= k ; hash *= m ;
        };
        hash ^= hash >> 13; hash *= m ; hash ^= hash >> 15; // Do final mixes .
        return hash ;
    }

    int compress(int e){
        double a = 0.6180339887498949;
        double prod = a*e;
        double frac_part = prod - int(prod);
        int compressed_value = int(float(map_size)*frac_part);
        return compressed_value;
    }

    void insert(std::string key, T data){ 
        int len = key.length();
        const char* char_key = key.c_str();
        int hash = murmurhash2(char_key,len,100); //setting seed as 100
        int index = compress(hash);
        table[index] = data;
        size++;
    }

    T value(std::string key){
        int len = key.length();
        const char* char_key = key.c_str();
        int hash = murmurhash2(char_key,len,100); //setting seed as 100
        int index = compress(hash);
        return table[index];
    }

    private :
    T table[map_size] = {};
    int size=0;
};