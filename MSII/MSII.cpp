// Compute or estimate Multi-Scale Integral Invariants (MSII) --------------------------------------------------------------------------------------------------

float Mesh::fetchSphereCubeVolume25D( Vertex*     seedVertex,            //!< equals sphere center
                                      set<Face*>* facesInSphere,         //!< pre-selected list of faces
                                      float       radius,                //!< radius_max of our spheres
                                      double*     rasterArray,           //!< pre-allocated depth-map array of size cubeEdgeLengthInVoxels^2.
                                      int         cubeEdgeLengthInVoxels //!< equals xDim equals yDim equals zDim for our sparse 2.5D voxel cube as well as the sqrt( size of rasterAray )
) {
	if( seedVertex->isSolo() ) {
		return _NOT_A_NUMBER_;
	}

	// 1. Fetch a list of the vertices describing the faces
	int     vertexSize  = facesInSphere->size() * 3;
	double* vertexArray = getTriangleVertices( facesInSphere );

	if( vertexArray == NULL ) {
		cerr << "[Mesh::fetchSphereCubeVolume25D] getTriangleVertices failed!" << endl;
		return _NOT_A_NUMBER_;
	}

	// 2. Translate the Mesh into the origin:
	Matrix4D matAllTransformations( -(seedVertex->getX()), -(seedVertex->getY()), -(seedVertex->getZ()) );
	
	// 3. Find the orientation of the Mesh:
	//--- Accurate (Face area) -------------------------------------------------
	double normalAverageInSphereArr[3];
	double patchArea = averageNormalByArea( facesInSphere, normalAverageInSphereArr );

	Vector3D normalAverageInSphere( normalAverageInSphereArr[0], normalAverageInSphereArr[1], normalAverageInSphereArr[2], 0.0 );
	normalAverageInSphere.normalize3();
	Vector3D rotAboutAxis = normalAverageInSphere % Vector3D( 0.0, 0.0, 1.0, 0.0 );
	double angleUnsigned = angle( normalAverageInSphere, Vector3D( 0.0, 0.0, 1.0, 0.0 ), rotAboutAxis );

	// Rotate only if the angle is >~ 0!
	if( fabs( angleUnsigned ) > DBL_EPSILON*10 ) {
		// 4. Rotate the Mesh so that the average normal is parallel to the z-axis:
		matAllTransformations *= Matrix4D( Vector3D( 0.0, 0.0, 0.0, 1.0 ), rotAboutAxis, -angleUnsigned );
	}

	// 5. Scale the Mesh for rasterizing
	matAllTransformations *= Matrix4D( _MATRIX4D_INIT_SCALE_, (float)(cubeEdgeLengthInVoxels-1)/(radius*2.0) );

	// 6. Shift the Mesh for rasterizing
	matAllTransformations *= Matrix4D( (float)(cubeEdgeLengthInVoxels)/2.0, -0.5+(float)(cubeEdgeLengthInVoxels)/2.0, 0.0 );

	// 7. Apply the transformatio
	matAllTransformations.applyTo( vertexArray, vertexSize );

	// 8. Raster the vertices
	rasterViewFromZ( vertexArray, vertexSize, rasterArray, cubeEdgeLengthInVoxels, cubeEdgeLengthInVoxels );
	free( vertexArray );
	return patchArea;
}

double* Mesh::getTriangleVertices( set<Face*>* someFaceList ) {
	//! Returns an array of homogenous vectors of the Vertices describing the Faces in the list.
	//! The returned array will be of size length(someFaceList) x 3 x 4
	//!
	//! Typically called by Mesh::fetchSphereCubeVolume25D()
	int     vertexSize  = someFaceList->size()*3;
	double* vertexArray = new double[vertexSize*4];
	int     vertexNr    = 0;
	set<Face*>::iterator itFace;
	for( itFace=someFaceList->begin(); itFace!=someFaceList->end(); itFace++ ) {
		(*itFace)->getVertA()->copyCoordsTo( &vertexArray[vertexNr*4] );
		vertexArray[vertexNr*4+3] = 1.0;
		vertexNr++;
		(*itFace)->getVertB()->copyCoordsTo( &vertexArray[vertexNr*4] );
		vertexArray[vertexNr*4+3] = 1.0;
		vertexNr++;
		(*itFace)->getVertC()->copyCoordsTo( &vertexArray[vertexNr*4] );
		vertexArray[vertexNr*4+3] = 1.0;
		vertexNr++;
	}
	return vertexArray;
}

//! More efficent version as we only pass references not objects nor sets/lists.
//! normalVec has to be a pointer to float[3] (or float[4] - while we will not
//! modify the homogenous coordinate, which would lead to segmentation faults,
//! when float[3] is encountered).
//!
//! \return Total area of faces.
double Mesh::averageNormalByArea( set<Face*>* someFaces, //!< input: list of faces
                                  double*     normalVec  //!< output: average vector weighted by the faces areas. will be initalized to ( 0.0, 0.0, 0.0 )
	) {
	normalVec[0] = 0.0;
	normalVec[1] = 0.0;
	normalVec[2] = 0.0;
	float area = 0.0;
	set<Face*>::iterator itFace;
	for( itFace=someFaces->begin(); itFace!=someFaces->end(); itFace++ ) {
		(*itFace)->addNormalTo( normalVec );
		area += (*itFace)->getNormalLen();
	}
	normalVec[0] /= area;
	normalVec[1] /= area;
	normalVec[2] /= area;
	return area/2.0;
}