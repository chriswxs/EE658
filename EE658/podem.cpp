
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
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#define MAXLINE 81               /* Input buffer size */
#define MAXNAME 31               /* File name size */
//#define DEBUG

#define Upcase(x) ((isalpha(x) && islower(x))? toupper(x) : (x))
#define Lowcase(x) ((isalpha(x) && isupper(x))? tolower(x) : (x))
#define ONES 1

using namespace std;

enum e_com {READ, PC, HELP, QUIT};
enum e_state {EXEC, CKTLD};         /* Gstate values */
enum e_ntype {GATE, PI, FB, PO};    /* column 1 of circuit format */
enum e_gtype {IPT, BRCH, XOR, OR, NOR, NOT, NAND, AND};  /* gate types */

struct cmdstruc {
   char name[MAXNAME];        /* command syntax */
   //int (*fptr)();             /* function pointer of the commands */
   enum e_state state;        /* execution state sequence */
};

char * FileName; // file name added by Luhao

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


} ;

/*(struct fault {
	bool  isSa1;
    int   parent;
	int   index;
	//int   num;
};*/
// ------------------- Podem --------------
class podem_class
{
	NSTRUC *np;
	int Nfaults; // number of total faults
	int gtype;  // gate type
	int faultindx; //line indx of the fault
	int faultvalue; // fault s-a-vaule
	int myobjective;
	int objvalue;
	int pos;
	int backTraceGate;
	bool key;
	unsigned int *value1;
	unsigned int *value2; // good value of gate
	unsigned int *faultValue1;
	unsigned int *faultValue2; // faulty value of gate
	int  *D_frontier; // D_frontier array
	int *sched;      // scheduled on the wheel yet?
	int count;
	//int detected_count;
	bool isPathAvailable;
	bool isDetected;
	bool pathFound;

// ------- For simulation --------
	int **levelEvents; // event list for each level
	int *levelLen;     // event list length
	int numlevels;     // total number of levels in wheel
	int currLevel;     // current level
	int *activation;   // activation list for the current level in circuit
    int actLen;        // length of the activation list

public:
	int podem( char *);
	void ActFault(int);
	void setDontCares();
	int backtrace(int,int);
	void getDFrontier(int);
	void objective(int, bool);
	bool podemREcursion(int);
	bool CheckPathtoPO(int);       // check x-path from D-frontier to PO
	bool checkInputsForActivation(int);//to make sure that fault is getting excited
// -------- For simulation -------
	void insertEvent(int,int);
	void setupWheel();
	int retrieveEvent();
	void goodsim();
	void faultsim();
	int detected_count;

};


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
podem_class *PODEM_inst;
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
vector<int> faults;
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
int main(int argc, char ** argv)
{
   //enum e_com com;
   int com;
   char cline[MAXLINE], wstr[MAXLINE], *cp;
   FILE* readfault;
	//Preprocessing();
	cread(argv[1]); 
	readfault  = fopen(argv[3], "r");
	
	int fault_ind;
	int sa;
	int num_falut_Detected=0;
	faults.clear();
	while(fscanf(readfault, "%d %d", &fault_ind, &sa) != EOF) {
	   faults.push_back(  (fault_ind<<1)+sa );
	}
	
	//--------------  podem -----------
    PODEM_inst = new podem_class();
	PODEM_inst->podem( argv[2]);
	// -----------------------------
	cout<<"podem: "<< argv[1]<< endl;
	cout<<"============================="<<endl;
	cout<<"Received fault list size\t Number of test vectors\t\n" <<endl;
	cout<<faults.size()<<"\t\t\t     " << PODEM_inst->detected_count<< "\t"<<endl;
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
   FileName=&cp[0];
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

	levelization();
	

   fclose(fd);
   Gstate = CKTLD;
   //printf("==> OK\n");
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
   //char *gname(int);

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
			if(np->sa_parent[0]==-1) {  faults.push_back( (int)(i<<1));     cout<< np->num << " type: "<<gname(np->type) <<" sa0"<<endl; }
			if(np->sa_parent[1]==-1) {  faults.push_back( (int)((i<<1)+1)); cout<< np->num << " type: "<<gname(np->type) <<" sa1"<<endl; }
	}

   return 1;
}

// -------------------  podem ------------------------------


// all values are set to dont cares before starting PODEM
void podem_class::setDontCares(){
	for(int i=0;i<Nnodes;i++)
	{
		value1[i] = 0;
		value2[i] = ONES;
		faultValue1[i] = 0;
		faultValue2[i] = ONES;
	}
}

