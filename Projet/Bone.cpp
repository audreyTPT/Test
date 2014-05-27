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