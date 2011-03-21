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


#ifndef SIGMDD_H
#define SIGMDD_H

#include "sigMDDnode.hpp"
#include "andBDDnode.hpp"

#include <vector>

class sigMDD{
public:
  sigMDD(sigMDDnode* nodeBase_, const unsigned long int nodeBaseSize_, unsigned long int* nodeBaseCounter_);
  sigMDD(const sigMDD& c);
  
  sigMDDnode* rootNode;
  
  sigMDDnode* nodeBase;
  unsigned long int nodeBaseSize;
  unsigned long int* nodeBaseCounter;

  andBDDnode* find(const std::vector<const conBDDnode*>& ref) const;
  void insert(const std::vector<const conBDDnode*>& ref, const andBDDnode* andNode);
};


#endif