void podem_class::ActFault(int fault)
{
	faultindx=(fault>>1);
	faultvalue=fault%2;
	value1[faultindx] = ONES ^ faultvalue;
	value2[faultindx] = ONES ^ faultvalue;
	faultValue1[faultindx] = faultvalue;
	faultValue2[faultindx] = faultvalue;
	for(int i=0;i<Node[faultindx].fout;i++)
	{
		insertEvent(Node[Node[faultindx].dnodes[i]].level,Node[Node[faultindx].dnodes[i]].indx);
	}
}
void podem_class::setupWheel()
{
#ifdef DEBUG
	cout <<"inside setupWheel()"<<"\n";
#endif
	int *levsize_each;
	int levelSize=0;
	NSTRUC *np;
	int numLevels=max_level+1;
	levsize_each= new int [numLevels];
	for(int i=0; i<numLevels ; i++) // initialize the level size
	{
		levsize_each[i]=0;
	}
	for(int j=0; j<Nnodes;j++) // calculate the number at each level
	{
		sched[j]=0;
		np= &Node[node_index_queue[j]];
		levsize_each[np->level]++;
	}

	for(int i=0; i<numLevels ; i++) // calculate the largest level size
	{
		if(levsize_each[i]>levelSize)
		{
			levelSize=levsize_each[i];
		}
	}
#ifdef DEBUG
	for(int kk=0;kk<=max_level;kk++)
	{
		cout<<"level: "<< kk << " size is " <<levsize_each[kk]<<"\n";
	}
	cout << "levelSize = " << levelSize<<"\n";
#endif
	levelLen = new int [numLevels];
	levelEvents = new int *[numLevels];
	for(int i=0; i < numLevels; i++)
	{
		levelEvents[i] = new int[levelSize];
		levelLen[i] = 0;
	}
	activation = new int[levelSize];
}

void podem_class::insertEvent(int levelN, int gateinx)
{
#ifdef DEBUG
	cout << "inside insertEvent()" << "\n";
#endif
	levelEvents[levelN][levelLen[levelN]] = gateinx;
#ifdef DEBUG
	cout <<"\t gate indx="<<levelEvents[levelN][levelLen[levelN]]<<"\n";
	cout <<"\t gate num="<<Node[gateinx].num<<"\n";
#endif
	levelLen[levelN]++;
#ifdef DEBUG
	cout << "\t levelLen["<<levelN<<"]="<<levelLen[levelN]<<"\n";
#endif
}

int podem_class::retrieveEvent()
{
#ifdef DEBUG
	cout << "inside retrieveEvent()" << "\n";
	cout << "level: "<<currLevel<< "\n";
#endif
    while ((levelLen[currLevel] == 0) && (currLevel <= max_level))
        currLevel++;

    if ((currLevel <= max_level) && (levelLen[currLevel]>0))
    {
#ifdef DEBUG
		cout << "level: "<<currLevel<<": "<<levelLen[currLevel]<< "\n";
#endif
        levelLen[currLevel]--;
#ifdef DEBUG
		cout <<"\t gate num="<<Node[levelEvents[currLevel][levelLen[currLevel]]].num<<"\n";
#endif
        return(levelEvents[currLevel][levelLen[currLevel]]);
    }
    else
        return(-1);
}

