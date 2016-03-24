

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <typeinfo>
#include <algorithm>
using namespace std;

int num_gates;
vector<vector<int> >* faultList = new vector<vector<int> >(5000);

struct gates {
    int id;
    int* complete;
    void* ptr;

};


int comp (gates a[])
{
    int i;
    int complete = *a[0].complete;
    for(i=1; i<num_gates;i++)
    {
        complete = complete && *a[i].complete;
    }
    return (complete);
}
class inverter
{
    public:
    int* ip;
    int* def1;
    int* def2;
    int* op;
      int in;
      int out;

    vector<int> LIp;
    vector<int> Op;
    vector<int> FList;
      
    void func();
    void error();

};
void inverter::func()
{
    (*op) = !(*ip);
}

void inverter::error()
{
for(int i=0;i<(*faultList)[in].size();i++)
{     
LIp.push_back((*faultList)[in][i]);
}
for(int i=0;i<(*faultList)[out].size();i++)
{
Op.push_back((*faultList)[out][i]);
}
            std::sort(LIp.begin(),LIp.end());
            std::sort(Op.begin(),Op.end());
      Op.erase( unique(Op.begin(), Op.end()), Op.end());
      int size_1 = 2*LIp.size();
      int size_2 = 2*size_1;
      vector<int> C2(size_1);
            //cout<< "Union of faults\n";
            vector<int>::iterator itC2 = set_union(LIp.begin(),LIp.end(),Op.begin(),Op.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            C2.erase( unique(C2.begin(), C2.end()), C2.end());

//cout<<"LEngth of C2: " << C2.size()<<endl;
for(size_t i=0;i<C2.size();i++)
{
//cout<<"copying local fault to class fault"<<endl;
//cout<< "C2 ele :" << C2[i]<<endl; 
(*faultList)[out].push_back(C2[i]);
}
            sort((*faultList)[out].begin(),(*faultList)[out].end());
            (*faultList)[out].erase( unique((*faultList)[out].begin(), (*faultList)[out].end()), (*faultList)[out].end());

}

class buffer
{
    public:
    int* ip;
    int* def1;
    int* def2;
    int* op;
      int in;
      int out;

    vector<int> LIp;
    vector<int> Op;
    vector<int> FList;
      
    void func();
    void error();
    
};
void buffer::func()
{
    (*op) = (*ip);
}
void buffer::error()
{ 
//cout<<"BUFFER ERROR FUNC"<<endl;
for(int i=0;i<(*faultList)[in].size();i++)
{     
LIp.push_back((*faultList)[in][i]);
}
for(int i=0;i<(*faultList)[out].size();i++)
{
Op.push_back((*faultList)[out][i]);
}

 //cout << "in-->"<<in<<endl; 
            std::sort(LIp.begin(),LIp.end());
            std::sort(Op.begin(),Op.end());
      Op.erase( unique(Op.begin(), Op.end()), Op.end());
      int size_1 = 2*LIp.size();
      int size_2 = 2*size_1;
      vector<int> C2(size_1);
//          cout<< "Union of faults in BUF\n";
            vector<int>::iterator itC2 = set_union(LIp.begin(),LIp.end(),Op.begin(),Op.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            C2.erase( unique(C2.begin(), C2.end()), C2.end());


//cout<<"LEngth of C2: " << C2.size()<<endl;
for(size_t i=0;i<C2.size();i++)
{
//cout<<"copying local fault to class fault"<<endl;
//cout<< "C2 ele :" << C2[i]<<endl; 
(*faultList)[out].push_back(C2[i]);
}
            sort((*faultList)[out].begin(),(*faultList)[out].end());
            (*faultList)[out].erase( unique((*faultList)[out].begin(), (*faultList)[out].end()), (*faultList)[out].end());
//cout<<"done with buffer err"<<endl;
}

class andgate
{
    public:
    int *ip1, *ip2;
    int* def1;
    int* def2;
    int* def3;
    int* op;
    void func();
    void error();

      int in1;
      int in2;
      int out;
    
    vector<int> LIp1;
    vector<int> LIp2;
    vector<int> Op;
    vector<int> FList;
};
void andgate::func()
{
    (*op) = (*ip1) && (*ip2);
}
//error check function
void andgate::error()
{
for(int i=0;i<(*faultList)[in1].size();i++)
{     
LIp1.push_back((*faultList)[in1][i]);
}
for(int i=0;i<(*faultList)[in2].size();i++)
{     
LIp2.push_back((*faultList)[in2][i]);
}
for(int i=0;i<(*faultList)[out].size();i++)
{
Op.push_back((*faultList)[out][i]);
}



 
            std::sort(LIp1.begin(),LIp1.end());
            std::sort(LIp2.begin(),LIp2.end());
            std::sort(Op.begin(),Op.end());
            Op.erase( unique(Op.begin(), Op.end()), Op.end());
//cout<<"Op size :"<<Op.size()<<endl;
/*for(int x=0;x<Op.size();x++)
{
//cout<<Op[x]<<endl;
}*/

      int size_1 = 2*(max (LIp1.size(),LIp2.size()));
      int size_2 = 2*size_1;
      vector<int> C2(size_1);
      vector<int> F2(size_2);
if((*ip1)==0)
{
      if((*ip2)==0) 
       {    //cout<<"Intersection of inputs faults with union of output stuck at 1\n";
            vector<int>::iterator itC2 = set_intersection(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
      else
      {
            //cout << "(fault list ip1 - fault list ip2)union of op stuck at 1\n";
            vector<int>::iterator itC2 = set_difference(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
}

else if ((*ip2)==0) 
      {
            //cout<< "(fault list ip2 - fault list ip1)union of op stuck at 1\n";
            vector<int>::iterator itC2 = set_difference(LIp2.begin(),LIp2.end(),LIp1.begin(),LIp1.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
else
      { 
            //cout<< "Union of input fault lists with op stuck at 0\n";
            vector<int>::iterator itC2 = set_union(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }

//to remove duplicates
            sort(F2.begin(),F2.end());
            F2.erase( unique(F2.begin(), F2.end()), F2.end());
//cout<<"LEngth of F2: " << F2.size()<<endl;
for(size_t i=0;i<F2.size();i++)
{
//cout<< "F2 ele :" << F2[i]<<endl; 
size_t a=F2[i];
(*faultList)[out].push_back(a);
}
            sort((*faultList)[out].begin(),(*faultList)[out].end());
            (*faultList)[out].erase( unique((*faultList)[out].begin(), (*faultList)[out].end()), (*faultList)[out].end());
//cout<<"to check whether its updatin the outputnode"<<endl;
/*for(int f=0;f<(*faultList)[out].size();f++)
{ //cout<<"fault list at op node element "<< f << "-> "<<(*faultList)[out][f]<<endl;
}*/
}

class nandgate
{
    public:
    int *ip1, *ip2;
    int* def1;
    int* def2;
    int* def3;
//    int* def;
    int* op;
      int in1;
      int in2;
      int out;

    void func();
    void error(); 
    vector<int> LIp1;
    vector<int> LIp2;
    vector<int> Op;
    vector<int> FList;
};
void nandgate::func()
{
    (*op) = !((*ip1) && (*ip2));
}
void nandgate::error()
{ 
//cout<<"NANDGATE ERROR FUNC"<<endl;
for(int i=0;i<(*faultList)[in1].size();i++)
{     
LIp1.push_back((*faultList)[in1][i]);
}
for(int i=0;i<(*faultList)[in2].size();i++)
{     
LIp2.push_back((*faultList)[in2][i]);
}
for(int i=0;i<(*faultList)[out].size();i++)
{
Op.push_back((*faultList)[out][i]);
}
 //cout << "in1-->"<<in1<<endl;     
 //cout << "in2-->"<<in2<<endl;     
      //cout<< "size of LIp1:" << LIp1.size()<<endl;
            std::sort(LIp1.begin(),LIp1.end());
            std::sort(LIp2.begin(),LIp2.end());
            std::sort(Op.begin(),Op.end());
            Op.erase( unique(Op.begin(), Op.end()), Op.end());
      int size_1 = 2*(max (LIp1.size(),LIp2.size()));
      int size_2 = 2*size_1;
      vector<int> C2(size_1);
      vector<int> F2(size_2);
if((*ip1)==0)
{
      if((*ip2)==0) 
       {    //cout<<"Intersection of inputs faults with union of output stuck at 1\n";
            vector<int>::iterator itC2 = set_intersection(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
      else
      {
            //cout << "(fault list ip1 - fault list ip2)union of op stuck at 1\n";
            vector<int>::iterator itC2 = set_difference(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
}

else if ((*ip2)==0) 
      {
            //cout<< "(fault list ip2 - fault list ip1)union of op stuck at 1\n";
            vector<int>::iterator itC2 = set_difference(LIp2.begin(),LIp2.end(),LIp1.begin(),LIp1.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
else
      { 
            //cout<< "Union of input fault lists with op stuck at 0\n";
            vector<int>::iterator itC2 = set_union(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
//to remove duplicates
            sort(F2.begin(),F2.end());
            F2.erase( unique(F2.begin(), F2.end()), F2.end());
//cout<<"LEngth of F2: " << F2.size()<<endl;
for(size_t i=0;i<F2.size();i++)
{
size_t a=F2[i];
(*faultList)[out].push_back(a);
}
            sort((*faultList)[out].begin(),(*faultList)[out].end());
            (*faultList)[out].erase( unique((*faultList)[out].begin(), (*faultList)[out].end()), (*faultList)[out].end());
}

class orgate
{
    public:
    int *ip1, *ip2;
    int* def1;
    int* def2;
    int* def3;
//    int* def;
    int* op;

      int in1;
      int in2;
      int out;

    void func();
    void error(); 
    vector<int> LIp1;
    vector<int> LIp2;
    vector<int> Op;
    vector<int> FList;
};
void orgate::func()
{
    (*op) = (*ip1) || (*ip2);
}
void orgate::error()
{
//cout << "in1-->"<<in1<<endl;      
 //cout << "in2-->"<<in2<<endl;     
 //cout << "out-->"<<out<<endl;     
//cout<<"fault list at in2 length"<<(*faultList)[in2].size()<<endl;
//cout<<"fault list at in1 length"<<(*faultList)[in1].size()<<endl;
//cout<<"fault list at out length"<<(*faultList)[out].size()<<endl;
 
//cout<<"ORGATE ERROR FUNC"<<endl;
for(int i=0;i<(*faultList)[in1].size();i++)
{//cout<<(*faultList)[in1][i]<<endl;      
LIp1.push_back((*faultList)[in1][i]);
}
for(int i=0;i<(*faultList)[in2].size();i++)
{     
LIp2.push_back((*faultList)[in2][i]);
}
for(int i=0;i<(*faultList)[out].size();i++)
{
Op.push_back((*faultList)[out][i]);
}

/*to chech whether 3 is appended*/

 //cout << "in1-->"<<in1<<endl;     
 //cout << "in2-->"<<in2<<endl;     
      
      std::sort(LIp1.begin(),LIp1.end());
      std::sort(LIp2.begin(),LIp2.end());
      std::sort(Op.begin(),Op.end());
      Op.erase( unique(Op.begin(), Op.end()), Op.end());
      int size_1 = 2*(max (LIp1.size(),LIp2.size()));
      int size_2 = 2*size_1;
      vector<int> C2(size_1);
      vector<int> F2(size_2);
if((*ip1)==1)
{
      if((*ip2)==1) 
       {    //cout<<"Intersection of inputs faults with union of output stuck at 1\n";
            vector<int>::iterator itC2 = set_intersection(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
      else
      {
            //cout << "(fault list ip1 - fault list ip2)union of op stuck at 1\n";
            vector<int>::iterator itC2 = set_difference(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
}

else if ((*ip2)==1) 
      {
            //cout<< "(fault list ip2 - fault list ip1)union of op stuck at 1\n";
            vector<int>::iterator itC2 = set_difference(LIp2.begin(),LIp2.end(),LIp1.begin(),LIp1.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
else
      { 
            //cout<< "Union of input fault lists with op stuck at 0\n";
            vector<int>::iterator itC2 = set_union(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
//to remove duplicates
            sort(F2.begin(),F2.end());
            F2.erase( unique(F2.begin(), F2.end()), F2.end());
for(size_t i=0;i<F2.size();i++)
{
//cout<< "F2 ele :" << F2[i]<<endl; 
size_t a=F2[i];
(*faultList)[out].push_back(a);
}
            sort((*faultList)[out].begin(),(*faultList)[out].end());
            (*faultList)[out].erase( unique((*faultList)[out].begin(), (*faultList)[out].end()), (*faultList)[out].end());

//cout<<"to check whether its updatin the outputnode"<<endl;
/*for(int f=0;f<(*faultList)[out].size();f++)
{ //cout<<"fault list at op node element "<< f << "-> "<<(*faultList)[out][f]<<endl;
}*/
}


class norgate
{
    public:
    int *ip1, *ip2;
    int* def1;
    int* def2;
    int* def3;
//    int* def;
    int* op;
      int in1;
      int in2;
      int out;

    void func();
    void error(); 
    vector<int> LIp1;
    vector<int> LIp2;
    vector<int> Op;
    vector<int> FList;
};
void norgate::func()
{
    (*op) = !((*ip1) || (*ip2));
}

void norgate::error()
{ 
//cout<<"NORGATE ERROR FUNC"<<endl;
for(int i=0;i<(*faultList)[in1].size();i++)
{     
LIp1.push_back((*faultList)[in1][i]);
}
for(int i=0;i<(*faultList)[in2].size();i++)
{     
LIp2.push_back((*faultList)[in2][i]);
}
for(int i=0;i<(*faultList)[out].size();i++)
{
Op.push_back((*faultList)[out][i]);
}
 //cout << "in1-->"<<in1<<endl;     
 //cout << "in2-->"<<in2<<endl;     
            std::sort(LIp1.begin(),LIp1.end());
            std::sort(LIp2.begin(),LIp2.end());
            std::sort(Op.begin(),Op.end());
            Op.erase( unique(Op.begin(), Op.end()), Op.end());
      int size_1 = 2*(max (LIp1.size(),LIp2.size()));
      int size_2 = 2*size_1;
      vector<int> C2(size_1);
      vector<int> F2(size_2);
if((*ip1)==1)
{
      if((*ip2)==1) 
       {    //cout<<"Intersection of inputs faults with union of output stuck at 1\n";
            vector<int>::iterator itC2 = set_intersection(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
      else
      {
            //cout << "(fault list ip1 - fault list ip2)union of op stuck at 1\n";
            vector<int>::iterator itC2 = set_difference(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
}

else if ((*ip2)==1) 
      {
            //cout<< "(fault list ip2 - fault list ip1)union of op stuck at 1\n";
            vector<int>::iterator itC2 = set_difference(LIp2.begin(),LIp2.end(),LIp1.begin(),LIp1.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
else
      { 
            //cout<< "Union of input fault lists with op stuck at 0\n";
            vector<int>::iterator itC2 = set_union(LIp1.begin(),LIp1.end(),LIp2.begin(),LIp2.end(),C2.begin());
            C2.resize(itC2-C2.begin());
            sort(C2.begin(),C2.end());
            vector<int>::iterator itOp2 = set_union(C2.begin(),C2.end(),Op.begin(),Op.end(),F2.begin());
            F2.resize(itOp2-F2.begin());
      }
//to remove duplicates
            sort(F2.begin(),F2.end());
            F2.erase( unique(F2.begin(), F2.end()), F2.end());
////cout<<"LEngth of F2: " << F2.size()<<endl;
for(size_t i=0;i<F2.size();i++)
{
size_t a=F2[i];
//cout<<"a="<<a<<endl;
(*faultList)[out].push_back(a);
}
            sort((*faultList)[out].begin(),(*faultList)[out].end());
            (*faultList)[out].erase( unique((*faultList)[out].begin(), (*faultList)[out].end()), (*faultList)[out].end());
//cout<<"to check whether its updatin the outputnode"<<endl;
/*for(int f=0;f<(*faultList)[out].size();f++)
{ //cout<<"fault list at op node element "<< f << "-> "<<(*faultList)[out][f]<<endl;
}*/

}


int main()
{
 system ("clear");
 int p[210]={0};  //array that stores the input values
 int gate[300] = {0} ;  //number of gates
 int valdef[210] = {0}; //validity 
 int i,j,k,val;
 
 //type_info g_type[20];
 
 inverter inv_arr[1000];
 andgate and_arr[1000];
 buffer buf_arr[1000];
 orgate or_arr[1000];
 norgate nor_arr[1000];
 nandgate nand_arr[1000];

 int inv_idx = 0;
 int buf_idx = 0;
 int and_idx=0;
 int or_idx = 0;
 int nor_idx=0;
 int nand_idx=0;
 vector<int> ip_val;//vector containing all the primary input nodes
 vector<int> op_val;//vector containing all the primary output nodes
 gates i_gates[300];
 
 string line;
 std::vector<std::string> seglist;
 ifstream myfile ("c17_ckt.txt",ios::in);
 if(myfile.is_open())
 { //cout<<"**********VALUE OF P = "<< p << "*********************\n";
    for(i=0;getline(myfile,line);i++)
    {
        stringstream fline(line);
        while(getline(fline,line,' '))
        {
            seglist.push_back(line);
        }
        ////cout << seglist[0] << "\t";
        if(seglist[0].compare("INV") == 0)
        {   //cout<<"INV"<<endl;
            //cout <<".." << seglist[0]<<"\n";
            i_gates[i].id = i;
            i_gates[i].ptr = inv_arr + inv_idx; 
            inv_idx++;
            val = atoi(seglist[1].c_str())-1;
//        static_cast<inverter *>(i_gates[i].ptr)->LIp.pus_back(val+1);// change [i] to push_back
          static_cast<inverter *>(i_gates[i].ptr)->in= val+1;// change [i] to push_back
            static_cast<inverter *>(i_gates[i].ptr)->ip= p + val;
            static_cast<inverter *>(i_gates[i].ptr)->def1= valdef + val;
          val = atoi(seglist[2].c_str())-1;
        static_cast<inverter *>(i_gates[i].ptr)->Op.push_back(val+1); //This would directly include the output node in the fault list. when we call in error function, we can append it with new values as any gate output will surely have the output node in fault list, only its input changes. 
          static_cast<inverter *>(i_gates[i].ptr)->out= val+1;
         // (*faultList)[out].push_back(out);
            static_cast<inverter *>(i_gates[i].ptr)->op= p + val;
            static_cast<inverter *>(i_gates[i].ptr)->def2= valdef + val;
            i_gates[i].complete = valdef+val;
        }
    
        if(seglist[0].compare("BUF") == 0)
        {   //cout<<"BUF"<<endl;
            ////cout <<".." << seglist[0];
            i_gates[i].id = i;
            i_gates[i].ptr = buf_arr + buf_idx; 
            buf_idx++;
            val = atoi(seglist[1].c_str())-1;
//        static_cast<buffer *>(i_gates[i].ptr)->LIp.push_back(val+1);// change [i] to push_back
          static_cast<buffer *>(i_gates[i].ptr)->in= val+1;// change [i] to push_back
            static_cast<buffer *>(i_gates[i].ptr)->ip= p + val;
            static_cast<buffer *>(i_gates[i].ptr)->def1= valdef + val;
            val = atoi(seglist[2].c_str())-1;
          static_cast<buffer *>(i_gates[i].ptr)->Op.push_back(val+1);
          static_cast<buffer *>(i_gates[i].ptr)->out= val+1;
         // (*faultList)[out].push_back(out);
            static_cast<buffer *>(i_gates[i].ptr)->op= p + val;
            static_cast<buffer *>(i_gates[i].ptr)->def2= valdef + val;
            i_gates[i].complete = valdef+val;
        }
    
        if(seglist[0].compare("AND") == 0)
        {
           //cout<<"AND"<<endl;
 //cout <<".." << seglist[0]<<"\n";
            i_gates[i].id = i;
            i_gates[i].ptr = and_arr + and_idx; 
            and_idx++;
            val = atoi(seglist[1].c_str())-1;
//        static_cast<andgate *>(i_gates[i].ptr)->LIp1.push_back(val+1);// change [i] to push_back
          static_cast<andgate *>(i_gates[i].ptr)->in1= val+1;// change [i] to push_back
            static_cast<andgate *>(i_gates[i].ptr)->ip1= p + val;
            static_cast<andgate *>(i_gates[i].ptr)->def1= valdef + val;
            val = atoi(seglist[2].c_str())-1;
//        static_cast<andgate *>(i_gates[i].ptr)->LIp2.push_back(val+1);
          static_cast<andgate *>(i_gates[i].ptr)->in2= val+1;
            static_cast<andgate *>(i_gates[i].ptr)->ip2= p + val;
            static_cast<andgate *>(i_gates[i].ptr)->def2= valdef + val;
            val = atoi(seglist[3].c_str())-1;
         static_cast<andgate *>(i_gates[i].ptr)->Op.push_back(val+1);
          static_cast<andgate *>(i_gates[i].ptr)->out= val+1;
         // (*faultList)[out].push_back(out);
            static_cast<andgate *>(i_gates[i].ptr)->op= p + val;
            static_cast<andgate *>(i_gates[i].ptr)->def3= valdef + val;
            i_gates[i].complete = valdef+val;
 }
        
        if(seglist[0].compare("NAND") == 0)
        {// cout<<"NAND"<<endl;
            ////cout <<".." << seglist[0];
            i_gates[i].id = i;
            i_gates[i].ptr = nand_arr + nand_idx; 
            nand_idx++;
            val = atoi(seglist[1].c_str())-1;
//        static_cast<nandgate *>(i_gates[i].ptr)->LIp1.push_back(val+1);// change [i] to push_back
          static_cast<nandgate *>(i_gates[i].ptr)->in1= val+1;// change [i] to push_back
            static_cast<nandgate *>(i_gates[i].ptr)->ip1= p + val;
            static_cast<nandgate *>(i_gates[i].ptr)->def1= valdef + val;
            val = atoi(seglist[2].c_str())-1;
//        static_cast<nandgate *>(i_gates[i].ptr)->LIp2.push_back(val+1);
          static_cast<nandgate *>(i_gates[i].ptr)->in2= val+1;
            static_cast<nandgate *>(i_gates[i].ptr)->ip2= p + val;  
            static_cast<nandgate *>(i_gates[i].ptr)->def2= valdef + val;
            val = atoi(seglist[3].c_str())-1;
          static_cast<nandgate *>(i_gates[i].ptr)->Op.push_back(val+1);
          static_cast<nandgate *>(i_gates[i].ptr)->out= val+1;
         // (*faultList)[out].push_back(out);
            static_cast<nandgate *>(i_gates[i].ptr)->op= p + val;
            static_cast<nandgate *>(i_gates[i].ptr)->def3= valdef + val;
            i_gates[i].complete = valdef+val;
        }
       
        if(seglist[0].compare("OR") == 0)
        {   //cout<<"OR"<<endl;
            //cout <<".." << seglist[0];
            i_gates[i].id = i;
            i_gates[i].ptr = or_arr + or_idx; 
            or_idx++;
            val = atoi(seglist[1].c_str())-1;
//        static_cast<orgate *>(i_gates[i].ptr)->LIp1.push_back(val+1);// change [i] to push_back
          static_cast<orgate *>(i_gates[i].ptr)->in1= val+1;// change [i] to push_back
            static_cast<orgate *>(i_gates[i].ptr)->ip1= p + val;
            static_cast<orgate *>(i_gates[i].ptr)->def1= valdef + val;
            val = atoi(seglist[2].c_str())-1;
//        static_cast<orgate *>(i_gates[i].ptr)->LIp2.push_back(val+1);
          static_cast<orgate *>(i_gates[i].ptr)->in2= val+1;
            static_cast<orgate *>(i_gates[i].ptr)->ip2= p + val;  
            static_cast<orgate *>(i_gates[i].ptr)->def2= valdef + val;
            val = atoi(seglist[3].c_str())-1;
          static_cast<orgate *>(i_gates[i].ptr)->Op.push_back(val+1);
          static_cast<orgate *>(i_gates[i].ptr)->out= val+1;
          //(*faultList)[out].push_back(out);
            static_cast<orgate *>(i_gates[i].ptr)->op= p + val;
            static_cast<orgate *>(i_gates[i].ptr)->def3= valdef + val;
            i_gates[i].complete = valdef+val;
        }
        
        if(seglist[0].compare("NOR") == 0)
        {     //  cout<<"NOR"<<endl;
            ////cout <<".." << seglist[0];
            i_gates[i].id = i;
            i_gates[i].ptr = nor_arr + nor_idx; 
            nor_idx++;
            val = atoi(seglist[1].c_str())-1;
//        static_cast<norgate *>(i_gates[i].ptr)->LIp1.push_back(val+1);// change [i] to push_back
          static_cast<norgate *>(i_gates[i].ptr)->in1= val+1;// change [i] to push_back
            static_cast<norgate *>(i_gates[i].ptr)->ip1= p + val;
            static_cast<norgate *>(i_gates[i].ptr)->def1= valdef + val;
            val = atoi(seglist[2].c_str())-1;
      //    static_cast<norgate *>(i_gates[i].ptr)->LIp2.push_back(val+1);
          static_cast<norgate *>(i_gates[i].ptr)->in2= val+1;
            static_cast<norgate *>(i_gates[i].ptr)->ip2= p + val;  
            static_cast<norgate *>(i_gates[i].ptr)->def2= valdef + val;
            val = atoi(seglist[3].c_str())-1;
          static_cast<norgate *>(i_gates[i].ptr)->Op.push_back(val+1);
          static_cast<norgate *>(i_gates[i].ptr)->out= val+1;
          //(*faultList)[out].push_back(out);
            static_cast<norgate *>(i_gates[i].ptr)->op= p + val;
            static_cast<norgate *>(i_gates[i].ptr)->def3= valdef + val;
            i_gates[i].complete = valdef+val;
        }



        if(seglist[0].compare("INPUT") == 0)
        {   cout<<"The number of primary inputs :"<< (seglist.size()-2)<<endl;
            for(j=1; j < seglist.size()-1 ; j++)
            {
                ip_val.push_back(atoi(seglist[j].c_str())-1);

       }
       // below is the loop where u need to assign the primary input node fault list to global list 
//cout<<"input nodes are"<<endl; 
for(int i=0;i<ip_val.size();i++)
      {     
             size_t PI=ip_val[i]+1;
      //    cout<<"PI:"<<PI<<endl;
            (*faultList)[PI].push_back(PI);
      }   
        }

        
        if(seglist[0].compare("OUTPUT") == 0)
        {
            for(j=1; j < seglist.size()-1 ; j++)
            {
                op_val.push_back(atoi(seglist[j].c_str())-1);
            }
            
        }
        seglist.clear();
    }       

    myfile.close();
    num_gates = i - 2 ;
cout<<"no of gates:"<<num_gates<<endl;
}

ifstream myfile1 ("TG_P17.txt",ios::in);
 if(myfile1.is_open())
 {
/*//cout<<"to clear fault list"<<endl;
for(int c=0;c<(*faultList).size();c++)
{
(*faultList)[c].clear();
}*/// cout<<"reading s298f_2.chat"<<endl;

    for(k=0;getline(myfile1,line);k++)
    {
        memset(valdef, 0, sizeof(valdef));
        for(j=0; j<line.length();j++)  //assign input
        { 
      //    cout<<"entering here"<<endl;
                  p[ip_val[j]] = line.at(j) - '0';//
               
            //cout<<"p[ip_val"<<j<<"]= "<<p[ip_val[j]]<<endl;  
                valdef[ip_val[j]] = 1; // if its defined
      
      }

        while (!(comp(i_gates)))
        { //cout<<"mostly repeating this*********************"<<endl;
            for (i=0;i<inv_idx;i++)
            {     
                if((*inv_arr[i].def1))
                 {      //cout<<"calling inv's functions"<<endl;
                inv_arr[i].inverter::func();
                  //cout<<"evaluated for logic value"<<endl;
                (*faultList)[inv_arr[i].out].push_back(inv_arr[i].out);
                  //cout<<"pushing to out node"<<endl;
                  /*  for(int x=0;x<(*faultList)[inv_arr[i].out].size();x++)
            { cout<<"index"<<inv_arr[i].out<<endl;
                 cout <<"THE op at node INV op "<<(*faultList)[inv_arr[i].out][x]<<endl; 
             }*/
            inv_arr[i].inverter::error();

                    (*inv_arr[i].def2) = 1;
                  //cout<<"inv evaluated"<<endl;
                }
            }
                
            for (i=0;i<buf_idx;i++)
            {     //cout<<"calling buf's func"<<endl;
                if((*buf_arr[i].def1))
                {
                    buf_arr[i].buffer::func();
                (*faultList)[buf_arr[i].out].push_back(buf_arr[i].out);
                buf_arr[i].buffer::error();
              (*buf_arr[i].def2) = 1;  
               } 
            }
            
            for (i=0;i<and_idx;i++)
            {    
                if((*and_arr[i].def1)&&(*and_arr[i].def2))
                { //    cout<<"and_functions"<<endl;
                    and_arr[i].andgate::func();
                (*faultList)[and_arr[i].out].push_back(and_arr[i].out);
      /*    for(int x=0;x<(*faultList)[and_arr[i].out].size();x++)
            { //cout<<"index"<<and_arr[i].out<<endl;
                 //cout <<"THE op at node od and op "<<(*faultList)[and_arr[i].out][x]<<endl; 
             }*/
            and_arr[i].andgate::error();
                    (*and_arr[i].def3) = 1;
                }
            }
            
            for (i=0;i<nand_idx;i++)
            {     //cout<<"nand functions"<<endl;
                if((*nand_arr[i].def1)&&(*nand_arr[i].def2))
                {
                    nand_arr[i].nandgate::func();
                (*faultList)[nand_arr[i].out].push_back(nand_arr[i].out);
                    nand_arr[i].nandgate::error();
                    (*nand_arr[i].def3) = 1;
            }
            }
        
            for (i=0;i<or_idx;i++)
            {     //    cout<<"or functions"<<endl;
                if((*or_arr[i].def1)&&(*or_arr[i].def2))
                {
                    or_arr[i].orgate::func();
                (*faultList)[or_arr[i].out].push_back(or_arr[i].out);
                    or_arr[i].orgate::error();
                    (*or_arr[i].def3) = 1;
                }
            }
        
            for (i=0;i<nor_idx;i++)
            {    //     cout<<"nor functions"<<endl;
                if((*nor_arr[i].def1)&&(*nor_arr[i].def2))
                {
                    nor_arr[i].norgate::func();
               (*faultList)[nor_arr[i].out].push_back(nor_arr[i].out);
                    nor_arr[i].norgate::error();
                    (*nor_arr[i].def3) = 1;
        
                }
            }
    
        }
vector<int> index_op;
vector<int> C2(500);


 //cout << "Output nodes are : " <<endl;
for(j=0;j<op_val.size();j++)
        {
            //cout << op_val[j]+1 <<"\t";
      index_op.push_back(op_val[j]+1);
        //to find fault list at op nodes
      ////cout << "fault list at "<< op_va
      }
//cout<<"\n";
size_t op_nodes=index_op.size();
////cout<<"NUM OF OP NODES:"<<op_nodes<<endl;

for(int j=0; j<op_nodes; j++)
{
      size_t row_id=index_op[j];
      size_t row_length=(*faultList)[row_id].size();
      //cout<<"fault list at node " << row_id<<endl;
      cout<<"Fault detected at node " <<row_id<<endl;
      for(i=0;i<row_length;i++)
      {
//    cout<<(*faultList)[row_id][i]<<"\t";
      size_t fault_value = (p[(*faultList)[row_id][i]]);
      cout<<"STUCK AT "<<fault_value<<" FAULT AT NODE "<<(*faultList)[row_id][i]<<endl;
      }
cout<<"\n";
}

size_t Nodes=0;
//to count non zero fault nodes i.e total nodes in the ckt
for(int n=0;n<(*faultList).size();n++)
{
if((*faultList)[n].size()!=0)
{
 Nodes++;
}
}




        for(j=0;j<op_val.size();j++)
        {
            cout << "Output at node " << op_val[j]+1 << " is " << p[op_val[j]] << "\n";
        }
        cout << "Finished pattern " << k+1 << "\n\n";
  //cout<<"to clear fault list"<<endl;
for(int c=0;c<(*faultList).size();c++)
{
(*faultList)[c].clear();
}
  }
}
myfile1.close();

return 0;

}












