#include "receiver.h"
#include <vector>

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
};

int main() {

    Receiver rcv;
    sleep(5);
    std::string message = rcv.readIML();
    std::string message1="bincho";
    while(message1.length()!=0){
        sleep(5);
        message1 = rcv.readIML();
        message+=message1;
    }
    map <int> last_trade; //last traded price for that stock
    map <std::vector<int>*> sell; // stores active orders available to sell
    map <std::vector<int>*> buy; // stores active orders available to buy

    std::vector<std::vector<int>> buy_stock(10); //temp number of rows in vector
    std::vector<std::vector<int>> sell_stock(10); //temp number of rows in vector

    int i=0;
    std::string stock = "";
    int price=0;
    std::string choice="";
    price=0;
    int counter_of_row = 0;
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
        if(last_price == 0){ // if a new stock comes then make a new row in the 2d vector and insert the addresses of these 2 new rows in the map
            buy_stock.emplace_back();
            sell_stock.emplace_back();
            std::vector <int>* temp_buy = &buy_stock[counter_of_row];
            std::vector <int>* temp_sell = &sell_stock[counter_of_row];
            counter_of_row++;
            sell.insert(stock,temp_sell);
            buy.insert(stock,temp_buy);
        }
        // int last_price = last_trade.value(stock);
        if(choice == "s"){
            sell.value(stock)->push_back(price);
        }
        else{
            buy.value(stock)->push_back(price);
        }
        if(last_price!=0){
            bool isValid = true;

            if(choice == "s" && !sell.value(stock)->empty()){ // checking for active buy orders
                for(auto itr = sell.value(stock)->begin(); itr != sell.value(stock)->end(); ++itr){
                    if(*itr > sell.value(stock)->back()){
                        isValid = false;
                        break;
                    }
                }
            }
            else if(choice=="b" && !buy.value(stock)->empty()){ // checking for active sell orders
                for(auto itr = buy.value(stock)->begin(); itr != buy.value(stock)->end(); ++itr){
                    if(*itr < buy.value(stock)->back()){
                        isValid = false;
                        break;
                    }
                }
            }
            if(!sell.value(stock)->empty() && !buy.value(stock)->empty()){ // checking for cancellation
                if(choice == "s"){
                    for(auto itr = buy.value(stock)->begin(); itr != buy.value(stock)->end(); ++itr){
                        if(*itr == sell.value(stock)->back()){
                            isValid = false;
                            last_trade.insert(stock,sell.value(stock)->back());
                            sell.value(stock)->pop_back();
                            buy.value(stock)->erase(itr);
                            break;
                        }
                    }
                }
                else {
                    for(auto itr = sell.value(stock)->begin(); itr != sell.value(stock)->end(); ++itr){
                        if(*itr == buy.value(stock)->back()){
                            isValid = false;
                            last_trade.insert(stock,buy.value(stock)->back());
                            buy.value(stock)->pop_back();
                            sell.value(stock)->erase(itr);
                            break;
                        }
                    }
                }
            }
            if(isValid){ 
                if(choice == "s"){ // selling a stock
                    if(last_price<price){ //sell
                        std::cout<<stock<<" "<<price<<" "<<choice<<std::endl;
                        last_trade.insert(stock,price);
                        sell.value(stock)->pop_back();
                    }
                    else{ //No trade
                        std::cout<<"No Trade"<<std::endl;
                    }
                }
                else{ // buying a stock
                    if(last_price<=price){ //No trade
                        std::cout<<"No Trade"<<std::endl;
                    }
                    else{ //buy
                        std::cout<<stock<<" "<<price<<" "<<choice<<std::endl;
                        last_trade.insert(stock,price);
                        buy.value(stock)->pop_back();
                    }
                }
            }
            else{
                std::cout<<"No Trade"<<std::endl;
            }
        }
        else{
            last_trade.insert(stock,price);
            if(choice == "s"){
                sell.value(stock)->pop_back();
            }
            else{
                buy.value(stock)->pop_back();
            }
            std::cout<<stock<<" "<<price<<" "<<choice<<std::endl;
        }
        stock=choice="";
        price=0;
        mult_fac=1;
    }
    return 0;
}