void podem_class::faultsim()
{
	NSTRUC *np;
    int sucLevel;
    int gateN, predecessor, successor;
    int i;
    unsigned int val1, val2, tmpVal;

    currLevel = 0;
    actLen = 0;
	while (currLevel <= max_level)
    {
        gateN = retrieveEvent();
        if (gateN != -1 && gateN!=faultindx)    // if a valid event
        {
			sched[gateN]=0;
			np = & Node[gateN];
			switch (np->type)
			{
				case 0: // PI
					val1 = faultValue1[gateN];
					val2 = faultValue2[gateN];
					break;
				case 1: // BRANCH
					for (i=0;i<np->fin;i++)
					{
						val1 = faultValue1[Node[np->unodes[i]].indx];
						val2 = faultValue2[Node[np->unodes[i]].indx];
					}
					break;
				case 2: // XOR
					val1 = faultValue1[Node[np->unodes[0]].indx];
                    val2 = faultValue2[Node[np->unodes[0]].indx];
					for(i=1; i<np->fin; i++)
					{
						predecessor = Node[np->unodes[i]].indx;
						tmpVal = ONES^(((ONES^faultValue1[predecessor]) &
                                       (ONES^val1)) | (faultValue2[predecessor]&val2));
						val2 = ((ONES^faultValue1[predecessor]) & val2) |
								(faultValue2[predecessor] & (ONES^val1));
						val1 = tmpVal;
                    }
					break;
				case 3: // OR
					val1 = val2 = 0;
					for(i=0; i<np->fin; i++)
					{
						predecessor = Node[np->unodes[i]].indx;
						val1 |= faultValue1[Node[np->unodes[i]].indx];
						val2 |= faultValue2[Node[np->unodes[i]].indx];
                    }
					break;
				case 4: // NOR
					val1 = val2 = 0;
					for(i=0; i<np->fin; i++)
					{
						predecessor = Node[np->unodes[i]].indx;
						val1 |= faultValue1[Node[np->unodes[i]].indx];
						val2 |= faultValue2[Node[np->unodes[i]].indx];
                    }
					tmpVal = val1;
					val1 = ONES ^ val2;
					val2 = ONES ^ tmpVal;
					break;
				case 5: // NOT
					predecessor = Node[np->unodes[0]].indx;
					val1 = ONES ^ faultValue2[predecessor];
					val2 = ONES ^ faultValue1[predecessor];
					break;
				case 6: // NAND
					val1 = val2 = ONES;
					for(i=0; i<np->fin; i++)
					{
						predecessor = Node[np->unodes[i]].indx;
						val1 &= faultValue1[Node[np->unodes[i]].indx];
						val2 &= faultValue2[Node[np->unodes[i]].indx];
                    }
					tmpVal = val1;
					val1 = ONES ^ val2;
					val2 = ONES ^ tmpVal;
					break;
				case 7: // AND
					val1 = val2 = ONES;
					for(i=0; i<np->fin; i++)
					{
						predecessor = Node[np->unodes[i]].indx;
						val1 &= faultValue1[Node[np->unodes[i]].indx];
						val2 &= faultValue2[Node[np->unodes[i]].indx];
                    }
					break;
				default:
					cout<< "illegal gate type:  "<< np->type <<"\n";
					exit(-1);
			}
#ifdef DEBUG
			cout<<"inside faultsim()\n";
			cout<<"\t gatenum="<<Node[gateN].num<<"  faultValue1: "<<faultValue1[gateN]<<"->"<<val1<<"\n";
			cout<<"\t gatenum="<<Node[gateN].num<<"  faultValue2: "<<faultValue2[gateN]<<"->"<<val2<<"\n";
#endif

			 // if gate value changed
			if ((val1 != faultValue1[gateN]) || (val2 != faultValue2[gateN]))
			{
				faultValue1[gateN] = val1;
				faultValue2[gateN] = val2;
				for(i=0; i<np->fout; i++)
				{
					successor = Node[np->dnodes[i]].indx;
					sucLevel = Node[successor].level;
					if (sched[successor] == 0)
					{
						if (sucLevel != 0)
							insertEvent(sucLevel, successor);
						else // same level
						{
							activation[actLen] = successor;
							actLen++;
						}
						sched[successor] = 1;
					}
				}
			}

		}
	}
}

