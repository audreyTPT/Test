//
//  Armature.h
//  Projet
//
//  Created by Audrey FOURNERET on 11/06/14.
//  Copyright (c) 2014 Audrey FOURNERET. All rights reserved.
//

#ifndef __Projet__Armature__
#define __Projet__Armature__

#include <iostream>
#include "BoundingBox.h"
#include "Vertex.h"

#endif /* defined(__Projet__Armature__) */

class Armature {
    //classe dont hérite bone et handle
public:
    inline Armature(){ };
    inline virtual ~Armature () {}
    inline virtual BoundingBox getBoundingBox() const { return box; }
    
    virtual unsigned int getVertex (unsigned int i = 0) const =0;
    virtual void setVertex (unsigned int i, unsigned int vertex) =0;
    virtual bool contains (unsigned int vertex) const =0;
    virtual std::string getType() =0;
        
protected:
    BoundingBox box;
    
    inline void initBox( BoundingBox _box){
        box = _box;
    }
    
};

extern std::ostream & operator<< (std::ostream & output, const Armature & t);