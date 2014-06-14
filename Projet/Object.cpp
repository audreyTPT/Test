// *********************************************************
// Object Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "Object.h"

using namespace std;

void Object::updateBoundingBox () {
    const vector<Vertex> & V = mesh.getVertices ();
    if (V.empty ())
        bbox = BoundingBox ();
    else {
        bbox = BoundingBox (V[0].getPos ());
        for (unsigned int i = 1; i < V.size (); i++)
            bbox.extendTo (V[i].getPos ());
    }
}

bool Object::getBoneSelected(const Ray & ray, int & idx_bone, Vec3Df & intersectionPoint){
    
    
    for (unsigned int i =0 ; i < mesh.getBones().size() ; i++){
        
        //il faut vérifier pour chaque bone si il est intersecté par le rayon vers la souris
        bool intersection = false;
        BoundingBox box = mesh.getBones()[i]->getBoundingBox();
        intersection = ray.intersect( box, intersectionPoint );
        
        if (intersection){
            idx_bone = i;
            return true;
        }
    }
    
    return false;
    
}