void podem_class::goodsim()
{
	NSTRUC *np;
    int sucLevel;
    int gateN, predecessor, successor;
    int i;
    unsigned int val1, val2, tmpVal;

    currLevel = 0;
    actLen = 0;
	while (currLevel <= max_level)
    {
        gateN = retrieveEvent();
        if (gateN != -1)    // if a valid event
        {
			sched[gateN]=0;
			np = & Node[gateN];
			switch (np->type)
			{
				case 0: // PI
					val1 = value1[gateN];
					val2 = value2[gateN];
					break;
				case 1: // BRANCH
						val1 = value1[Node[np->unodes[0]].indx];
						val2 = value2[Node[np->unodes[0]].indx];
					break;
				case 2: // XOR
					val1 = value1[Node[np->unodes[0]].indx];
                    val2 = value2[Node[np->unodes[0]].indx];
					for(i=1; i<np->fin; i++)
					{
						predecessor = Node[np->unodes[i]].indx;
						tmpVal = ONES^(((ONES^value1[predecessor]) &
                                       (ONES^val1)) | (value2[predecessor]&val2));
						val2 = ((ONES^value1[predecessor]) & val2) |
								(value2[predecessor] & (ONES^val1));
						val1 = tmpVal;
                    }
					break;
				case 3: // OR
					val1 = val2 = 0;
					for(i=0; i<np->fin; i++)
					{
						predecessor = Node[np->unodes[i]].indx;
						val1 |= value1[Node[np->unodes[i]].indx];
						val2 |= value2[Node[np->unodes[i]].indx];
                    }
					break;
				case 4: // NOR
					val1 = val2 = 0;
					for(i=0; i<np->fin; i++)
					{
						predecessor = Node[np->unodes[i]].indx;
						val1 |= value1[Node[np->unodes[i]].indx];
						val2 |= value2[Node[np->unodes[i]].indx];
                    }
					tmpVal = val1;
					val1 = ONES ^ val2;
					val2 = ONES ^ tmpVal;
					break;
				case 5: // NOT
					predecessor = Node[np->unodes[0]].indx;
					val1 = ONES ^ value2[predecessor];
					val2 = ONES ^ value1[predecessor];
					break;
				case 6: // NAND
					val1 = val2 = ONES;
					for(i=0; i<np->fin; i++)
					{
						predecessor = Node[np->unodes[i]].indx;
						val1 &= value1[Node[np->unodes[i]].indx];
						val2 &= value2[Node[np->unodes[i]].indx];
                    }
					tmpVal = val1;
					val1 = ONES ^ val2;
					val2 = ONES ^ tmpVal;
					break;
				case 7: // AND
					val1 = val2 = ONES;
					for(i=0; i<np->fin; i++)
					{
						predecessor = Node[np->unodes[i]].indx;
						val1 &= value1[Node[np->unodes[i]].indx];
						val2 &= value2[Node[np->unodes[i]].indx];
                    }
					break;
				default:
					cout<< "illegal gate type\n";
					exit(-1);
			}
#ifdef DEBUG
			cout<<"inside goodsim()\n";
			cout<<"\t gatenum="<<Node[gateN].num<<"  value1: "<<value1[gateN]<<"->"<<val1<<"\n";
			cout<<"\t gatenum="<<Node[gateN].num<<"  value2: "<<value2[gateN]<<"->"<<val2<<"\n";
#endif

#ifdef DEBUG
#endif

			 // if gate value changed
            if ((val1 != value1[gateN]) || (val2 != value2[gateN]))
			{
				value1[gateN] = val1;
				value2[gateN] = val2;
				for(i=0; i<np->fout; i++)
				{
					successor = Node[np->dnodes[i]].indx;
					sucLevel = Node[successor].level;
					//cout <<"good here!\t"<<sucLevel<<"  "<<successor<<"\n";
					if (sched[successor] == 0)
					{
						if (sucLevel != 0)
						{
							insertEvent(sucLevel, successor);
						}
						else // same level
						{
							activation[actLen] = successor;
							actLen++;
						}
						sched[successor] = 1;
					}
				}
			}
		}
	}
}

int podem_class::backtrace(int gate,int value){
	int g_b=gate;
	int val_b=value;
    int num_inversion=0;
    int back_num=0;
    while(Node[g_b].type!=0)
    {
    	back_num++;
#ifdef DEBUG
    	        cout<<"backtraced to "<<Node[g_b].num<<"\n";
#endif
        if(Node[g_b].type==4 || Node[g_b].type==5 || Node[g_b].type==6)
            num_inversion++;
        for(int i=0;i<Node[g_b].fin;i++)
        {
            if((value1[Node[g_b].unodes[i]] && !value2[Node[g_b].unodes[i]]) || (value2[Node[g_b].unodes[i]] && !value1[Node[g_b].unodes[i]]))
            {
#ifdef DEBUG
            	cout<<"found next gate "<<Node[Node[g_b].unodes[i]].num<<"\n";
#endif
                g_b=Node[g_b].unodes[i];
                break;
            }
        }
        if(back_num>5000)
        {
        	//cout<<"backtrace exceed the limit for fault: "<<Node[faultindx].num<<" "<<faultvalue<<"\n";
        	return -1;
        }
    }
    if(num_inversion%2!=0)
    {
#ifdef DEBUG
    	cout<<"checking\n";
#endif
        val_b=!val_b;
    }
    backTraceGate=g_b;
    return val_b;
}

bool podem_class::CheckPathtoPO(int pos){
#ifdef DEBUG
	cout<<"inside CheckPathtoPO\n";
#endif
	for(int m=0;m<Npo;m++)
	{
		if(pos==Poutput[m]->indx)
		{
			pathFound=true;
			return pathFound;
		}
	}
    for(int i=0;i<Node[pos].fout;i++)
    {
        if((value1[Node[pos].dnodes[i]] && !value2[Node[pos].dnodes[i]]) || (!value1[Node[pos].dnodes[i]] && value2[Node[pos].dnodes[i]])
            ||(faultValue1[Node[pos].dnodes[i]] && !faultValue2[Node[pos].dnodes[i]]) || (!faultValue1[Node[pos].dnodes[i]] && faultValue2[Node[pos].dnodes[i]]))
        {
#ifdef DEBUG
        	cout<<"x available in one of the fanouts of dfrontier\n";
#endif
            CheckPathtoPO(Node[pos].dnodes[i]);
        }
    }
    return pathFound;
}

