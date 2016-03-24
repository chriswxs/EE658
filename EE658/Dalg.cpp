

/*=======================================================================
  A simple parser for "self" format

  The circuit format (called "self" format) is based on outputs of
  a ISCAS 85 format translator written by Dr. Sandeep Gupta.
  The format uses only integers to represent circuit information.
  The format is as follows:

1        2        3        4           5           6 ...
------   -------  -------  ---------   --------    --------
0 GATE   outline  0 IPT    #_of_fout   #_of_fin    inlines
                  1 BRCH
                  2 XOR(currently not implemented)
                  3 OR
                  4 NOR
                  5 NOT
                  6 NAND
                  7 AND

1 PI     outline  0        #_of_fout   0
2 FB     outline  1 BRCH   inline
3 PO     outline  2 - 7    0           #_of_fin    inlines




                                    Author: Chihang Chen
                                    Date: 9/16/94

=======================================================================*/

/*=======================================================================
  - Write your program as a subroutine under main().
    The following is an example to add another command 'lev' under main()

enum e_com {READ, PC, HELP, QUIT, LEV};
#define NUMFUNCS 5
int cread(), pc(), quit(), lev();
struct cmdstruc command[NUMFUNCS] = {
   {"READ", cread, EXEC},
   {"PC", pc, CKTLD},
   {"HELP", help, EXEC},
   {"QUIT", quit, EXEC},
   {"LEV", lev, CKTLD},
};

lev()
{
   ...
}
=======================================================================*/

#include <stdio.h>
//#include <ctype.h>
#include <string.h>
#include <cstdlib>
#include <deque>
#include <stdlib.h>
#include <iostream>
#include <vector>
#define MAXLINE 81               /* Input buffer size */
#define MAXNAME 31               /* File name size */

#define Upcase(x) ((isalpha(x) && islower(x))? toupper(x) : (x))
#define Lowcase(x) ((isalpha(x) && isupper(x))? tolower(x) : (x))

using namespace std;

enum e_com {READ, PC, HELP, QUIT};
enum e_state {EXEC, CKTLD};         /* Gstate values */
enum e_ntype {GATE, PI, FB, PO};    /* column 1 of circuit format */
enum e_gtype {IPT, BRCH, XOR, OR, NOR, NOT, NAND, AND};  /* gate types */
//enum e_FiveV {ZERO, ONE , X, D, Dbar};

struct cmdstruc {
   char name[MAXNAME];        /* command syntax */
   //int (*fptr)();             /* function pointer of the commands */
   enum e_state state;        /* execution state sequence */
};
 struct NSTRUC {
   unsigned indx;             /* node index(from 0 to NumOfLine - 1 */
   unsigned num;              /* line number(May be different from indx */
   enum e_gtype type;         /* gate type */
   unsigned fin;              /* number of fanins */
   unsigned fout;             /* number of fanouts */
   
   int * unodes;   /* pointer to array of up nodes */
   int * dnodes;   /* pointer to array of down nodes */
   int level;                 /* level of the gate output */
   int level_not_assign;
   int sa_parent[2];
   int value;
   int assign_level;
   
} ;

/*(struct fault {
	bool  isSa1;
    int   parent;
	int   index;
	//int   num;
};*/

/*----------------- Command definitions ----------------------------------*/
#define NUMFUNCS 4
int cread(char * cp);
//int cread();
int pc( char * cp);
//int pc( );
int help();
int quit();
void levelization();
char * gname( int tp);
void allocate();
void clear();
int Preprocessing();
bool imply_and_check( );
bool DalgCall( int fault);
int Dalg_count=0;
int getDfront();
bool Dalg(int level) ;
 cmdstruc command[NUMFUNCS] = {
   {"READ",  EXEC},
   {"PC",   CKTLD},
   {"HELP",  EXEC},
   {"QUIT",  EXEC},
};

/*------------------------------------------------------------------------*/
enum e_state Gstate = EXEC;     /* global exectution sequence */
NSTRUC *Node;                   /* dynamic array of nodes */
NSTRUC **Pinput;                /* pointer to array of primary inputs */
NSTRUC **Poutput;               /* pointer to array of primary outputs */
int *node_index_queue;
int Nnodes;                     /* number of nodes */
int Npi;                        /* number of primary inputs */
int Npo;                        /* number of primary outputs */
int Done = 0;                   /* status bit to terminate program */
int queue_index=0;
int max_level=0;
int check_fault=0;
vector<int> faults;
deque<int> Dfront;
vector<int> Jfront;
vector<int> imply;
vector<int> detected_fault;
vector< vector<int> > test_vectors;
/*------------------------------------------------------------------------*/




/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: shell
description:
  This is the main program of the simulator. It displays the prompt, reads
  and parses the user command, and calls the corresponding routines.
  Commands not reconized by the parser are passed along to the shell.
  The command is executed according to some pre-determined sequence.
  For example, we have to read in the circuit description file before any
  action commands.  The code uses "Gstate" to check the execution
  sequence.
  Pointers to functions are used to make function calls which makes the
  code short and clean.
-----------------------------------------------------------------------*/
int main(int argc, char * argv[])
{
   //enum e_com com;
   int com;
   char cline[MAXLINE], wstr[MAXLINE], *cp;
   FILE* outputFile;
  
   FILE* readfault;
	cread(argv[1]);      
	faults.clear();
//	Preprocessing();
	
	int num_fault_Detected=0;
	
	outputFile = fopen(argv[2], "w");
	readfault  = fopen(argv[3], "r");
	
	int fault_ind;
	int sa;
	int num_falut_Detected=0;
	while(fscanf(readfault, "%d %d", &fault_ind, &sa) != EOF) {
	   faults.push_back(  (fault_ind<<1)+sa );
	}
	///
	detected_fault.clear();
	for(int j=0; j<faults.size(); j++){
	 // bool find_fault;
	  // DalgCall(faults[j]);
		if( DalgCall(faults[j])) { //cout<< "detected fault "<<  (faults[j]>>1)<<" sa"<<faults[j]%2<<endl;
		num_fault_Detected++;
		detected_fault.push_back(faults[j]);
		}
	}
	
	for (int j=0; j<test_vectors.size(); j++){
	   vector<int> tmp = test_vectors[j];
	   fprintf(outputFile, "%d %d ", (detected_fault[j]>>1) , detected_fault[j]%2);
	   for(int m=0;m< tmp.size(); m++){
			fprintf(outputFile, "%d", tmp[m]);
	   }
	   fprintf(outputFile, "\n");
	}
	
	///
	
	//detected_fault.clear();
    fclose(outputFile);
	fclose(readfault);
	cout<<"Dalg: "<< argv[1]<< endl;
	cout<<"============================="<<endl;
	cout<<"Received fault list size\t Number of test vectors\t\n" <<endl;
	cout<<faults.size()<<"\t\t\t     " << test_vectors.size()<< "\t"<<endl;
	cout<<"============================="<<endl;
	
		 
	
   
   return 1;
}

