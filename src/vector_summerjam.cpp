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

bool operator==( Vector2 const& lhs, Vector2 const& rhs )
{
	return ((lhs.x == rhs.x) && (lhs.y == rhs.y));
}

Vector2 operator*( Vector2 const& lhs, Vector2 const& rhs )
{
	Vector2 result;
	result.x = lhs.x * rhs.x;
	result.y = lhs.y * rhs.y;
	return result;
}

#define VECTOR_SUMMERJAM_H
#endif //VECTOR_SUMMERJAM_H