bool podem_class::checkInputsForActivation(int fault){
	faultindx=(fault>>1);
	faultvalue=fault%2;
	int onecount=0,zerocount=0;
    /*for(int i=0;i<Node[faultindx].fin;i++)
    {
        if(value1[Node[faultindx].unodes[i]] && value2[Node[faultindx].unodes[i]])
            onecount++;
        if(!value1[Node[faultindx].unodes[i]] && !value2[Node[faultindx].unodes[i]])
            zerocount++;

    }*/
    if(value1[Node[faultindx].indx] && value2[Node[faultindx].indx])
        onecount++;
    if(!value1[Node[faultindx].indx] && !value2[Node[faultindx].indx])
        zerocount++;
    if(onecount>0 || zerocount>0)
        return true;
    else
        return false;
}

void podem_class::getDFrontier(int faultindx){
	//cout<<"faultindx: "<<faultindx<<"  gatenum  "<<Node[faultindx].num<<"\n";
	if(Node[faultindx].type==0 || Node[faultindx].type==1 )
	{
        for(int jj = 0; jj <Node[faultindx].fout; jj++)
        {
            getDFrontier(Node[faultindx].dnodes[jj]);
		//	cout<<"\ninside d-frontier: Getting next one "<<Node[Node[faultindx].dnodes[jj]].num<<"\n";
        }
	}
	else
	{
		if(((value1[faultindx] == 0 && value2[faultindx]) || (faultValue1[faultindx] == 0 && faultValue2[faultindx]) || (value1[faultindx] && value2[faultindx]==0) || (faultValue1[faultindx] && faultValue2[faultindx]==0) ))
	    {
	        D_frontier[count]=faultindx;
	       // cout<<"\t inside d-frontier: faultindx:"<<faultindx<<"  gatenum"<<Node[faultindx].num<<"\n";
	        count++;
	    }
		else if((value1[faultindx] && value2[faultindx] && (faultValue1[faultindx] == 0) && (faultValue2[faultindx] == 0)) || (value1[faultindx] == 0 && value2[faultindx] == 0 && (faultValue1[faultindx]) && (faultValue2[faultindx])))
	    {
	        for(int jj = 0; jj <Node[faultindx].fout; jj++)
	        {
	            getDFrontier(Node[faultindx].dnodes[jj]);
			//	cout<<"\ninside d-frontier: Getting next one "<<Node[Node[faultindx].dnodes[jj]].num<<"\n";
	        }
	    }
	}
	D_frontier[count]=0;


/*	for(int i=0;i<np->fout;i++){
		cout << "current Node: " <<np->num<<"\t fanout: "<<np->fout<<"\n";
		cout << "next Node: " <<Node[(np->dnodes[i])].num<<"\t for i= "<<i<<"\n";
		if((value1[Node[(np->dnodes[i])].indx] && value2[Node[(np->dnodes[i])].indx] && (faultValue1[Node[(np->dnodes[i])].indx] == 0) && (faultValue2[Node[(np->dnodes[i])].indx] == 0)) || (value1[Node[(np->dnodes[i])].indx] == 0 && value2[Node[(np->dnodes[i])].indx] == 0 && (faultValue1[Node[(np->dnodes[i])].indx]) && (faultValue2[Node[(np->dnodes[i])].indx])))  )
		{
			cout << "output NOde: " <<Node[(np->dnodes[i])].num<<"\n";
			getDFrontier(Node[(np->dnodes[i])].indx);
			cout<<Node[(np->dnodes[i])].num<<"\n";
			//cout<< "getDFrontier(Node[(np->dnodes[i])].num)[0]:  "<< Dfrontier[Dfrontier.size()-1]<< "\n";
		}
		else if(((value1[Node[(np->dnodes[i])].indx] == 0 && value2[Node[(np->dnodes[i])].indx]) || (faultValue1[Node[(np->dnodes[i])].indx] == 0 && faultValue2[Node[(np->dnodes[i])].indx]) || (value1[Node[(np->dnodes[i])].indx] && value2[Node[(np->dnodes[i])].indx]==0) || (faultValue1[Node[(np->dnodes[i])].indx] && faultValue2[Node[(np->dnodes[i])].indx]==0) ))
		{
			D_frontier[count]=Node[(np->dnodes[i])].indx;
			count++;
		}
	}*/

	//cout<< "Dfrontier:  "<< Dfrontier.size()<<"\t np-fout:  "<<np->fout<<"   \n ";
}

