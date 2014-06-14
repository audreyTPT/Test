//
//  Handle.h
//  Projet
//
//  Created by Audrey FOURNERET on 11/06/14.
//  Copyright (c) 2014 Audrey FOURNERET. All rights reserved.
//

#ifndef __Projet__Handle__
#define __Projet__Handle__

#include <iostream>

#endif /* defined(__Projet__Handle__) */

#ifndef _Armature_
#define _Armature_
#include "Armature.h"
#endif

class Handle : public Armature{
    
public:
    inline Handle () { init(0); }
    inline Handle (unsigned int v0) { init (v0); }
    inline Handle (const unsigned int * vp) { init (vp[0]); }
    inline Handle (const Handle & it) { init (it.v); initBox(it.box);}
    inline virtual ~Handle () {}
    inline Handle & operator= (const Handle & it) { init (it.v); initBox(it.box); return (*this); }
    inline bool operator== (const Handle & t) const { return (v == t.v); }
    inline unsigned int getVertex (unsigned int i = 0) const { return v; }
    inline void setVertex (unsigned int i, unsigned int vertex) { v = vertex; }
    inline bool contains (unsigned int vertex) const { return (v == vertex) ; }
    inline virtual std::string getType() { return "handle"; }
    
    void buildBox(Vertex v0);
    
protected:
    inline void init (unsigned int v0) {
        v= v0;
    }
    
private:
    unsigned int v;

};

extern std::ostream & operator<< (std::ostream & output, const Handle & t);

