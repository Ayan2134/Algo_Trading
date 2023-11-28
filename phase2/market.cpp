#include "market.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "heap.h"
#include "map.h"

struct trade{
    int timestamp = -1; //initial value
    int expiry = -3; 
    int price;
    int quantity=-1;
    std::string choice;
    std::string broker = "null";
    std::string stock; 

    bool operator<(const trade& other) const {
        if (price == other.price) {
            if (timestamp == other.timestamp) {
                return stock < other.stock;
            }
            return timestamp > other.timestamp;  //the time at which they arrive less means more priority
        }
        return price < other.price;
    }

    bool operator>(const trade& other) const {
        return other < *this;
    }

    bool operator==(const trade& other) const {
        return (timestamp == other.timestamp) &&
               (price == other.price) &&
               (broker == other.broker)&&(choice == other.choice);
    }

    bool operator<=(const trade& other) const {
        return (*this < other) || (*this == other);
    }

    bool operator>=(const trade& other) const {
        return (*this > other) || (*this == other);
    }
};

std::vector<std::string> input;

market::market(int argc, char** argv)
{
	std::string line;
    std::ifstream file("output.txt");
    const char delim = ' ';

    if(file.is_open()){                                
        while(std::getline(file, line)){
            if(std::isdigit(line[0])){
                input.push_back(line);
            }
        }
        file.close();
    }
    return ;
}

void bubbleSort(std::vector<std::pair<int, std::string>>& vec) {
    int n = vec.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (vec[j].first > vec[j + 1].first) {
                std::swap(vec[j], vec[j + 1]);
            }
        }
    }
}

std::vector<std::string> brokers;
map <int> broker_bought;
map <int> broker_sold;
map <int> broker_net_money;

trade parseTrade(const std::string& line) {
    trade temp;
    bool flag = false;
    std::istringstream iss(line);
    std::string element;
    std::vector<std::pair<int, std::string>> pairedVector;
    std::vector<int> quantity;
    std::vector<std::string> stock;
    while (iss >> element) {
        if(isdigit(element[0]) && temp.timestamp == -1){
            temp.timestamp = std::stoi(element);  
        }
        else if(temp.broker == "null"){
            temp.broker = element;
            if(brokers.empty()){
                brokers.push_back(temp.broker);
                broker_bought.insert(temp.broker,0);
                broker_sold.insert(temp.broker,0);
                broker_net_money.insert(temp.broker,0);
            }
            else{
                bool flag = true;
                for(int i=0;i<brokers.size();i++){
                    if(temp.broker==brokers[i]){
                        flag = false;
                    }
                }
                if(flag){
                    brokers.push_back(temp.broker);
                    broker_bought.insert(temp.broker,0);
                    broker_sold.insert(temp.broker,0);
                    broker_net_money.insert(temp.broker,0);
                }
            }
        }
        else if(element == "SELL" || element == "BUY"){
            temp.choice = element;
        }
        else if(element[0]=='$'){
            temp.price = std::stoi(element.substr(1));
        }
        else if(element[0]=='#'){
            temp.quantity = std::stoi(element.substr(1));
        }
        else if((isdigit(element[0]) || element=="-1") && temp.quantity!=-1){
            temp.expiry = std::stoi(element);
        }
        else if((isdigit(element[0]) || (element[0]=='-' && isdigit(element[1])))){
            quantity.push_back(std::stoi(element));  //quantity of stock is exists in case of multiple stocks 
            flag = true;
        }
        else{
            stock.push_back(element);
        }
    }
    if(flag){
        std::string new_stock = "";
        for (int i = 0; i < stock.size(); ++i) {
            pairedVector.push_back(std::make_pair(quantity[i], stock[i]));
        }
        bubbleSort(pairedVector);
        for (const auto& pair : pairedVector) {
            new_stock+=pair.second;
            new_stock+=std::to_string(pair.first);
        }
        temp.stock = new_stock;
    }
    else{
        temp.stock = stock.back();
    }
    return temp;
}

map <heap<trade>*>buy_orders;
map <heap<trade>*>sell_orders;
int total_money = 0;
int complete_trades = 0;
int shares_traded = 0;

void insertBuy(trade temp){
    if(buy_orders.value(temp.stock)==nullptr){
        heap <trade>*h_buy = new heap<trade>;
        h_buy->insert(temp);
        buy_orders.insert(temp.stock,h_buy); //inserting heap of sell_orders of a trade in map if a heap for it is not already present
    }
    else{
        buy_orders.value(temp.stock)->insert(temp);
    } 
}
void insertSell(trade temp){
    temp.price = (-1)*std::abs(temp.price);
    if(sell_orders.value(temp.stock)==nullptr){
        heap <trade>*h_sell = new heap<trade>;
        h_sell->insert(temp);
        sell_orders.insert(temp.stock,h_sell); //inserting heap of sell_orders of a trade in map if a heap for it is not already present
    }
    else{
        sell_orders.value(temp.stock)->insert(temp);
    }
}

