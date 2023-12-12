#include "receiver.h"
#include <vector>
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

struct deal{
    std::vector<std::string> stock;
    std::vector<int> quantity;
    std::vector<std::pair<int, std::string>> pairedVector;
    int price;
    std::string choice;
};

void bubbleSort(std::vector<std::pair<int, std::string>>& vec) {
    int n = vec.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (vec[j].second > vec[j + 1].second) {
                std::swap(vec[j], vec[j + 1]);
            }
        }
    }
}

deal parseStocks(std::string line){
    deal temp;
    int startPos = 0;
    int endPos;
    while ((endPos = line.find(' ', startPos)) != std::string::npos || (endPos = line.find('#', startPos)) != std::string::npos) {
        std::string element = line.substr(startPos, endPos - startPos);
        if(element == "b" || element == "s"){
            temp.choice = element;
        }
        else if((isdigit(element[0]) || (element[0]=='-' && isdigit(element[1]))) && (line[endPos+1] == 'b' || line[endPos+1] == 's')){
            temp.price = std::stoi(element);
        }
        else if(isdigit(element[0]) || (element[0]=='-' && isdigit(element[1]))){
            int int_quantity = std::stoi(element);
            temp.quantity.push_back(int_quantity);
        }
        else{
            temp.stock.push_back(element);
        }
        startPos = endPos + 1;
    }
    for (size_t i = 0; i < temp.stock.size(); ++i) {
        temp.pairedVector.push_back(std::make_pair(temp.quantity[i], temp.stock[i]));
    }
    bubbleSort(temp.pairedVector);
    return temp;
}

std::vector<deal>active_deals;
std::vector<deal> all_deals;

int max_price=-2147483648; //stores max price among all arbitrages for a certain n reset to MIN_INT after every iteration in n i.e. number of deals
std::string max_order=""; //stores the deals which gave the max price arbitrage same for this
bool foundArbitrage = false;
int final_price = 0;

void checkArbitrage(int binary_arr[] , int n){
    map <int> temp;
    bool isArbitrage = true;
    int total_price=0;
    int count = 0;
    std::vector<std::string> all_stock; //all stocks in a case
    for(int i=0;i<n;i++){
        if(binary_arr[i]==1){
            for(int k=0;k<active_deals[i].stock.size();k++){
                all_stock.push_back(active_deals[i].stock[k]);
            }
        }
    }
    for(int i=0;i<n;i++){
        if(binary_arr[i]==1){
            count++;
            if(active_deals[i].choice == "s"){
                for(int k=0;k<active_deals[i].stock.size();k++){
                    int temp_val = temp.value(active_deals[i].stock[k]);
                    temp.insert(active_deals[i].stock[k],active_deals[i].quantity[k]+temp_val);
                }
                total_price = total_price - active_deals[i].price;
            }
            else{ 
                for(int k=0;k<active_deals[i].stock.size();k++){
                    int temp_val = temp.value(active_deals[i].stock[k]);
                    temp.insert(active_deals[i].stock[k],temp_val-active_deals[i].quantity[k]);
                }
                total_price = total_price + active_deals[i].price;
            }
        }
    }
    for(auto itr = all_stock.begin();itr!=all_stock.end();itr++){
        if(temp.value(*itr)!=0){
            isArbitrage = false;
            break;
        }
    }
    if(isArbitrage && (count>1)){
        if(total_price>max_price){
            max_price = total_price;
            std::string arb_str;
            for (int i = 0; i < n; ++i) {
                arb_str += std::to_string(binary_arr[i]);
            }
            max_order = arb_str;
            foundArbitrage = true;
        }
    }
    all_stock.clear();
}

void findArbitrage(int n,int arr[],int i){ //n is size of vector
    if (i == n) {
        checkArbitrage(arr, n);
        return;
    }
    arr[i] = 0;
    findArbitrage(n, arr, i + 1);
    arr[i] = 1;
    findArbitrage(n, arr, i + 1);
}


