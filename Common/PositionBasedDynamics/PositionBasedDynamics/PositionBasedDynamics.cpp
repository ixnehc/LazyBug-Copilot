#include "stdh.h"
#include "PositionBasedDynamics.h"
#include "MathFunctions.h"
#include <cfloat>

using namespace PBD;

const Real eps = static_cast<Real>(1e-6);

//////////////////////////////////////////////////////////////////////////
// PositionBasedDynamics
//////////////////////////////////////////////////////////////////////////

bool PositionBasedDynamics::solve_DistanceConstraint(
	const i_math::vector3df &p0, Real invMass0, 
	const i_math::vector3df &p1, Real invMass1,
	const Real restLength,
	const Real compressionStiffness,
	const Real stretchStiffness,
	i_math::vector3df &corr0, i_math::vector3df &corr1)
{				
	Real wSum = invMass0 + invMass1;
	if (wSum == 0.0)
		return false;

	i_math::vector3df n = p1 - p0;
	Real d = n.getLength();
	n.normalize();
	
	i_math::vector3df corr;
	if (d < restLength)
		corr = compressionStiffness * n * (d - restLength) / wSum;
	else
		corr = stretchStiffness * n * (d - restLength) / wSum;

	corr0 =  invMass0 * corr;
	corr1 = -invMass1 * corr;
	return true;
}

bool PositionBasedDynamics::solve_PullConstraint(
	const i_math::vector3df &p0, Real invMass0, 
	const i_math::vector3df &p1, Real invMass1,
	const Real length,
	const Real minStiffness,
	const Real maxStiffness,
	i_math::vector3df &dir,
	i_math::vector3df &corr0, i_math::vector3df &corr1,bool &clamp1)
{				
	corr0.setZero();
	corr1.setZero();
	clamp1=false;

	Real wSum = invMass0 + invMass1;
	if (wSum == 0.0)
		return false;

	i_math::vector3df n = p1 - p0;
	Real d = n.getLength();
	n.normalize();

	if (n.dotProduct(dir)>0.0f)
	{
		corr1=p0-p1;
		clamp1=true;
		return true;
	}

	i_math::vector3df corr;
	if (d > length)
	{
		corr = maxStiffness * n * (d - length) / wSum;
		corr0 =  invMass0 * corr;
		corr1 = -invMass1 * corr;
	}
	else
	{
		if (d<length/5.0f)
			d=length/5.0f;
		corr = minStiffness * n * (d) / wSum;
		corr1 = -invMass1 * corr;
		if (corr1.getLength()>d)
			corr1.setLength(d);
	}

	return true;
}


bool PositionBasedDynamics::solve_SimpleContactConstraint(
	const i_math::vector3df &p0, Real invMass0, 
	const i_math::vector3df &p1, Real invMass1,
	const Real minLength,
	const Real stiffness,
	i_math::vector3df &corr0, i_math::vector3df &corr1)
{				
	Real wSum = invMass0 + invMass1;
	if (wSum == 0.0)
		return false;

	i_math::vector3df n = p1 - p0;
	Real d = n.getLength();
	n.normalize();
	
	i_math::vector3df corr(0.0f,0.0f,0.0f);
	if (d < minLength)
		corr = stiffness * n * (d - minLength) / wSum;

	corr0 =  invMass0 * corr;
	corr1 = -invMass1 * corr;
	return true;
}


bool PositionBasedDynamics::solve_DihedralConstraint(
	const i_math::vector3df &p0, Real invMass0,		
	const i_math::vector3df &p1, Real invMass1,
	const i_math::vector3df &p2, Real invMass2,
	const i_math::vector3df &p3, Real invMass3,
	const Real restAngle,
	const Real stiffness,		
	i_math::vector3df &corr0, i_math::vector3df &corr1, i_math::vector3df &corr2, i_math::vector3df &corr3)
{
	// derivatives from Bridson, Simulation of Clothing with Folds and Wrinkles
	// his modes correspond to the derivatives of the bending angle arccos(n1 dot n2) with correct scaling

	if (invMass0 == 0.0 && invMass1 == 0.0)
		return false;

	i_math::vector3df e = p3-p2;
	Real  elen = e.getLength();
	if (elen < eps)
		return false;

	Real invElen = static_cast<Real>(1.0) / elen;

	i_math::vector3df n1 = (p2-p0).crossProduct(p3-p0); n1 /= n1.getLengthSQ();
	i_math::vector3df n2 = (p3 - p1).crossProduct(p2 - p1); n2 /= n2.getLengthSQ();

	i_math::vector3df d0 = elen*n1;
	i_math::vector3df d1 = elen*n2;
	i_math::vector3df d2 = (p0-p3).dotProduct(e) * invElen * n1 + (p1-p3).dotProduct(e) * invElen * n2;
	i_math::vector3df d3 = (p2-p0).dotProduct(e) * invElen * n1 + (p2-p1).dotProduct(e) * invElen * n2;

	n1.normalize();
	n2.normalize();
	Real dot = n1.dotProduct(n2);

	if (dot < -1.0) dot = -1.0;
	if (dot >  1.0) dot =  1.0;
	Real phi = acos(dot);	

	// Real phi = (-0.6981317 * dot * dot - 0.8726646) * dot + 1.570796;	// fast approximation

	Real lambda = 
		invMass0 * d0.getLengthSQ() +
		invMass1 * d1.getLengthSQ() +
		invMass2 * d2.getLengthSQ() +
		invMass3 * d3.getLengthSQ();

	if (lambda == 0.0)
		return false;	

	// stability
	// 1.5 is the largest magic number I found to be stable in all cases :-)
	//if (stiffness > 0.5 && fabsf(phi - b.restAngle) > 1.5)		
	//	stiffness = 0.5;

	lambda = (phi - restAngle) / lambda * stiffness;

	if (n1.crossProduct(n2).dotProduct(e) > 0.0)
		lambda = -lambda;	

	corr0 = - invMass0 * lambda * d0;
	corr1 = - invMass1 * lambda * d1;
	corr2 = - invMass2 * lambda * d2;
	corr3 = - invMass3 * lambda * d3;

	return true;
}

bool PositionBasedDynamics::solve_BendConstraint(
	const i_math::vector3df &p0, Real invMass0,		
	const i_math::vector3df &p1, Real invMass1,
	const i_math::vector3df &p2, Real invMass2,
	const i_math::vector3df &dirDef,
	const Real toleranceAngle,
	const Real stiffness,		
	i_math::vector3df &corr0, i_math::vector3df &corr1, i_math::vector3df &corr2)
{
	corr0.setZero();
	corr1.setZero();
	corr2.setZero();


	if (invMass0 == 0.0 && invMass1 == 0.0&& invMass2 == 0.0)
		return false;

	float wSum=invMass0+invMass2;

	i_math::vector3df v_1to0=p0-p1;
	if (v_1to0.getLengthSQ()<0.1f*0.1f)
		v_1to0=-dirDef;
	i_math::vector3df v_1to2=p2-p1;
	if (v_1to2.getLengthSQ()<0.01f*0.01f)
		return false;

	i_math::vector3df n0;
	if (TRUE)
	{
		i_math::vector3df v=v_1to0.crossProduct(v_1to2);
		n0=v_1to0.crossProduct(v);
		n0.normalize();
	}

	i_math::vector3df n2;
	if (TRUE)
	{
		i_math::vector3df v=v_1to2.crossProduct(v_1to0);
		n2=v_1to2.crossProduct(v);
		n2.normalize();
	}

	float angle=acos(i_math::clamp_f(n0.dotProduct(n2),0.0f,0.99999f));

	angle=i_math::clamp_f(angle-toleranceAngle,0.0f,i_math::Pi);

//	corr0 = n0* invMass0 * stiffness*angle/wSum;
	corr2 = n2* invMass2 * stiffness*angle/wSum*v_1to2.getLength();
// 	corr1 = -(n0* stiffness*angle/wSum+n2* stiffness*angle/wSum)*invMass1;

	return true;
}


