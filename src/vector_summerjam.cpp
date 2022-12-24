//vector_summerjam.cpp

#ifndef VECTOR_SUMMERJAM_H

Vector2 operator+( Vector2 const& lhs, Vector2 const& rhs )
{
	Vector2 result;
	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;
	return result;
}

Vector2 operator+=(Vector2 const& lhs, Vector2 const& rhs )
{
	Vector2 result;
	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;
	return result;
}

Vector2 operator-( Vector2 const& lhs, Vector2 const& rhs )
{
	Vector2 result;
	result.x = lhs.x - rhs.x;
	result.y = lhs.y - rhs.y;
	return result;
}

Vector2 operator*( Vector2 const& lhs, Vector2 const& rhs )
{
	Vector2 result;
	result.x = lhs.x * rhs.x;
	result.y = lhs.y * rhs.y;
	return result;
}

static Vector2 Rotate(Vector2 const& vec, float rotate)
{
	Vector2 result = vec;
	f32 st = sinf(rotate);
	f32 ct = cosf(rotate);

	result.x = vec.x * ct + vec.y * st;
	result.y = vec.x * -st + vec.y * ct;
	return result;
}

#define VECTOR_SUMMERJAM_H
#endif //VECTOR_SUMMERJAM_H