void podem_class::objective(int gate, bool value)
{
    objvalue=1;
#ifdef DEBUG
    cout<<"inside objective()"<<"\n";
#endif
    if((value1[gate] && !value2[gate])|| (value2[gate]&& !value1[gate]))
    {
        myobjective=gate;
        objvalue=!value;
#ifdef DEBUG
        cout<<"\t objective: "<<Node[myobjective].num<<"  objalue:  "<<objvalue<<"\n";
#endif
        return;
    }
#ifdef DEBUG
    for(int i=0;i<count;i++)
    {
       cout<<"D_frontier: "<<Node[D_frontier[i]].num<<"\n";
    }
#endif


    int nextGate=D_frontier[0];
    for(int i=0;i<Node[nextGate].fin;i++)
    {
        if((value1[Node[nextGate].unodes[i]] && !value2[Node[nextGate].unodes[i]]) || (!value1[Node[nextGate].unodes[i]] && value2[Node[nextGate].unodes[i]]))
        {
            myobjective=Node[nextGate].unodes[i];
        }
    }
        if(Node[nextGate].type==6 || Node[nextGate].type==7)
            objvalue=1;
        else if(Node[nextGate].type==3 || Node[nextGate].type==4)
            objvalue=0;
#ifdef DEBUG
        cout<<"\t objective: "<<Node[myobjective].num<<"  objvalue:  "<<objvalue<<"\n";
#endif

        return;
}

