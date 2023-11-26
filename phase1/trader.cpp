#include "receiver.h"
#include <vector>
#include <string>


struct deal{
    std::vector<std::string> stock;
    std::vector<int> quantity;
    int price;
    std::string choice;
};

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
    return temp;
}

int main(){
    Receiver rcv;
    sleep(5);
    std::string message = rcv.readIML();
    std::string message1="bincho";
    while(message1.length()!=0){
        sleep(5);
        message1 = rcv.readIML();
        message+=message1;
    }

    std::vector<deal> all_deals;

    int startPos = 0;
    int endPos;
    char new_line = static_cast<char>(13);
    while ((endPos = message.find(new_line, startPos)) != std::string::npos || (endPos = message.find('$', startPos)) != std::string::npos) {
        std::string line = message.substr(startPos, endPos - startPos);
        deal new_deal = parseStocks(line);
        all_deals.push_back(new_deal);
        startPos = endPos + 1;
    }
    for(auto itr = all_deals.begin();itr!=all_deals.end();itr++){
        std::cout<<itr->quantity.back()<<std::endl;
    }
    return 0;
}
