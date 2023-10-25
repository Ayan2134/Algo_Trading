#include "receiver.h"

using namespace std;

#define map_size 100000

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

    void insert(string key, int data){ 
        int len = key.length();
        const char* char_key = key.c_str();
        int hash = murmurhash2(char_key,len,100); //setting seed as 100
        int index = compress(hash);
        table[index] = data;
    }

    int value(string key){
        int len = key.length();
        const char* char_key = key.c_str();
        int hash = murmurhash2(char_key,len,100); //setting seed as 100
        int index = compress(hash);
        return table[index];
    }

    private :
    int table[map_size]={-1};
};

int main() {

    Receiver rcv;
    sleep(5);
    std::string message = rcv.readIML();

    map last_trade; //last traded price for that stock

    int i=0;
    string stock = "";
    int price=0;
    string choice="";
    price=0;
    while(message[i]!='$'){
        while(message[i]!=' '){
            stock+=message[i];
            i++;
        }
        i++; //to move forward from white space

        int num_digit = 0;
        int counter = i;
        while(message[counter]!=' '){ //calculating number of digits in price of stock
            num_digit++;
            counter++;
        }
        int mult_fac=1;
        while(num_digit>1){ //calculating the factor to multiply the digits of price
            mult_fac*=10;
            num_digit--;
        }

        while(message[i]!=' '){
            price+=(int(message[i])-48)*mult_fac;
            mult_fac/=10;
            i++;
        }
        i++; 
        choice=(message[i] == 'b' ? "s" : "b");  
        i+=3;
        int last_price = last_trade.value(stock);
        if(last_price!=0){
            if(choice == "s"){ // selling a stock
                if(last_price<price){ //sell
                    cout<<stock<<" "<<price<<" "<<choice<<endl;
                    last_trade.insert(stock,price);
                }
                else{ //No trade
                    cout<<"No Trade"<<endl;
                }
            }
            else{ // buying a stock
                if(last_price<=price){ //No trade
                    cout<<"No Trade"<<endl;
                }
                else{ //buy
                    cout<<stock<<" "<<price<<" "<<choice<<endl;
                    last_trade.insert(stock,price);
                }
            }
        }
        else{
            last_trade.insert(stock,price);
            cout<<stock<<" "<<price<<" "<<choice<<endl;
        }
        stock=choice="";
        price=0;
        mult_fac=1;
    }
    return 0;
}