bool podem_class::podemREcursion(int fault)
{
	count=0;
	isPathAvailable=false;
	isDetected=false;
	int Cin;
	int successorGate;
	faultindx=(fault>>1);
	faultvalue=fault%2;
	np = & Node[faultindx];
	getDFrontier(faultindx);

#ifdef DEBUG
	cout<< np->num << " type: "<<gname(np->type) <<"  sa"<<faultvalue <<"\n";
#endif


	// ------------  CHECK if the fault is propogated to the output
	for(int i=0;i<Npo;i++)
	{
		if((value1[Poutput[i]->indx] && value2[Poutput[i]->indx] && !faultValue1[Poutput[i]->indx] && !faultValue2[Poutput[i]->indx])
	||(!value1[Poutput[i]->indx] && !value2[Poutput[i]->indx] && faultValue1[Poutput[i]->indx] && faultValue2[Poutput[i]->indx]))
		{
			isDetected=true;
		//	cout<<"\nPODEM detected the fault: "<<Node[faultindx].num<<" stuck at "<<faultvalue<<"\n";
			return true;
        }
	}
	for(int i=0;D_frontier[i]>0;i++)
	{
#ifdef DEBUG
		cout<<"D-frontier not empty\n";
		cout<<"checking D-frontier: "<< Node[D_frontier[i]].num<<"\n";
#endif

		if(CheckPathtoPO(D_frontier[i]))
		{
#ifdef DEBUG
			cout<<"path available\n";
#endif

			isPathAvailable=true;
			break;
		}
	}
	if(isPathAvailable)
	{
#ifdef DEBUG
		cout<<"path found\n";
#endif

		objective(faultindx,faultvalue);
		Cin=backtrace(myobjective,objvalue);
#ifdef DEBUG
		cout<<"backtraced to "<<Node[backTraceGate].num<<" = "<<Cin<<"\n";
#endif

		if( Cin == 0 )  // Assign values to PI
        {
            value1[backTraceGate] = 0;
            value2[backTraceGate] = 0;
            faultValue1[backTraceGate] = 0;
            faultValue2[backTraceGate] = 0;
        }
        else if(Cin == 1)
        {
			value1[backTraceGate] = ONES;
            value2[backTraceGate] = ONES;
            faultValue1[backTraceGate] = ONES;
            faultValue2[backTraceGate] = ONES;
        }
        else if(Cin == -1)
        {
        	return false;

        }

		for(int i=0;i<Node[backTraceGate].fout;i++)
        {
            successorGate = Node[backTraceGate].dnodes[i];
			if (sched[successorGate] == 0)
            {
                insertEvent(Node[successorGate].level, successorGate);
                sched[successorGate] = 1;
            }
        }
		goodsim(); //simulating the value found by backtrace method
		//add by luhao
		for(int i=0;i<Node[backTraceGate].fout;i++)
        {
            successorGate = Node[backTraceGate].dnodes[i];
			if (sched[successorGate] == 0)
            {
                insertEvent(Node[successorGate].level, successorGate);
                sched[successorGate] = 1;
            }
        }
		key=checkInputsForActivation(fault);
		if(faultvalue==1)
        {
            faultValue1[faultindx] = 1;
            faultValue2[faultindx] = 1;
        }
        else
        {
            faultValue1[faultindx] = 0;
            faultValue2[faultindx] = 0;
		}
		for(int i=0;i<Node[faultindx].fout;i++)
        {
            successorGate = Node[faultindx].dnodes[i];
			if (sched[successorGate] == 0)
            {
                insertEvent(Node[successorGate].level, successorGate);
                sched[successorGate] = 1;
            }
        }
		//simulating with faulty value at gate
		faultsim();
		//recursive call to podem for propagating the fault
        if(podemREcursion(fault) && key)
        {
            return true;
        }


		Cin=!Cin;
		if( Cin == 0 )  // Assign values to PI
        {
            value1[backTraceGate] = 0;
            value2[backTraceGate] = 0;
            faultValue1[backTraceGate] = 0;
            faultValue2[backTraceGate] = 0;
        }
        else
        {
			value1[backTraceGate] = ONES;
            value2[backTraceGate] = ONES;
            faultValue1[backTraceGate] = ONES;
            faultValue2[backTraceGate] = ONES;
        }

		for(int i=0;i<Node[backTraceGate].fout;i++)
        {
            successorGate = Node[backTraceGate].dnodes[i];
			if (sched[successorGate] == 0)
            {
                insertEvent(Node[successorGate].level, successorGate);
                sched[successorGate] = 1;
            }
        }
		goodsim(); //simulating the value found by backtrace method
		key=checkInputsForActivation(fault);
		//add by luhao
		for(int i=0;i<Node[backTraceGate].fout;i++)
        {
            successorGate = Node[backTraceGate].dnodes[i];
			if (sched[successorGate] == 0)
            {
                insertEvent(Node[successorGate].level, successorGate);
                sched[successorGate] = 1;
            }
        }
		if(faultvalue==1)
        {
            faultValue1[faultindx] = 1;
            faultValue2[faultindx] = 1;
        }
        else
        {
            faultValue1[faultindx] = 0;
            faultValue2[faultindx] = 0;
		}
		for(int i=0;i<Node[faultindx].fout;i++)
        {
            successorGate = Node[faultindx].dnodes[i];
			if (sched[successorGate] == 0)
            {
                insertEvent(Node[successorGate].level, successorGate);
                sched[successorGate] = 1;
            }
        }
		//simulating with faulty value at gate
		faultsim();
		//recursive call to podem for propogating the fault
        if(podemREcursion(fault) && key)
        {
            return true;
        }

		value1[backTraceGate] = 0;
        value2[backTraceGate] = ONES;
		for(int i=0;i<Node[backTraceGate].fout;i++)
        {
            successorGate = Node[backTraceGate].dnodes[i];
			if (sched[successorGate] == 0)
            {
                insertEvent(Node[successorGate].level, successorGate);
                sched[successorGate] = 1;
            }
        }
		goodsim(); //simulating the value found by backtrace method
		key=checkInputsForActivation(fault);
		if(faultvalue==1)
        {
            faultValue1[faultindx] = 1;
            faultValue2[faultindx] = 1;
        }
        else
        {
            faultValue1[faultindx] = 0;
            faultValue2[faultindx] = 0;
		}
		for(int i=0;i<Node[faultindx].fout;i++)
        {
            successorGate = Node[faultindx].dnodes[i];
			if (sched[successorGate] == 0)
            {
                insertEvent(Node[successorGate].level, successorGate);
                sched[successorGate] = 1;
            }
        }
		//simulating with faulty value at gate
		faultsim();
		//recursive call to podem for propogating the fault
        //cout<<"couldn't detect\n";
        return false;
	}
	else
	{
		//cout<<"couldn't detect. no xpath\n";
        return false;
	}
}