/*-----------------------------------------------------------------------
input: circuit description file name
output: nothing
called by: main
description:
  This routine reads in the circuit description file and set up all the
  required data structure. It first checks if the file exists, then it
  sets up a mapping table, determines the number of nodes, PI's and PO's,
  allocates dynamic data arrays, and fills in the structural information
  of the circuit. In the ISCAS circuit description format, only upstream
  nodes are specified. Downstream nodes are implied. However, to facilitate
  forward implication, they are also built up in the data structure.
  To have the maximal flexibility, three passes through the circuit file
  are required: the first pass to determine the size of the mapping table
  , the second to fill in the mapping table, and the third to actually
  set up the circuit information. These procedures may be simplified in
  the future.
-----------------------------------------------------------------------*/
int cread(char * cp)
//int cread(cp)
//char * cp
{
   char buf[MAXLINE];
   int ntbl, *tbl, i, j, k, nd, tp, fo, fi, ni = 0, no = 0;
   FILE *fd;
   NSTRUC *np;

   sscanf(cp, "%s", buf);
   if((fd = fopen(buf,"r")) == NULL) {
      printf("File %s does not exist!\n", buf);
      return 0;
   }
   if(Gstate >= CKTLD) clear();
   Nnodes = Npi = Npo = ntbl = 0;
   while(fgets(buf, MAXLINE, fd) != NULL) {
      if(sscanf(buf,"%d %d", &tp, &nd) == 2) {
         if(ntbl < nd) ntbl = nd;
         Nnodes ++;
         if(tp == (int)PI) Npi++;
         else if(tp == PO) Npo++;
      }
   }
   tbl = (int *) malloc(++ntbl * sizeof(int));

   fseek(fd, 0L, 0);
   i = 0;
   while(fgets(buf, MAXLINE, fd) != NULL) {
      if(sscanf(buf,"%d %d", &tp, &nd) == 2) tbl[nd] = i++;
   }
   allocate();

   fseek(fd, 0L, 0);
   while(fscanf(fd, "%d %d", &tp, &nd) != EOF) {
      np = &Node[tbl[nd]];
      np->num = nd;
	  np->sa_parent[0] = -1;
	  np->sa_parent[1] = -1;

	  
      if(tp == (int)PI) Pinput[ni++] = np;
      else if(tp == (int)PO) Poutput[no++] = np;
      switch(tp) {
         case PI:
         case PO:
         case GATE:
            fscanf(fd, "%d %d %d", &np->type, &np->fout, &np->fin);
			np->level_not_assign= np->fin;
            break;

         case FB:
            np->level_not_assign= np->fout = np->fin = 1;
            fscanf(fd, "%d", &np->type);
            break;

         default:
            printf("Unknown node type!\n");
            exit(-1);
         }
      np->unodes = (int *) malloc(np->fin  * sizeof(int));
      np->dnodes = (int *) malloc(np->fout * sizeof(int));
      for(i = 0; i < np->fin; i++) {
         fscanf(fd, "%d", &nd);
         np->unodes[i] = tbl[nd];
         }
      for(i = 0; i < np->fout; np->dnodes[i++] = -1);
	  

	  
      }
	for(i = 0; i < Nnodes; i++) {
      for(j = 0; j < Node[i].fin; j++) {
         np = &Node[ Node[i].unodes[j] ];
         k = 0;
         while(np->dnodes[k] != -1) k++;
         np->dnodes[k] = i;
         }
      }
	  
//	  for(i = 0; i < Nnodes; i++) {
	//     cout<<"num: " <<Node[i].num<< "indx: "<< Node[i].indx<<endl;
     // }
	  //for(i = 0; i < Npi; i++) {
	   //  cout<<"input num: " <<Pinput[i]->num<< "indx: "<< Pinput[i]->indx<<endl;
      //}
	levelization();  
	
   fclose(fd);
   Gstate = CKTLD;
   //printf("==> read circuit OK\n");
   return 1;
}

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: main
description:
  The routine prints out the circuit description from previous READ command.
-----------------------------------------------------------------------*/
int pc( char * cp)
//int pc( cp)
//char *cp;
{
   int i, j;
   NSTRUC *np;
  // char *gname();

   printf(" Node   Type \tIn     \t\t\tOut   \t level \n");
   printf("------ ------\t-------\t\t\t-------\t-----\n");
   for(i = 0; i<Nnodes; i++) {
      np = &Node[i];
      //printf("\t\t\t\t\t");
	  
	   printf("\r%5d  %s\t", np->num, gname(np->type));
	   for(j = 0; j<np->fin; j++) 
	  {printf("%d ", Node[np->unodes[j]].num);}
	  printf("\t\t\t");
      for(j = 0; j<np->fout; j++)
	  { printf("%d ",Node[np->dnodes[j]].num);}
     
      
      printf("\t%d \n", np->level);
   }
   printf("Primary inputs:  ");
   for(i = 0; i<Npi; i++) printf("%d ",Pinput[i]->num);
   printf("\n");
   printf("Primary outputs: ");
   for(i = 0; i<Npo; i++) printf("%d ",Poutput[i]->num);
   printf("\n\n");
   printf("Number of nodes = %d\n", Nnodes);
   printf("Number of primary inputs = %d\n", Npi);
   printf("Number of primary outputs = %d\n", Npo);

   return 1;
   }

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: main
description:
  The routine prints out help information for each command.
