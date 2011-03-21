//$Author: behle $
//$Date: 2007-04-07 15:16:25 +0200 (Sat, 07 Apr 2007) $

// azove, Another Zero One Vertex Enumeration tool
// Copyright (C) 2007, Markus Behle
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


#include <iostream>
#include <fstream>
#include <set>
#include <list>
#include <vector>
#include <stack>

#include "azove.hpp"


const char* VERSION = "2.0";
  
using namespace std;

int main(int argc, char** argv){
  cout<<"* azove v"<<VERSION<<", Another Zero One Vertex Enumeration tool"<<endl
      <<"* Copyright (C) 2007, Markus Behle"<<endl;

  bool error = false;  
  bool justCount = false;
  bool randomVarOrder = false;
  bool outputNrOfQOBDDpaths = false;
  
  if(argc == 1 || argc > 5)
    error = true;
  else{
    for(int i=1; i<argc-1; ++i){
      if(strcmp(argv[i], "-c")!=0
         && strcmp(argv[i], "-r")!=0
         && strcmp(argv[i], "-e")!=0)
        error = true;
      else if(strcmp(argv[i], "-c")==0)
        justCount = true;
      else if(strcmp(argv[i], "-r")==0)
        randomVarOrder = true;
      else if(strcmp(argv[i], "-e")==0)
        outputNrOfQOBDDpaths = true;
    }
  }
  
  if(error){
    cerr<<"Usage: "<<argv[0]<<" [-c] [-r] [-e] FILE"<<endl
        <<"       -c  Only count the number of 0/1 vertices [default: enumeration]"<<endl
        <<"       -r  Generate random variable order for the BDDs (may help in case of hard instances)"<<endl
        <<"           [default: variable order given by input]"<<endl
        <<"       Output number of 0/1 vertices of QOBDDs with: (slows down computation time) [default: no output]"<<endl
        <<"         -e  exact arbitrary precision"<<endl
        <<endl
        <<"       FILE is a file in .ine format"<<endl
        <<"       statistics and timings will be written to stderr"<<endl
        <<"       output in .ext format will be written to stdout"<<endl;
    exit(1);
  }

  const char* filename = argv[argc - 1];
  azove vertices(filename, randomVarOrder, outputNrOfQOBDDpaths);
  vertices.buildConstraintBDDs();
  vertices.andBDDs();
  vertices.countPathsToLeaf1();
  if(!justCount)
    vertices.output01Points();
  
  //vertices.outputBDD(); //this is just for debugging purposes
  
  //vertices.outputDotFile("test.dot");
  //convert to ps via: dot -Tps test.dot -o test.ps
  //system("dot -Tps test.dot -o test.ps");
     
  return 0;
}


//-----------------------------------------------------------------------------------


