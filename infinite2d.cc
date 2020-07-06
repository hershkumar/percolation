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

int main(int argc, char *argv[]) {
	// first argument is the probability
	float prob = std::stof(argv[1]);
	
	// second argument is the second probability (these two give the bounds of the graph)
	float prob2 = std::stof(argv[2]);
	
	// third argument is the number of repetitions per data point
	int reps = std::stoi(argv[3]);
	// number of data points is given by the last arg
	int num_data_points = std::stoi(argv[4]);
	// seed is based on the current time
	int seed = std::chrono::system_clock::now().time_since_epoch().count();

	std::vector<float> x = LinearSpacedArray(prob, prob2, num_data_points);
	std::vector<double> y;
	std::vector<double> error;

	
	//std::vector<int> all_cluster_sizes;
	for (int i = 0; i < x.size(); i++){
		std::vector<cluster> clusters;
		float current_prob = x[i];
		for (int r = 0; r < reps; r++) {
			seed++;
			rbg_md4<point> rand(seed);
			std::queue<point> q;
			std::set<point> c;
			q.push({0,0});
			c.insert(q.front());
			while (!q.empty()) {
				point p0 = q.front();
				q.pop();
				// List all neighbors.
				for (point p : neighbors(p0)) {
					// If this site is in the cluster, just ignore it.
					if (c.count(p) > 0) continue;
					// Otherwise, decide if it's on. If so, add to cluster.
					if (rand.get(p, current_prob)) {
						c.insert(p);
						q.push(p);
					}
				}

				if (c.size() > 1 << 27) { // 1 << 27 is 1 with 27 0's after it in binary (big number)
					// Consider this infinite.
					std::cerr << "Infinite cluster found: p = " << current_prob << std::endl;
					return 1;
				}
			}

			clusters.push_back({c.size()});
			//all_cluster_sizes.push_back(c.size());
		}

		double msize = 0.0;
		for (cluster c : clusters) {
			msize += (float)c.size / (float)clusters.size();
		}
		std::vector<double> temp;
		for (cluster c : clusters){
			temp.push_back(c.size);
		}

		y.push_back(msize);
		error.push_back(get_error(temp, 100));
		std::cout << current_prob << " " << msize << std::endl;
	}
	std::ofstream file;
	file.open("data.txt");
	std::cout << "Writing data to file" << std::endl;
	for (int i = 0; i < x.size(); i++){
		file << x[i] << " " << y[i] << " " << error[i] <<"\n";
	}
	//return 0;
}