-----------------------------------------------------------------------*/
int help()
{
   printf("READ filename - ");
   printf("read in circuit file and creat all data structures\n");
   printf("PC - ");
   printf("print circuit information\n");
   printf("HELP - ");
   printf("print this help information\n");
   printf("QUIT - ");
   printf("stop and exit\n");
   return 1;
}

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: main
description:
  Set Done to 1 which will terminates the program.
-----------------------------------------------------------------------*/
int quit()
{
   Done = 1;
   return 1;
}

/*======================================================================*/

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: cread
description:
  This routine clears the memory space occupied by the previous circuit
  before reading in new one. It frees up the dynamic arrays Node.unodes,
  Node.dnodes, Node.flist, Node, Pinput, Poutput, and Tap.
-----------------------------------------------------------------------*/
void clear()
{
   int i;

  /* for(i = 0; i<Nnodes; i++) {
      delete[] Node[i].unodes;
      delete[] Node[i].dnodes;
   }
   delete[] Node;
   delete[] Pinput;
   delete[] Poutput;*/
   
   for(i = 0; i<Nnodes; i++) {
      free(Node[i].unodes);
      free(Node[i].dnodes);
   }
   free(Node);
   free(Pinput);
   free(Poutput);
   
   
   Gstate = EXEC;
}

/*-----------------------------------------------------------------------
input: nothing
output: nothing
called by: cread
description:
  This routine allocatess the memory space required by the circuit
  description data structure. It allocates the dynamic arrays Node,
  Node.flist, Node, Pinput, Poutput, and Tap. It also set the default
  tap selection and the fanin and fanout to 0.
-----------------------------------------------------------------------*/
void allocate()
{
   int i;
   
   Node = (NSTRUC *) malloc(Nnodes * sizeof(NSTRUC));
   node_index_queue = (int *) malloc((Nnodes+1)* sizeof(int));
   Pinput = (NSTRUC **) malloc(Npi * sizeof(NSTRUC *));
   Poutput = (NSTRUC **) malloc(Npo * sizeof(NSTRUC *));
  
   /* Node = new  NSTRUC  [Nnodes];
    node_index_queue = new int [Nnodes+1]  ;
    Pinput =  new NSTRUC * [Npi] ;
    Poutput = new NSTRUC * [Npo] ;
  */
   for(i = 0; i<Nnodes; i++) {
      Node[i].indx = i;
      Node[i].fin = Node[i].fout = 0;
	  Node[i].level = -1 ;
   }
}

/*-----------------------------------------------------------------------
input: gate type
output: string of the gate type
called by: pc
description:
  The routine receive an integer gate type and return the gate type in
  character string.
-----------------------------------------------------------------------*/
char *gname( int tp)

{
   switch(tp) {
      case 0: return("PI");
      case 1: return("BRANCH");
      case 2: return("XOR");
      case 3: return("OR");
      case 4: return("NOR");
      case 5: return("NOT");
      case 6: return("NAND");
      case 7: return("AND");
   }
}

/*
*/
void levelization() 

{
	NSTRUC *np;
	int ii,jj,kk;
	for(ii = 0; ii<Npi; ii++) {
		node_index_queue[queue_index] =(Pinput[ii]->indx);
		(Node[Pinput[ii]->indx]).level=0;
		queue_index++;
	}
		
		for( jj=0; jj<Nnodes; jj++) {
			np=&Node[node_index_queue[jj]];
			for( kk=0 ;  kk< (np->fout); kk++){
				//(np->level_not_assign)--;
				(Node[(np->dnodes[kk])].level_not_assign)-=1;
				if((Node[(np->dnodes[kk])].level_not_assign)==0){
					node_index_queue[queue_index] =(Node[(np->dnodes[kk])].indx);
					Node[(np->dnodes[kk])].level= (np->level)+1;
					if((np->level)+1> max_level) max_level = (np->level)+1; 
					//printf("pusch into %d, with node %d, level=%d\n", queue_index, (Node[(np->dnodes[kk])].indx),(np->level)+1);
					queue_index++;
				}
			}
		}
		
	//return 0;
	//cout<< "--> fininsh levelization" <<endl;
	
}

