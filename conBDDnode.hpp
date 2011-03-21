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


#ifndef CONBDDNODE_H
#define CONBDDNODE_H

#include <iostream>

#include "gmpxx.h"


class conBDDnode{
public:
  conBDDnode();

  int lb;
  int ub;
    
  conBDDnode* zeroEdge;
  conBDDnode* oneEdge;
  
  mpz_class counter;
  unsigned int sig;
  
  friend std::ostream& operator << (std::ostream& strm, const conBDDnode& n);
};

#endif
