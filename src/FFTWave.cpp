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

#include "FFTWave.hpp"

#include <sstream>
#include <fstream>
#include <cstdlib> //For rand()
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif // M_PI

/*
cTimer::cTimer() {
	clock_gettime(CLOCK_REALTIME, &process_start);
	frame_start = process_start;
}

cTimer::~cTimer() {

}

double cTimer::elapsed(bool frame) {
	clock_gettime(CLOCK_REALTIME, &current);
	double elapsed = frame ? (current.tv_sec + current.tv_nsec / 1000000000.0 -   frame_start.tv_sec -   frame_start.tv_nsec / 1000000000.0) :
				 (current.tv_sec + current.tv_nsec / 1000000000.0 - process_start.tv_sec - process_start.tv_nsec / 1000000000.0);
	frame_start = current;
	return elapsed;
}
*/
unsigned int complex::additions = 0;
unsigned int complex::multiplications = 0;

complex::complex() : a(0.0f), b(0.0f) { }
complex::complex(float a, float b) : a(a), b(b) { }
complex complex::conj() { return complex(this->a, -this->b); }

complex complex::operator*(const complex& c) const {
	complex::multiplications++;
	return complex(this->a*c.a - this->b*c.b, this->a*c.b + this->b*c.a);
}

complex complex::operator+(const complex& c) const {
	complex::additions++;
	return complex(this->a + c.a, this->b + c.b);
}

complex complex::operator-(const complex& c) const {
	complex::additions++;
	return complex(this->a - c.a, this->b - c.b);
}

complex complex::operator-() const {
	return complex(-this->a, -this->b);
}

complex complex::operator*(const float c) const {
	return complex(this->a*c, this->b*c);
}

complex& complex::operator=(const complex& c) {
	this->a = c.a; this->b = c.b;
	return *this;
}

void complex::reset() {
	complex::additions = 0;
	complex::multiplications = 0;
}

vector3::vector3() : x(0.0f), y(0.0f), z(0.0f) { }
vector3::vector3(float x, float y, float z) : x(x), y(y), z(z) { }

float vector3::operator*(const vector3& v) {
	return this->x*v.x + this->y*v.y + this->z*v.z;
}

vector3 vector3::cross(const vector3& v) {
	return vector3(this->y*v.z - this->z*v.y, this->z*v.x - this->x*v.z, this->x*v.y - this->y*v.z);
}

vector3 vector3::operator+(const vector3& v) {
	return vector3(this->x + v.x, this->y + v.y, this->z + v.z);
}

vector3 vector3::operator-(const vector3& v) {
	return vector3(this->x - v.x, this->y - v.y, this->z - v.z);
}

vector3 vector3::operator*(const float s) {
	return vector3(this->x*s, this->y*s, this->z*s);
}

vector3& vector3::operator=(const vector3& v) {
	this->x = v.x; this->y = v.y; this->z = v.z;
	return *this;
}

float vector3::length() {
	return sqrt(this->x*this->x + this->y*this->y + this->z*this->z);
}

vector3 vector3::unit() {
	float l = this->length();
	return vector3(this->x/l, this->y/l, this->z/l);
}



vector2::vector2() : x(0.0f), y(0.0f) { }
vector2::vector2(float x, float y) : x(x), y(y) { }

float vector2::operator*(const vector2& v) {
	return this->x*v.x + this->y*v.y;
}

vector2 vector2::operator+(const vector2& v) {
	return vector2(this->x + v.x, this->y + v.y);
}

vector2 vector2::operator-(const vector2& v) {
	return vector2(this->x - v.x, this->y - v.y);
}

vector2 vector2::operator*(const float s) {
	return vector2(this->x*s, this->y*s);
}

vector2& vector2::operator=(const vector2& v) {
	this->x = v.x; this->y = v.y;
	return *this;
}

float vector2::length() {
	return sqrt(this->x*this->x + this->y*this->y);
}