int Preprocessing (){
	
	int k;
	NSTRUC *np;
    for(k=0; k<= max_level; k++){
	
		for(int i=0; i< Nnodes; i++){
		      np = & Node[i];
			if(Node[i].level ==k) {
				
				if( Node[i].type== 3)      {	for(int j=1;j<np->fin;j++){  if( Node[np->unodes[j]].sa_parent[1] ==-1) {Node[np->unodes[j]].sa_parent[1]= (np->unodes[0]<<1)+1 ;}
																			 else {int check_index=Node[np->unodes[j]].sa_parent[1];
																					while ( Node[(int)(check_index>>1)].sa_parent[check_index%2] != -1) { check_index= Node[(int)(check_index>>1)].sa_parent[check_index%2]; }
																					Node[(int)(check_index>>1)].sa_parent[check_index%2] = (np->unodes[0]<<1)+1;
																				  }
																		  }     
												np->sa_parent[1] =(np->unodes[0]<<1)+1;    np->sa_parent[0] =(np->unodes[0]<<1);  
											} //or
				else if( Node[i].type== 4) {    for(int j=1;j<np->fin;j++){ if( Node[np->unodes[j]].sa_parent[1] ==-1) Node[np->unodes[j]].sa_parent[1]= (np->unodes[0]<<1)+1 ;
																			else {int check_index=Node[np->unodes[j]].sa_parent[1];
																					while ( Node[(int)(check_index>>1)].sa_parent[check_index%2] != -1) { check_index= Node[(int)(check_index>>1)].sa_parent[check_index%2]; }
																					Node[(int)(check_index>>1)].sa_parent[check_index%2] = (np->unodes[0]<<1)+1;
																				  }
																		 }   
												np->sa_parent[0] =(np->unodes[0]<<1)+1;    np->sa_parent[1] =(np->unodes[0]<<1); 
											} //nor
				else if (Node[i].type== 5) { np->sa_parent[1]= (np->unodes[0])<<1; np->sa_parent[0]= ((np->unodes[0])<<1 )+1; }
				else if (Node[i].type== 6) { 	for(int j=1;j<np->fin;j++){ if(Node[np->unodes[j]].sa_parent[0]==-1) Node[np->unodes[j]].sa_parent[0]= (np->unodes[0]<<1) ;
																			else {int check_index=Node[np->unodes[j]].sa_parent[0];
																					while ( Node[(int)(check_index>>1)].sa_parent[check_index%2] != -1) { check_index= Node[(int)(check_index>>1)].sa_parent[check_index%2]; }
																					Node[(int)(check_index>>1)].sa_parent[check_index%2] = (np->unodes[0]<<1);
																				  }
																		}       
												np->sa_parent[1] =(np->unodes[0]<<1);    np->sa_parent[0] =(np->unodes[0]<<1)+1;   
										   } //nand
				else if (Node[i].type== 7) { for(int j=1;j<np->fin;j++){  if(Node[np->unodes[j]].sa_parent[0]==-1) Node[np->unodes[j]].sa_parent[0]= (np->unodes[0]<<1) ;
																			else {int check_index=Node[np->unodes[j]].sa_parent[0];
																					while ( Node[(int)(check_index>>1)].sa_parent[check_index%2] != -1) { check_index= Node[(int)(check_index>>1)].sa_parent[check_index%2]; }
																					Node[(int)(check_index>>1)].sa_parent[check_index%2] = (np->unodes[0]<<1);
																				  }
																		}       
				                             np->sa_parent[0] =(np->unodes[0]<<1);    np->sa_parent[1] =(np->unodes[0]<<1)+1;   
											} //and
			
			}
		}
	} 	
    faults.clear();
    for(int i=0; i<Nnodes; i++)
    {
	 
	 np=& Node[i];
			if(np->sa_parent[0]==-1) {  faults.push_back( (int)(i<<1));    // cout<< np->num << " type: "<<gname(np->type) <<" sa0 "<<np->indx <<endl;
			}
			if(np->sa_parent[1]==-1) {  faults.push_back( (int)((i<<1)+1));// cout<< np->num << " type: "<<gname(np->type) <<" sa1 "<<np->indx <<endl; 
			}
	}

   return 1;	
}



bool unassign(int level) {  for(int kk=0;kk<Nnodes;kk++) { if(Node[kk].assign_level>level ) {Node[kk].value=2; Node[kk].assign_level=-1;} } return true;}
bool check_Dfront (int node){
								NSTRUC *np; np =&Node[node];
								int Dnum=0; int Dbarnum=0, Onenum=0, Zeronum=0;
									  for(int j=0;j<(np->fin); j++ ){
										  if(Node[ np->unodes[j]].value ==3) Dnum++;
										  else if(Node[ np->unodes[j]].value ==4) Dbarnum++;
										  else if(Node[ np->unodes[j]].value ==1) Onenum++;
										  else if(Node[ np->unodes[j]].value ==0) Zeronum++;
									  }
									 if( (Dnum==0 && Dbarnum==0)|| np->value!=2 ||(Dnum>0 && Dbarnum>0) || (Onenum>0 && (np->type==3 || np->type==4  || np->type==5 || np->type==1))
											|| (Zeronum>0 && (np->type==6 || np->type==7 || np->type==5 || np->type==1 )))
									 { return false;}
									 else { return true; }
 }
 
 int getDfront(){
 
 vector<int> tmp; tmp.clear(); //Dfront.clear();
 Dfront.clear();
 tmp.push_back((int)(check_fault>>1));
 
	 for(int i=0; i<tmp.size(); i++){
		NSTRUC *np =&Node[tmp[i]];
		if(np->fout>0){
			for( int k=0;k<np->fout; k++){
				if  ( check_Dfront(np->dnodes[k]) ){ 
						Dfront.push_back(np->dnodes[k]);
				}
				else if ( Node[np->dnodes[k]].value==3 || Node[np->dnodes[k]].value==4 ){
					tmp.push_back(np->dnodes[k]);
				}
			}
		}
	 }
	 return 0;
}
 