azove::azove(const char* fileName, bool randomVarOrder, bool outputNrOfQOBDDpaths_) :
    outputNrOfQOBDDpaths(outputNrOfQOBDDpaths_)
{
  ifstream inp(fileName, ios::in);

  if(!inp){
    cerr<<"ERROR: Can't open "<<fileName<<endl;
    exit(1);
  }

  string command;
  bool found = false;
  while(!found){
    if(inp.eof()){
      cerr<<"ERROR: Could not find line H-representation!"<<endl;
      exit(1);
    }
    else {
      inp >> command;
      if(command == "H-representation")
        found = true;
    }
  }
  
  found = false;
  set<unsigned int> linearitiesSet;
  while(!found){
    if(inp.eof()){
      cerr<<"ERROR: Could not find line begin!"<<endl;
      exit(1);
    }
    else {
      inp >> command;
      if(command == "begin")
        found = true;
      else if(command == "linearity"){
        unsigned int nrOfLinearities;
        inp >> nrOfLinearities;
        for(unsigned int i=0; i<nrOfLinearities; ++i){
          unsigned int readUInt;
          inp>>readUInt;
          linearitiesSet.insert(readUInt - 1);
        }
      }
    }
  }
  unsigned int nrOfRows;
  inp >> nrOfRows;
  inp >> dim;
  --dim;
  inp >> command;
  if(command != "integer"){
    cerr<<"ERROR: Could not find numbertype integer!"<<endl;
    exit(1);
  }

  createVarOrder(randomVarOrder);
    
  int readIn; //read in b -a^T means b - a^T x >=0
  for(unsigned int i=0; i<nrOfRows; ++i){
    conBDD* readInConstraint = new conBDD(dim);
    set<unsigned int>::iterator findIt = linearitiesSet.find(i);
    if(findIt != linearitiesSet.end()){ //i is in linearitiesSet
      linearitiesSet.erase(findIt);
      readInConstraint->equality = true;
    }
      
    //convert eq/inequalities to a^T x <=/= b with vector a being non-negative
    inp >> readInConstraint->rhs;
    for(unsigned int j=0; j<dim; ++j){
      inp >> readIn;
      readIn *= -1; //we have to invert the sign because of the .ine format
      if(readIn < 0){
        readIn *= -1; //make it positive
        readInConstraint->inv[index2level[j]] = true;
        readInConstraint->rhs += readIn; //add it to the rhs
      }
      else{
        readInConstraint->inv[index2level[j]] = false;
      }
      readInConstraint->coeff[index2level[j]] = readIn;
    }
    //coeff now contains all coefficients in level order and they are all positive
    
    if(readInConstraint->rhs < 0){
      cerr<<"Constraint "<<i+1<<": unsatisfiable"<<endl;
      exit(0); 
    }
    
    if(!readInConstraint->equality && accumulate(readInConstraint->coeff.begin(), readInConstraint->coeff.end(), 0) <= readInConstraint->rhs){
      cerr<<"Constraint "<<i+1<<": redundant"<<endl;
      delete readInConstraint;
    }
    else{
      conBDDs.push_back(readInConstraint);
      //for every level calculate the most expensive path to leaf1
      readInConstraint->maxToCome[dim] = 0;
      for(int j=dim-1; j>=0; --j)
        readInConstraint->maxToCome[j] = readInConstraint->maxToCome[j+1] + readInConstraint->coeff[j];
    }
  }
  inp >> command;
  if(command != "end"){
    cerr<<"ERROR: Could not find end!"<<endl;
    exit(1);
  }
  
  nrOfConstraints = conBDDs.size();
}


//-----------------------------------------------------------------------------------

azove::~azove(){
  if(sigMDDnodeBase != NULL)
    delete[] sigMDDnodeBase;
  for(vector<conBDD*>::iterator Vit = conBDDs.begin(); Vit != conBDDs.end(); ++Vit)
    delete *Vit;
  for(vector<__gnu_cxx::hash_multimap<unsigned long int, andBDDnode*> >::iterator Vit = levels.begin(); Vit != levels.end(); ++Vit)
    for(__gnu_cxx::hash_multimap<unsigned long int, andBDDnode*>::iterator Lit = Vit->begin(); Lit != Vit->end(); ++Lit)
      delete Lit->second;
}

//-----------------------------------------------------------------------------------
    
void azove::createVarOrder(bool randomVarOrder){
  index2level.resize(dim);

  for(unsigned int i=0; i<dim; ++i){
    index2level[i] = i;
  }

  if(randomVarOrder){
    srand48(time(0));
    srand(time(0));
    random_shuffle(index2level.begin(), index2level.end());
  }

  level2index.resize(dim);
  for(unsigned int i=0; i<dim; ++i){
    const unsigned int l = index2level[i];
    level2index[l] = i;
  }

  if(randomVarOrder){  
    cerr<<"Using random variable order:";
    for(unsigned int l=0; l<dim; ++l)
      cerr<<" "<<level2index[l];
    cerr<<endl;
  }
}



//-----------------------------------------------------------------------------------