vector2 vector2::unit() {
	float l = this->length();
	return vector2(this->x/l, this->y/l);
}

cFFT::cFFT(unsigned int N) : N(N), reversed(0), T(0), pi2(2 * M_PI) {
	c[0] = c[1] = 0;

	log_2_N = log(N)/log(2);

	reversed = new unsigned int[N];		// prep bit reversals
	for (unsigned int i = 0; i < N; i++) reversed[i] = reverse(i);

	int pow2 = 1;
	T = new complex*[log_2_N];		// prep T
	for (unsigned int i = 0; i < log_2_N; i++) {
		T[i] = new complex[pow2];
		for (int j = 0; j < pow2; j++) T[i][j] = t(j, pow2 * 2);
		pow2 *= 2;
	}

	c[0] = new complex[N];
	c[1] = new complex[N];
	which = 0;
}

cFFT::~cFFT() {
	if (c[0]) delete [] c[0];
	if (c[1]) delete [] c[1];
	if (T) {
		for (unsigned int i = 0; i < log_2_N; i++) if (T[i]) delete [] T[i];
		delete [] T;
	}
	if (reversed) delete [] reversed;
}

unsigned int cFFT::reverse(unsigned int i) {
	unsigned int res = 0;
	for (unsigned int j = 0; j < log_2_N; j++) {
		res = (res << 1) + (i & 1);
		i >>= 1;
	}
	return res;
}

complex cFFT::t(unsigned int x, unsigned int N) {
	return complex(cos(pi2 * x / N), sin(pi2 * x / N));
}

void cFFT::fft(complex* input, complex* output, int stride, int offset) {
	for (unsigned int i = 0; i < N; i++) c[which][i] = input[reversed[i] * stride + offset];

	int loops       = N>>1;
	int size        = 1<<1;
	int size_over_2 = 1;
	int w_          = 0;
	for (unsigned int i = 1; i <= log_2_N; i++) {
		which ^= 1;
		for (int j = 0; j < loops; j++) {
			for (int k = 0; k < size_over_2; k++) {
				c[which][size * j + k] =  c[which^1][size * j + k] +
							  c[which^1][size * j + size_over_2 + k] * T[w_][k];
			}

			for (int k = size_over_2; k < size; k++) {
				c[which][size * j + k] =  c[which^1][size * j - size_over_2 + k] -
							  c[which^1][size * j + k] * T[w_][k - size_over_2];
			}
		}
		loops       >>= 1;
		size        <<= 1;
		size_over_2 <<= 1;
		w_++;
	}

	for (unsigned int i = 0; i < N; i++) output[i * stride + offset] = c[which][i];
}

//MAIN WAVE CODE:

float cOcean::uniformRandomVariable() {
	return (float)rand()/RAND_MAX;
}

complex cOcean::gaussianRandomVariable() {
	float x1, x2, w;
	do {
	    x1 = 2.f * uniformRandomVariable() - 1.f;
	    x2 = 2.f * uniformRandomVariable() - 1.f;
	    w = x1 * x1 + x2 * x2;
	} while ( w >= 1.f );
	w = sqrt((-2.f * log(w)) / w);
	return complex(x1 * w, x2 * w);
}