bool Dalg(int level) {
	
	Dalg_count++;
	if(Dalg_count> 20000) return false;
	for ( int kk=0;kk< Nnodes;kk++) {
	     if( Node[kk].assign_level> level) {  Node[kk].value=2; Node[kk].assign_level=-1; }
	
	}
	if(imply_and_check()){
		for(int kk=0; kk< imply.size(); kk++){ Node[imply[kk]].assign_level=level;  }  
		//getDfront();
		//cout<< "Dfront:"<< Dfront.size() << "imply: "<<imply.size() <<"level: "<<level<<endl;
		//if(imply.size()>0) cout<< imply[0]<< " value: "<<Node[imply[0]].value << "type:" <<gname(Node[imply[0]].type) <<endl;
	}
	else {
		for(int i=0; i<imply.size(); i++ ){  Node[imply[i]].value=2; Node[imply[i]].assign_level=-1; }
		//cout<< " false            " << "imply: "<<imply.size() <<"level: "<<level<<endl;
		//if(imply.size()>0) cout<< imply[0]<< "               value: "<<Node[imply[0]].value << "type:" <<gname(Node[imply[0]].type) <<endl;
	imply.clear(); return false;}
	imply.clear();
	//vector<int> Dfront; 
	//Dfront=getDfront();
	getDfront();
	//for(int kk=0; kk< Dfront.size(); kk++){ cout<< Dfront[kk]<<" ";  }
    //cout<<endl;	
	bool DatOut=false;
	NSTRUC *np;
    for(int i = 0; i<Npo; i++) {if(Poutput[i]->value==3||Poutput[i]->value==4) DatOut=true;}// check D or D' at output
	// propagate D frontier to output
	if(!DatOut) {
					if(Dfront.size()==0) { //cout<<" Dfront size=0"<<endl;
					unassign(level-1); 
					return false;}
					else  { 
						int iter_while= 0;
								for(int i = 0; i<Dfront.size(); i++) {
								
								//while (Dfront.size()!=0){
										
									 //if(iter_while> Dfront.size()) break;
									 //if( Node[Dfront[i]].fout >0 )
									 np =& Node[Dfront[i]];
									 
									  int Dnum=0; int Dbarnum=0, Onenum=0, Zeronum=0;
									  for(int j=0;j<(np->fin); j++ ){
									    
											   if(Node[ np->unodes[j]].value ==3) Dnum++;
										  else if(Node[ np->unodes[j]].value ==4) Dbarnum++;
										  else if(Node[ np->unodes[j]].value ==1) Onenum++;
										  else if(Node[ np->unodes[j]].value ==0) Zeronum++;
									  }
									  
									 if( (Dnum==0 && Dbarnum==0)|| np->value!=2 ||(Dnum>0 && Dbarnum>0) ){// Dfront.pop_front();
									 continue;}
									 
									 //-------
									 
									 
									 //----------
									 
									 if(Dnum>0)         {
										  if(np->type ==3 || np->type==7 || np->type==1 || np->type==2)      {   np->value=3; } 
										  else if(np->type ==4 || np->type==6 || np->type==5) {   np->value=4; }
										  
										  //Node[np->indx].assign_level=level+1;
                                          imply.push_back(np->indx);
										 for(int j=0;j<(np->fin); j++ ){
									     if( Node[ np->unodes[j]].value ==2){
										  
                                                 if(np->type ==3 || np->type==4 || np->type==2) {   Node[ np->unodes[j]].value=0; } 
											else if(np->type ==6 || np->type==7) {   Node[ np->unodes[j]].value=1; }
										  imply.push_back(np->unodes[j]);
										  }
									  }	

										  
									 }
									 else if(Dbarnum>0) {
									    	if(np->type ==3 || np->type==7  || np->type==1 || np->type==2)      {   np->value=4; } 
										 else if(np->type ==4 || np->type==6 || np->type==5) {   np->value=3; }
										 
										//  Node[np->indx].assign_level=level+1; 
										  imply.push_back(np->indx);
										 for(int j=0;j<(np->fin); j++ ){
									     if( Node[ np->unodes[j]].value ==2){
										  
                                                 if(np->type ==3 || np->type==4 || np->type==2) {   Node[ np->unodes[j]].value=0; } 
											else if(np->type ==6 || np->type==7)                {   Node[ np->unodes[j]].value=1; }
										  imply.push_back(np->unodes[j]);
										  }
										}
									 
									 }
									 int pop_ind=Dfront[0];
	//								 Dfront.pop_front();
									 
									/* for( int k=0;k<np->fout; k++){
											if  ( Node[np->dnodes[k]].value==2){ 
												if(check_Dfront(np->dnodes[k]))
													Dfront.push_back(np->dnodes[k]);
											}
										}*/
									 
									 if(Dalg(level+1)) { // cout<<"true Dfront:"<< i<<endl; 
									 return true;}
									 else { 
											 
											unassign(level); 
											getDfront();
										//	cout<<"false Dfront:"<< Dfront[i] << " i: "<< i<<" level: " <<level<<endl;
									 }
									// iter_while++;
			//						 cout<<"iter in while:"<< iter_while <<endl;
								}
								//Dfront.
								 //cout<<"finish Dfront search "<<endl;
								unassign(level-1);
								return false;
					}
	}
	//check Jfront
	//cout<< "enter Jfront"<<endl;
	vector<int> temp2; temp2.clear();
	for(int i=0;i <Jfront.size();i++){
	        int Xnum=0;
			int Dnum=0;
			int Dbarnum=0;
			int Onenum=0;
			int Zeronum=0;
			np = &Node[Jfront[i]];
			for(int j=0;j< np->fin ;j++){
													if      (Node[np->unodes[j]].value==2) Xnum++;
													else if (Node[np->unodes[j]].value==3) Dnum++;
													else if (Node[np->unodes[j]].value==4) Dbarnum++;
													else if (Node[np->unodes[j]].value==1) Onenum++; 
													else if (Node[np->unodes[j]].value==0) Zeronum++;
			}
			if(Node[Jfront[i]].value==1 && np->type==3) { // or 
				if(Xnum>0 && !(Dnum>0&& Dbarnum>0) && Onenum==0) {temp2.push_back(Jfront[i]);}
			}
			else if(Node[Jfront[i]].value==1 && np->type==6) { //  nand
				if(Xnum>0 && !(Dnum>0&& Dbarnum>0) && Zeronum==0) {temp2.push_back(Jfront[i]);}
			}
			else if(Node[Jfront[i]].value==0 && np->type==4){ // nor
				if(Xnum>0 && !(Dnum>0&& Dbarnum>0) && Onenum==0 ) {temp2.push_back(Jfront[i]);}
			}
			else if(Node[Jfront[i]].value==0 && np->type==7){ //and
				if(Xnum>0 && !(Dnum>0&& Dbarnum>0) && Zeronum==0) {temp2.push_back(Jfront[i]);}
			}
	}
	Jfront= temp2;
	if(temp2.size()==0) return true;
	for(int i=0;i <temp2.size();i++){
			int Xnum=0;
			int Dnum=0;
			int Dbarnum=0;
			int Onenum=0;
			int Zeronum=0;
			np = &Node[temp2[i]];
			for(int j=0;j< np->fin ;j++){
			    if(Node[np->unodes[j]].value==2){
					if(np->type==3 || np->type==4){
					    Node[np->unodes[j]].value =  1;   imply.push_back(np->unodes[j]);	
						 if(Dalg(level+1)) {return true;} else { unassign(level);}
						Node[np->unodes[j]].value =  0;   imply.push_back(np->unodes[j]);
						Node[np->unodes[j]].assign_level= level;

					//	if(imply_and_check()) {Node[np->unodes[j]].assign_level= level;	}     else { imply.clear(); Node[np->unodes[j]].value = 2; Node[np->unodes[j]].level = -1;unassign(level-1); return false;}
					}
					else if(np->type==7 || np->type==6){
						 Node[np->unodes[j]].value = 0;   imply.push_back(np->unodes[j]);	
						 if(Dalg(level+1)) {return true;} else { unassign(level);}
						 Node[np->unodes[j]].value =  1;   imply.push_back(np->unodes[j]);
						 Node[np->unodes[j]].assign_level= level;
						 
					//	 if(imply_and_check()) {Node[np->unodes[j]].assign_level= level;} 	 else { Node[np->unodes[j]].value = 2; Node[np->unodes[j]].level = -1; imply.clear();unassign(level-1); return false;}
					}
				}
			}
			imply.clear();
			unassign(level-1);
			return false;
	}
}