void azove::buildConstraintBDDs(void){
  cerr<<"Building "<<nrOfConstraints<<" QOBDDs with "<<dim<<" variables: (nodes in QOBDD";
  if(outputNrOfQOBDDpaths)
    cerr<<", number of 0/1 vertices";
  cerr<<")"<<endl;
  unsigned int bddCounter = 1;
  
#ifdef USE_TIMER
  timeConBDDs.start();
#endif

  for(vector<conBDD*>::iterator it = conBDDs.begin(); it != conBDDs.end(); ++it){
    cerr<<"QOBDD "<< bddCounter++<<": (";
    (*it)->build();
    cerr<<(*it)->conBDDnodeNr;
    if(outputNrOfQOBDDpaths){
      cerr<<", ";
      (*it)->countPathsToLeaf1();
      cerr<<(*it)->leaf1Node->counter;
    }
    cerr<<")"<<endl;
  }
  
#ifdef USE_TIMER
  timeConBDDs.stop();
#endif

  unsigned int numerator = 0;
  unsigned int denomiator = 0;
  for(vector<conBDD*>::iterator it = conBDDs.begin(); it != conBDDs.end(); ++it){
    for(unsigned int l=0; l<dim; ++l){
      numerator += (*it)->levels[l].size();
      ++denomiator;
    }
  }

  cerr<<"Average width of all levels in all constraint BDDs + 1: "<<(double)numerator/(double)denomiator + 1<<endl
      <<"Number of signature MDD node sons: "
#ifdef FIX_NUMBER_OF_SIGMDDNODE_SONS
      <<NUMBER_OF_SIGMDDNODE_SONS
#else
      <<"dynamic allocation"
#endif
      <<endl;
}


//-----------------------------------------------------------------------------------

