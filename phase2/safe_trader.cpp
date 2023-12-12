#include <iostream>
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <fstream>
#include "map.h"

extern std::atomic<int> commonTimer;
extern std::mutex printMutex;  // Declare the mutex for printing

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

void bubbleSort(std::vector<std::pair<int, int>>& vec) {
    int n = vec.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (vec[j].first > vec[j + 1].first) {
                std::swap(vec[j], vec[j + 1]);
            }
        }
    }
}

void bubbleSort_str(std::vector<std::pair<int, std::string>>& vec) {
    int n = vec.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (vec[j].first > vec[j + 1].first) {
                std::swap(vec[j], vec[j + 1]);
            }
        }
    }
}

std::vector<std::pair<int, std::string>> pairedVector;

trade parseTrade1(const std::string& line) {
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
        bubbleSort_str(pairedVector);
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

map <std::vector<std::pair<int, int>>*> pairs;
map <double> median;


double weightedMedian(const std::vector<std::pair<int, int>>& pairs) {
    std::vector<std::pair<int, int>> cumulativeWeights;
    std::vector<std::pair<int, int>> sortedPairs = pairs;
    bubbleSort(sortedPairs);
    int cumulativeWeight = 0;
    for (const auto& pair : sortedPairs) {
        cumulativeWeight += pair.second;
        cumulativeWeights.emplace_back(pair.first, cumulativeWeight);
    }
    int totalWeight = cumulativeWeight;
    int medianIndex = 0;
    for (const auto& pair : cumulativeWeights) {
        if (pair.second * 2 >= totalWeight) {
            break;
        }
        medianIndex++;
    }
    return cumulativeWeights[medianIndex].first;
}

int prev_size = 0;
int prev_size_actual = 0;
int curr_size = 0;
int reader(int time)
{
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::vector<std::string> lines;
        std::ifstream inputFile ("output.txt");
        std::string line;
        while (std::getline(inputFile,line)) {
            if (line.compare("TL") == 0) {
                continue;
            }

            if (line.compare("!@") == 0 || line.compare("Successfully Initiated!") == 0) {
                return 1;
                break;
            }
            lines.push_back(line);
            prev_size = curr_size;
            curr_size = lines.size();
        }
        if(prev_size == curr_size){
            break;
        }
        int count=0;
        // what to trade
        std::vector<trade> trades;
        for(int i=prev_size_actual;i<curr_size;i++){
            trade temp;
            temp = parseTrade1(lines[i]);
            trades.push_back(temp);
            if(pairs.value(temp.stock) == nullptr){
                std::vector<std::pair<int, int>>* temp_vec = new std::vector<std::pair<int, int>>;
                temp_vec->push_back(std::make_pair(temp.price,temp.quantity));
                pairs.insert(temp.stock,temp_vec);
            }
            else{
                pairs.value(temp.stock)->push_back(std::make_pair(temp.price,temp.quantity));
            }
            double temp_median = weightedMedian(*pairs.value(temp.stock));
            median.insert(temp.stock,temp_median);
            
        }
        for(int i=prev_size;i<curr_size;i++){
            if(trades[i].broker != "22b0931"){
                int median_val = median.value(trades[i].stock);
                if(trades[i].choice == "BUY"){
                    if(median_val < trades[i].price){
                        //sell this stock
                        count++;
                        std::cout<<commonTimer.load()<<" 22b0931"<<" SELL "<<trades[i].stock<<" $"<<trades[i].price<<" #"<<trades[i].quantity<<" "<<(trades[i].timestamp+trades[i].expiry-commonTimer.load()+1)<<std::endl;
                    }
                    else{
                        //no trade
                    }
                }
                else{
                    if(median_val > trades[i].price){
                        //buy this stock
                        count++;
                        std::cout<<commonTimer.load()<<" 22b0931"<<" BUY "<<trades[i].stock<<" $"<<trades[i].price<<" #"<<trades[i].quantity<<" "<<(trades[i].timestamp+trades[i].expiry-commonTimer.load()+1)<<std::endl;
                    }
                    else{
                        //no trade
                    }
                }
            }
            else{
                // do not include yourself
            }
        }
    }
    return 1;
}

int trader(std::string *message)
{
    return 1;
}

void* userThread(void* arg)
{
    int thread_id = *(int*)arg;
    while (true) {
        int currentTime;
        {
            currentTime = commonTimer.load();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
        int end = reader(currentTime);
        if (end) break;
    }
    return nullptr;
}

void* userTrader(void* arg)
{
    return nullptr;
}
