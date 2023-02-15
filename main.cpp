#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>
#include <pthread.h>

using namespace std;

struct node{
    node*left;
    node *right;
    int weight;
    char c;
    bool leaf; 
    pthread_mutex_t lock;
};
struct wrapperArg{
    string road;
    vector<int> places;
    node * head;
    string *result;

    
};


struct Compare {

bool operator()(const node* a, const node* b) const{
    if ((*a).weight>(*b).weight){
        return true;
    }else{
        if ((*a).weight<(*b).weight){
            return false;
        } else{
            return (*a).c>(*b).c;
        }
    }
}
};

void ReadOccurencesFile(ifstream & file,map<char,int> &occur);
int GetDigit(string s);
void BuildPriorQueue(priority_queue<node*,vector<node*>,Compare>& q,map<char,int> & mp);
node * BuildHuffmanTree(priority_queue<node*,vector<node*>,Compare>& q);
void PrintNodes(node * n,string road);
void *ConcurrentDecompress(void * arg);
void Destroy(node* head);
int main(){
    node k;
    priority_queue<node*,vector<node*>,Compare> pqueue;
    
    ifstream file("input.txt");
    if (!file.is_open()){
        cout<<"File isn't Opened";
        return 0;
    }
    map<char,int> occur;
    ReadOccurencesFile(file,occur);
    file.close();
    BuildPriorQueue(pqueue,occur);

    node * head  = BuildHuffmanTree(pqueue);
    PrintNodes(head,"");
    
    ifstream defile("decomp.txt");
    if (!defile.is_open()){
        cout<<"Decompressed file  isn't Opened";
        return 0;
    }
    string line;
    
    vector<vector<int>> matrix;
    vector<string> roads;
    while(getline(defile,line)){
        stringstream tok(line);
     
        string cur,road;
        
     int i=0;
     vector<int> place; 
        while(getline(tok, cur, ' ')){   
            
            if (i==0){
                road = cur;
                i++;
            }else{
                
                place.push_back(stoi(cur));
            }
          
        }
        roads.push_back(road);
        matrix.push_back(place);
        
    }


    defile.close();
    pthread_t threads[roads.size()];
    string res;
    res.append(head->weight,'a');
   
    vector<wrapperArg> args(roads.size());
    for(int j=0;j<roads.size();j++){
        
        args[j].road = roads[j];
        args[j].places  = matrix[j];
        args[j].head = head;
        args[j].result = &res;
        
        
        pthread_create(&threads[j],NULL,ConcurrentDecompress,(void*)&(args[j]));
    }
    for (int j = 0; j < roads.size(); j++) {
        pthread_join(threads[j], NULL);
    }
    
    
    cout<<"Original Message: "<<res<<endl;
    
    Destroy(head);
}


void ReadOccurencesFile(ifstream & file,map<char,int> &occur){
    string line;
    while(getline(file,line)){
        char c;
        int num;
        c = line[0];
        num = GetDigit(line);
        occur[c]= num;
        
    }
    
}
int GetDigit(string s){
    string tmp;
    for(int i=2;i<s.length();i++) tmp+=s[i];
    return stoi(tmp);
}

void BuildPriorQueue(priority_queue<node*,vector<node*>,Compare>& q,map<char,int> & mp){

    for (const auto &pair : mp) {
        node *cur = new node{
            left:NULL,
            right:NULL,
            weight:pair.second,
            c:pair.first,
            leaf:true,
        };
        
        q.push(cur);
    }
    
}

node * BuildHuffmanTree(priority_queue<node*,vector<node*>,Compare>& q){
   node  * cur;
   while(!q.empty()){
    cur = new node;
    cur->leaf = false;
    
    if (q.size()==1){
        node *right = q.top(); q.pop();
        cur->weight = right->weight;
        cur->left = right;
        cur->right =  NULL;
        cur->leaf = false;
        pthread_mutex_init(&cur->lock,NULL);
    }else{
        node * left;
        left = q.top(); q.pop();
        node * right;
        right = q.top(); q.pop();
        cur->weight = left->weight + right->weight;
        cur->left = left;
        cur->right = right;
        cur->leaf = false;
        pthread_mutex_init(&cur->lock,NULL);
    }
    if (q.empty()) break;
    q.push(cur);
   }
   return cur;
}

void PrintNodes(node * n,string road){
    if(n->leaf){
     
        cout<<"Symbol: "<<n->c<<", Frequency: "<<n->weight<<", Code: "<<road<<endl;
    }else{
        if (n->left){
            PrintNodes(n->left,road+"0");
        }
        if (n->right){
            PrintNodes(n->right,road+"1");
        }
    }

}

void *ConcurrentDecompress(void * arg){
    
    wrapperArg cur = *((wrapperArg*)arg);
    int i=0;
     
    pthread_mutex_lock(&cur.head->lock);
    
    node* head = (node*)cur.head;
    
    
    while(!head->leaf){
        
        if (cur.road[i]=='0'){
            
            pthread_mutex_unlock(&head->lock);
            pthread_mutex_lock(&head->left->lock);
            head = head->left;
        }else{
             if (cur.road[i]=='1'){
                
                pthread_mutex_unlock(&head->lock);
                pthread_mutex_lock(&head->right->lock);
                   head = head->right;
            }
        }
        i++;
        
    }
    pthread_mutex_unlock(&head->lock);
    
        char k = head->c;
        
        
       
        
       
    for(int j=0;j<cur.places.size();j++){
        
        (*cur.result)[cur.places[j]] = k;
        
    }
  
}

void Destroy(node* head){
    if (!head){

    }else{
        Destroy(head->left);
        Destroy(head->right);
        delete(head);
    }
}