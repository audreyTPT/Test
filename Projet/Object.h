// *********************************************************
// Object Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#ifndef OBJECT_H
#define OBJECT_H

#include <iostream>
#include <vector>

#include "Mesh.h"
#include "BoundingBox.h"
#include "Ray.h"

class Object {
public:
    inline Object () {}
    inline Object (const Mesh & mesh) : mesh (mesh) {
        updateBoundingBox ();
    }
    virtual ~Object () {}

    inline const Vec3Df & getTrans () const { return trans;}
    inline void setTrans (const Vec3Df & t) { trans = t; }

    inline const Mesh & getMesh () const { return mesh; }
    inline Mesh & getMesh () { return mesh; }

    inline const BoundingBox & getBoundingBox () const { return bbox; }
    void updateBoundingBox ();
    bool getBoneSelected(const Ray &ray , Bone & bone, Vec3Df & intersectionPoint);
    
private:
    Mesh mesh;
    BoundingBox bbox;
    Vec3Df trans;
};


#endif // Scene_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
