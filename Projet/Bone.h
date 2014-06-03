//
//  Bone.h
//  Projet
//
//  Created by Audrey FOURNERET on 25/05/14.
//  Copyright (c) 2014 Audrey FOURNERET. All rights reserved.
//

#ifndef __Projet__Bone__
#define __Projet__Bone__

#include <iostream>
#include <vector>
#include "BoundingBox.h"
#include "Vertex.h"

class Bone {
public:
    inline Bone () { init(0, 0); }
    inline Bone (unsigned int v0, unsigned int v1) { init (v0, v1); }
    inline Bone (const unsigned int * vp) { init (vp[0], vp[1]); }
    inline Bone (const Bone & it) { init (it.v[0], it.v[1]); initBox(it.box);}
    inline virtual ~Bone () {}
    inline Bone & operator= (const Bone & it) { init (it.v[0], it.v[1]); initBox(it.box); return (*this); }
    inline bool operator== (const Bone & t) const { return (v[0] == t.v[0] && v[1] == t.v[1]); }
    inline unsigned int getVertex (unsigned int i) const { return v[i]; }
    inline void setVertex (unsigned int i, unsigned int vertex) { v[i] = vertex; }
    inline bool contains (unsigned int vertex) const { return (v[0] == vertex || v[1] == vertex) ; }
    inline BoundingBox getBoundingBox() const { return box; }
    
    void buildBox(Vertex v0, Vertex v1);
    
protected:
    inline void init (unsigned int v0, unsigned int v1) {
        v[0] = v0; v[1] = v1;
    }
    
    inline void initBox( BoundingBox _box){
        box = _box;
    }
    
private:
    unsigned int v[2];
    BoundingBox box;
};

extern std::ostream & operator<< (std::ostream & output, const Bone & t);



#endif /* defined(__Projet__Bone__) */
