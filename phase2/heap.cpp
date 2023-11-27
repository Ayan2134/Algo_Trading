#include <vector>
#include <iostream>
// implement heapsort - perhaps using a heap class  --- this is max heap
template <typename T>
class heap{
    public :
    int left (int i){
        return (2*i + 1);
    }
    int right(int i){
        return (2*i+2);
    }
    int parent(int i){
        return (i-1)/2;
    }
    void swap(T x, T y ){
        T temp = v[x];
        v[x]=v[y];
        v[y]=temp;
    }

    T max_alt(T x,T y){
        if(x>y){
            return x;
        }
        else{
            return y;
        }
    }

    void heapify(int i){
        T max_val;
        if (left(i)>=size && right(i)>=size){
            max_val = v[i];
        }
        else if (right(i)>=size){
            max_val = max_alt(v[i],v[left(i)]);
        }
        else{
            max_val = max_alt(v[i],max_alt(v[left(i)],v[right(i)]));
        }
        if (max_val == v[i]) {
            return;
        }
        else if(max_val == v[left(i)]){
            swap(left(i),i);
            heapify(left(i));
        }
        else{
            swap(right(i),i);
            heapify(right(i));
        }
    }

    void build_heap(){
        for(int i=size-1;i>=0;i--){
            heapify(i);
        }
    }

    void delete_max(){
        swap(0,size-1);
        size--;
        heapify(0);
    }

    void hsort() {
        build_heap(v);
        while(size>0){
            delete_max();
        }
    }

    T max_val(){
        return v[0];
    }

    void insert(T k){
        int i=size;
    	v.push_back(k);
        swap(v.size()-1,size); //swap between where the element should actually be placed vs where it is placed
    	while (i>0 && v[parent(i)]<v[i]){
    		swap(parent(i),i);
    		i=parent(i);
    	}
        size++;
    }

    T value(int i){
        return v[i];
    }

    int heap_size(){
        return size;
    }

    private :
        std::vector<T> v;
        int size=0;
};

int main(){
    heap <int> mine;
    mine.insert(4);
    mine.insert(5);
    mine.insert(1);
    mine.insert(89);
    mine.insert(2);
    mine.insert(7);

    for(int i=0;i<mine.heap_size();i++){
        std::cout<<mine.value(i)<<std::endl;
    }
    mine.delete_max();
    for(int i=0;i<mine.heap_size();i++){
        std::cout<<mine.value(i)<<std::endl;
    }
        mine.delete_max();
    for(int i=0;i<mine.heap_size();i++){
        std::cout<<mine.value(i)<<std::endl;
    }
    std::cout<<mine.heap_size()<<std::endl;
}