int main(int argc, char*argv[]){
    if(argv[1][0]=='1'){
    Receiver rcv;
    std::string message = rcv.readIML();
    std::string message1="bincho";
    while(message1.length()!=0){
        message1 = rcv.readIML();
        message+=message1;
    }
    map <int> last_trade; //last traded price for that stock
    map <int> sell; // stores active orders available to sell
    map <int> buy; // stores active orders available to buy

    int i=0;
    std::string stock = "";
    int price=0;
    std::string choice="";
    price=0;
    int counter_of_row = 0;
    while(message[i]!='$'){
        bool flag = false;
        while(message[i]!=' '){
            stock+=message[i];
            i++;
        }
        i++; //to move forward from white space

        int num_digit = 0;
        int counter = i;
        if(message[counter]=='-'){
            flag = true;
            counter++;
            i++;
        }
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
        if(flag){
            price = -1*price;
        }
        i++; 
        choice=(message[i] == 'b' ? "s" : "b");  
        i+=3;
        int last_price = last_trade.value(stock);
        
        if(last_price!=0){
            bool isValid = true;
            if(choice == "s" && sell.value(stock)!=0){ // checking for active buy orders
                if(price>sell.value(stock)){
                    sell.insert(stock,price);
                }
                else{
                    isValid = false;
                }
            }
            else if(choice == "s" && sell.value(stock)==0){
                sell.insert(stock,price);
            }
            else if(choice=="b" && buy.value(stock)==0){
                buy.insert(stock,price);
            }
            else if(choice=="b" && buy.value(stock)!=0){ // checking for active sell orders
                if(price<buy.value(stock)){
                    buy.insert(stock,price);
                }
                else{
                    isValid = false;
                }
            }
            if(sell.value(stock)!=0 && buy.value(stock)!=0 && isValid){ // checking for cancellation
                if(sell.value(stock)==buy.value(stock)){
                    isValid = false;
                    sell.insert(stock,0);
                    buy.insert(stock,0);
                }
            }
            if(isValid){ 
                if(choice == "s"){ // selling a stock
                    if(last_price<price){ //sell
                        std::cout<<stock<<" "<<price<<" "<<choice<<std::endl;
                        last_trade.insert(stock,price);
                        sell.insert(stock,0);
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
                        buy.insert(stock,0);
                    }
                }
            }
            else{
                std::cout<<"No Trade"<<std::endl;
            }
        }
        else{
            last_trade.insert(stock,price);
            std::cout<<stock<<" "<<price<<" "<<choice<<std::endl;
        }
        stock=choice="";
        price=0;
        mult_fac=1;
    }
    }

    else if(argv[1][0]=='2'){
    Receiver rcv;
    std::string message = rcv.readIML();
    std::string message1="bincho";
    while(message1.length()!=0){
        message1 = rcv.readIML();
        message+=message1;
    }

    int startPos = 0;
    int endPos;
    char new_line = static_cast<char>(13);
    while ((endPos = message.find(new_line, startPos)) != std::string::npos || (endPos = message.find('$', startPos)) != std::string::npos) {
        std::string line = message.substr(startPos, endPos - startPos);
        if(line.length()>0){
            deal new_deal = parseStocks(line);
            all_deals.push_back(new_deal);
        } 
        startPos = endPos + 1;
    }
    for(auto itr = all_deals.begin();itr!=all_deals.end();itr++){
        bool isValid = true;
        if(!active_deals.empty()){
        // active_deals.push_back(*itr);
        int size_active = active_deals.size();
        for(int it = 0;it<size_active;it++){
            bool sameDeal = false;
            if(itr->pairedVector.size() == active_deals[it].pairedVector.size()){
                for (int i = 0; i < active_deals[it].pairedVector.size() && i < itr->pairedVector.size(); i++) {
                    if ((active_deals[it].pairedVector[i].second == itr->pairedVector[i].second) && (active_deals[it].pairedVector[i].first == itr->pairedVector[i].first)) {
                        sameDeal = true;
                    } else {
                        sameDeal = false;
                        break;
                    }
                }
            }
            else{
                sameDeal = false;
            }
            if(sameDeal){
                if(active_deals[it].choice == itr->choice){
                    if(active_deals[it].choice == "s"){
                        if(active_deals[it].price <= itr->price){
                            // active_deals.pop_back();
                            isValid = false;
                        }
                        else{
                            active_deals.erase(active_deals.begin()+it);
                        }
                    }
                    else{
                        if(active_deals[it].price >= itr->price){
                            // active_deals.pop_back();
                            isValid = false;
                        }
                        else{
                            active_deals.erase(active_deals.begin()+it);
                        }
                    }
                }
                if(isValid && active_deals[it].choice != itr->choice){
                    if(active_deals[it].price == itr->price){
                        // active_deals.pop_back();
                        active_deals.erase(active_deals.begin()+it);
                        isValid = false;
                    }
                }
            }
        }
            
        if(isValid){
            active_deals.push_back(*itr);
        }
        }
        else{
            active_deals.push_back(*itr);
        }
        if(isValid){
            int binary_array[active_deals.size()];
            findArbitrage(active_deals.size(),binary_array,0);
            if(foundArbitrage && max_price>0){
                for(int i=max_order.length()-1;i>=0;i--){
                    if(max_order[i]=='1'){
                        for(int k=0;k<active_deals[i].stock.size();k++){
                            std::cout<<active_deals[i].stock[k]<<" "<<active_deals[i].quantity[k]<<" ";
                        }
                        std::cout<<active_deals[i].price<<" ";
                        std::cout<<(active_deals[i].choice == "b" ? "s" : "b")<<std::endl;
                        active_deals.erase(active_deals.begin() + i);
                    }
                }
                final_price+=max_price;
                max_price = -2147483648;
                max_order = "";
                foundArbitrage = false;
            }
            else{
                std::cout<<"No Trade"<<std::endl;
            }
        }
        else{
            std::cout<<"No Trade"<<std::endl;
        }
    }
    std::cout<<final_price<<std::endl;

    }
    else if(argv[1][0]=='3'){
        return 0;
    }
    return 0;
}