void sellTrade(trade temp){
    if(buy_orders.value(temp.stock)!=nullptr){
                while(((buy_orders.value(temp.stock)->max_val().timestamp + buy_orders.value(temp.stock)->max_val().expiry) < temp.timestamp)&&(buy_orders.value(temp.stock)->heap_size()!=0) && buy_orders.value(temp.stock)->max_val().expiry!=-1){


                    //mostly redundant but not sure
                    if(buy_orders.value(temp.stock)->max_val().expiry==-1){
                        break;
                    }


                    buy_orders.value(temp.stock)->delete_max();
                }
                if(buy_orders.value(temp.stock)->heap_size()!=0){
                    if(buy_orders.value(temp.stock)->max_val().price >= temp.price){
                        int commom_quantity = std::min(buy_orders.value(temp.stock)->max_val().quantity,temp.quantity);

                        //trade happened
                        std::cout<<buy_orders.value(temp.stock)->max_val().broker<<" purchased "<<commom_quantity<<" share of "<<temp.stock<<" from "<<temp.broker<<" for $"<<buy_orders.value(temp.stock)->max_val().price<<"/share"<<std::endl;
                        total_money+=commom_quantity*buy_orders.value(temp.stock)->max_val().price;
                        complete_trades++;
                        shares_traded+=commom_quantity;
                        std::string buy_broker = buy_orders.value(temp.stock)->max_val().broker;
                        std::string sell_broker = temp.broker;
                        int temp_buy = broker_bought.value(buy_broker)+commom_quantity;
                        broker_bought.insert(buy_broker,temp_buy);
                        int temp_sell = broker_sold.value(sell_broker) + commom_quantity;
                        broker_sold.insert(sell_broker,temp_sell);
                        int net_temp = commom_quantity*buy_orders.value(temp.stock)->max_val().price;
                        broker_net_money.insert(buy_broker,(broker_net_money.value(buy_broker)-net_temp));
                        broker_net_money.insert(sell_broker,(broker_net_money.value(sell_broker)+net_temp));
                        if((buy_orders.value(temp.stock)->max_val().quantity - commom_quantity) > 0){
                            //then decrease the quantity and reinsert
                            trade temp_trade;
                            temp_trade.broker = buy_orders.value(temp.stock)->max_val().broker;
                            temp_trade.choice = buy_orders.value(temp.stock)->max_val().choice;
                            temp_trade.expiry = buy_orders.value(temp.stock)->max_val().expiry;
                            temp_trade.price = buy_orders.value(temp.stock)->max_val().price;
                            temp_trade.quantity = buy_orders.value(temp.stock)->max_val().quantity - commom_quantity;
                            temp_trade.stock = buy_orders.value(temp.stock)->max_val().stock;
                            temp_trade.timestamp = buy_orders.value(temp.stock)->max_val().timestamp;
                            buy_orders.value(temp.stock)->delete_max();
                            buy_orders.value(temp.stock)->insert(temp_trade);
                        }
                        else{
                            buy_orders.value(temp.stock)->delete_max();
                        }

                        if(temp.quantity - commom_quantity > 0){
                            //then insert in heap
                            temp.price = (-1)*std::abs(temp.price); // made the price -ve so that by using max_heap i can get results like min_heap
                            temp.quantity = temp.quantity - commom_quantity;
                            sellTrade(temp);
                            if(sell_orders.value(temp.stock)==nullptr){
                                heap <trade>*h_sell = new heap<trade>;
                                h_sell->insert(temp);
                                sell_orders.insert(temp.stock,h_sell); //inserting heap of sell_orders of a trade in map if a heap for it is not already present
                            }
                            else{
                                sell_orders.value(temp.stock)->insert(temp);
                            }
                        }
                        else{
                            return;
                        }
                    }
                    else{
                        insertSell(temp);
                    }
                }
                else{
                    insertSell(temp);
                }
            }
            else{
                insertSell(temp);
            }
}

