#pragma once

#include "common.h"

namespace _7SegmentsFont {
  
  U8 _BV(int shift) {
    return (1 << shift);
  }
  
  const U8 
    /*
       000 
      5   1
      5   1
      5   1
       666
      4   2
      4   2
      4   2
       333
    */
    s0 = _BV(0), 
    s1 = _BV(1), 
    s2 = _BV(2), 
    s3 = _BV(3), 
    s4 = _BV(4), 
    s5 = _BV(5), 
    s6 = _BV(6),
    
    d0 = (s0 | s1 | s2 | s3 | s4 | s5), 
    d1 = (s1 | s2), 
    d2 = (s0 | s1 | s6 | s4 | s3), 
    d3 = (s0 | s1 | s6 | s2 | s3), 
    d4 = (s5 | s6 | s1 | s2), 
    d5 = (s0 | s5 | s6 | s2 | s3), 
    d6 = (s0 | s5 | s6 | s4 | s2 | s3), 
    d7 = (s0 | s1 | s2), 
    d8 = (s0 | s1 | s2 | s3 | s4 | s5 | s6), 
    d9 = (s0 | s1 | s2 | s3 | s5 | s6), 
    L = (s5 | s4 | s3), 
    H = (s1 | s2 | s4 | s5 | s6), 
    C = (s0 | s5 | s4 | s3), 
    S = (s0 | s5 | s6 | s2 | s3),  
    P = (s0 | s5 | s1 | s6 | s4) 
  ;
  
  const U8 digits[] = {
    d0, 
    d1, 
    d2, 
    d3, 
    d4, 
    d5, 
    d6, 
    d7, 
    d8, 
    d9
  };
}


