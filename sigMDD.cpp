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


#include<iostream>

#include "sigMDD.hpp"

using namespace std;

sigMDD::sigMDD(sigMDDnode* nodeBase_, const unsigned long int nodeBaseSize_, unsigned long int* nodeBaseCounter_) :
  nodeBase(nodeBase_),
  nodeBaseSize(nodeBaseSize_),
  nodeBaseCounter(nodeBaseCounter_)
{
  rootNode = &nodeBase[(*nodeBaseCounter)++];
#ifdef FIX_NUMBER_OF_SIGMDDNODE_SONS
  for(unsigned int i=0; i<NUMBER_OF_SIGMDDNODE_SONS; ++i)
    rootNode->sons[i] = NULL;
#else
  rootNode->sons.clear();
#endif
}


sigMDD::sigMDD(const sigMDD& c){
  nodeBase = c.nodeBase;
  nodeBaseSize = c.nodeBaseSize;
  nodeBaseCounter = c.nodeBaseCounter;
  rootNode = &nodeBase[(*nodeBaseCounter)++];
#ifdef FIX_NUMBER_OF_SIGMDDNODE_SONS
  for(unsigned int i=0; i<NUMBER_OF_SIGMDDNODE_SONS; ++i)
    rootNode->sons[i] = NULL;
#else
  rootNode->sons.clear();
#endif
}


//-----------------------------------------------------------------------------------

andBDDnode* sigMDD::find(const vector<const conBDDnode*>& ref) const{
  sigMDDnode* Mit = rootNode;
  for(vector<const conBDDnode*>::const_iterator refIt = ref.begin(); refIt != ref.end(); ++refIt){
    const unsigned int sig = (*refIt == NULL ? 0 : (*refIt)->sig);
    if(
#ifdef FIX_NUMBER_OF_SIGMDDNODE_SONS
       NUMBER_OF_SIGMDDNODE_SONS
#else
       Mit->sons.size()
#endif
                         > sig && Mit->sons[sig] != NULL)
      Mit = Mit->sons[sig];
    else{
      Mit = NULL;
      break; 
    }
  }

  return (Mit != NULL ? (andBDDnode*)(Mit->sons[0]) : NULL);
}

//-----------------------------------------------------------------------------------

void sigMDD::insert(const vector<const conBDDnode*>& ref, const andBDDnode* andNode){
  sigMDDnode* Mit = rootNode;
  for(vector<const conBDDnode*>::const_iterator refIt = ref.begin(); refIt != ref.end(); ++refIt){
    const unsigned int sig = (*refIt == NULL ? 0 : (*refIt)->sig);
#ifdef FIX_NUMBER_OF_SIGMDDNODE_SONS
    if(NUMBER_OF_SIGMDDNODE_SONS <= sig)
      return;
#else
    if(Mit->sons.size() <= sig)
      Mit->sons.resize(sig + 1, NULL);
#endif
    if(Mit->sons[sig] != NULL)
      Mit = Mit->sons[sig];
    else{
      if(*nodeBaseCounter < nodeBaseSize){
        Mit = Mit->sons[sig] = &nodeBase[(*nodeBaseCounter)++];
#ifdef FIX_NUMBER_OF_SIGMDDNODE_SONS
        for(unsigned int i=0; i<NUMBER_OF_SIGMDDNODE_SONS; ++i)
          Mit->sons[i] = NULL;
#else
        Mit->sons.clear();
#endif
      }
      else{
        cerr<<"max signature nodes reached... ";
        //this is a trick to get the dimension
        const conBDDnode* searchNode = NULL;
        for(vector<const conBDDnode*>::const_iterator refIt = ref.begin(); refIt != ref.end(); ++refIt){
          if(*refIt != NULL){
            searchNode = *refIt;
            break;
          }
        }
        do{
          searchNode = searchNode->zeroEdge; 
        }while(searchNode->zeroEdge != NULL);
        
        const unsigned int dim = searchNode->sig;
        //refNodesMDDs.resize(dim) leads to 1 call to constructor and dim calls to copy constructor
        // so we have to start at d=1
        for(unsigned int d=1; d<dim+1; ++d){
#ifdef FIX_NUMBER_OF_SIGMDDNODE_SONS
          for(unsigned int i=0; i<NUMBER_OF_SIGMDDNODE_SONS; ++i)
            (&nodeBase[d])->sons[i] = NULL;
#else
          (&nodeBase[d])->sons.clear();
#endif
        }
        *nodeBaseCounter = (long unsigned int)dim + 1;
        cerr<<"restarting"<<endl;
        return;
      }
    }
  }

#ifndef FIX_NUMBER_OF_SIGMDDNODE_SONS
  Mit->sons.resize(2);
#endif
  Mit->sons[0] = (sigMDDnode*) andNode;
  Mit->sons[1] = (sigMDDnode*) 1; //this is used as a marker for the sigMDDnode destructor
}