bool DalgCall( int fault){

	Dfront.clear();
	Jfront.clear(); imply.clear();
	Dalg_count=0;
	 for(int kk=0;kk< Nnodes; kk++){
		Node[kk].value= 2; // set to unknown 0=0, 1=1 , 2= X, 3=D, 4=Dbar
		Node[kk].assign_level=-1;
	 }
	  NSTRUC *np =&Node[(int) (fault>>1)];
	check_fault =fault;
	 for(int kk=0;kk< np->fout;kk++){ // if(fault%2==0) Node[np->dnodes[kk]].value=3;  else Node[np->dnodes[kk]].value=4;
										Dfront.push_back(Node[np->dnodes[kk]].indx);
	 }
	if(fault%2==0)				    {  np->value= 3; if(np->type==1) Node[np->unodes[0]].value=1; imply.push_back(np->unodes[0]);}//D 
	else 						    {  np->value= 4; if(np->type==1) Node[np->unodes[0]].value=0; imply.push_back(np->unodes[0]);}//D 
	
	 bool find=Dalg(0);
	 
	 
	 
	 if(find) { 
			vector<int> temp; temp.clear();
			for(int i=0;i<Npi  ;i++){
				if(Pinput[i]->value==3)
				temp.push_back(1);
				else if(Pinput[i]->value==4) temp.push_back(0);
				
				else temp.push_back(Pinput[i]->value);
			}
			test_vectors.push_back(temp);
			return true;
	 }
	 else return false;
}

