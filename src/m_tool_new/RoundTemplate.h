
// Not actually header file...
// Maybe it must be Source file?

template <class T>
int Round (const T &t)
{
	if (t >= 0.0)
	    return (int) (t + 0.5);
	else
		return (int) (t - 0.5);
};