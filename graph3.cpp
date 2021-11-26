#include <iostream>
#include <vector>
using namespace std;
class Node{
    public:
        int number;
        vector<Node> neigbours;
    
    Node(int val){
        number = val;
    }
};

class asdf{
    public:
        vector<Node> qwer;
        // vector<pair<Node,Node>> zxcv; 
};


int main(){
    vector<int> a{1,2,5,4};
    Node *n = new Node(1);
    asdf *p = new asdf();
    p->qwer.push_back(*n);
    for(int i = 2; i< 10; i++){
        Node *x = new Node(i);
        p->qwer.push_back(*x);
        cout << "XXXXXXXXXXXXXXXXXXXx" ; 
        p->qwer[0].neigbours.push_back(*x);
    }
    return 0; 
}