void azove::andBDDs(void){
  FILE* pf = fopen("/proc/meminfo", "r");
  if(pf){
    unsigned int memtotal = 0;
    unsigned int memfree = 0;
    unsigned int buffers = 0;
    unsigned int cached = 0;
    while(memtotal == 0 || memfree == 0 || buffers == 0 || cached == 0){
      if(fscanf(pf, "MemTotal: %u kB", &memtotal) == 1)
        fscanf(pf, "\n");
      if(fscanf(pf, "MemFree: %u kB \n" , &memfree) == 1)
        fscanf(pf, "\n");
      if(fscanf(pf, "Buffers: %u kB \n", &buffers) == 1)
        fscanf(pf, "\n");
      if(fscanf(pf, "Cached: %u kB \n", &cached) == 1)
        fscanf(pf, "\n");
    }
#define LEFTOVER_FREE_MEMORY 0.20
    //this value should be between 0.10 and 0.70, depending on the memory usage of the machine
    //0.xx means that around xx% memory will still be free after allocating memory
    const unsigned long int myMemoryDesire = ((unsigned long int)(memfree + buffers + cached - LEFTOVER_FREE_MEMORY * memtotal)) * 1024;
    sigMDDnodeBaseSize = myMemoryDesire / sizeof(sigMDDnode);
    fclose(pf);
  }
  else{
#define FIXED_MDDNODEBASESIZE 10000000
    sigMDDnodeBaseSize = FIXED_MDDNODEBASESIZE;
    cerr<<"/proc/meminfo not found! Using fixed number of signature nodes."<<endl;
  }
  cerr<<"Allocating "<<sigMDDnodeBaseSize<<" signature nodes... ";
  sigMDDnodeBase = new sigMDDnode[sigMDDnodeBaseSize];
  cerr<<"done"<<endl;
  sigMDDnodeBaseCounter = 0;

#ifdef USE_TIMER
  timeAndBDD.start();
#endif

  andBDDnodeNr = 0;
  nrOfStackNodes = 0;
  nrOfSuccessfulFinds = 0;
  nrOfInserts = 0;
  nrOfEliminationNodes = 0;
  nrOfMergedNodes = 0;
  
  levels.resize(dim+1);
  refNodesMDDs.resize(dim, sigMDD(sigMDDnodeBase, sigMDDnodeBaseSize, &sigMDDnodeBaseCounter));
  
  leaf0Node = new andBDDnode(dim, NULL, NULL);
  levels[dim].insert(pair<unsigned long int, andBDDnode*>((unsigned long int)leaf0Node->zeroEdge, leaf0Node));
  ++andBDDnodeNr;
  leaf1Node = new andBDDnode(dim, NULL, NULL);
  levels[dim].insert(pair<unsigned long int, andBDDnode*>((unsigned long int)leaf1Node->zeroEdge, leaf1Node));
  ++andBDDnodeNr;

  int actLevel = 0;
  vector<stackAndBDDnode> unfinishedNodes(dim+1, stackAndBDDnode(nrOfConstraints));
  for(unsigned int j=0; j<nrOfConstraints; ++j)
    unfinishedNodes[0].refNodes[j] = conBDDs[j]->rootNode;
  ++nrOfStackNodes;
  
  andBDDnode* lastCreatedANDbddNode = NULL;
  while(actLevel >= 0){
    const stackAndBDDnode& actNode = unfinishedNodes[actLevel];
    if(actNode.zeroEdge == NULL)
      buildSonOfAndBDDnode<false>(actLevel, unfinishedNodes, lastCreatedANDbddNode);
    else if(actNode.oneEdge == NULL)
      buildSonOfAndBDDnode<true>(actLevel, unfinishedNodes, lastCreatedANDbddNode);
    else{//apply reduction rules
      //elimination rule
      if(actNode.zeroEdge == actNode.oneEdge && actLevel > 0){
        ++nrOfEliminationNodes;
        lastCreatedANDbddNode = actNode.zeroEdge;
      }
      else{
        //merging rule
        andBDDnode* foundNode = findNodeWithSameSuccessors(actLevel, actNode.zeroEdge, actNode.oneEdge);
        if(foundNode != NULL){
          ++nrOfMergedNodes;
          lastCreatedANDbddNode = foundNode;
          refNodesMDDs[actLevel].insert(actNode.refNodes, lastCreatedANDbddNode); //insertSignature
          ++nrOfInserts;
        }
        else{
          lastCreatedANDbddNode = new andBDDnode(actLevel, actNode.zeroEdge, actNode.oneEdge);
          refNodesMDDs[actLevel].insert(actNode.refNodes, lastCreatedANDbddNode); //insertSignature
          ++nrOfInserts;
          levels[actLevel].insert(pair<unsigned long int, andBDDnode*>((unsigned long int)lastCreatedANDbddNode->zeroEdge, lastCreatedANDbddNode));
          ++andBDDnodeNr;
        }
      }
      --actLevel;
    }
  }
  
  rootNode = levels[0].begin()->second;
  
#ifdef USE_TIMER
  timeAndBDD.stop();
#endif
  
#ifdef USE_TIMER
  cerr<<"User time for building the QOBDDs: "<<timeConBDDs.getUserTime()<<"s"<<endl;
  cerr<<"User time for building the final BDD: "<<timeAndBDD.getUserTime()<<"s"<<endl;
#endif
  cerr<<"Tried "<<nrOfStackNodes<<" nodes, thereof eliminated "<<nrOfEliminationNodes
      <<", merged "<<nrOfMergedNodes<<" and substituted "<<nrOfSuccessfulFinds<<" nodes"<<endl
      <<"Final BDD consists of "<<andBDDnodeNr<<" nodes"<<endl;
}


//-----------------------------------------------------------------------------------

void azove::countPathsToLeaf1(void){
  rootNode->counter = 1;
  for(unsigned int i=0; i<dim; ++i)
    rootNode->counter *= 2;
  
  for(unsigned int l=0; l<dim; ++l){
    for(__gnu_cxx::hash_multimap<unsigned long int, andBDDnode*>::iterator it = levels[l].begin(); it != levels[l].end(); ++it){
      it->second->zeroEdge->counter += it->second->counter / 2;
      it->second->oneEdge->counter += it->second->counter / 2;
    }
  }
 
  cerr<<"Number of 0/1 vertices = "<<leaf1Node->counter<<endl;
}


//-----------------------------------------------------------------------------------

