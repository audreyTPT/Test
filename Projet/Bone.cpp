//
//  Bone.cpp
//  Projet
//
//  Created by Audrey FOURNERET on 25/05/14.
//  Copyright (c) 2014 Audrey FOURNERET. All rights reserved.
//

#include "Bone.h"

using namespace std;

ostream & operator<< (ostream & output, const Bone & t) {
    output << t.getVertex (0) << " " << t.getVertex (1);
    return output;
}

void Bone::buildBox(Vertex v0, Vertex v1){
    Vec3Df min;
    Vec3Df max;
    
    if (v0.getPos()[0] > v1.getPos()[0]){
        min[0] = v1.getPos()[0];
        max[0] = v0.getPos()[0];
    }else{
        min[0] = v0.getPos()[0];
        max[0] = v1.getPos()[0];
    }
    
    if (v0.getPos()[1] > v1.getPos()[1]){
        min[1] = v1.getPos()[1];
        max[1] = v0.getPos()[1];
    }else{
        min[1] = v0.getPos()[1];
        max[1] = v1.getPos()[1];
    }
    
    if (v0.getPos()[2] > v1.getPos()[2]){
        min[2] = v1.getPos()[2];
        max[2] = v0.getPos()[2];
    }else{
        min[2] = v0.getPos()[2];
        max[2] = v1.getPos()[2];
    }
    
    //on ajoute une marge d'erreur, pas selon x car ca peut toucher les autres bones !
    /*min[1] -= 2;
    min[2] -= 2;
    max[1] += 2;
    max[2] += 2;*/
    
    box = BoundingBox(min, max);
    
}