bool PositionBasedDynamics::solve_VolumeConstraint(
	const i_math::vector3df &p0, Real invMass0,		
	const i_math::vector3df &p1, Real invMass1,
	const i_math::vector3df &p2, Real invMass2,
	const i_math::vector3df &p3, Real invMass3,
	const Real restVolume,
	const Real negVolumeStiffness,			
	const Real posVolumeStiffness,
	i_math::vector3df &corr0, i_math::vector3df &corr1, i_math::vector3df &corr2, i_math::vector3df &corr3)
{
	Real volume = static_cast<Real>(1.0 / 6.0) * (p1 - p0).crossProduct(p2 - p0).dotProduct(p3 - p0);

	corr0.setZero(); corr1.setZero(); corr2.setZero(); corr3.setZero();

	if (posVolumeStiffness == 0.0 && volume > 0.0)
		return false;

	if (negVolumeStiffness == 0.0 && volume < 0.0)
		return false;


	i_math::vector3df grad0 = (p1 - p2).crossProduct(p3 - p2);
	i_math::vector3df grad1 = (p2 - p0).crossProduct(p3 - p0);
	i_math::vector3df grad2 = (p0 - p1).crossProduct(p3 - p1);
	i_math::vector3df grad3 = (p1 - p0).crossProduct(p2 - p0);

	Real lambda = 
		invMass0 * grad0.getLengthSQ() +
		invMass1 * grad1.getLengthSQ() +
		invMass2 * grad2.getLengthSQ() +
		invMass3 * grad3.getLengthSQ();

	if (fabsf(lambda) < eps)
		return false;

	if (volume < 0.0)
		lambda = negVolumeStiffness * (volume - restVolume) / lambda;
	else
		lambda = posVolumeStiffness * (volume - restVolume) / lambda;

	corr0 = -lambda * invMass0 * grad0;
	corr1 = -lambda * invMass1 * grad1;
	corr2 = -lambda * invMass2 * grad2;
	corr3 = -lambda * invMass3 * grad3;

	return true;
}

// ----------------------------------------------------------------------------------------------
bool PositionBasedDynamics::init_IsometricBendingConstraint(
	const i_math::vector3df &p0, 
	const i_math::vector3df &p1, 
	const i_math::vector3df &p2, 
	const i_math::vector3df &p3, 
	i_math::matrix44f &Q)
{
	// Compute matrix Q for quadratic bending
	const i_math::vector3df *x[4] = { &p2, &p3, &p0, &p1 };

	const i_math::vector3df e0 = *x[1] - *x[0];
	const i_math::vector3df e1 = *x[2] - *x[0];
	const i_math::vector3df e2 = *x[3] - *x[0];
	const i_math::vector3df e3 = *x[2] - *x[1];
	const i_math::vector3df e4 = *x[3] - *x[1];

	const Real c01 = MathFunctions::cotTheta(e0, e1);
	const Real c02 = MathFunctions::cotTheta(e0, e2);
	const Real c03 = MathFunctions::cotTheta(-e0, e3);
	const Real c04 = MathFunctions::cotTheta(-e0, e4);

	const Real A0 = static_cast<Real>(0.5) * (e0.crossProduct(e1)).getLength();
	const Real A1 = static_cast<Real>(0.5) * (e0.crossProduct(e2)).getLength();

	const Real coef = -3.f / (2.f*(A0 + A1));
	const Real K[4] = { c03 + c04, c01 + c02, -c01 - c03, -c02 - c04 };
	const Real K2[4] = { coef*K[0], coef*K[1], coef*K[2], coef*K[3] };

	for (unsigned char j = 0; j < 4; j++)
	{
		for (unsigned char k = 0; k < j; k++)
		{
			Q(j, k) = Q(k, j) = K[j] * K2[k];
		}
		Q(j, j) = K[j] * K2[j];
	}

	return true;
}