int podem_class::podem( char * outfile)
{
	detected_count=0;
	/*
	//cout <<"max_level: \t"<<max_level<<"\n";
	Nfaults=faults.size();
	count=0;
	//for(int i=0;i<Nfaults;i++){
	for(int i=4;i<5;i++){
		value1 = new unsigned int [Nnodes];
		value2 = new unsigned int [Nnodes];
		faultValue1 = new unsigned int [Nnodes];
		faultValue2 = new unsigned int [Nnodes];
		sched = new int[Nnodes];
		D_frontier = new int [Nnodes];
		faultindx=(faults[i]>>1);
	    faultvalue=faults[i]%2;
		np = &Node[faultindx];
		currLevel=0;

		setupWheel();
		setDontCares(); // set all values as Dont care
		ActFault(faults[i]); // activate the current fault
		faultsim();
		ActFault(faults[i]); // activate the current fault
		goodsim();

    }

*/

/*		getDFrontier(faultindx);
		cout<< "D_frontier:  "<< count<<"    ";
		for(int j=0;j<count;j++){
			cout<< ""<<Node[D_frontier[j]].num << "\t";
		}
		cout << "\n";
		cout<<"Node-8 "<<value1[3]<<value2[3]<<faultValue1[3]<<faultValue2[3]<<"\n";
		cout<<"Node-9 "<<value1[4]<<value2[4]<<faultValue1[4]<<faultValue2[4]<<"\n";
		cout<<"Node-11 "<<value1[8]<<value2[8]<<faultValue1[8]<<faultValue2[8]<<"\n";
		cout<<"Node-10 "<<value1[7]<<value2[7]<<faultValue1[7]<<faultValue2[7]<<"\n";
		cout<<"Node-fault  "<<value1[faultindx]<<value2[faultindx]<<faultValue1[faultindx]<<faultValue2[faultindx]<<"\n";
*/
	ofstream myfile;
	char str[]="podem_";
	strcat(str,FileName);
	
	myfile.open(outfile);
	Nfaults=faults.size();
	for(int i=0;i<Nfaults;i++)
	//for(int i=16;i<17;i++)
	{
		value1 = new unsigned int [Nnodes];
		value2 = new unsigned int [Nnodes];
		faultValue1 = new unsigned int [Nnodes];
		faultValue2 = new unsigned int [Nnodes];
		sched = new int[Nnodes];
		D_frontier = new int [Nnodes];
		faultindx=(faults[i]>>1);
	    faultvalue=faults[i]%2;
		np = &Node[faultindx];
		currLevel=0;


		//myfile <<Node[faultindx].num<<" "<<faultvalue<<" ";
#ifdef DEBUG
		cout <<Node[faultindx].num<<" "<<faultvalue<<" ";
#endif

		setupWheel();
		setDontCares(); // set all values as Dont care
		if(podemREcursion(faults[i]) == true)
		{
			//cout<<detected_count<<"\n";
			myfile <<Node[faultindx].indx<<" "<<faultvalue<<" ";
			detected_count++;;
			char buffer[4];
			for (int j=0;j<Npi; j++)
			{
				sprintf(buffer,"%d%d%d%d", value1[Pinput[j]->indx],value2[Pinput[j]->indx],faultValue1[Pinput[j]->indx],faultValue2[Pinput[j]->indx]);
				if(buffer[0]=='0' && buffer[1]=='1' && buffer[2]=='0' && buffer[3]=='1' )
				{
					myfile<<2;
					//cout<<"x";
				}
				else if(buffer[0]=='1' && buffer[1]=='0' && buffer[2]=='1' && buffer[3]=='0')
				{
					myfile<<2;
					//cout<<"x";
				}
				else if(buffer[0]=='1' && buffer[1]=='1' && buffer[2]=='0' && buffer[3]=='0')
				{
					myfile<<"1";
					//cout<<"1";
				}
				else if(buffer[0]=='0' && buffer[1]=='0' && buffer[2]=='1' && buffer[3]=='1')
				{
					myfile<<"0";
					//cout<<"0";
				}
				else if(buffer[0]=='0' && buffer[1]=='0' && buffer[2]=='0' && buffer[3]=='0')
				{
					myfile<<"0";
					//cout<<"0";
				}
				else if(buffer[0]=='1' && buffer[1]=='1' && buffer[2]=='1' && buffer[3]=='1')
				{
					myfile<<"1";
					//cout<<"1";
				}
				//myfile<<"Input: "<<Pinput[j]->num<<" value: "<<value1[Pinput[j]->indx]<<value2[Pinput[j]->indx]<<faultValue1[Pinput[j]->indx]<<faultValue2[Pinput[j]->indx]<<"\n";
			}
			myfile<<"\n";
		}
		
		//cout<<"\n";
	}

	myfile.close();

//	cout<<"fault coverage = "<<float(float(detected_count) / float(Nfaults))*100<<"%\n";
	return 1;
}





/*========================= End of program ============================*/

