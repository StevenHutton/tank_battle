static void MatrixToIdentity(float mOut[4][4])
{
	mOut[0][0] = 1.0f;
	mOut[0][1] = 0.0f;
	mOut[0][2] = 0.0f;
	mOut[0][3] = 0.0f;
	
	mOut[1][0] = 0.0f;
	mOut[1][1] = 1.0f;
	mOut[1][2] = 0.0f;
	mOut[1][3] = 0.0f;
	
	mOut[2][0] = 0.0f;
	mOut[2][1] = 0.0f;
	mOut[2][2] = 1.0f;
	mOut[2][3] = 0.0f;
	
	mOut[3][0] = 0.0f;
	mOut[3][1] = 0.0f;
	mOut[3][2] = 0.0f;
	mOut[3][3] = 1.0f;
}

static void MatrixMul44(float m1[4][4], float m2[4][4], float mOut[4][4])
{
	mOut[0][0] = (m1[0][0] * m2[0][0]) + (m1[0][1] * m2[1][0]) + (m1[0][2] * m2[2][0]) + (m1[0][3] * m2[3][0]);
	mOut[0][1] = (m1[0][0] * m2[0][1]) + (m1[0][1] * m2[1][1]) + (m1[0][2] * m2[2][1]) + (m1[0][3] * m2[3][1]);
	mOut[0][2] = (m1[0][0] * m2[0][2]) + (m1[0][1] * m2[1][2]) + (m1[0][2] * m2[2][2]) + (m1[0][3] * m2[3][2]);
	mOut[0][3] = (m1[0][0] * m2[0][3]) + (m1[0][1] * m2[1][3]) + (m1[0][2] * m2[2][3]) + (m1[0][3] * m2[3][3]);
	
	mOut[1][0] = (m1[1][0] * m2[0][0]) + (m1[1][1] * m2[1][0]) + (m1[1][2] * m2[2][0]) + (m1[1][3] * m2[3][0]);
	mOut[1][1] = (m1[1][0] * m2[0][1]) + (m1[1][1] * m2[1][1]) + (m1[1][2] * m2[2][1]) + (m1[1][3] * m2[3][1]);
	mOut[1][2] = (m1[1][0] * m2[0][2]) + (m1[1][1] * m2[1][2]) + (m1[1][2] * m2[2][2]) + (m1[1][3] * m2[3][2]);
	mOut[1][3] = (m1[1][0] * m2[0][3]) + (m1[1][1] * m2[1][3]) + (m1[1][2] * m2[2][3]) + (m1[1][3] * m2[3][3]);
	
	mOut[2][0] = (m1[2][0] * m2[0][0]) + (m1[2][1] * m2[1][0]) + (m1[2][2] * m2[2][0]) + (m1[2][3] * m2[3][0]);
	mOut[2][1] = (m1[2][0] * m2[0][1]) + (m1[2][1] * m2[1][1]) + (m1[2][2] * m2[2][1]) + (m1[2][3] * m2[3][1]);
	mOut[2][2] = (m1[2][0] * m2[0][2]) + (m1[2][1] * m2[1][2]) + (m1[2][2] * m2[2][2]) + (m1[2][3] * m2[3][2]);
	mOut[2][3] = (m1[2][0] * m2[0][3]) + (m1[2][1] * m2[1][3]) + (m1[2][2] * m2[2][3]) + (m1[2][3] * m2[3][3]);
	
	mOut[3][0] = (m1[3][0] * m2[0][0]) + (m1[3][1] * m2[1][0]) + (m1[3][2] * m2[2][0]) + (m1[3][3] * m2[3][0]);
	mOut[3][1] = (m1[3][0] * m2[0][1]) + (m1[3][1] * m2[1][1]) + (m1[3][2] * m2[2][1]) + (m1[3][3] * m2[3][1]);
	mOut[3][2] = (m1[3][0] * m2[0][2]) + (m1[3][1] * m2[1][2]) + (m1[3][2] * m2[2][2]) + (m1[3][3] * m2[3][2]);
	mOut[3][3] = (m1[3][0] * m2[0][3]) + (m1[3][1] * m2[1][3]) + (m1[3][2] * m2[2][3]) + (m1[3][3] * m2[3][3]);
}

static void CreateQuaternion(float angleDeg, float x, float y, float z, float quat[4])
{
	float invMag = 1.0f/sqrtf((x*x)+(y*y)+(z*z));
	float sin = sinf(angleDeg/2);
	quat[0] = (x*invMag) * sin;
	quat[1] = (y*invMag) * sin;
	quat[2] = (z*invMag) * sin;
	quat[3] = cosf(angleDeg/2);
}

static void QuaternionToRotMatrix(float quat[4], float mat[4][4])
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;	
	
	// calculate coefficients
	x2 = quat[0] + quat[0]; 
	y2 = quat[1] + quat[1];
	z2 = quat[2] + quat[2];
	xx = quat[0] * x2;
	xy = quat[0] * y2;
	xz = quat[0] * z2;
	yy = quat[1] * y2;
	yz = quat[1] * z2;
	zz = quat[2] * z2;
	wx = quat[3] * x2;
	wy = quat[3] * y2; 
	wz = quat[3] * z2;	
	
	mat[0][0] = 1.0f - (yy + zz); 
	mat[1][0] = xy - wz;
	mat[2][0] = xz + wy;
	mat[3][0] = 0.0;
	
	mat[0][1] = xy + wz;
	mat[1][1] = 1.0f - (xx + zz);
	mat[2][1] = yz - wx;
	mat[3][1] = 0.0;
		
	mat[0][2] = xz - wy;
	mat[1][2] = yz + wx;
	mat[2][2] = 1.0f - (xx + yy);
	mat[3][2] = 0.0;
	
	mat[0][3] = 0; 
	mat[1][3] = 0;
	mat[2][3] = 0; 
	mat[3][3] = 1;
}

static void MatrixTranslate44(float x, float y, float z, float mat[4][4])
{
	mat[3][0] += x;
	mat[3][1] += y;
	mat[3][2] += z;
}