/*
The MIT License (MIT)

Copyright (c) 2015 Keith Lantz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __FFTWAVE_HPP_INCLUDED__
/*
#include <time.h>
class cTimer {
  private:
	timespec process_start, frame_start, current;
  protected:
  public:
	cTimer();
	~cTimer();
	double elapsed(bool frame);
};
*/
class complex {
  private:
  protected:
  public:
    float a, b;
    static unsigned int additions, multiplications;
    complex();
    complex(float a, float b);
    complex conj();
    complex operator*(const complex& c) const;
    complex operator+(const complex& c) const;
    complex operator-(const complex& c) const;
    complex operator-() const;
    complex operator*(const float c) const;
    complex& operator=(const complex& c);
    static void reset();
};

#include <math.h>

class vector3 {
  private:
  protected:
  public:
    float x, y, z;
    vector3();
    vector3(float x, float y, float z);
    float operator*(const vector3& v);
    vector3 cross(const vector3& v);
    vector3 operator+(const vector3& v);
    vector3 operator-(const vector3& v);
    vector3 operator*(const float s);
    vector3& operator=(const vector3& v);
    float length();
    vector3 unit();
};

class vector2 {
  private:
  protected:
  public:
    float x, y;
    vector2();
    vector2(float x, float y);
    float operator*(const vector2& v);
    vector2 operator+(const vector2& v);
    vector2 operator-(const vector2& v);
    vector2 operator*(const float s);
    vector2& operator=(const vector2& v);
    float length();
    vector2 unit();
};

class cFFT {
  private:
	unsigned int N, which;
	unsigned int *reversed;
	complex **T;
	float pi2;
	unsigned int log_2_N;
	complex *c[2];
  protected:
  public:
	cFFT(unsigned int N);
	~cFFT();
	unsigned int reverse(unsigned int i);
	complex t(unsigned int x, unsigned int N);
	void fft(complex* input, complex* output, int stride, int offset);
};

struct vertex_ocean {
	float   x,   y,   z; // vertex
	float  nx,  ny,  nz; // normal
	float   a,   b,   c; // htilde0
	float  _a,  _b,  _c; // htilde0mk conjugate
	float  ox,  oy,  oz; // original position
};

struct complex_vector_normal {	// structure used with discrete fourier transform
	complex h;		// wave height
	vector2 D;		// displacement
	vector3 n;		// normal
};

class cOcean {
  private:

	float g;				// gravity constant
	int N, Nplus1;				// dimension -- N should be a power of 2
	float A;				// phillips spectrum parameter -- affects heights of waves
	vector2 w;				// wind parameter
	float length;				// length parameter
	vertex_ocean *vertices;			// vertices for vertex buffer object
	bool reInitialiseWaves; // If waves should be re-created (as new A or w?)

	complex *h_tilde,			// for fast fourier transform
		*h_tilde_slopex, *h_tilde_slopez,
		*h_tilde_dx, *h_tilde_dz;
	cFFT *fft;				// fast fourier transform

	//unsigned int *indices;			// indicies for vertex buffer object
	//unsigned int indices_count;		// number of indices to render

	//Utility methods
	float uniformRandomVariable();
	complex gaussianRandomVariable();

	//Main internal methods
	float dispersion(int n_prime, int m_prime);		// deep water
	float phillips(int n_prime, int m_prime);		// phillips spectrum
	complex hTilde_0(int n_prime, int m_prime);
	complex hTilde(float t, int n_prime, int m_prime);
	//complex_vector_normal h_D_and_n(vector2 x, float t);
	
	int localisinf(double x) const;
	int localisnan(double x) const;

  protected:
  public:
	cOcean(const int N, const float A, const vector2 w, const float length);
	~cOcean();

	void resetParameters(float A, vector2 w);
	void evaluateWavesFFT(float t);
	vertex_ocean* getVertices();
};

#endif