void buyTrade(trade temp){
    if(sell_orders.value(temp.stock)!=nullptr){
                while(((sell_orders.value(temp.stock)->max_val().timestamp + sell_orders.value(temp.stock)->max_val().expiry) < temp.timestamp)&&(sell_orders.value(temp.stock)->heap_size()!=0)&&sell_orders.value(temp.stock)->max_val().expiry!=-1){
                    //mostly reduntant but not sure
                    if(sell_orders.value(temp.stock)->max_val().expiry==-1){
                        break;
                    }


                    sell_orders.value(temp.stock)->delete_max();
                }
                if(sell_orders.value(temp.stock)->heap_size()!=0){
                    if(std::abs(sell_orders.value(temp.stock)->max_val().price) <= temp.price){
                        int commom_quantity = std::min(sell_orders.value(temp.stock)->max_val().quantity,temp.quantity);
                        
                        //trade happened
                        std::cout<<temp.broker<<" purchased "<<commom_quantity<<" share of "<<temp.stock<<" from "<<sell_orders.value(temp.stock)->max_val().broker<<" for $"<<std::abs(sell_orders.value(temp.stock)->max_val().price)<<"/share"<<std::endl;
                        total_money+=commom_quantity*abs(sell_orders.value(temp.stock)->max_val().price);
                        complete_trades++;
                        shares_traded+=commom_quantity;
                        std::string buy_broker = temp.broker;
                        std::string sell_broker = sell_orders.value(temp.stock)->max_val().broker;
                        int temp_buy = broker_bought.value(buy_broker)+commom_quantity;
                        broker_bought.insert(buy_broker,temp_buy);
                        int temp_sell = broker_sold.value(sell_broker) + commom_quantity;
                        broker_sold.insert(sell_broker,temp_sell);
                        int net_temp = commom_quantity*sell_orders.value(temp.stock)->max_val().price;
                        broker_net_money.insert(buy_broker,(broker_net_money.value(buy_broker)-net_temp));
                        broker_net_money.insert(sell_broker,(broker_net_money.value(sell_broker)+net_temp));
                        if((sell_orders.value(temp.stock)->max_val().quantity - commom_quantity) > 0){
                            //then decrease the quantity and reinsert
                            trade temp_trade;
                            temp_trade.broker = sell_orders.value(temp.stock)->max_val().broker;
                            temp_trade.choice = sell_orders.value(temp.stock)->max_val().choice;
                            temp_trade.expiry = sell_orders.value(temp.stock)->max_val().expiry;
                            temp_trade.price = sell_orders.value(temp.stock)->max_val().price;
                            temp_trade.quantity = sell_orders.value(temp.stock)->max_val().quantity - commom_quantity;
                            temp_trade.stock = sell_orders.value(temp.stock)->max_val().stock;
                            temp_trade.timestamp = sell_orders.value(temp.stock)->max_val().timestamp;
                            sell_orders.value(temp.stock)->delete_max();
                            sell_orders.value(temp.stock)->insert(temp_trade);
                        }
                        else{
                            sell_orders.value(temp.stock)->delete_max();
                        }

                        if(temp.quantity - commom_quantity > 0){
                            //then insert in heap
                            temp.quantity = temp.quantity - commom_quantity;
                            buyTrade(temp);
                            if(buy_orders.value(temp.stock)==nullptr){
                                heap <trade>*h_buy = new heap<trade>;
                                h_buy->insert(temp);
                                buy_orders.insert(temp.stock,h_buy); //inserting heap of sell_orders of a trade in map if a heap for it is not already present
                            }
                            else{
                                buy_orders.value(temp.stock)->insert(temp);
                            } 
                        }
                        else{
                            return;
                        }
                    }
                    else{
                        insertBuy(temp);
                    }
                }
                else{
                    insertBuy(temp);
                }
            }
            else{
                insertBuy(temp); 
            }
}

void market::start()
{
    for(auto itr  = input.begin();itr!=input.end();itr++){
        trade temp;
        temp = parseTrade(*itr);

        // // now writing the part where the trades will be made
        if(temp.choice == "SELL"){
            sellTrade(temp);
        }
        // remember prices of sell are in negative
        else{
            buyTrade(temp);
        }
    }
    std::cout<<"\n---End of Day---"<<std::endl;
    std::cout<<"Total Amount of Money Transferred: $"<<total_money<<std::endl;
    std::cout<<"Number of Completed Trades: "<<complete_trades<<std::endl;
    std::cout<<"Number of Shares Traded: "<<shares_traded<<std::endl;
    for(int i=0;i<brokers.size();i++){
        std::cout<<brokers[i]<<" bought "<<broker_bought.value(brokers[i])<<" and sold "<<broker_sold.value(brokers[i])<<" for a net trandfer of $"<<broker_net_money.value(brokers[i])<<std::endl;
    }
}


// ((sell_orders.value(temp.stock)->max_val().timestamp + sell_orders.value(temp.stock)->max_val().expiry) <= temp.timestamp)&&

