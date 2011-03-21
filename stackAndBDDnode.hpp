//$Author: behle $
//$Date: 2006-12-21 15:06:59 +0100 (Thu, 21 Dec 2006) $

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


#ifndef STACKANDBDDNODE_H
#define STACKANDBDDNODE_H

#include <iostream>
#include <vector>

#include "andBDDnode.hpp"
#include "conBDD.hpp"


class stackAndBDDnode{
public:
  stackAndBDDnode(const unsigned int refNodesSize_) :
    zeroEdge(NULL),
    oneEdge(NULL),
    refNodes(refNodesSize_) {};
  
  andBDDnode* zeroEdge;
  andBDDnode* oneEdge;
  
  std::vector<const conBDDnode*> refNodes;
};


#endif
