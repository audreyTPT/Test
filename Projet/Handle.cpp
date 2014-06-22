//
//  Handle.cpp
//  Projet
//
//  Created by Audrey FOURNERET on 11/06/14.
//  Copyright (c) 2014 Audrey FOURNERET. All rights reserved.
//

#include "Handle.h"

using namespace std;

ostream & operator<< (ostream & output, const Handle & t) {
    output << t.getVertex (0) << " " << t.getVertex (1);
    return output;
}

void Handle::buildBox(Vertex v0){
    
    //on fait pour l'instant une boundingBox de taille 0.6.
    Vec3Df min = v0.getPos() - 0.1 *Vec3Df(1,1,1);
    Vec3Df max = v0.getPos() + 0.1 * Vec3Df(1,1,1);
    
    box = BoundingBox(min, max);
    
}