cOcean::cOcean(const int N, const float A, const vector2 w, const float length) :
	g(9.81), N(N), Nplus1(N+1), A(A), w(w), length(length),
	vertices(0), h_tilde(0), h_tilde_slopex(0), h_tilde_slopez(0), h_tilde_dx(0), h_tilde_dz(0), fft(0)
{
	h_tilde        = new complex[N*N];
	h_tilde_slopex = new complex[N*N];
	h_tilde_slopez = new complex[N*N];
	h_tilde_dx     = new complex[N*N];
	h_tilde_dz     = new complex[N*N];
	fft            = new cFFT(N);
	vertices       = new vertex_ocean[Nplus1*Nplus1];

	int index;

	//seed random number generator with srand, so we get repeatable random waves
    srand(10);

	//NOTE: Code from here duplicated in hTilde()
	complex htilde0, htilde0mk_conj;
	for (int m_prime = 0; m_prime < Nplus1; m_prime++) {
		for (int n_prime = 0; n_prime < Nplus1; n_prime++) {
			index = m_prime * Nplus1 + n_prime;

			htilde0        = hTilde_0( n_prime,  m_prime);
			htilde0mk_conj = hTilde_0(-n_prime, -m_prime).conj();

			vertices[index].a  = htilde0.a;
			vertices[index].b  = htilde0.b;
			vertices[index]._a = htilde0mk_conj.a;
			vertices[index]._b = htilde0mk_conj.b;

			vertices[index].ox = vertices[index].x =  (n_prime - N / 2.0f) * length / N;
			vertices[index].oy = vertices[index].y =  0.0f;
			vertices[index].oz = vertices[index].z =  (m_prime - N / 2.0f) * length / N;

			vertices[index].nx = 0.0f;
			vertices[index].ny = 1.0f;
			vertices[index].nz = 0.0f;
		}
	}

	reInitialiseWaves = false;
}

cOcean::~cOcean() {
	if (h_tilde)		delete [] h_tilde;
	if (h_tilde_slopex)	delete [] h_tilde_slopex;
	if (h_tilde_slopez)	delete [] h_tilde_slopez;
	if (h_tilde_dx)		delete [] h_tilde_dx;
	if (h_tilde_dz)		delete [] h_tilde_dz;
	if (fft)		delete fft;
	if (vertices)		delete [] vertices;
}

float cOcean::dispersion(int n_prime, int m_prime) {
	float w_0 = 2.0f * M_PI / 200.0f;
	float kx = M_PI * (2 * n_prime - N) / length;
	float kz = M_PI * (2 * m_prime - N) / length;
	return floor(sqrt(g * sqrt(kx * kx + kz * kz)) / w_0) * w_0;
}

float cOcean::phillips(int n_prime, int m_prime) {
	vector2 k(M_PI * (2 * n_prime - N) / length,
		  M_PI * (2 * m_prime - N) / length);
	
	//std::cout << k.x << " " << k.y << std::endl;
	
	float k_length  = k.length();
	if (k_length < 0.000001) return 0.0;

	float k_length2 = k_length  * k_length;
	float k_length4 = k_length2 * k_length2;

	float k_dot_w   = k.unit() * w.unit();
	float k_dot_w2  = k_dot_w * k_dot_w * k_dot_w * k_dot_w * k_dot_w * k_dot_w;

	float w_length  = w.length();
	float L         = w_length * w_length / g;
	float L2        = L * L;

	float damping   = 0.001;
	float l2        = L2 * damping * damping;

	float returnVal =  A * exp(-1.0f / (k_length2 * L2)) / k_length4 * k_dot_w2 * exp(-k_length2 * l2);
	
	//std::cout << A << " " << k_length2 << " " << L2 << " " << k_length4 << " " << k_dot_w2 << " " << l2 << " " << returnVal<< std::endl;

	return returnVal;
}

complex cOcean::hTilde_0(int n_prime, int m_prime) {
	complex r = gaussianRandomVariable();
	
	float phillipsVal = phillips(n_prime, m_prime);
	
	r = r * sqrt(phillipsVal / 2.0f);
	//std::cout << r.a << " " << r.b << " " << phillipsVal << " " << sqrt(phillipsVal) <<  std::endl;
	return r;
}