andBDDnode* azove::findNodeWithSameSuccessors(const int actLevel, const andBDDnode* actZeroEdge, const andBDDnode* actOneEdge){
  andBDDnode* foundNode = NULL;
  
  pair<__gnu_cxx::hash_multimap<unsigned long int, andBDDnode*>::iterator, __gnu_cxx::hash_multimap<unsigned long int, andBDDnode*>::iterator> sameZeroSonsRange = levels[actLevel].equal_range((unsigned long int)actZeroEdge);
  
  for(__gnu_cxx::hash_multimap<unsigned long int, andBDDnode*>::iterator searchIt = sameZeroSonsRange.first; searchIt != sameZeroSonsRange.second; ++searchIt){
    if(actOneEdge == searchIt->second->oneEdge){
      foundNode = searchIt->second;
      break;
    }
  }
    
  return foundNode; 
}



//-----------------------------------------------------------------------------------


void azove::outputBDD(void){
  if(rootNode != NULL)  
    cerr<<"rootNode: "<<rootNode<<endl;
  if(leaf0Node != NULL)  
    cerr<<"leaf0Node: "<<leaf0Node<<endl;
  if(leaf1Node != NULL)  
    cerr<<"leaf1Node: "<<leaf1Node<<endl;

  cerr<<"levels of BDD"<<endl;
  for(unsigned int l=0; l<=dim; ++l){
    cerr<<"========= level "<<l<<" ==========="<<endl;
    for(__gnu_cxx::hash_multimap<unsigned long int, andBDDnode*>::const_iterator it = levels[l].begin(); it != levels[l].end(); ++it)
      cerr<<*(it->second);
  }
}



//-----------------------------------------------------------------------------------

void azove::output01Points(void){
  //it would be more efficient to start from leaf1node
  //at the moment we are enumerating "all" 0/1 points
  
#ifdef USE_TIMER
  timeOutputPoints.start();
#endif

  cout<<"V-representation"<<endl
      <<"begin"<<endl
      <<leaf1Node->counter<<" "<<dim + 1<<" integer"<<endl;

  unsigned short int point[dim];
  for(unsigned int i=0; i<dim; ++i)
    point[i] = 2;

  stack<const andBDDnode*> nodeStack;
  const andBDDnode* node = rootNode;
  do{
    unsigned short int typeOfEdge;
    const andBDDnode* tail;
    if(node != NULL){
      //save this node for visiting tail1 later
      nodeStack.push(node);
      //now we have to visit tail0
      tail = node->zeroEdge;
      typeOfEdge = 0;
    }
    else{
      node = nodeStack.top();
      nodeStack.pop();
      //now we have to visit tail1
      tail = node->oneEdge;
      typeOfEdge = 1;
    }

    if(tail == leaf0Node){
      node = NULL;
    }
    else{
      const unsigned int source_level = node->level;
      const unsigned int source_index = level2index[source_level];
      point[source_index] = typeOfEdge;

      if(tail != leaf1Node){
        const unsigned int target_level = tail->level;
        for(unsigned int l = source_level + 1; l < target_level; ++l)
          point[level2index[l]] = 2;
        node = tail;
      }
      else{
        for(unsigned int l = source_level + 1; l < dim; ++l)
          point[level2index[l]] = 2;
        output(point);
        node = NULL;
      }
    }
  } while(!(node == NULL && nodeStack.empty()));
  
  cout<<"end"<<endl;

#ifdef USE_TIMER
  timeOutputPoints.stop();
  cerr<<"User time for building the QOBDDs: "<<timeConBDDs.getUserTime()<<"s"<<endl
      <<"User time for building the final BDD: "<<timeAndBDD.getUserTime()<<"s"<<endl
      <<"User time for output of points: "<<timeOutputPoints.getUserTime()<<"s"<<endl;
#endif
}


//-----------------------------------------------------------------------------------


