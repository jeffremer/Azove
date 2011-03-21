//$Author: behle $
//$Date: 2006-12-22 15:12:46 +0100 (Fri, 22 Dec 2006) $

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


#ifndef ANDBDDNODE_H
#define ANDBDDNODE_H

#include <iostream>
#include <vector>

#include "conBDD.hpp"


class andBDDnode{
public:
  andBDDnode(const unsigned int level_, andBDDnode* zeroEdge_, andBDDnode* oneEdge_) :
    level(level_),
    zeroEdge(zeroEdge_),
    oneEdge(oneEdge_),
    counter(0) {};
  
  const unsigned int level;
  
  andBDDnode* zeroEdge;
  andBDDnode* oneEdge;
  
  mpz_class counter;

  friend std::ostream& operator << (std::ostream& strm, const andBDDnode& n);
};


#endif
