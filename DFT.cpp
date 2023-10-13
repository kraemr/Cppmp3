#include <math.h>
#include <vector>

// Implementation of Discrete Fourier Transform
void TraceDFT(double* realInput, std::vector<double>& spectrum, unsigned int N){
	double realOutput[N] = { 0 };
	double imagOutput[N] = { 0 };
	for (int k = 0; k < N/2;++k){
		for (int n =0; n<N;++n){
			double x = 2 * 3.14159265 * k * n / N;
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
	for (int j=0+offset;j<points+offset;j+=2){
		short sample = ( (buffer[j] << 8)  +  buffer[j+1]); // 16bit sample
		realInput[i] = double(sample) / t;
		i++;
	}
	TraceDFT(realInput,spectrum,points);
	return spectrum;
}