void azove::output(const unsigned short int* point) const{
  //output all possible points for this path
  unsigned int nrOfPossibilities = 1;
  for(unsigned int i = 0; i < dim; ++i)
    if(point[i] == 2)
      nrOfPossibilities = nrOfPossibilities<<1;  // *2
  for(unsigned int k = 0; k < nrOfPossibilities; ++k){
    unsigned int selectBit = nrOfPossibilities;
    cout<<"1";
    for(unsigned int i = 0; i < dim; ++i){
      if(point[i] == 2){
        selectBit = selectBit>>1; // /2
        cout<<" "<<((k & selectBit) ? 1 : 0);
      }
      else
        cout<<" "<<point[i];
    }
    cout<<endl;
  }
}


//-----------------------------------------------------------------------------------


void azove::outputDotFile(const char* filename){
  //first we free some memory
  cerr<<"Clearing signatures...";
  refNodesMDDs.clear();
  cerr<<" done"<<endl;

  //this function abuses the member variable "counter" of the andBDDnodes...
  ofstream of(filename);
  
  of<<"digraph \"DD\" {"<<endl
    <<"size = \"7.5,10\""<<endl
    <<"center = true;"<<endl
    <<"edge [dir = none];"<<endl
    <<"{ node [shape = plaintext];"<<endl
    <<"  edge [style = invis];"<<endl
    <<"  \"LEAFS\" [style = invis];"<<endl;

  for(unsigned int l=0; l<dim; ++l)
    of<<"\" "<<level2index[l]<<" \" -> ";
  of<<"\"LEAFS\";"<<endl
    <<"}"<<endl;
  
  for(unsigned int l=0; l<dim; ++l){
    unsigned int abusedCounter = 0;
    of<<"{ rank = same; \" "<<level2index[l]<<" \";"<<endl;
    for(__gnu_cxx::hash_multimap<unsigned long int, andBDDnode*>::iterator it = levels[l].begin(); it != levels[l].end(); ++it){
      it->second->counter = abusedCounter++;
      of<<"\""<<l<<"_"<<it->second->counter<<"\";"<<endl;
    }
    of<<"}"<<endl;
  }
  of<<"{ rank = same; \"LEAFS\";"<<endl
    <<"  { node [shape = box];"<<endl
    <<"  \"leaf0\";"<<endl
    <<"  \"leaf1\";"<<endl
    <<"  }"<<endl
    <<"}"<<endl;
  
  for(unsigned int l=0; l<dim; ++l){
    for(__gnu_cxx::hash_multimap<unsigned long int, andBDDnode*>::iterator it = levels[l].begin(); it != levels[l].end(); ++it){
      of<<"\""<<it->second->level<<"_"<<it->second->counter<<"\" -> ";
      if(it->second->oneEdge == leaf0Node)
        of<<"\"leaf0\"";
      else if(it->second->oneEdge == leaf1Node)
        of<<"\"leaf1\"";
      else
        of<<"\""<<it->second->oneEdge->level<<"_"<<it->second->oneEdge->counter<<"\"";
      of<<";"<<endl;
      
      of<<"\""<<it->second->level<<"_"<<it->second->counter<<"\" -> ";
      if(it->second->zeroEdge == leaf0Node)
        of<<"\"leaf0\"";
      else if(it->second->zeroEdge == leaf1Node)
        of<<"\"leaf1\"";
      else
        of<<"\""<<it->second->zeroEdge->level<<"_"<<it->second->zeroEdge->counter<<"\"";
      of<<" [style = dashed];"<<endl;
    }
  }
  of<<"\"leaf0\" [label = \"0\"];"<<endl
    <<"\"leaf1\" [label = \"1\"];"<<endl
    <<"}"<<endl;
        
  of.close();
}


//-----------------------------------------------------------------------------------


ostream& operator<<(ostream& strm, const andBDDnode& n){
  strm<<"node ("<<&n<<"): l="<<n.level<<" ";
  if(n.zeroEdge != NULL)
    strm<<" 0-edge: "<<n.zeroEdge;
  if(n.oneEdge != NULL)
    strm<<"  1-edge: "<<n.oneEdge;
  strm<<endl;
  
  return strm;
}