bool imply_and_check( ){
	
	for(int kk=0; kk< imply.size(); kk++){
	   NSTRUC *np =&Node[imply[kk]];
	   if(np->value !=3 && np->value!=4) {
	   //backward imply
			   if(np->type == 0) return true;
			   else if( np->type == 1) {		
			   //if( (int)(check_fault>>1)==np->indx) continue;	
												for(int jj=0;jj<(np->fin);jj++){
														if(np->value==0) { if(Node[np->unodes[jj]].value==2) { Node[np->unodes[jj]].value = 0;   imply.push_back(np->unodes[jj]);}       else if(Node[np->unodes[0]].value!=0 ) {return false;}}
														if(np->value==1) { if(Node[np->unodes[jj]].value==2) { Node[np->unodes[jj]].value = 1;   imply.push_back(np->unodes[jj]);}       else if(Node[np->unodes[0]].value!=1 ) {return false;}}
													} 
			   
			   }// branch
			   else if   ( np->type ==2) { int num_inputX=0; int oneXind=0; int implyvalue=np->value; for(int jj=0;jj<(np->fin);jj++){ if(Node[np->unodes[jj]].value==1 || Node[np->unodes[jj]].value==0){implyvalue^=Node[np->unodes[jj]].value;}  else if(Node[np->unodes[jj]].value==3 || Node[np->unodes[jj]].value==4){return false;} else{  oneXind=jj;num_inputX++; }  }
									   if(num_inputX==1) { Node[np->unodes[oneXind]].value =  implyvalue;   imply.push_back(np->unodes[oneXind]);   }
									   else  {Jfront.push_back(np->indx);} 
									 } //xor
									 // j frontier : 1. at least two inputs are unknown  2. no control value of input(so that output can be implied)
				else if  ( np->type ==3) {
				
											if(np->value==0)  { 
													for(int jj=0;jj<(np->fin);jj++){
														if(Node[np->unodes[jj]].value!=0 && Node[np->unodes[jj]].value!=2)     { return false;}
														else if(Node[np->unodes[jj]].value==2){ Node[np->unodes[jj]].value = 0;   imply.push_back(np->unodes[jj]);   }
													}   
											}
											if(np->value==1){ bool has_controlvalue=false; int unknown_num=0;int oneXind=0; int Dnum=0; int Dbarnum=0;
												for(int jj=0;jj<(np->fin);jj++) {
													   if(Node[np->unodes[jj]].value==1) { has_controlvalue=true; break;}
													   else if (Node[np->unodes[jj]].value==2) { unknown_num++;  oneXind=jj;}
													   else if (Node[np->unodes[jj]].value==3) { Dnum++; }  
													   else if (Node[np->unodes[jj]].value==4) { Dbarnum++; }
												}
												if(has_controlvalue==false && unknown_num==0) {if(Dnum==0 || Dbarnum==0){ return false;}}
												else if(has_controlvalue==false && unknown_num ==1) {     
													if(!(Dnum>0 && Dbarnum>0)) //{Jfront.push_back(np->indx);}
													
													{Node[np->unodes[oneXind]].value =  1;   imply.push_back(np->unodes[oneXind]); }
												}              
												else if(has_controlvalue==false && unknown_num>1 && !((Dnum>0 && Dbarnum>0))) { Jfront.push_back(np->indx);}
											}
				}// or
				
				else if  ( np->type ==4) {
											if(np->value==1)  { 
													for(int jj=0;jj<(np->fin);jj++){
														if(Node[np->unodes[jj]].value!=0 && Node[np->unodes[jj]].value!=2)     { return false;}
														else if(Node[np->unodes[jj]].value==2){ Node[np->unodes[jj]].value = 0;   imply.push_back(np->unodes[jj]);   }
													}   
											}
											if(np->value==0){ bool has_controlvalue=false; int unknown_num=0;int oneXind=0; int Dnum=0; int Dbarnum=0;
												for(int jj=0;jj<(np->fin);jj++) {
													   if(Node[np->unodes[jj]].value==1) { has_controlvalue=true; break;}
													   else if (Node[np->unodes[jj]].value==2) { unknown_num++;  oneXind=jj;}
													   else if (Node[np->unodes[jj]].value==3) { Dnum++; }  
													   else if (Node[np->unodes[jj]].value==4) { Dbarnum++; }
												}
												if(has_controlvalue==false && unknown_num==0) {if(Dnum==0 || Dbarnum==0){ return false;}}
												else if(has_controlvalue==false && unknown_num ==1) {     
													if(!(Dnum>0 && Dbarnum>0)) 
													{Node[np->unodes[oneXind]].value =  1;   imply.push_back(np->unodes[oneXind]); }
												}              
												else  if(has_controlvalue==false && unknown_num>1 && !((Dnum>0 && Dbarnum>0))){ Jfront.push_back(np->indx);}
											}
				} // nor
				else if  ( np->type ==5) {
				
					if(np->value==0) { if(Node[np->unodes[0]].value==2) { Node[np->unodes[0]].value = 1;   imply.push_back(np->unodes[0]);}       else if(Node[np->unodes[0]].value!=1) {return false;}}
					if(np->value==1) { if(Node[np->unodes[0]].value==2) { Node[np->unodes[0]].value = 0;   imply.push_back(np->unodes[0]);}       else if(Node[np->unodes[0]].value!=0) {return false;}}
				
				} //not
				else if  ( np->type ==6) {
				
											if(np->value==0)  { 
													for(int jj=0;jj<(np->fin);jj++){
														if(Node[np->unodes[jj]].value!=1 && Node[np->unodes[jj]].value!=2)     { return false;}
														else if(Node[np->unodes[jj]].value==2){ Node[np->unodes[jj]].value = 1;   imply.push_back(np->unodes[jj]);   }
													}   
											}
											if(np->value==1){ bool has_controlvalue=false; int unknown_num=0;int oneXind=0; int Dnum=0; int Dbarnum=0;
												for(int jj=0;jj<(np->fin);jj++) {
													   if(Node[np->unodes[jj]].value==0) { has_controlvalue=true; break;}
													   else if (Node[np->unodes[jj]].value==2) { unknown_num++;  oneXind=jj;}
													   else if (Node[np->unodes[jj]].value==3) { Dnum++; }  
													   else if (Node[np->unodes[jj]].value==4) { Dbarnum++; }
												}
												if(has_controlvalue==false && unknown_num==0) {if(Dnum==0 || Dbarnum==0){ return false;}}
												else if(has_controlvalue==false && unknown_num ==1) {     
													if(!(Dnum>0 && Dbarnum>0)) //{Jfront.push_back(np->indx);}
													
													{Node[np->unodes[oneXind]].value =  0;   imply.push_back(np->unodes[oneXind]); }
												}              
												else  if(has_controlvalue==false && unknown_num>1 && !((Dnum>0 && Dbarnum>0))){ Jfront.push_back(np->indx);}
											}
				}// Nand
				
				else if  ( np->type ==7) {
											if(np->value==1)  { 
													for(int jj=0;jj<(np->fin);jj++){
														if(Node[np->unodes[jj]].value!=1 && Node[np->unodes[jj]].value!=2)     { return false;}
														else if(Node[np->unodes[jj]].value==2){ Node[np->unodes[jj]].value = 1;   imply.push_back(np->unodes[jj]);   }
													}   
											}
											if(np->value==0){ bool has_controlvalue=false; int unknown_num=0;int oneXind=0; int Dnum=0; int Dbarnum=0;
												for(int jj=0;jj<(np->fin);jj++) {
													   if(Node[np->unodes[jj]].value==0) { has_controlvalue=true; break;}
													   else if (Node[np->unodes[jj]].value==2) { unknown_num++;  oneXind=jj;}
													   else if (Node[np->unodes[jj]].value==3) { Dnum++; }  
													   else if (Node[np->unodes[jj]].value==4) { Dbarnum++; }
												}
												if(has_controlvalue==false && unknown_num==0) {if(Dnum==0 || Dbarnum==0){ return false;}}
												else if(has_controlvalue==false && unknown_num ==1) {     
													if(!(Dnum>0 && Dbarnum>0))// {Jfront.push_back(np->indx);}
													//else
													{Node[np->unodes[oneXind]].value =  0;   imply.push_back(np->unodes[oneXind]); }
												}              
												else if(has_controlvalue==false && unknown_num>1 && !((Dnum>0 && Dbarnum>0))) { Jfront.push_back(np->indx);}
											}
				} // and
		}
	   //forward imply
		//if( (int)(check_fault>>1)==np->indx) continue ;								 
									for(int jj=0;jj<(np->fout);jj++){
											NSTRUC *np_out;		
											np_out = &Node[np->dnodes[jj]];	
											if(np->dnodes[jj] == (check_fault>>1) ) continue;
											int Xnum=0;  
											int Dnum=0;
											int Dbarnum=0;
											int Onenum=0;
											int Zeronum=0;
											int expect_outvalue= 0;
											for(int jj=0;jj<(np_out->fin);jj++) {
													if      (Node[np_out->unodes[jj]].value==2) Xnum++;
													else if (Node[np_out->unodes[jj]].value==3) Dnum++;
													else if (Node[np_out->unodes[jj]].value==4) Dbarnum++;
													else if (Node[np_out->unodes[jj]].value==1) Onenum++; 
													else if (Node[np_out->unodes[jj]].value==0) Zeronum++;
											}											
												//branch
												if(np_out->type==1){
													if(np->value==0) { if(np_out->value==2) { np_out->value = 0;   imply.push_back(np_out->indx);}       else if( np_out->indx!=(int)(check_fault>>1) &&!(np_out->value==0) && ! (np_out->value==4)) {return false;}}
													if(np->value==1) { if(np_out->value==2) { np_out->value = 1;   imply.push_back(np_out->indx);}       else if( np_out->indx!=(int)(check_fault>>1) &&!(np_out->value==1) && ! (np_out->value==3)) {return false;}}
												    if(np->value==3 || np->value==4 ) {  if(np_out->value==2) { np_out->value = np->value;   imply.push_back(np_out->indx);} }
												}
												else if(np_out->type==5){
													if(np->value==0) { if(np_out->value==2) { np_out->value = 1;   imply.push_back(np_out->indx);}       else if(np_out->indx!=(int)(check_fault>>1) && !(np_out->value==1)  && ! (np_out->value==3) ) {return false;}}
													if(np->value==1) { if(np_out->value==2) { np_out->value = 0;   imply.push_back(np_out->indx);}       else if(np_out->indx!=(int)(check_fault>>1) && !(np_out->value==0) && ! (np_out->value==4)) {return false;}}
												    if(np->value==3) { if(np_out->value==2) { np_out->value = 4;   imply.push_back(np_out->indx);}       }
													if(np->value==4) { if(np_out->value==2) { np_out->value = 3;   imply.push_back(np_out->indx);}       }
												}//not
											    else if(np_out->type==2){
													//try to imply
														if(np_out->value ==2 && Xnum==0 ) { 
																if(Dnum%2==0 && Dbarnum%2==0)  		  {if(Onenum%2==1) {np_out->value=1;} else {np_out->value=0;} imply.push_back(np_out->indx);}
																else if(Dnum==Dbarnum && Onenum%2==1) { np_out->value=0; imply.push_back(np_out->indx);}
																else if(Dnum==Dbarnum && Onenum%2==0) { np_out->value=1; imply.push_back(np_out->indx);}
														}
														
														// check
														else if(np_out->value==1 && Xnum==0) { 
															if(Dnum%2==0 && Dbarnum%2==0)  		  {if(Onenum%2==1) { if(np_out->value==0) return false;} }
															//else if(Dnum==Dbarnum && Onenum%2==1) { np_out->value=0; imply.push_back(np_out->indx);}
															else if(Dnum==Dbarnum && Onenum%2==0) { if(np_out->value==0) return false;}
															
														}
														else if(np_out->value==0 && Xnum==0) { 
															if(Dnum%2==0 && Dbarnum%2==0)  		  {if(Onenum%2==0) { if(np_out->value==1) return false;} }
															else if(Dnum==Dbarnum && Onenum%2==1) { if(np_out->value==1) return false;}
														}
												}//xor
												
												else if(np_out->type==3){
													//try to imply
														if(np_out->value ==2 ) { 
																if(Onenum>0 ||  (Dnum>0&& Dbarnum>0))    {np_out->value=1; imply.push_back(np_out->indx);}
																else if(Xnum==0 && Zeronum==np_out->fin) {np_out->value=0; imply.push_back(np_out->indx);}
																else if( (Zeronum+Dnum) == np_out->fin)    {np_out->value=3; imply.push_back(np_out->indx);}
																else if( (Zeronum+Dbarnum) == np_out->fin) {np_out->value=4; imply.push_back(np_out->indx);}
														}
														// check
														else if(np_out->value==1  ) { if(!(Onenum>0 ||  (Dnum>0&& Dbarnum>0))) return false;}
														else if(np_out->value==0  ) {if(!(Xnum==0 && Zeronum==np_out->fin)) return false;}
														
												}//or
												else if(np_out->type==4){
													//try to imply
														if(np_out->value ==2 ) { 
																if(Onenum>0 ||  (Dnum>0&& Dbarnum>0))    {np_out->value=0; imply.push_back(np_out->indx);}
																else if(Xnum==0 && Zeronum==np_out->fin) {np_out->value=1; imply.push_back(np_out->indx);}
																else if( (Zeronum+Dnum) == np_out->fin)    {np_out->value=4; imply.push_back(np_out->indx);}
																else if( (Zeronum+Dbarnum) == np_out->fin) {np_out->value=3; imply.push_back(np_out->indx);}
														}
														// check
														else if(np_out->value==0 ) { if(!(Onenum>0 ||  (Dnum>0&& Dbarnum>0))) return false;}
														else if(np_out->value==1 ) {if(!(Xnum==0 && Zeronum==np_out->fin)) return false;}
												}//nor
												else if(np_out->type==6){
													//try to imply
														if(np_out->value ==2 ) { 
																if(Zeronum>0 ||  (Dnum>0&& Dbarnum>0))    {np_out->value=1; imply.push_back(np_out->indx);}
																else if(Xnum==0 && Onenum==np_out->fin)   {np_out->value=0; imply.push_back(np_out->indx);}
																else if( (Onenum+Dnum) == np_out->fin)    {np_out->value=4; imply.push_back(np_out->indx);}
																else if( (Onenum+Dbarnum) == np_out->fin) {np_out->value=3; imply.push_back(np_out->indx);}
														}
														// check
														else if(np_out->value==1 ) { if(!(Zeronum>0 ||  (Dnum>0&& Dbarnum>0))) return false;}
														else if(np_out->value==0 ) {if(!(Xnum==0 && Onenum==np_out->fin)) return false;}
												}//nand
												else if(np_out->type==7){
													//try to imply
														if(np_out->value ==2 ) { 
																if(Zeronum>0 ||  (Dnum>0&& Dbarnum>0))    {np_out->value=0; imply.push_back(np_out->indx);}
																else if(Xnum==0 && Onenum==np_out->fin)   {np_out->value=1; imply.push_back(np_out->indx);}
																else if( (Onenum+Dnum) == np_out->fin)    {np_out->value=3; imply.push_back(np_out->indx);}
																else if( (Onenum+Dbarnum) == np_out->fin) {np_out->value=4; imply.push_back(np_out->indx);}
														}
														// check
														else if(np_out->value==0) { if(!(Zeronum>0 ||  (Dnum>0&& Dbarnum>0))) return false;}
														else if(np_out->value==1) {if(!(Xnum==0 && Onenum==np_out->fin)) return false;}
														
														
												}//and
								    }  
										
	}
    
 return true;
}

/*========================= End of program ============================*/

