//useful stuff

#ifndef USEFUL_SUMMERJAM_STUFF

static f32 clamp(f32 in, f32 clamp)
{
	float out = in;
	if(out < -clamp)
		out = -clamp;
	if(out > clamp)
		out = clamp;
	return out;
}

static int32 clamp_min_max(int32 min, int32 value, int32 max)
{
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

static f32 clampf_min_max(f32 min, f32 value, f32 max)
{
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

static f32 move_towards(f32 value, f32 target, f32 speed)
{
    if(value > target) return clampf_min_max(target, value - speed, value);
    if(value < target) return clampf_min_max(value, value + speed, target);
    return value;
}

static f32 lerp(f32 a, f32 t, f32 b)
{
    return (1.0f - t)*a + t*b;
}

static f32 abs(f32 in)
{
	if(in < 0)
		return -in;
	
	return in;
}

static f32 minf(f32 a, f32 b)
{
	return a < b ? a : b;
}

static f32 maxf(f32 a, f32 b)
{
	return a > b ? a : b;
}

static int32 maxint32(int32 a, int32 b)
{
	return a > b ? a : b;
}

static f32 round_down_to_dp(f32 num, int dp)
{
	uint32 pow = 1;
	for (int i = 0; i < dp; i++)
	{
		pow = pow * 10;
	}
	int val = 0; 
	if (num > 0.f) val = (int)(num*pow);
	else if (num < 0.f) val = (int)(num*pow)-1;
	return ((f32)val)/pow;
}

static f32 round_up_to_dp(f32 num, int dp)
{
	uint32 pow = 1;
	for (int i = 0; i < dp; i++)
	{
		pow = pow * 10;
	}
	int val = 0; 
	if (num > 0.f) val = (int)(num*pow)+1;
	else if (num < 0.f) val = (int)(num*pow);
	return ((f32)val)/pow;
}

static f32 round_to_dp(f32 num, int dp)
{
	uint32 pow = 1;
	for (int i = 0; i < dp; i++)
	{
		pow = pow * 10;
	}
	int val = 0;
	if (num < 0.0f) val = (int)((num*pow) - .5f);
	else val = (int)((num*pow) + .5f);
	return ((f32)val)/pow;
}

inline void AppendCString(char *StartAt, char *Text)
{
	while(*Text)
	{
		*StartAt++ = *Text++;
	}
    
    while(*StartAt)
    {
        *StartAt++ = 0;
    }
}

inline bool32
IsEndOfLine(char c)
{
	bool32 result = ((c == '\n') ||
                     (c == '\r'));
    
	return result;
}

inline bool32
IsWhitespace(char c)
{
	bool32 result = ((c == ' ') ||
                     (c == '\t') ||
                     (c == '\v') ||
                     (c == '\f') ||
                     IsEndOfLine(c));
    
	return result;
}

#define USEFUL_SUMMERJAM_STUFF
#endif //USEFUL_SUMMERJAM_STUFF