
struct transform_tag 
{
   // have to store partial values for rotation and
   // scale... since these terms may not be all together
   // specified... the remaining values in the matrix
   // can just be set - since they have no other dependant terms.

   // may have to keep matrix full symetry for multiplication
   // purposes... so that a sub-translation matrix may be modified
   // from a parent's...

   // requires [4][4] for use with opengl
   MATRIX m;       // s*rcos[0][0]*rcos[0][1] sin sin   (0)
                   // sin s*rcos[1][0]*rcos[1][1] sin   (0)
                   // sin sin s*rcos[2][0]*rcos[2][0]   (0)
                   // tx  ty  tz                        (1)

   RCOORD s[3];
        // [x][0] [x][1] = partials... [x][2] = multiplied value.

	RCOORD speed[3]; // speed right, up, forward
   RCOORD accel[3];
   RCOORD rotation[3]; // pitch, yaw, roll delta
   // rot_accel is not used... just rotation velocity.
	RCOORD rot_accel[3]; // pitch, yaw, roll delta
	// these are working factors updated by Move
	RCOORD time_scale; // scale of time for scaling accelleration and speeds
	_32 last_tick;
   //RCOORD next_time; // next time this is set to update... different objects may have different scales
	int nTime; // rotation stepping for consistant rotation
	PLIST callbacks; // list of void(*)(PTRSZVAL,PTRANSFORM)
	PLIST userdata; // actually PTRSZVAL storage...
/**************************************
    AKA
   MATRIX m;    // vRight   (n,n,n,n)
                // vUp      (n,n,n,n)
                // vForward (n,n,n,n)
                // vIn      (n,n,n,n)
*************************************/
};

