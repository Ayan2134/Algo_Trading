#include "receiver.h"
#include <map>
using namespace std;

int main() {

    Receiver rcv;
    sleep(5);
    std::string message = rcv.readIML();
    map<string,string> last_trade; //last traded price for that stock
    map<string,string> bos; //buy or sell

    int i=0;
    string stock , price , choice;
    stock=price=choice="";
    while(message[i]!='$'){
        while(message[i]!=' '){
            stock+=message[i];
            i++;
        }
        i++; //to move forward from white space
        while(message[i]!=' '){
            price+=message[i];
            i++;
        }
        i++; 
        choice=(message[i] == 'b' ? "s" : "b");
        i+=3;
        if(last_trade.count(stock)>0){
            if(choice == "s"){ // selling a stock
                if(last_trade[stock]<price){ //sell
                    cout<<stock<<" "<<price<<" "<<choice<<endl;
                    last_trade[stock]=price;
                }
                else{ //No trade
                    cout<<"No Trade"<<endl;
                }
            }
            else{ // buying a stock
                if(last_trade[stock]<=price){ //No trade
                    cout<<"No Trade"<<endl;
                }
                else{ //buy
                    cout<<stock<<" "<<price<<" "<<choice<<endl;
                    last_trade[stock]=price;
                }
            }
        }
        else{
            last_trade[stock]=price;
            bos[stock]=choice;
            cout<<stock<<" "<<price<<" "<<choice<<endl;
        }
        stock=price=choice="";
    }
    return 0;
}
