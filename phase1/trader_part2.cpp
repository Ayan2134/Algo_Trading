#include "receiver.h"

using namespace std;

int main(){
    Receiver rcv;
    sleep(5);
    std::string message = rcv.readIML();
    string message1="bincho";
    while(message1.length()!=0){
        sleep(5);
        message1 = rcv.readIML();
        message+=message1;
    }
    return 0;
}