// ----------------------------------------------------------------------------------------------
bool PositionBasedDynamics::solve_IsometricBendingConstraint(
	const i_math::vector3df &p0, Real invMass0, 
	const i_math::vector3df &p1, Real invMass1, 
	const i_math::vector3df &p2, Real invMass2, 
	const i_math::vector3df &p3, Real invMass3, 
	const i_math::matrix44f &Q, 
	const Real stiffness, 
	i_math::vector3df &corr0, i_math::vector3df &corr1, i_math::vector3df &corr2, i_math::vector3df &corr3)
{
	const i_math::vector3df *x[4] = { &p2, &p3, &p0, &p1 };
	Real invMass[4] = { invMass2, invMass3, invMass0, invMass1 };

	Real energy = 0.0;
	for (unsigned char k = 0; k < 4; k++)
		for (unsigned char j = 0; j < 4; j++)
			energy += Q(j, k)*(x[k]->dotProduct(*x[j]));
	energy *= 0.5;

	i_math::vector3df gradC[4];
	gradC[0].setZero();
	gradC[1].setZero();
	gradC[2].setZero();
	gradC[3].setZero();
	for (unsigned char k = 0; k < 4; k++)
		for (unsigned char j = 0; j < 4; j++)
			gradC[j] += Q(j,k) * *x[k];


	Real sum_normGradC = 0.0;
	for (unsigned int j = 0; j < 4; j++)
	{
		// compute sum of squared gradient norms
		if (invMass[j] != 0.0)
			sum_normGradC += invMass[j] * gradC[j].getLengthSQ();
	}

	// exit early if required
	if (fabsf(sum_normGradC) > eps)
	{
		// compute impulse-based scaling factor
		const Real s = energy / sum_normGradC;

		corr0 = -stiffness * (s*invMass[2]) * gradC[2];
		corr1 = -stiffness * (s*invMass[3]) * gradC[3];
		corr2 = -stiffness * (s*invMass[0]) * gradC[0];
		corr3 = -stiffness * (s*invMass[1]) * gradC[1];

		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------------------------
bool PositionBasedDynamics::solve_EdgePointDistanceConstraint(
	const i_math::vector3df &p, Real invMass,
	const i_math::vector3df &p0, Real invMass0,
	const i_math::vector3df &p1, Real invMass1,
	const Real restDist,
	const Real compressionStiffness,
	const Real stretchStiffness,
	i_math::vector3df &corr, i_math::vector3df &corr0, i_math::vector3df &corr1)
{
	i_math::vector3df d = p1 - p0;
	Real t;
	if ((p0-p1).getLengthSQ() < eps * eps)
		t = 0.5;
	else {
		Real d2 = d.dotProduct(d);
		t = d.dotProduct(p - p1) / d2;
		if (t < 0.0)
			t = 0.0;
		else if (t > 1.0)
			t = 1.0;
	}
	i_math::vector3df q = p0 + d*t;	// closest point on edge
	i_math::vector3df n = p - q;
	Real dist = n.getLength();
	n.normalize();
	Real C = dist - restDist;
	Real b0 = static_cast<Real>(1.0) - t;
	Real b1 = t;
	i_math::vector3df grad = n;
	i_math::vector3df grad0 = -n * b0;
	i_math::vector3df grad1 = -n * b1;

	Real s = invMass + invMass0 * b0 * b0 + invMass1 * b1 * b1;
	if (s == 0.0)
		return false;

	s = C / s;
	if (C < 0.0)
		s *= compressionStiffness;
	else
		s *= stretchStiffness;

	if (s == 0.0)
		return false;

	corr = -s * invMass * grad;
	corr0 = -s * invMass0 * grad0;
	corr1 = -s * invMass1 * grad1;
	return true;
}

// ----------------------------------------------------------------------------------------------
bool PositionBasedDynamics::solve_TrianglePointDistanceConstraint(
	const i_math::vector3df &p, Real invMass,
	const i_math::vector3df &p0, Real invMass0,
	const i_math::vector3df &p1, Real invMass1,
	const i_math::vector3df &p2, Real invMass2,
	const Real restDist,
	const Real compressionStiffness,
	const Real stretchStiffness,
	i_math::vector3df &corr, i_math::vector3df &corr0, i_math::vector3df &corr1, i_math::vector3df &corr2)
{
	// find barycentric coordinates of closest point on triangle

	Real b0 = static_cast<Real>(1.0 / 3.0);		// for singular case
	Real b1 = b0;
	Real b2 = b0;

	i_math::vector3df d1 = p1 - p0;
	i_math::vector3df d2 = p2 - p0;
	i_math::vector3df pp0 = p - p0;
	Real a = d1.dotProduct(d1);
	Real b = d2.dotProduct(d1);
	Real c = pp0.dotProduct(d1);
	Real d = b;
	Real e = d2.dotProduct(d2);
	Real f = pp0.dotProduct(d2);
	Real det = a*e - b*d;

	if (det != 0.0) {
		Real s = (c*e - b*f) / det;
		Real t = (a*f - c*d) / det;
		b0 = static_cast<Real>(1.0) - s - t;		// inside triangle
		b1 = s;
		b2 = t;
		if (b0 < 0.0) {		// on edge 1-2
			i_math::vector3df d = p2 - p1;
			Real d2 = d.dotProduct(d);
			Real t = (d2 == static_cast<Real>(0.0)) ? static_cast<Real>(0.5) : d.dotProduct(p - p1) / d2;
			if (t < 0.0) t = 0.0;	// on point 1
			if (t > 1.0) t = 1.0;	// on point 2
			b0 = 0.0;
			b1 = (static_cast<Real>(1.0) - t);
			b2 = t;
		}
		else if (b1 < 0.0) {	// on edge 2-0
			i_math::vector3df d = p0 - p2;
			Real d2 = d.dotProduct(d);
			Real t = (d2 == static_cast<Real>(0.0)) ? static_cast<Real>(0.5) : d.dotProduct(p - p2) / d2;
			if (t < 0.0) t = 0.0;	// on point 2
			if (t > 1.0) t = 1.0; // on point 0
			b1 = 0.0;
			b2 = (static_cast<Real>(1.0) - t);
			b0 = t;
		}
		else if (b2 < 0.0) {	// on edge 0-1
			i_math::vector3df d = p1 - p0;
			Real d2 = d.dotProduct(d);
			Real t = (d2 == static_cast<Real>(0.0)) ? static_cast<Real>(0.5) : d.dotProduct(p - p0) / d2;
			if (t < 0.0) t = 0.0;	// on point 0
			if (t > 1.0) t = 1.0;	// on point 1
			b2 = 0.0;
			b0 = (static_cast<Real>(1.0) - t);
			b1 = t;
		}
	}
	i_math::vector3df q = p0 * b0 + p1 * b1 + p2 * b2;
	i_math::vector3df n = p - q;
	Real dist = n.getLength();
	n.normalize();
	Real C = dist - restDist;
	i_math::vector3df grad = n;
	i_math::vector3df grad0 = -n * b0;
	i_math::vector3df grad1 = -n * b1;
	i_math::vector3df grad2 = -n * b2;

	Real s = invMass + invMass0 * b0*b0 + invMass1 * b1*b1 + invMass2 * b2*b2;
	if (s == 0.0)
		return false;

	s = C / s;
	if (C < 0.0)
		s *= compressionStiffness;
	else
		s *= stretchStiffness;

	if (s == 0.0)
		return false;

	corr = -s * invMass * grad;
	corr0 = -s * invMass0 * grad0;
	corr1 = -s * invMass1 * grad1;
	corr2 = -s * invMass2 * grad2;
	return true;
}

// ----------------------------------------------------------------------------------------------
bool PositionBasedDynamics::solve_EdgeEdgeDistanceConstraint(
	const i_math::vector3df &p0, Real invMass0,
	const i_math::vector3df &p1, Real invMass1,
	const i_math::vector3df &p2, Real invMass2,
	const i_math::vector3df &p3, Real invMass3,
	const Real restDist,
	const Real compressionStiffness,
	const Real stretchStiffness,
	i_math::vector3df &corr0, i_math::vector3df &corr1, i_math::vector3df &corr2, i_math::vector3df &corr3)
{
	i_math::vector3df d0 = p1 - p0;
	i_math::vector3df d1 = p3 - p2;

	Real a = d0.getLengthSQ();
	Real b = -d0.dotProduct(d1);
	Real c = d0.dotProduct(d1);
	Real d = -d1.getLengthSQ();
	Real e = (p2 - p0).dotProduct(d0);
	Real f = (p2 - p0).dotProduct(d1);
	Real det = a*d - b*c;
	Real s, t;
	if (det != 0.0) {
		det = static_cast<Real>(1.0) / det;
		s = (e*d - b*f) * det;
		t = (a*f - e*c) * det;
	}
	else {	// d0 and d1 parallel
		Real s0 = p0.dotProduct(d0);
		Real s1 = p1.dotProduct(d0);
		Real t0 = p2.dotProduct(d0);
		Real t1 = p3.dotProduct(d0);
		bool flip0 = false;
		bool flip1 = false;

		if (s0 > s1) { Real f = s0; s0 = s1; s1 = f; flip0 = true; }
		if (t0 > t1) { Real f = t0; t0 = t1; t1 = f; flip1 = true; }

		if (s0 >= t1) {
			s = !flip0 ? static_cast<Real>(0.0) : static_cast<Real>(1.0);
			t = !flip1 ? static_cast<Real>(1.0) : static_cast<Real>(0.0);
		}
		else if (t0 >= s1) {
			s = !flip0 ? static_cast<Real>(1.0) : static_cast<Real>(0.0);
			t = !flip1 ? static_cast<Real>(0.0) : static_cast<Real>(1.0);
		}
		else {		// overlap
			Real mid = (s0 > t0) ? (s0 + t1) * static_cast<Real>(0.5) : (t0 + s1) * static_cast<Real>(0.5);
			s = (s0 == s1) ? static_cast<Real>(0.5) : (mid - s0) / (s1 - s0);
			t = (t0 == t1) ? static_cast<Real>(0.5) : (mid - t0) / (t1 - t0);
		}
	}
	if (s < 0.0) s = 0.0;
	if (s > 1.0) s = 1.0;
	if (t < 0.0) t = 0.0;
	if (t > 1.0) t = 1.0;

	Real b0 = static_cast<Real>(1.0) - s;
	Real b1 = s;
	Real b2 = static_cast<Real>(1.0) - t;
	Real b3 = t;

	i_math::vector3df q0 = p0 * b0 + p1 * b1;
	i_math::vector3df q1 = p2 * b2 + p3 * b3;
	i_math::vector3df n = q0 - q1;
	Real dist = n.getLength();
	n.normalize();
	Real C = dist - restDist;
	i_math::vector3df grad0 = n * b0;
	i_math::vector3df grad1 = n * b1;
	i_math::vector3df grad2 = -n * b2;
	i_math::vector3df grad3 = -n * b3;

	s = invMass0 * b0*b0 + invMass1 * b1*b1 + invMass2 * b2*b2 + invMass3 * b3*b3;
	if (s == 0.0)
		return false;

	s = C / s;
	if (C < 0.0)
		s *= compressionStiffness;
	else
		s *= stretchStiffness;

	if (s == 0.0)
		return false;

	corr0 = -s * invMass0 * grad0;
	corr1 = -s * invMass1 * grad1;
	corr2 = -s * invMass2 * grad2;
	corr3 = -s * invMass3 * grad3;
	return true;
}

// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::init_ShapeMatchingConstraint(
// 	const i_math::vector3df x0[], const Real invMasses[], int numPoints,
// 	i_math::vector3df &restCm, Matrix3r &invRestMat)
// {
// 	invRestMat.setIdentity();
// 
// 	// center of mass
// 	restCm.setZero();
// 	Real wsum = 0.0;
// 	for (int i = 0; i < numPoints; i++) {
// 		Real wi = static_cast<Real>(1.0) / (invMasses[i] + eps);
// 		restCm += x0[i] * wi;
// 		wsum += wi;
// 	}
// 	if (wsum == 0.0)
// 		return false;
// 	restCm /= wsum;
// 
// 	// A
// 	Matrix3r A;
// 	A.setZero();
// 	for (int i = 0; i < numPoints; i++) {
// 		const i_math::vector3df qi = x0[i] - restCm;
// 		Real wi = static_cast<Real>(1.0) / (invMasses[i] + eps);
// 		Real x2 = wi * qi[0] * qi[0];
// 		Real y2 = wi * qi[1] * qi[1];
// 		Real z2 = wi * qi[2] * qi[2];
// 		Real xy = wi * qi[0] * qi[1];
// 		Real xz = wi * qi[0] * qi[2];
// 		Real yz = wi * qi[1] * qi[2];
// 		A(0, 0) += x2; A(0, 1) += xy; A(0, 2) += xz;
// 		A(1, 0) += xy; A(1, 1) += y2; A(1, 2) += yz;
// 		A(2, 0) += xz; A(2, 1) += yz; A(2, 2) += z2;
// 	}
// 	Real det = A.determinant();
// 	if (fabsf(det) > eps)
// 	{
// 		invRestMat = A.inverse();
// 		return true;
// 	}
// 	return false;
// }

// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::solve_ShapeMatchingConstraint(
// 	const i_math::vector3df x0[], const i_math::vector3df x[], const Real invMasses[], int numPoints,
// 	const i_math::vector3df &restCm, 
// 	const Matrix3r &invRestMat,
// 	const Real stiffness,
// 	const bool allowStretch,
// 	i_math::vector3df corr[], Matrix3r *rot)
// {
// 	for (int i = 0; i < numPoints; i++)
// 		corr[i].setZero();
// 
// 	// center of mass
// 	i_math::vector3df cm(0.0, 0.0, 0.0);
// 	Real wsum = 0.0;
// 	for (int i = 0; i < numPoints; i++) 
// 	{
// 		Real wi = static_cast<Real>(1.0) / (invMasses[i] + eps);
// 		cm += x[i] * wi;
// 		wsum += wi;
// 	}
// 	if (wsum == 0.0)
// 		return false;
// 	cm /= wsum;
// 
// 	// A
// 	Matrix3r mat;
// 	mat.setZero();
// 	for (int i = 0; i < numPoints; i++) {
// 		i_math::vector3df q = x0[i] - restCm;
// 		i_math::vector3df p = x[i] - cm;
// 
// 		Real w = static_cast<Real>(1.0) / (invMasses[i] + eps);
// 		p *= w;
// 
// 		mat(0, 0) += p[0] * q[0]; mat(0, 1) += p[0] * q[1]; mat(0, 2) += p[0] * q[2];
// 		mat(1, 0) += p[1] * q[0]; mat(1, 1) += p[1] * q[1]; mat(1, 2) += p[1] * q[2];
// 		mat(2, 0) += p[2] * q[0]; mat(2, 1) += p[2] * q[1]; mat(2, 2) += p[2] * q[2];
// 	}
// 
// 	mat = mat * invRestMat;
// 
// 	Matrix3r R, U, D;
// 	R = mat;
// 	if (allowStretch)
// 		R = mat;
// 	else
// 		//MathFunctions::polarDecomposition(mat, R, U, D);
// 		MathFunctions::polarDecompositionStable(mat, eps, R);
// 
// 	for (int i = 0; i < numPoints; i++) {
// 		i_math::vector3df goal = cm + R * (x0[i] - restCm);
// 		corr[i] = (goal - x[i]) * stiffness;
// 	}
// 
// 	if (rot)
// 		*rot = R;
// 
// 	return true;
// }


// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::init_StrainTriangleConstraint(
// 	const i_math::vector3df &p0,
// 	const i_math::vector3df &p1,
// 	const i_math::vector3df &p2,
// 	Matrix2r &invRestMat)
// {
// 	Real a = p1[0] - p0[0]; Real b = p2[0] - p0[0];
// 	Real c = p1[1] - p0[1]; Real d = p2[1] - p0[1];
// 
// 	// inverse
// 	Real det = a*d - b*c;
// 	if (fabsf(det) < eps)
// 		return false;
// 
// 	Real s = static_cast<Real>(1.0) / det;
// 	invRestMat(0,0) =  d*s;  invRestMat(0,1) = -b*s;
// 	invRestMat(1,0) = -c*s;  invRestMat(1,1) =  a*s;
// 
// 	return true;
// }

// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::solve_StrainTriangleConstraint(		
// 		const i_math::vector3df &p0, Real invMass0, 
// 		const i_math::vector3df &p1, Real invMass1,
// 		const i_math::vector3df &p2, Real invMass2,
// 		const Matrix2r &invRestMat,
// 		const Real xxStiffness, 
// 		const Real yyStiffness, 
// 		const Real xyStiffness,
// 		const bool normalizeStretch,
// 		const bool normalizeShear,
// 		i_math::vector3df &corr0, i_math::vector3df &corr1, i_math::vector3df &corr2)
// {
// 	i_math::vector3df c[2];
// 	c[0] = i_math::vector3df(invRestMat(0, 0), invRestMat(1, 0), 0.0);
// 	c[1] = i_math::vector3df(invRestMat(0, 1), invRestMat(1, 1), 0.0);
// 
// 	i_math::vector3df r[3];
// 
// 	corr0.setZero();
// 	corr1.setZero();
// 	corr2.setZero();
// 
// 	for (int i = 0; i < 2; i++) {
// 		for (int j = 0; j <= i; j++) {
// 
// // 			r[0] = i_math::vector3df(p1[0] - p0[0], p2[0] - p0[0], 0.0);  // Jacobi
// // 			r[1] = i_math::vector3df(p1[1] - p0[1], p2[1] - p0[1], 0.0);
// // 			r[2] = i_math::vector3df(p1[2] - p0[2], p2[2] - p0[2], 0.0);
// 
// 			r[0] = i_math::vector3df((p1[0] + corr1[0]) - (p0[0] + corr0[0]), (p2[0] + corr2[0]) - (p0[0] + corr0[0]), 0.0);		// Gauss - Seidel
// 			r[1] = i_math::vector3df((p1[1] + corr1[1]) - (p0[1] + corr0[1]), (p2[1] + corr2[1]) - (p0[1] + corr0[1]), 0.0);
// 			r[2] = i_math::vector3df((p1[2] + corr1[2]) - (p0[2] + corr0[2]), (p2[2] + corr2[2]) - (p0[2] + corr0[2]), 0.0);
// 
// 
// 			Real Sij = 0.0;
// 			for (int k = 0; k < 3; k++)
// 				Sij += r[k].dotProduct(c[i]) * r[k].dotProduct(c[j]);
// 
// 			i_math::vector3df d[3];
// 			d[0] = i_math::vector3df(0.0, 0.0, 0.0);
// 
// 			for (int k = 0; k < 2; k++) {
// 				d[k+1]  = i_math::vector3df(r[0].dotProduct(c[j]), r[1].dotProduct(c[j]), r[2].dotProduct(c[j])) * invRestMat(k, i);
// 				d[k+1] += i_math::vector3df(r[0].dotProduct(c[i]), r[1].dotProduct(c[i]), r[2].dotProduct(c[i])) * invRestMat(k, j);
// 				d[0] -= d[k+1];
// 			}
// 
// 			if (i != j && normalizeShear) {
// 				Real fi2 = 0.0;
// 				Real fj2 = 0.0;
// 				for (int k = 0; k < 3; k++) {
// 					fi2 += r[k].dotProduct(c[i]) * r[k].dotProduct(c[i]);
// 					fj2 += r[k].dotProduct(c[j]) * r[k].dotProduct(c[j]);
// 				}
// 				Real fi = sqrtf(fi2);
// 				Real fj = sqrtf(fj2);
// 
// 				d[0] = i_math::vector3df(0.0, 0.0, 0.0);
// 				Real s = Sij / (fi2*fi*fj2*fj);
// 				for (int k = 0; k < 2; k++) {
// 					d[k+1] /= fi * fj;
// 					d[k+1] -= fj*fj * i_math::vector3df(r[0].dotProduct(c[i]), r[1].dotProduct(c[i]), r[2].dotProduct(c[i])) * invRestMat(k, i) * s;
// 					d[k+1] -= fi*fi * i_math::vector3df(r[0].dotProduct(c[j]), r[1].dotProduct(c[j]), r[2].dotProduct(c[j])) * invRestMat(k, j) * s;
// 					d[0] -= d[k+1];
// 				}
// 				Sij = Sij / (fi * fj);
// 			}
// 
// 			Real lambda = 
// 				invMass0 * d[0].getLengthSQ() +
// 				invMass1 * d[1].getLengthSQ() +
// 				invMass2 * d[2].getLengthSQ();
// 
// 			if (lambda == 0.0)
// 				continue;
// 
// 			if (i == 0 && j == 0) {
// 				if (normalizeStretch) {
// 					Real s = sqrtf(Sij);
// 					lambda = static_cast<Real>(2.0) * s * (s - static_cast<Real>(1.0)) / lambda * xxStiffness;
// 				}
// 				else {
// 					lambda = (Sij - static_cast<Real>(1.0)) / lambda * xxStiffness;
// 				}
// 			}
// 			else if (i == 1 && j == 1) {
// 				if (normalizeStretch) {
// 					Real s = sqrtf(Sij);
// 					lambda = static_cast<Real>(2.0) * s * (s - static_cast<Real>(1.0)) / lambda * yyStiffness;
// 				}
// 				else {
// 					lambda = (Sij - static_cast<Real>(1.0)) / lambda * yyStiffness;
// 				}
// 			}
// 			else {
// 				lambda = Sij / lambda * xyStiffness;
// 			}
// 
// 			corr0 -= lambda * invMass0 * d[0];
// 			corr1 -= lambda * invMass1 * d[1];
// 			corr2 -= lambda * invMass2 * d[2];
// 		}
// 	}
// 	return true;
// }

// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::init_StrainTetraConstraint(		
// 	const i_math::vector3df &p0,
// 	const i_math::vector3df &p1,
// 	const i_math::vector3df &p2,
// 	const i_math::vector3df &p3,
// 	Matrix3r &invRestMat)
// {
// 	Matrix3r m;
// 	m.col(0) = p1 - p0;
// 	m.col(1) = p2 - p0;
// 	m.col(2) = p3 - p0;
// 
// 	Real det = m.determinant();
// 	if (fabsf(det) > eps)
// 	{
// 		invRestMat = m.inverse();
// 		return true;
// 	}
// 	return false;
// }

// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::solve_StrainTetraConstraint(
// 	const i_math::vector3df &p0, Real invMass0, 
// 	const i_math::vector3df &p1, Real invMass1,
// 	const i_math::vector3df &p2, Real invMass2,
// 	const i_math::vector3df &p3, Real invMass3,
// 	const Matrix3r &invRestMat,
// 	const i_math::vector3df &stretchStiffness,	
// 	const i_math::vector3df &shearStiffness,	
// 	const bool normalizeStretch,
// 	const bool normalizeShear,
// 	i_math::vector3df &corr0, i_math::vector3df &corr1, i_math::vector3df &corr2, i_math::vector3df &corr3)
// {
// 	corr0.setZero();
// 	corr1.setZero();
// 	corr2.setZero();
// 	corr3.setZero();
// 
// 	i_math::vector3df c[3];
// 	c[0] = invRestMat.col(0);
// 	c[1] = invRestMat.col(1);
// 	c[2] = invRestMat.col(2);
// 
// 	for (int i = 0; i < 3; i++) {
// 		for (int j = 0; j <= i; j++) {
// 
// 			Matrix3r P;
// // 			P.col(0) = p1 - p0;		// Jacobi
// // 			P.col(1) = p2 - p0;
// // 			P.col(2) = p3 - p0;
// 
// 			P.col(0) = (p1 + corr1) - (p0 + corr0);		// Gauss - Seidel
// 			P.col(1) = (p2 + corr2) - (p0 + corr0);
// 			P.col(2) = (p3 + corr3) - (p0 + corr0);
// 
// 			i_math::vector3df fi = P * c[i];
// 			i_math::vector3df fj = P * c[j];
// 
// 			Real Sij = fi.dotProduct(fj);
// 
// 			Real wi,wj,s1,s3;
// 			if (normalizeShear && i != j) {
// 				wi = fi.getLength();
// 				wj = fj.getLength();
// 				s1 = static_cast<Real>(1.0) / (wi*wj);
// 				s3 = s1 * s1 * s1;
// 			}
// 
// 			i_math::vector3df d[4];
// 			d[0] = i_math::vector3df(0.0, 0.0, 0.0);
// 
// 			for (int k = 0; k < 3; k++) {
// 				d[k+1] = fj * invRestMat(k,i) + fi * invRestMat(k,j);
// 
// 				if (normalizeShear && i != j) {
// 					d[k+1] = s1 * d[k+1] - Sij*s3 * (wj*wj * fi*invRestMat(k,i) + wi*wi * fj*invRestMat(k,j));
// 				}
// 
// 				d[0] -= d[k+1];
// 			}
// 
// 			if (normalizeShear && i != j)
// 				Sij *= s1;
// 
// 			Real lambda = 
// 				invMass0 * d[0].getLengthSQ() +
// 				invMass1 * d[1].getLengthSQ() +
// 				invMass2 * d[2].getLengthSQ() +
// 				invMass3 * d[3].getLengthSQ();
// 
// 			if (fabsf(lambda) < eps)		// foo: threshold should be scale dependent
// 				continue;
// 
// 			if (i == j) {	// diagonal, stretch
// 				if (normalizeStretch)  {
// 					Real s = sqrtf(Sij);
// 					lambda = static_cast<Real>(2.0) * s * (s - static_cast<Real>(1.0)) / lambda * stretchStiffness[i];
// 				}
// 				else {
// 					lambda = (Sij - static_cast<Real>(1.0)) / lambda * stretchStiffness[i];
// 				}
// 			}
// 			else {		// off diagonal, shear
// 				lambda = Sij / lambda * shearStiffness[i + j - 1];
// 			}
// 
// 			corr0 -= lambda * invMass0 * d[0];
// 			corr1 -= lambda * invMass1 * d[1];
// 			corr2 -= lambda * invMass2 * d[2];
// 			corr3 -= lambda * invMass3 * d[3];
// 		}
// 	}
// 	return true;
// }

// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::init_FEMTriangleConstraint(	
// 	const i_math::vector3df &p0,
// 	const i_math::vector3df &p1,
// 	const i_math::vector3df &p2,
// 	Real &area, 
// 	Matrix2r &invRestMat)
// {
// 	i_math::vector3df normal0 = (p1 - p0).crossProduct(p2 - p0);
// 	area = normal0.getLength() * static_cast<Real>(0.5);
// 
// 	i_math::vector3df axis0_1 = p1 - p0;
// 	axis0_1.normalize();
// 	i_math::vector3df axis0_2 = normal0.crossProduct(axis0_1);
// 	axis0_2.normalize();
// 
// 	i_math::vector2df p[3];
// 	p[0] = i_math::vector2df(p0.dotProduct(axis0_2), p0.dotProduct(axis0_1));
// 	p[1] = i_math::vector2df(p1.dotProduct(axis0_2), p1.dotProduct(axis0_1));
// 	p[2] = i_math::vector2df(p2.dotProduct(axis0_2), p2.dotProduct(axis0_1));
// 
// 	Matrix2r P;
// 	P(0, 0) = p[0][0] - p[2][0];
// 	P(1, 0) = p[0][1] - p[2][1];
// 	P(0, 1) = p[1][0] - p[2][0];
// 	P(1, 1) = p[1][1] - p[2][1];
// 
// 	const Real det = P.determinant();
// 	if (fabsf(det) > eps)
// 	{
// 		invRestMat = P.inverse();
// 		return true;
// 	}
// 	return false;
// }

// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::solve_FEMTriangleConstraint(		
// 	const i_math::vector3df &p0, Real invMass0, 
// 	const i_math::vector3df &p1, Real invMass1,
// 	const i_math::vector3df &p2, Real invMass2,
// 	const Real &area,
// 	const Matrix2r &invRestMat,
// 	const Real youngsModulusX,
// 	const Real youngsModulusY,
// 	const Real youngsModulusShear,
// 	const Real poissonRatioXY,
// 	const Real poissonRatioYX,
// 	i_math::vector3df &corr0, i_math::vector3df &corr1, i_math::vector3df &corr2)
// {
// 	// Orthotropic elasticity tensor
// 	Matrix3r C;
// 	C.setZero();
// 	C(0, 0) = youngsModulusX / (static_cast<Real>(1.0) - poissonRatioXY*poissonRatioYX);
// 	C(0, 1) = youngsModulusX*poissonRatioYX / (static_cast<Real>(1.0) - poissonRatioXY*poissonRatioYX);
// 	C(1, 1) = youngsModulusY / (static_cast<Real>(1.0) - poissonRatioXY*poissonRatioYX);
// 	C(1, 0) = youngsModulusY*poissonRatioXY / (static_cast<Real>(1.0) - poissonRatioXY*poissonRatioYX);
// 	C(2, 2) = youngsModulusShear;
// 
// 	// Determine \partial x/\partial m_i
// 	Eigen::Matrix<Real, 3, 2> F;
// 	const i_math::vector3df p13 = p0 - p2;
// 	const i_math::vector3df p23 = p1 - p2;
// 	F(0,0) = p13[0] * invRestMat(0,0) + p23[0] * invRestMat(1,0);
// 	F(0,1) = p13[0] * invRestMat(0,1) + p23[0] * invRestMat(1,1);
// 	F(1,0) = p13[1] * invRestMat(0,0) + p23[1] * invRestMat(1,0);
// 	F(1,1) = p13[1] * invRestMat(0,1) + p23[1] * invRestMat(1,1);
// 	F(2,0) = p13[2] * invRestMat(0,0) + p23[2] * invRestMat(1,0);
// 	F(2,1) = p13[2] * invRestMat(0,1) + p23[2] * invRestMat(1,1);
// 
// 	// epsilon = 0.5(F^T * F - I)
// 	Matrix2r epsilon;
// 	epsilon(0,0) = static_cast<Real>(0.5)*(F(0,0) * F(0,0) + F(1,0) * F(1,0) + F(2,0) * F(2,0) - static_cast<Real>(1.0));		// xx
// 	epsilon(1,1) = static_cast<Real>(0.5)*(F(0,1) * F(0,1) + F(1,1) * F(1,1) + F(2,1) * F(2,1) - static_cast<Real>(1.0));		// yy
// 	epsilon(0,1) = static_cast<Real>(0.5)*(F(0,0) * F(0,1) + F(1,0) * F(1,1) + F(2,0) * F(2,1));			// xy
// 	epsilon(1,0) = epsilon(0,1);
// 
// 	// P(F) = det(F) * C*E * F^-T => E = green strain
// 	Matrix2r stress;
// 	stress(0,0) = C(0,0) * epsilon(0,0) + C(0,1) * epsilon(1,1) + C(0,2) * epsilon(0,1);
// 	stress(1,1) = C(1,0) * epsilon(0,0) + C(1,1) * epsilon(1,1) + C(1,2) * epsilon(0,1);
// 	stress(0,1) = C(2,0) * epsilon(0,0) + C(2,1) * epsilon(1,1) + C(2,2) * epsilon(0,1);
// 	stress(1,0) = stress(0,1);
// 
// 	const Eigen::Matrix<Real, 3, 2> piolaKirchhoffStres = F * stress;
// 
// 	Real psi = 0.0;
// 	for (unsigned char j = 0; j < 2; j++)
// 		for (unsigned char k = 0; k < 2; k++)
// 			psi += epsilon(j,k) * stress(j,k);
// 	psi = static_cast<Real>(0.5)*psi;
// 	Real energy = area*psi;
// 
// 	// compute gradient
// 	Eigen::Matrix<Real, 3, 2> H = area * piolaKirchhoffStres * invRestMat.transpose();
// 	i_math::vector3df gradC[3];
// 	for (unsigned char j = 0; j < 3; ++j)
// 	{
// 		gradC[0][j] = H(j,0);
// 		gradC[1][j] = H(j,1);
// 	}
// 	gradC[2] = -gradC[0] - gradC[1];
// 
// 
// 	Real sum_normGradC = invMass0 * gradC[0].getLengthSQ();
// 	sum_normGradC += invMass1 * gradC[1].getLengthSQ();
// 	sum_normGradC += invMass2 * gradC[2].getLengthSQ();
// 
// 	// exit early if required
// 	if (fabsf(sum_normGradC) > eps)
// 	{
// 		// compute scaling factor
// 		const Real s = energy / sum_normGradC;
// 
// 		// update positions
// 		corr0 = -(s*invMass0) * gradC[0];
// 		corr1 = -(s*invMass1) * gradC[1];
// 		corr2 = -(s*invMass2) * gradC[2];
// 
// 		return true;
// 	}
// 
// 	return false;
// }

// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::init_FEMTetraConstraint(			// compute only when rest shape changes
// 		const i_math::vector3df &p0,
// 		const i_math::vector3df &p1,
// 		const i_math::vector3df &p2,
// 		const i_math::vector3df &p3,
// 		Real &volume,
// 		Matrix3r &invRestMat)
// {
// 	volume = fabsf(static_cast<Real>(1.0 / 6.0) * (p3 - p0).dotProduct((p2 - p0).crossProduct(p1 - p0)));
// 
// 	Matrix3r m;
// 	m.col(0) = p0 - p3;
// 	m.col(1) = p1 - p3;
// 	m.col(2) = p2 - p3;
// 
// 	Real det = m.determinant();
// 	if (fabsf(det) > eps)
// 	{
// 		invRestMat = m.inverse();
// 		return true;
// 	}
// 	return false;
// }

// ----------------------------------------------------------------------------------------------
// void PositionBasedDynamics::computeGreenStrainAndPiolaStress(
// 	const i_math::vector3df &x1, const i_math::vector3df &x2, const i_math::vector3df &x3, const i_math::vector3df &x4,
// 	const Matrix3r &invRestMat,
// 	const Real restVolume,
// 	const Real mu, const Real lambda, Matrix3r &epsilon, Matrix3r &sigma, Real &energy)
// {
// 	// Determine \partial x/\partial m_i
// 	Matrix3r F;
// 	const i_math::vector3df p14 = x1 - x4;
// 	const i_math::vector3df p24 = x2 - x4;
// 	const i_math::vector3df p34 = x3 - x4;
// 	F(0, 0) = p14[0]*invRestMat(0, 0) + p24[0]*invRestMat(1, 0) + p34[0]*invRestMat(2, 0);
// 	F(0, 1) = p14[0]*invRestMat(0, 1) + p24[0]*invRestMat(1, 1) + p34[0]*invRestMat(2, 1);
// 	F(0, 2) = p14[0]*invRestMat(0, 2) + p24[0]*invRestMat(1, 2) + p34[0]*invRestMat(2, 2);
// 
// 	F(1, 0) = p14[1]*invRestMat(0, 0) + p24[1]*invRestMat(1, 0) + p34[1]*invRestMat(2, 0);
// 	F(1, 1) = p14[1]*invRestMat(0, 1) + p24[1]*invRestMat(1, 1) + p34[1]*invRestMat(2, 1);
// 	F(1, 2) = p14[1]*invRestMat(0, 2) + p24[1]*invRestMat(1, 2) + p34[1]*invRestMat(2, 2);
// 
// 	F(2, 0) = p14[2]*invRestMat(0, 0) + p24[2]*invRestMat(1, 0) + p34[2]*invRestMat(2, 0);
// 	F(2, 1) = p14[2]*invRestMat(0, 1) + p24[2]*invRestMat(1, 1) + p34[2]*invRestMat(2, 1);
// 	F(2, 2) = p14[2]*invRestMat(0, 2) + p24[2]*invRestMat(1, 2) + p34[2]*invRestMat(2, 2);
// 
// 	// epsilon = 1/2 F^T F - I
// 
// 	epsilon(0, 0) = static_cast<Real>(0.5)*(F(0, 0) * F(0, 0) + F(1, 0) * F(1, 0) + F(2, 0) * F(2, 0) - static_cast<Real>(1.0));		// xx
// 	epsilon(1, 1) = static_cast<Real>(0.5)*(F(0, 1) * F(0, 1) + F(1, 1) * F(1, 1) + F(2, 1) * F(2, 1) - static_cast<Real>(1.0));		// yy
// 	epsilon(2, 2) = static_cast<Real>(0.5)*(F(0, 2) * F(0, 2) + F(1, 2) * F(1, 2) + F(2, 2) * F(2, 2) - static_cast<Real>(1.0));		// zz
// 	epsilon(0, 1) = static_cast<Real>(0.5)*(F(0, 0) * F(0, 1) + F(1, 0) * F(1, 1) + F(2, 0) * F(2, 1));			// xy
// 	epsilon(0, 2) = static_cast<Real>(0.5)*(F(0, 0) * F(0, 2) + F(1, 0) * F(1, 2) + F(2, 0) * F(2, 2));			// xz
// 	epsilon(1, 2) = static_cast<Real>(0.5)*(F(0, 1) * F(0, 2) + F(1, 1) * F(1, 2) + F(2, 1) * F(2, 2));			// yz
// 	epsilon(1, 0) = epsilon(0, 1);
// 	epsilon(2, 0) = epsilon(0, 2);
// 	epsilon(2, 1) = epsilon(1, 2);
// 
// 	// P(F) = F(2 mu E + lambda tr(E)I) => E = green strain
// 	const Real trace = epsilon(0, 0) + epsilon(1, 1) + epsilon(2, 2);
// 	const Real ltrace = lambda*trace;
// 	sigma = epsilon * 2.0*mu;
// 	sigma(0, 0) += ltrace;
// 	sigma(1, 1) += ltrace;
// 	sigma(2, 2) += ltrace;
// 	sigma = F * sigma;
// 
// 	Real psi = 0.0;
// 	for (unsigned char j = 0; j < 3; j++)
// 		for (unsigned char k = 0; k < 3; k++)
// 			psi += epsilon(j, k) * epsilon(j, k);
// 	psi = mu*psi + static_cast<Real>(0.5)*lambda * trace*trace;
// 	energy = restVolume * psi;
// }

// ----------------------------------------------------------------------------------------------
// void PositionBasedDynamics::computeGradCGreen(Real restVolume, const Matrix3r &invRestMat, const Matrix3r &sigma, i_math::vector3df *J)
// {
// 	Matrix3r H;
// 	Matrix3r T;
// 	T = invRestMat.transpose();
// 	H = sigma * T * restVolume;
// 
// 	J[0][0] = H(0, 0);
// 	J[1][0] = H(0, 1);
// 	J[2][0] = H(0, 2);
// 
// 	J[0][1] = H(1, 0);
// 	J[1][1] = H(1, 1);
// 	J[2][1] = H(1, 2);
// 
// 	J[0][2] = H(2, 0);
// 	J[1][2] = H(2, 1);
// 	J[2][2] = H(2, 2);
// 
// 	J[3] = -J[0] - J[1] - J[2];
// }

// ----------------------------------------------------------------------------------------------
// void PositionBasedDynamics::computeGreenStrainAndPiolaStressInversion(
// 	const i_math::vector3df &x1, const i_math::vector3df &x2, const i_math::vector3df &x3, const i_math::vector3df &x4,
// 	const Matrix3r &invRestMat,
// 	const Real restVolume,
// 	const Real mu, const Real lambda, Matrix3r &epsilon, Matrix3r &sigma, Real &energy)
// {
// 	// Determine \partial x/\partial m_i
// 	Matrix3r F;
// 	const i_math::vector3df p14 = x1 - x4;
// 	const i_math::vector3df p24 = x2 - x4;
// 	const i_math::vector3df p34 = x3 - x4;
// 	F(0, 0) = p14[0]*invRestMat(0, 0) + p24[0]*invRestMat(1, 0) + p34[0]*invRestMat(2, 0);
// 	F(0, 1) = p14[0]*invRestMat(0, 1) + p24[0]*invRestMat(1, 1) + p34[0]*invRestMat(2, 1);
// 	F(0, 2) = p14[0]*invRestMat(0, 2) + p24[0]*invRestMat(1, 2) + p34[0]*invRestMat(2, 2);
// 
// 	F(1, 0) = p14[1]*invRestMat(0, 0) + p24[1]*invRestMat(1, 0) + p34[1]*invRestMat(2, 0);
// 	F(1, 1) = p14[1]*invRestMat(0, 1) + p24[1]*invRestMat(1, 1) + p34[1]*invRestMat(2, 1);
// 	F(1, 2) = p14[1]*invRestMat(0, 2) + p24[1]*invRestMat(1, 2) + p34[1]*invRestMat(2, 2);
// 
// 	F(2, 0) = p14[2]*invRestMat(0, 0) + p24[2]*invRestMat(1, 0) + p34[2]*invRestMat(2, 0);
// 	F(2, 1) = p14[2]*invRestMat(0, 1) + p24[2]*invRestMat(1, 1) + p34[2]*invRestMat(2, 1);
// 	F(2, 2) = p14[2]*invRestMat(0, 2) + p24[2]*invRestMat(1, 2) + p34[2]*invRestMat(2, 2);
// 
// 	Matrix3r U, VT;
// 	i_math::vector3df hatF;
// 	MathFunctions::svdWithInversionHandling(F, hatF, U, VT);
// 
// 	// Clamp small singular values
// 	const Real minXVal = static_cast<Real>(0.577);
// 
// 	for (unsigned char j = 0; j < 3; j++)
// 	{
// 		if (hatF[j] < minXVal)
// 			hatF[j] = minXVal;
// 	}
// 
// 	// epsilon for hatF
// 	i_math::vector3df epsilonHatF(	static_cast<Real>(0.5)*(hatF[0]*hatF[0] - static_cast<Real>(1.0)), 
// 							static_cast<Real>(0.5)*(hatF[1]*hatF[1] - static_cast<Real>(1.0)), 
// 							static_cast<Real>(0.5)*(hatF[2]*hatF[2] - static_cast<Real>(1.0)));
// 
// 	const Real trace = epsilonHatF[0] + epsilonHatF[1] + epsilonHatF[2];
// 	const Real ltrace = lambda*trace;
// 	i_math::vector3df sigmaVec = epsilonHatF * 2.0*mu;
// 	sigmaVec[0] += ltrace;
// 	sigmaVec[1] += ltrace;
// 	sigmaVec[2] += ltrace;
// 	sigmaVec[0] = hatF[0] * sigmaVec[0];
// 	sigmaVec[1] = hatF[1] * sigmaVec[1];
// 	sigmaVec[2] = hatF[2] * sigmaVec[2];
// 
// 	Matrix3r sigmaDiag, epsDiag;
// 
// 	sigmaDiag.row(0) = i_math::vector3df(sigmaVec[0], 0.0, 0.0);
// 	sigmaDiag.row(1) = i_math::vector3df(0.0, sigmaVec[1], 0.0);
// 	sigmaDiag.row(2) = i_math::vector3df(0.0, 0.0, sigmaVec[2]);
// 
// 	epsDiag.row(0) = i_math::vector3df(epsilonHatF[0], 0.0, 0.0);
// 	epsDiag.row(1) = i_math::vector3df(0.0, epsilonHatF[1], 0.0);
// 	epsDiag.row(2) = i_math::vector3df(0.0, 0.0, epsilonHatF[2]);
// 
// 	epsilon = U*epsDiag*VT;
// 	sigma = U*sigmaDiag*VT;
// 
// 	Real psi = 0.0;
// 	for (unsigned char j = 0; j < 3; j++)
// 		for (unsigned char k = 0; k < 3; k++)
// 			psi += epsilon(j, k) * epsilon(j, k);
// 	psi = mu*psi + static_cast<Real>(0.5)*lambda * trace*trace;
// 	energy = restVolume*psi;
// }



// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::solve_FEMTetraConstraint(
// 	const i_math::vector3df &p0, Real invMass0, 
// 	const i_math::vector3df &p1, Real invMass1,
// 	const i_math::vector3df &p2, Real invMass2,
// 	const i_math::vector3df &p3, Real invMass3,
// 	const Real restVolume,
// 	const Matrix3r &invRestMat,
// 	const Real youngsModulus,
// 	const Real poissonRatio,
// 	const bool  handleInversion,
// 	i_math::vector3df &corr0, i_math::vector3df &corr1, i_math::vector3df &corr2, i_math::vector3df &corr3)
// {
// 	corr0.setZero();
// 	corr1.setZero();
// 	corr2.setZero();
// 	corr3.setZero();
// 
// 	if (youngsModulus <= 0.0)
// 		return true;
// 
// 	if (poissonRatio < 0.0 || poissonRatio > 0.49)
// 		return false;
// 
// 	Real C = 0.0;
// 	i_math::vector3df gradC[4];
// 	Matrix3r epsilon, sigma;
// 	Real volume = (p1 - p0).crossProduct(p2 - p0).dotProduct(p3 - p0) / static_cast<Real>(6.0);
// 
// 	Real mu = youngsModulus / static_cast<Real>(2.0) / (static_cast<Real>(1.0) + poissonRatio);
// 	Real lambda = youngsModulus * poissonRatio / (static_cast<Real>(1.0) + poissonRatio) / (static_cast<Real>(1.0) - static_cast<Real>(2.0) * poissonRatio);
// 
// 	if (!handleInversion || volume > 0.0)
// 	{
// 		computeGreenStrainAndPiolaStress(p0, p1, p2, p3, invRestMat, restVolume, mu, lambda, epsilon, sigma, C);
// 		computeGradCGreen(restVolume, invRestMat, sigma, gradC);
// 	}
// 	else
// 	{
// 		computeGreenStrainAndPiolaStressInversion(p0, p1, p2, p3, invRestMat, restVolume, mu, lambda, epsilon, sigma, C);
// 		computeGradCGreen(restVolume, invRestMat, sigma, gradC);
// 	}
// 
// 	Real sum_normGradC =
// 		invMass0 * gradC[0].getLengthSQ() +
// 		invMass1 * gradC[1].getLengthSQ() +
// 		invMass2 * gradC[2].getLengthSQ() +
// 		invMass3 * gradC[3].getLengthSQ();
// 
// 	if (sum_normGradC < eps)
// 		return false;
// 
// 	// compute scaling factor
// 	const Real s = C / sum_normGradC;
// 
// 	corr0 = -s * invMass0 * gradC[0];
// 	corr1 = -s * invMass1 * gradC[1];
// 	corr2 = -s * invMass2 * gradC[2];
// 	corr3 = -s * invMass3 * gradC[3];
// 
// 	return true;
// }

// ----------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::init_ParticleTetContactConstraint(
// 	const Real invMass0,							// inverse mass is zero if particle is static
// 	const i_math::vector3df &x0,								// particle which collides with tet
// 	const i_math::vector3df &v0,								// velocity of particle
// 	const Real invMass[],							// inverse masses of tet particles
// 	const i_math::vector3df x[],								// positions of tet particles
// 	const i_math::vector3df v[],								// velocities of tet particles
// 	const i_math::vector3df &bary,							// barycentric coordinates of contact point in tet
// 	const i_math::vector3df &normal,							// contact normal in body 1
// 	Eigen::Matrix<Real, 3, 3, Eigen::DontAlign> &constraintInfo)
// {
// 	// constraintInfo contains
// 	// 0:	contact normal in body 1 (global)
// 	// 1:	contact tangent (global)
// 	// 0,1:  1.0 / normal^T * K * normal
// 	// 1,2: maximal impulse in tangent direction
// 
// 	const Real bary0 = static_cast<Real>(1.0) - bary[0] - bary[1] - bary[2];
// 
// 	// compute world space contact point in body 2	
// 	const i_math::vector3df v1 = bary0*v[0] + bary[0] * v[1] + bary[1] * v[2] + bary[2] * v[3];
// 
// 	// compute goal velocity in normal direction after collision
// 	const i_math::vector3df u_rel = v0 - v1;
// 	const Real u_rel_n = normal.dotProduct(u_rel);
// 
// 	constraintInfo.col(0) = normal;
// 
// 	// tangent direction
// 	i_math::vector3df t = u_rel - u_rel_n*normal;
// 	Real tl2 = t.getLengthSQ();
// 	if (tl2 > 1.0e-6)
// 		t *= static_cast<Real>(1.0) / sqrtf(tl2);
// 
// 	constraintInfo.col(1) = t;
// 
// 	// determine 1/(J M^-1 J^T)	
// 	const Real JMinvJT = invMass0 + bary0*bary0*invMass[0] + bary[0]*bary[0]*invMass[1] + bary[1]*bary[1]*invMass[2] + bary[2]*bary[2]*invMass[3];
// 	constraintInfo(0, 2) = static_cast<Real>(1.0) / JMinvJT;
// 
// 	// maximal impulse in tangent direction
// 	constraintInfo(1, 2) = static_cast<Real>(1.0) / JMinvJT * u_rel.dotProduct(t);
// 
// 	return true;
// }

//--------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::solve_ParticleTetContactConstraint(
// 	const Real invMass0,							// inverse mass is zero if particle is static
// 	const i_math::vector3df &x0,								// particle which collides with tet
// 	const Real invMass[],							// inverse masses of tet particles
// 	const i_math::vector3df x[],								// positions of tet particles
// 	const i_math::vector3df &bary,							// barycentric coordinates of contact point in tet
// 	Eigen::Matrix<Real, 3, 3, Eigen::DontAlign> &constraintInfo,		// precomputed contact info
// 	Real &lambda,
// 	i_math::vector3df &corr0,
// 	i_math::vector3df corr[])
// {
// 	// constraintInfo contains
// 	// 0:	contact normal in body 1 (global)
// 	// 1:	contact tangent (global)
// 	// 0,2:  1.0 / normal^T * K * normal
// 	// 1,2: maximal impulse in tangent direction
// 
// 	if ((invMass0 == 0.0) && (invMass[0] == 0.0) && (invMass[1] == 0.0) && (invMass[2] == 0.0))
// 		return false;
// 
// 	const Real bary0 = static_cast<Real>(1.0) - bary[0] - bary[1] - bary[2];
// 
// 	// compute world space contact point in body 2	
// 	const i_math::vector3df cp1 = bary0*x[0] + bary[0] * x[1] + bary[1] * x[2] + bary[2] * x[3];
// 
// 	const i_math::vector3df &normal = constraintInfo.col(0);
// 
// 	// 1.0 / normal^T * K * normal
// 	const Real nKn_inv = constraintInfo(0, 2);
// 
// 	// penetration depth 
// 	const Real C = normal.dotProduct(x0 - cp1);
// 
// 	lambda = -nKn_inv * C;
// 
// 
// 	i_math::vector3df p(lambda * normal);
// 	if (invMass0 != 0.0)
// 	{
// 		corr0 = invMass0*p;
// 	}
// 
// 	if (invMass[0] != 0.0)
// 		corr[0] = -invMass[0] * bary0*p;
// 	if (invMass[1] != 0.0)
// 		corr[1] = -invMass[1] * bary[0] * p;
// 	if (invMass[2] != 0.0)
// 		corr[2] = -invMass[2] * bary[1] * p;
// 	if (invMass[3] != 0.0)
// 		corr[3] = -invMass[3] * bary[2] * p;
// 
// 	return true;
// }

//--------------------------------------------------------------------------------------------
// bool PositionBasedDynamics::velocitySolve_ParticleTetContactConstraint(
// 	const Real invMass0,							// inverse mass is zero if particle is static
// 	const i_math::vector3df &x0,								// particle which collides with tet
// 	const i_math::vector3df &v0,								// velocity of particle
// 	const Real invMass[],							// inverse masses of tet particles
// 	const i_math::vector3df x[],								// positions of tet particles
// 	const i_math::vector3df v[],								// velocities of tet particles
// 	const i_math::vector3df &bary,							// barycentric coordinates of contact point in tet
// 	const Real lambda, 
// 	const Real frictionCoeff,						// friction coefficient
// 	Eigen::Matrix<Real, 3, 3, Eigen::DontAlign> &constraintInfo,		// precomputed contact info
// 	i_math::vector3df &corr_v0,
// 	i_math::vector3df corr_v[])
// {
// 	// constraintInfo contains
// 	// 0:	contact normal in body 1 (global)
// 	// 1:	contact tangent (global)
// 	// 0,2:  1.0 / normal^T * K * normal
// 	// 1,2: maximal impulse in tangent direction
// 
// 	if ((invMass0 == 0.0) && (invMass[0] == 0.0) && (invMass[1] == 0.0) && (invMass[2] == 0.0))
// 		return false;
// 
//  	const Real bary0 = static_cast<Real>(1.0) - bary[0] - bary[1] - bary[2];
//  
//  	// Friction
//  	// maximal impulse in tangent direction
//  	const Real pMax = constraintInfo(1, 2);
//  	const i_math::vector3df &tangent = constraintInfo.col(1);
//  	i_math::vector3df pv;
//  	if (frictionCoeff * lambda > pMax)
//  		pv = -pMax * tangent;
//  	else if (frictionCoeff * lambda < -pMax)
//  		pv = pMax * tangent;
//  	else
//  		pv = -frictionCoeff * lambda * tangent;
//  
//  	if (invMass0 != 0.0)
//  	{
//  		corr_v0 = invMass0*pv;
//  	}
//  
//  	if (invMass[0] != 0.0)
//  		corr_v[0] = -invMass[0] * bary0*pv;
//  	if (invMass[1] != 0.0)
//  		corr_v[1] = -invMass[1] * bary[0] * pv;
//  	if (invMass[2] != 0.0)
//  		corr_v[2] = -invMass[2] * bary[1] * pv;
//  	if (invMass[3] != 0.0)
//  		corr_v[3] = -invMass[3] * bary[2] * pv;
// 
// 
// 	return true;
// }
