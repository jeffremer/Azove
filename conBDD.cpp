//$Author: behle $
//$Date: 2007-04-07 15:14:29 +0200 (Sat, 07 Apr 2007) $

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


#include "conBDD.hpp"

#include <stack>
#include <limits>

using namespace std;


conBDD::conBDD(const unsigned int dim_) :
  dim(dim_),
  coeff(dim),
  equality(false),
  inv(dim),
  maxToCome(dim + 1),
  rootNode(NULL),
  leaf0Node(NULL),
  leaf1Node(NULL),
  conBDDnodeNr(0) {}

  
conBDD::~conBDD(){
  for(vector<list<conBDDnode*> >::iterator Vit = levels.begin(); Vit != levels.end(); ++Vit)
    for(list<conBDDnode*>::iterator Lit = Vit->begin(); Lit != Vit->end(); ++Lit)
      delete *Lit;
}


void conBDD::build(void){
  levels.resize(dim+1);
  rootNode = new conBDDnode();
  levels[0].push_back(rootNode);
  ++conBDDnodeNr;
  rootNode->ub = rhs;

  leaf0Node = new conBDDnode();
  levels[dim].push_back(leaf0Node);
  ++conBDDnodeNr;
  leaf0Node->lb = -numeric_limits<int>::max();
  leaf0Node->ub = -1;
  
  leaf1Node = new conBDDnode();
  levels[dim].push_back(leaf1Node);
  ++conBDDnodeNr;
  leaf1Node->lb = 0;
  leaf1Node->ub = numeric_limits<int>::max();

  stack<conBDDnode*> unfinishedNodes;
  unfinishedNodes.push(rootNode);
  unsigned int actLevel = 0;
  while(!unfinishedNodes.empty()){
    conBDDnode* actNode = unfinishedNodes.top();
    if(actNode->zeroEdge == NULL)
      buildSonOfConBDDnode<false>(actNode, actLevel, unfinishedNodes);
    else if(actNode->oneEdge == NULL)
      buildSonOfConBDDnode<true>(actNode, actLevel, unfinishedNodes);
    else{
      const int zeroEdgeCosts = (inv[actLevel] ? coeff[actLevel] : 0);
      const int oneEdgeCosts = (inv[actLevel] ? 0 : coeff[actLevel]);
      
      //set the lower bound, which reflects the most expensive path to leaf1
      const int newLb0 = zeroEdgeCosts + actNode->zeroEdge->lb + ((actNode->zeroEdge == leaf1Node) ? maxToCome[actLevel + 1] : 0);
      const int newLb1 = oneEdgeCosts + actNode->oneEdge->lb + ((actNode->oneEdge == leaf1Node) ? maxToCome[actLevel + 1] : 0);
      actNode->lb = max(newLb0, newLb1);

      //set the upper bound
      const int newUb0 = zeroEdgeCosts + actNode->zeroEdge->ub;
      const int newUb1 = oneEdgeCosts + actNode->oneEdge->ub;
      actNode->ub = min(newUb0, newUb1);
      
      --actLevel;
      unfinishedNodes.pop();
    }
  }
  
  //update the signatures
  for(unsigned int l=0; l<dim; ++l){
    unsigned int newSig = 0;
    for(list<conBDDnode*>::iterator it = levels[l].begin(); it != levels[l].end(); ++it)
      (*it)->sig = ++newSig;
  }
  //this is a trick: we save the dimension in the signatures of the two leafs
  // since we need to get it when we reach max number of signature nodes
  leaf0Node->sig = dim;
  leaf1Node->sig = dim;
}


//-----------------------------------------------------------------------------------

conBDDnode* conBDD::findAdequateNode(const unsigned int nextLevel, const int nextLevelUb){
  conBDDnode* foundNode = NULL;
  for(list<conBDDnode*>::iterator searchIt = levels[nextLevel].begin(); searchIt != levels[nextLevel].end(); ++searchIt){
    if((*searchIt)->lb <= nextLevelUb && nextLevelUb <= (*searchIt)->ub){
      foundNode = *searchIt;
      break;
    }
  }
  return foundNode; 
}


//-----------------------------------------------------------------------------------

void conBDD::countPathsToLeaf1(void){
  rootNode->counter = 1;
  for(unsigned int i=0; i<dim; ++i)
    rootNode->counter *= 2;
  
  for(unsigned int l=0; l<dim; ++l){
    for(list<conBDDnode*>::iterator it = levels[l].begin(); it != levels[l].end(); ++it){
      (*it)->zeroEdge->counter += (*it)->counter / 2;
      (*it)->oneEdge->counter += (*it)->counter / 2;
    }
  }
}


//-----------------------------------------------------------------------------------


ostream& operator << (ostream& strm, const conBDD& c){
  for(unsigned int i = 0; i<c.dim; ++i)
    strm<<c.coeff[i]<<(c.inv[i] ? "~" : "")<<" ";
  strm<<"<= "<<c.rhs<<endl;
  strm<<"maxToCome: ";
  for(unsigned int i = 0; i<c.dim+1; ++i)
    strm<<"("<<i<<")"<<c.maxToCome[i]<<", ";
  strm<<std::endl;

  if(c.rootNode != NULL)  
    strm<<"rootNode: "<<endl<<*(c.rootNode);
  if(c.leaf0Node != NULL)  
    strm<<"leaf0Node: "<<endl<<*(c.leaf0Node);
  if(c.leaf1Node != NULL)  
    strm<<"leaf1Node: "<<endl<<*(c.leaf1Node);

  if(!c.levels.empty()){
    strm<<"levels of BDD:"<<endl;
    for(unsigned int l=0; l<=c.dim; ++l){
      strm<<"========= level "<<l<<" ==========="<<endl;
      for(list<conBDDnode*>::const_iterator it = c.levels[l].begin(); it != c.levels[l].end(); ++it)
        strm<<*(*it);
    }
  }
  strm<<"Number of nodes: "<<c.conBDDnodeNr<<endl;
  
  return strm;
}
