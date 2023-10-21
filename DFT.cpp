#include <complex>
#include <iostream>
#include <math.h>
#include <vector>

// Implementation of Discrete Fourier Transform
void TraceDFT(double* realInput, std::vector<double>& spectrum, unsigned int N){
	double realOutput[N] = { 0 };
	double imagOutput[N] = { 0 };
	double precomp=2 * 3.14159265;
	for (int k = 0; k < N/2;++k){
		for (int n =0; n<N;++n){
			double x = precomp * k * n / N;
			double re = realInput[n] * cos(x);
			double im = realInput[n] * -sin(x);
			realOutput[k] += re;
			imagOutput[k] += im;
		}
	}
	for (int i=0; i < N/2; ++i){
		double d = sqrt(realOutput[i] * realOutput[i] + imagOutput[i] * imagOutput[i]);
		d = d < 1 ? 0: 20 * log(d);
		spectrum[i] = d < 200 ? d : 200;
	}
}


// this traces the spectrum of a given block of samples n
std::vector<double> traceSpectrum(unsigned char* buffer,unsigned int n, unsigned int points,unsigned int offset){
	const double t=1000.0;
	double realInput[points] = { 0 };
	std::vector<double> spectrum(points / 2);
	unsigned int i = 0;
	std::cout << "trace: " << ( (buffer[0] << 8) + buffer[1]) << std::endl;
	for (int j=0;j<points;j+=2){
		short sample = ( (buffer[j] << 8)  +  buffer[j+1]); // 16bit sample
		realInput[i] = double(sample) / t;
		i++;
	}
	TraceDFT(realInput,spectrum,points);
	return spectrum;
}

void bitReverse(std::vector<double>& real, std::vector<double>& imag) {
    int n = real.size();
    int bits = log2(n);
    std::vector<double> temp_real(n);
    std::vector<double> temp_imag(n);

    for (int i = 0; i < n; ++i) {
        int j = 0;
        for (int bit = 0; bit < bits; ++bit) {
            j |= ((i >> bit) & 1) << (bits - 1 - bit);
        }
        temp_real[i] = real[j];
        temp_imag[i] = imag[j];
    }

    // Copy the rearranged data back to the original vectors
    real = temp_real;
    imag = temp_imag;
}

// Iterative FFT function
void fft(std::vector<double>& real, std::vector<double>& imag, bool inverse = false) {
    int n = real.size();
    bitReverse(real, imag);

    for (int s = 1; s <= log2(n); ++s) {
        int m = 1 << s;
        double theta = (inverse ? -1.0 : 1.0) * 2.0 * 3.141592653589793238460 / m;
        for (int k = 0; k < n; k += m) {
            double w_real = 1.0;
            double w_imag = 0.0;
            for (int j = 0; j < m / 2; ++j) {
                int u = k + j;
                int v = k + j + m / 2;
                double t_real = w_real * real[v] - w_imag * imag[v];
                double t_imag = w_real * imag[v] + w_imag * real[v];
                real[u] += t_real;
                imag[u] += t_imag;
                real[v] = real[u] - 2.0 * t_real;
                imag[v] = imag[u] - 2.0 * t_imag;

                double cos_t = cos(theta);
                double sin_t = sin(theta);
                double new_w_real = w_real * cos_t - w_imag * sin_t;
                double new_w_imag = w_real * sin_t + w_imag * cos_t;
                w_real = new_w_real;
                w_imag = new_w_imag;
            }
        }
    }
	}