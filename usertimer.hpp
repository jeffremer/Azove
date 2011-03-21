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


#ifndef USERTIMER_H
#define USERTIMER_H

#include <sys/time.h>
#include <sys/resource.h>

class userTimer{
public:
  userTimer() : totalUserTime(0) {};

  void start(){
    rusage now;
    getrusage(RUSAGE_SELF, &now);
    startUserTime = now.ru_utime;
  };
  
  void stop(){
    rusage now;
    getrusage(RUSAGE_SELF, &now);
    totalUserTime += interval(startUserTime, now.ru_utime);
  };
  
  double getUserTime() const { return totalUserTime; };
  
private:
  double interval(const timeval& t1, const timeval& t2){
    return 1.0 * (t2.tv_sec - t1.tv_sec) + 0.000001 * (t2.tv_usec - t1.tv_usec);  
  };
  
  double  totalUserTime;
  timeval startUserTime;
};

#endif
