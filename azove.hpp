//$Author: behle $
//$Date: 2007-03-01 17:12:27 +0100 (Thu, 01 Mar 2007) $

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


#ifndef AZOVE_H
#define AZOVE_H

#include <stdio.h>
#include <vector>
#include <list>
#include <ext/hash_map>
#include <numeric>

#include "conBDD.hpp"
#include "stackAndBDDnode.hpp"
#include "andBDDnode.hpp"
#include "sigMDD.hpp"

#include "gmpxx.h"

#ifdef USE_TIMER
#include "usertimer.hpp"
#endif


class azove{
public:
  azove(const char* problemName, bool randomVarOrder, bool outputNrOfPaths_);
  ~azove();
  
  void buildConstraintBDDs(void);
  void andBDDs(void);
  void countPathsToLeaf1(void);
  void output01Points(void);
  
  void outputBDD(void);    
  void outputDotFile(const char* filename);

private:
  void createVarOrder(bool randomVarOrder);
  andBDDnode* findNodeWithSameSuccessors(const int actLevel, const andBDDnode* actZeroEdge, const andBDDnode* actOneEdge);
  inline void output(const unsigned short int* point) const;
  
  unsigned int nrOfConstraints;  
  unsigned int dim; //number of read in columns (= number of levels in the BDD)

  bool outputNrOfQOBDDpaths;
  
  std::vector<unsigned int> index2level;
  std::vector<unsigned int> level2index;
    
  std::vector<conBDD*> conBDDs;

  //data structures for the sigMDDs
  std::vector<sigMDD> refNodesMDDs;
  sigMDDnode* sigMDDnodeBase;
  unsigned long int sigMDDnodeBaseSize;
  unsigned long int sigMDDnodeBaseCounter;
  
  //data structure for the andBDD  
  std::vector<__gnu_cxx::hash_multimap<unsigned long int, andBDDnode*> > levels;

  andBDDnode* rootNode;
  andBDDnode* leaf0Node;
  andBDDnode* leaf1Node;
  
  unsigned int andBDDnodeNr;

  unsigned long int nrOfSuccessfulFinds;
  unsigned long int nrOfInserts;
  unsigned long int nrOfStackNodes;
  unsigned long int nrOfEliminationNodes;
  unsigned long int nrOfMergedNodes;
  
#ifdef USE_TIMER
  userTimer timeConBDDs;
  userTimer timeAndBDD;
  userTimer timeOutputPoints;
#endif

  template<bool processOneEdge> void buildSonOfAndBDDnode(int& actLevel, std::vector<stackAndBDDnode>& unfinishedNodes, andBDDnode*& lastCreatedANDbddNode){
    stackAndBDDnode& actNode = unfinishedNodes[actLevel];
    andBDDnode*& actNodeZeroOrOneEdge = (!processOneEdge ? actNode.zeroEdge : actNode.oneEdge);
    if(lastCreatedANDbddNode != NULL){
      actNodeZeroOrOneEdge = lastCreatedANDbddNode; 
      lastCreatedANDbddNode = NULL;
    }
    else{
      bool toLeaf0 = false;
      bool toLeaf1 = true;
      std::vector<const conBDDnode*>::iterator nextLevelRefIt = unfinishedNodes[actLevel+1].refNodes.begin();
      std::vector<conBDD*>::iterator conBDDsIt = conBDDs.begin();
      for(std::vector<const conBDDnode*>::iterator actNodeRefNodesIt = actNode.refNodes.begin(); actNodeRefNodesIt != actNode.refNodes.end(); ++actNodeRefNodesIt, ++nextLevelRefIt, ++conBDDsIt){
        if(*actNodeRefNodesIt == NULL)
          *nextLevelRefIt = NULL;
        else{
          conBDDnode* actNodeRefNodesItSon = (!processOneEdge ? (*actNodeRefNodesIt)->zeroEdge : (*actNodeRefNodesIt)->oneEdge);
          if(actNodeRefNodesItSon == (*conBDDsIt)->leaf0Node){
            toLeaf0 = true;
            break; 
          }
          if(actNodeRefNodesItSon == (*conBDDsIt)->leaf1Node)
            *nextLevelRefIt = NULL;
          else{
            toLeaf1 = false;
            *nextLevelRefIt = actNodeRefNodesItSon;
          }
        }
      }
      
      if(toLeaf0)
        actNodeZeroOrOneEdge = leaf0Node;
      else if(toLeaf1)
        actNodeZeroOrOneEdge = leaf1Node;
      else{
        //findNodeWithSameSignature
        andBDDnode* foundNode = refNodesMDDs[actLevel + 1].find(unfinishedNodes[actLevel+1].refNodes);
        
        if(foundNode != NULL){
          actNodeZeroOrOneEdge = foundNode;
          ++nrOfSuccessfulFinds;
        }
        else{ //we have to "create" a new stackAndBDDnode
          ++actLevel;
          unfinishedNodes[actLevel].zeroEdge = NULL;
          unfinishedNodes[actLevel].oneEdge = NULL;
          ++nrOfStackNodes;
          
          if(nrOfStackNodes % 100000 == 1){
            std::cerr<<"... tried "<<nrOfStackNodes<<" nodes, thereof eliminated "<<nrOfEliminationNodes
                     <<", merged "<<nrOfMergedNodes<<" and substituted "<<nrOfSuccessfulFinds<<" nodes"<<std::endl
                     <<"    BDD nodes: "<<andBDDnodeNr<<", signatures: "<<nrOfInserts<<", hitrate: "
                      <<(double)nrOfSuccessfulFinds / (double)nrOfInserts * 100.0 <<"%";
            
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
              const double freeMemPercentage = (double)(memfree + buffers + cached) / (double)memtotal * 100.0;
              std::cerr<<", free memory: "<<freeMemPercentage<< "%";
              fclose(pf);
            }
            std::cerr<<std::endl;
          }
        }
      }
    }
  };
  
  
};


#endif
