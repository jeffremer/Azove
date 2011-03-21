//$Author: behle $
//$Date: 2006-12-24 13:32:57 +0100 (Sun, 24 Dec 2006) $

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


#ifndef SIGMDDNODE_H
#define SIGMDDNODE_H

#include "andBDDnode.hpp"


#ifdef FIX_NUMBER_OF_SIGMDDNODE_SONS
#define NUMBER_OF_SIGMDDNODE_SONS 2
// there have to be at least 2 sons!!
#endif

class sigMDDnode{
public:
#ifdef FIX_NUMBER_OF_SIGMDDNODE_SONS
  sigMDDnode* sons[NUMBER_OF_SIGMDDNODE_SONS];
#else
  std::vector<sigMDDnode*> sons;
#endif
};


#endif
