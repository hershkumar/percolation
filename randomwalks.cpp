#include <chrono>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include <openssl/md4.h>
#include <openssl/md5.h>

#include <fstream>
#include <math.h>
#include <time.h>

template <class X>
class rbg_md4 {
	int seed;

	MD4_CTX ctx;
public:
	rbg_md4(int s) : seed(s) {
	}

	bool get(X &x, float p) {
		MD4_Init(&ctx);
		MD4_Update(&ctx, &seed, sizeof(seed));
		MD4_Update(&ctx, &x, sizeof(x));
		unsigned char hash[32];
		MD4_Final(hash, &ctx);
		unsigned int v = hash[0] + (hash[1]<<8) + (hash[2]<<16) + (hash[3]<<24);
		return v < p*std::numeric_limits<unsigned int>().max();
	}
};

template <class X>
class rbg_md5 {
	int seed;

	MD5_CTX ctx;
public:
	rbg_md5(int s) : seed(s) {
	}

	bool get(X &x, float p) {
		MD5_Init(&ctx);
		MD5_Update(&ctx, &seed, sizeof(seed));
		MD5_Update(&ctx, &x, sizeof(x));
		unsigned char hash[32];
		MD5_Final(hash, &ctx);
		unsigned int v = hash[0] + (hash[1]<<8) + (hash[2]<<16) + (hash[3]<<24);
		return v < p*std::numeric_limits<unsigned int>().max();
	}
};

template <class X>
class rbg_mem {
	std::default_random_engine gen;
	std::map<X,bool> mem;
public:
	rbg_mem() {}

	rbg_mem(int s) {
		gen.seed(s);
	}

	bool get(X &x, float p) {
		if (mem.count(x) > 0)
			return mem[x];
		bool r = std::discrete_distribution<bool>({1-p,p})(gen);
		mem[x] = r;
		return r;
	}
};

typedef std::tuple<int,int> point;

std::vector<point> neighbors(point p) {
	auto [x,y] = p;
	return {
		{x-1, y},
		{x+1, y},
		{x, y-1},
		{x, y+1},
	};
}

class cluster {
public:
	long unsigned int size;
};
// https://gist.github.com/mortenpi/f20a93c8ed3ee7785e65
std::vector<float> LinearSpacedArray(float a, float b, std::size_t N)
{
	float h = (b - a) / static_cast<float>(N-1);
	std::vector<float> xs(N);
	std::vector<float>::iterator x;
	float val;
	for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h) {
		*x = val;
	}
	return xs;
}
double variance(std::vector<double> samples){
	int size = samples.size();

	double variance = 0;
	double t = samples[0];
	for (int i = 1; i < size; i++){
		t += samples[i];
		double diff = ((i + 1) * samples[i]) - t;
		variance += (diff * diff) / ((i + 1.0) *i);
	}

	return variance / (size - 1);
}

double stdev(std::vector<double> samples){
	return sqrt(variance(samples));
}



double get_error(std::vector<double> values, int num_resamples) {
	srand (time(NULL));
	int num_samples = values.size();
	std::vector<double> sampled_means;
	for (int i = 0; i < num_resamples; i++){
		std::vector<double> resampled;
		for (int j = 0; j < num_samples; j++){
			resampled.push_back(values[rand() % num_samples]);
		}
		double resampled_sum = 0;
		for (auto& n : resampled)
			resampled_sum += n;
		sampled_means.push_back(resampled_sum/resampled.size());
	}
	//get the stdev of the sampled means
	return stdev(sampled_means);
}
//TODO: make this work with electric fields
int main(int argc, char *argv[]) {
	// first argument is the probability
	float prob = std::stof(argv[1]);
	// number of steps each random walk takes
	int steps = std::stof(argv[2]);
	// number of repetitions
	int reps = std::stof(argv[3]);
	// seed is based on the current time
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	
	//std::vector<float> x = LinearSpacedArray(prob, prob2, datapoints);
	std::vector<double> y;
	std::vector<double> error;

	for (int i = 0; i < reps; i++){
		seed++;
		rbg_md4<point> md4(seed);
		float current_prob = prob;
		// take the origin as a starting point
		point current_point = {0,0};
		for (int step = 0; step < steps; step++){
			// look at neighbors
			std::vector<point> neb = neighbors(current_point);
			std::vector<int> temp;
			for (int j = 0; j < 4; j++){
				if (md4.get(neb[j], current_prob)){
					temp.push_back(j);
				}
				// case where {0,0} is actually off
				else if (std::get<0>(current_point) == 0 and std::get<1>(current_point) == 0){
					temp.push_back(j);
				}
			}
			// randomly pick one of the neighbors that is on (seed rng with time)
			if (temp.size() == 0){
				continue;
			}
			srand (time(NULL));

			int randomIndex = rand() % temp.size();
			
			// move to the new point
			
			current_point = neb[randomIndex];
		}
		// compute distance from origin
		double dist = abs(pow(std::get<0>(current_point), 2)) + abs(pow(std::get<1>(current_point), 2));
		dist = sqrt(abs(dist));
		y.push_back(dist);
	}
	double average = 0;
	for (int i = 0; i < y.size(); i++){
		average += y[i];
	}
	average /= y.size();
	double err = get_error(y, 100);
	std::cout << average  << " " << err << std::endl;
//	std::ofstream file;
//	file.open("data.txt");
//	std::cout << "Writing data to file" << std::endl;
//	for (int i = 0; i < x.size(); i++){
//		file << x[i] << " " << y[i] << " " << error[i] <<"\n";
//	}

}