complex cOcean::hTilde(float t, int n_prime, int m_prime) {
	int index = m_prime * Nplus1 + n_prime;

	if (reInitialiseWaves) { //NOTE: Code duplication from constructor here

        complex htilde0, htilde0mk_conj;
        htilde0        = hTilde_0( n_prime,  m_prime);
        htilde0mk_conj = hTilde_0(-n_prime, -m_prime).conj();
        
        //std::cout << htilde0.a << " " << htilde0.b << std::endl;

        vertices[index].a  = htilde0.a;
        vertices[index].b  = htilde0.b;
        vertices[index]._a = htilde0mk_conj.a;
        vertices[index]._b = htilde0mk_conj.b;

        vertices[index].ox = vertices[index].x =  (n_prime - N / 2.0f) * length / N;
        vertices[index].oy = vertices[index].y =  0.0f;
        vertices[index].oz = vertices[index].z =  (m_prime - N / 2.0f) * length / N;

        vertices[index].nx = 0.0f;
        vertices[index].ny = 1.0f;
        vertices[index].nz = 0.0f;
	}

	complex htilde0(vertices[index].a,  vertices[index].b);
	complex htilde0mkconj(vertices[index]._a, vertices[index]._b);

	float omegat = dispersion(n_prime, m_prime) * t;

	float cos_ = cos(omegat);
	float sin_ = sin(omegat);

	complex c0(cos_,  sin_);
	complex c1(cos_, -sin_);

	//complex res = htilde0 * c0 + htilde0mkconj * c1;

	return htilde0 * c0 + htilde0mkconj*c1;
}
/*
complex_vector_normal cOcean::h_D_and_n(vector2 x, float t) {
	complex h(0.0f, 0.0f);
	vector2 D(0.0f, 0.0f);
	vector3 n(0.0f, 0.0f, 0.0f);

	complex c, res, htilde_c;
	vector2 k;
	float kx, kz, k_length, k_dot_x;

	for (int m_prime = 0; m_prime < N; m_prime++) {
		kz = 2.0f * M_PI * (m_prime - N / 2.0f) / length;
		for (int n_prime = 0; n_prime < N; n_prime++) {
			kx = 2.0f * M_PI * (n_prime - N / 2.0f) / length;
			k = vector2(kx, kz);

			k_length = k.length();
			k_dot_x = k * x;

			c = complex(cos(k_dot_x), sin(k_dot_x));
			htilde_c = hTilde(t, n_prime, m_prime) * c;

			h = h + htilde_c;

			n = n + vector3(-kx * htilde_c.b, 0.0f, -kz * htilde_c.b);

			if (k_length < 0.000001) continue;
			D = D + vector2(kx / k_length * htilde_c.b, kz / k_length * htilde_c.b);
		}
	}

	n = (vector3(0.0f, 1.0f, 0.0f) - n).unit();

	complex_vector_normal cvn;
	cvn.h = h;
	cvn.D = D;
	cvn.n = n;
	return cvn;
}
*/

void cOcean::resetParameters(float A, vector2 w)
{
    //Only set if different
    if (this->A == A && this->w.x == w.x && this->w.y == w.y)
        return;

    this->A = A;
    this->w = w;
    reInitialiseWaves = true;
    //seed random number generator with srand, so we get repeatable random waves
    srand(10);
}

//From OpenCV via http://stackoverflow.com/a/20723890
int cOcean::localisinf(double x) const
{
    union { uint64_t u; double f; } ieee754;
    ieee754.f = x;
    return ( (unsigned)(ieee754.u >> 32) & 0x7fffffff ) == 0x7ff00000 &&
           ( (unsigned)ieee754.u == 0 );
}

int cOcean::localisnan(double x) const
{
    union { uint64_t u; double f; } ieee754;
    ieee754.f = x;
    return ( (unsigned)(ieee754.u >> 32) & 0x7fffffff ) +
           ( (unsigned)ieee754.u != 0 ) > 0x7ff00000;
}
//End From OpenCV via http://stackoverflow.com/a/20723890

