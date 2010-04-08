#pragma once


/// struct to store color information
struct Color {
	float r, ///< red component
		  g, ///< green
		  b, ///< blue
		  a; ///< alpha value
	/// create nec Color object with given channels
	Color(float r_ = 0.0, float g_ = 0.0, float b_ = 0.0, float a_ = 1.0): r(r_), g(g_), b(b_), a(a_) {}
	/// overload float cast
	operator float*() { return reinterpret_cast<float*>(this); }
	/// overload float const cast
	operator float const*() const { return reinterpret_cast<float const*>(this); }
};
