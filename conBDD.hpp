//$Author: behle $
//$Date: 2006-12-14 11:22:16 +0100 (Thu, 14 Dec 2006) $

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


#ifndef CONBDD_H
#define CONBDD_H

#include "conBDDnode.hpp"

#include <iostream>
#include <vector>
#include <list>
#include <stack>

class conBDD{
public:
  conBDD(const unsigned int dim_);
  ~conBDD();
    
  void countPathsToLeaf1(void);
    
  unsigned int dim;

  //constraint is saved as a^T x <=/= b with a being non-negative
  std::vector<int> coeff;
  bool equality;
  std::vector<bool> inv; //if inv[i] is true, the 0-edge costs nothing and the 1-edge costs coeff[i]
  std::vector<int> maxToCome; //costs of the most expensive path to leaf1
  int rhs;
  
  std::vector<std::list<conBDDnode*> > levels;
  
  conBDDnode* rootNode;
  conBDDnode* leaf0Node;
  conBDDnode* leaf1Node;
  
  unsigned int conBDDnodeNr;
  
  void build(void);

  friend std::ostream& operator<<(std::ostream& strm, const conBDD& c);
    
private:
  conBDDnode* findAdequateNode(const unsigned int nextLevel, const int newUb);
  
  template<bool processOneEdge> void buildSonOfConBDDnode(conBDDnode* actNode, unsigned int& actLevel, std::stack<conBDDnode*>& unfinishedNodes){
    bool toLeaf0 = false;
    bool toLeaf1 = false;
    //a XOR b <=> (a OR b) AND !(a AND b)
    const int nextLevelUb = actNode->ub - ((processOneEdge || inv[actLevel]) && !(processOneEdge && inv[actLevel]) ? coeff[actLevel] : 0);
    conBDDnode*& actNodeZeroOrOneEdge = (!processOneEdge ? actNode->zeroEdge : actNode->oneEdge);
    if(!equality){
      if(nextLevelUb < 0)
        toLeaf0 = true;
      else if(nextLevelUb >= maxToCome[actLevel + 1])
        toLeaf1 = true;
    }
    else{
      if(nextLevelUb < 0  || nextLevelUb > maxToCome[actLevel + 1])
        toLeaf0 = true;
      else if(nextLevelUb == 0 && nextLevelUb == maxToCome[actLevel + 1])
        toLeaf1 = true;
    }
    if(toLeaf0)
      actNodeZeroOrOneEdge = leaf0Node;
    else if(toLeaf1)
      actNodeZeroOrOneEdge = leaf1Node;
    else{
      conBDDnode* foundNode = findAdequateNode(actLevel + 1, nextLevelUb);
      if(foundNode != NULL)
        actNodeZeroOrOneEdge = foundNode;
      else{ //we have to create a new node
        ++actLevel;
        actNodeZeroOrOneEdge = new conBDDnode();
        levels[actLevel].push_back(actNodeZeroOrOneEdge);
        ++conBDDnodeNr;
        actNodeZeroOrOneEdge->ub = nextLevelUb;
        unfinishedNodes.push(actNodeZeroOrOneEdge);
      }
    }    
  }
};



#endif