void cOcean::evaluateWavesFFT(float t) {

	float kx, kz, len, lambda = -1.0f;
	int index, index1;

	for (int m_prime = 0; m_prime < N; m_prime++) {
		kz = M_PI * (2.0f * m_prime - N) / length;
		for (int n_prime = 0; n_prime < N; n_prime++) {
			kx = M_PI*(2 * n_prime - N) / length;
			len = sqrt(kx * kx + kz * kz);
			index = m_prime * N + n_prime;

			h_tilde[index] = hTilde(t, n_prime, m_prime);
			h_tilde_slopex[index] = h_tilde[index] * complex(0, kx);
			h_tilde_slopez[index] = h_tilde[index] * complex(0, kz);
			if (len < 0.000001f) {
				h_tilde_dx[index]     = complex(0.0f, 0.0f);
				h_tilde_dz[index]     = complex(0.0f, 0.0f);
			} else {
				h_tilde_dx[index]     = h_tilde[index] * complex(0, -kx/len);
				h_tilde_dz[index]     = h_tilde[index] * complex(0, -kz/len);
			}
		}
	}

	reInitialiseWaves = false; //If we had to re-initialise, this is done in hTilde, so should now be complete for all vertexes

	for (int m_prime = 0; m_prime < N; m_prime++) {
		fft->fft(h_tilde, h_tilde, 1, m_prime * N);
		fft->fft(h_tilde_slopex, h_tilde_slopex, 1, m_prime * N);
		fft->fft(h_tilde_slopez, h_tilde_slopez, 1, m_prime * N);
		fft->fft(h_tilde_dx, h_tilde_dx, 1, m_prime * N);
		fft->fft(h_tilde_dz, h_tilde_dz, 1, m_prime * N);
	}
	for (int n_prime = 0; n_prime < N; n_prime++) {
		fft->fft(h_tilde, h_tilde, N, n_prime);
		fft->fft(h_tilde_slopex, h_tilde_slopex, N, n_prime);
		fft->fft(h_tilde_slopez, h_tilde_slopez, N, n_prime);
		fft->fft(h_tilde_dx, h_tilde_dx, N, n_prime);
		fft->fft(h_tilde_dz, h_tilde_dz, N, n_prime);
	}

	int sign;
	float signs[] = { 1.0f, -1.0f };
	vector3 n;
	for (int m_prime = 0; m_prime < N; m_prime++) {
		for (int n_prime = 0; n_prime < N; n_prime++) {
			index  = m_prime * N + n_prime;		// index into h_tilde..
			index1 = m_prime * Nplus1 + n_prime;	// index into vertices

			sign = signs[(n_prime + m_prime) & 1];

			h_tilde[index]     = h_tilde[index] * sign;

			// height
			vertices[index1].y = h_tilde[index].a;

			// displacement
			h_tilde_dx[index] = h_tilde_dx[index] * sign;
			h_tilde_dz[index] = h_tilde_dz[index] * sign;
			vertices[index1].x = vertices[index1].ox + h_tilde_dx[index].a * lambda;
			vertices[index1].z = vertices[index1].oz + h_tilde_dz[index].a * lambda;
			
			//Checking - Bug workaround for NaNs on OSX
			if (localisinf(vertices[index1].y) || localisnan(vertices[index1].y)) {
				vertices[index1].y = 0;
			}
			if (localisinf(vertices[index1].x) || localisinf(vertices[index1].z) || localisnan(vertices[index1].x) || localisnan(vertices[index1].z)) {
				vertices[index1].x = vertices[index1].ox ;
				vertices[index1].z = vertices[index1].oz ;
			}

			// normal
			h_tilde_slopex[index] = h_tilde_slopex[index] * sign;
			h_tilde_slopez[index] = h_tilde_slopez[index] * sign;
			n = vector3(0.0f - h_tilde_slopex[index].a, 1.0f, 0.0f - h_tilde_slopez[index].a).unit();
			vertices[index1].nx =  n.x;
			vertices[index1].ny =  n.y;
			vertices[index1].nz =  n.z;

			// for tiling
			if (n_prime == 0 && m_prime == 0) {
				vertices[index1 + N + Nplus1 * N].y = h_tilde[index].a;

				vertices[index1 + N + Nplus1 * N].x = vertices[index1 + N + Nplus1 * N].ox + h_tilde_dx[index].a * lambda;
				vertices[index1 + N + Nplus1 * N].z = vertices[index1 + N + Nplus1 * N].oz + h_tilde_dz[index].a * lambda;

				vertices[index1 + N + Nplus1 * N].nx =  n.x;
				vertices[index1 + N + Nplus1 * N].ny =  n.y;
				vertices[index1 + N + Nplus1 * N].nz =  n.z;

				//Checking - Bug workaround for NaNs on OSX
				if (localisinf(vertices[index1 + N + Nplus1 * N].y) || localisnan(vertices[index1 + N + Nplus1 * N].y)) {
					vertices[index1 + N + Nplus1 * N].y = 0;
				}
				if (localisinf(vertices[index1 + N + Nplus1 * N].x) || localisinf(vertices[index1 + N + Nplus1 * N].z) || localisnan(vertices[index1 + N + Nplus1 * N].x) || localisnan(vertices[index1 + N + Nplus1 * N].z)) {
					vertices[index1 + N + Nplus1 * N].x = vertices[index1 + N + Nplus1 * N].ox ;
					vertices[index1 + N + Nplus1 * N].z = vertices[index1 + N + Nplus1 * N].oz ;
				}

				//std::cout << "At time " << t << " corner n'=m'=0, height = " << vertices[index1].y << std::endl;

			}
			if (n_prime == 0) {
				vertices[index1 + N].y = h_tilde[index].a;

				vertices[index1 + N].x = vertices[index1 + N].ox + h_tilde_dx[index].a * lambda;
				vertices[index1 + N].z = vertices[index1 + N].oz + h_tilde_dz[index].a * lambda;

				vertices[index1 + N].nx =  n.x;
				vertices[index1 + N].ny =  n.y;
				vertices[index1 + N].nz =  n.z;
				
				//Checking - Bug workaround for NaNs on OSX
				if (localisinf(vertices[index1 + N].y) || localisnan(vertices[index1 + N].y)) {
					vertices[index1 + N].y = 0;
				}
				if (localisinf(vertices[index1 + N].x) || localisinf(vertices[index1 + N].z) || localisnan(vertices[index1 + N].x) || localisnan(vertices[index1 + N].z)) {
					vertices[index1 + N].x = vertices[index1 + N].ox ;
					vertices[index1 + N].z = vertices[index1 + N].oz ;
				}
				
			}
			if (m_prime == 0) {
				vertices[index1 + Nplus1 * N].y = h_tilde[index].a;

				vertices[index1 + Nplus1 * N].x = vertices[index1 + Nplus1 * N].ox + h_tilde_dx[index].a * lambda;
				vertices[index1 + Nplus1 * N].z = vertices[index1 + Nplus1 * N].oz + h_tilde_dz[index].a * lambda;

				vertices[index1 + Nplus1 * N].nx =  n.x;
				vertices[index1 + Nplus1 * N].ny =  n.y;
				vertices[index1 + Nplus1 * N].nz =  n.z;
				
				//Checking - Bug workaround for NaNs on OSX
				if (localisinf(vertices[index1 + Nplus1 * N].y) || localisnan(vertices[index1 + Nplus1 * N].y)) {
					vertices[index1 + Nplus1 * N].y = 0;
				}
				if (localisinf(vertices[index1 + Nplus1 * N].x) || localisinf(vertices[index1 + Nplus1 * N].z) || localisnan(vertices[index1 + Nplus1 * N].x) || localisnan(vertices[index1 + Nplus1 * N].z)) {
					vertices[index1 + Nplus1 * N].x = vertices[index1 + Nplus1 * N].ox ;
					vertices[index1 + Nplus1 * N].z = vertices[index1 + Nplus1 * N].oz ;
				}

				
			}
		}
	}
}

vertex_ocean* cOcean::getVertices()
{
    return vertices;
}
