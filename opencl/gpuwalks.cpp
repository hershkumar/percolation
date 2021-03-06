
#include<CL/cl.hpp>
#include<iostream>
#include<fstream>

#include <iomanip>
#include <fstream>

#include <cmath>


/* We're only interested in hashing short (and fixed-length) amounts of data.
 * To simplify implementation and improve performance, all hash functions are
 * defined to take a 256-bit input (structured as 8 32-bit unsigned integers)
 * and return a 32-bit output. The 256 bits of input fit within the block size
 * of most hash functions, while permitting us to (naturally) work with
 * lattices of up to seven dimensions. The output is shorter than the output of
 * any hash function, but suffices to represent a random floating-point number
 * in [0,1) with high precision.
 */

 // https://tools.ietf.org/html/rfc1319
uint32_t MD2(uint32_t input[8]) {
	static uint8_t PI_SUBST[256] = {
		41, 46, 67, 201, 162, 216, 124, 1, 61, 54, 84, 161, 236, 240, 6,
		19, 98, 167, 5, 243, 192, 199, 115, 140, 152, 147, 43, 217, 188,
		76, 130, 202, 30, 155, 87, 60, 253, 212, 224, 22, 103, 66, 111, 24,
		138, 23, 229, 18, 190, 78, 196, 214, 218, 158, 222, 73, 160, 251,
		245, 142, 187, 47, 238, 122, 169, 104, 121, 145, 21, 178, 7, 63,
		148, 194, 16, 137, 11, 34, 95, 33, 128, 127, 93, 154, 90, 144, 50,
		39, 53, 62, 204, 231, 191, 247, 151, 3, 255, 25, 48, 179, 72, 165,
		181, 209, 215, 94, 146, 42, 172, 86, 170, 198, 79, 184, 56, 210,
		150, 164, 125, 182, 118, 252, 107, 226, 156, 116, 4, 241, 69, 157,
		112, 89, 100, 113, 135, 32, 134, 91, 207, 101, 230, 45, 168, 2, 27,
		96, 37, 173, 174, 176, 185, 246, 28, 70, 97, 105, 52, 64, 126, 15,
		85, 71, 163, 35, 221, 81, 175, 58, 195, 92, 249, 206, 186, 197,
		234, 38, 44, 83, 13, 110, 133, 40, 132, 9, 211, 223, 205, 244, 65,
		129, 77, 82, 106, 220, 55, 200, 108, 193, 171, 250, 36, 225, 123,
		8, 12, 189, 177, 74, 120, 136, 149, 139, 227, 99, 232, 109, 233,
		203, 213, 254, 59, 0, 29, 57, 242, 239, 183, 14, 102, 88, 208, 228,
		166, 119, 114, 248, 235, 117, 75, 10, 49, 68, 80, 180, 143, 237,
		31, 26, 219, 153, 141, 51, 159, 17, 131, 20
	};

	// Convert the message to bytes.
	uint8_t m[64];
	for (int i = 0; i < 8; i++) {
		m[4 * i + 3] = (uint8_t)input[i];
		m[4 * i + 2] = (uint8_t)(input[i] >> 8);
		m[4 * i + 1] = (uint8_t)(input[i] >> 16);
		m[4 * i + 0] = (uint8_t)(input[i] >> 24);
	}

	// Pad.
	for (int i = 32; i < 48; i++) m[i] = 0x10;

	// Append checksum.
	uint8_t* c = m + 48;
	uint8_t l = 0;
	for (int i = 0; i < 16; i++) c[i] = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 16; j++) {
			l = c[j] ^= PI_SUBST[m[i * 16 + j] ^ l];
		}
	}

	// Process. Two blocks are needed.
	uint8_t x[48];
	for (int j = 0; j < 48; j++) x[j] = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 16; j++) {
			x[16 + j] = m[i * 16 + j];
			x[32 + j] = x[16 + j] ^ x[j];
		}
		uint8_t t = 0;
		for (int j = 0; j < 18; j++) {
			for (int k = 0; k < 48; k++) {
				t = x[k] ^= PI_SUBST[t];
			}
			t = (t + j) & 0xff;
		}
	}

	return x[3] + (x[2] << 8) + (x[1] << 16) + (x[0] << 24);
}

/* F, G and H are basic MD4 functions. */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))

/* ROTATE_LEFT rotates x left n bits.
 *  */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

 /* FF, GG and HH are transformations for rounds 1, 2 and 3 */
 /* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s) { \
	    (a) += F ((b), (c), (d)) + (x); \
	    (a) = ROTATE_LEFT ((a), (s)); \
	  }
#define GG(a, b, c, d, x, s) { \
	    (a) += G ((b), (c), (d)) + (x) + (uint32_t)0x5a827999; \
	    (a) = ROTATE_LEFT ((a), (s)); \
	  }
#define HH(a, b, c, d, x, s) { \
	    (a) += H ((b), (c), (d)) + (x) + (uint32_t)0x6ed9eba1; \
	    (a) = ROTATE_LEFT ((a), (s)); \
	  }

#define S11 3
#define S12 7
#define S13 11
#define S14 19
#define S21 3
#define S22 5
#define S23 9
#define S24 13
#define S31 3
#define S32 9
#define S33 11
#define S34 15

// https://tools.ietf.org/html/rfc1320
uint32_t MD4(uint32_t input[8]) {
	// Copy into buffer, and pad, and append length.
	uint32_t m[16];
	for (int i = 0; i < 8; i++) m[i] = input[i];
	m[8] = 0x00000080;
	for (int i = 9; i < 16; i++) m[i] = 0;
	m[14] = 256;

	// Initialize buffer.
	uint32_t a = 0x67452301;
	uint32_t b = 0xefcdab89;
	uint32_t c = 0x98badcfe;
	uint32_t d = 0x10325476;

	// Process. There's only one block.
	uint32_t aa = a, bb = b, cc = c, dd = d;
	/* Round 1 */
	FF(a, b, c, d, m[0], S11); /* 1 */
	FF(d, a, b, c, m[1], S12); /* 2 */
	FF(c, d, a, b, m[2], S13); /* 3 */
	FF(b, c, d, a, m[3], S14); /* 4 */
	FF(a, b, c, d, m[4], S11); /* 5 */
	FF(d, a, b, c, m[5], S12); /* 6 */
	FF(c, d, a, b, m[6], S13); /* 7 */
	FF(b, c, d, a, m[7], S14); /* 8 */
	FF(a, b, c, d, m[8], S11); /* 9 */
	FF(d, a, b, c, m[9], S12); /* 10 */
	FF(c, d, a, b, m[10], S13); /* 11 */
	FF(b, c, d, a, m[11], S14); /* 12 */
	FF(a, b, c, d, m[12], S11); /* 13 */
	FF(d, a, b, c, m[13], S12); /* 14 */
	FF(c, d, a, b, m[14], S13); /* 15 */
	FF(b, c, d, a, m[15], S14); /* 16 */

	/* Round 2 */
	GG(a, b, c, d, m[0], S21); /* 17 */
	GG(d, a, b, c, m[4], S22); /* 18 */
	GG(c, d, a, b, m[8], S23); /* 19 */
	GG(b, c, d, a, m[12], S24); /* 20 */
	GG(a, b, c, d, m[1], S21); /* 21 */
	GG(d, a, b, c, m[5], S22); /* 22 */
	GG(c, d, a, b, m[9], S23); /* 23 */
	GG(b, c, d, a, m[13], S24); /* 24 */
	GG(a, b, c, d, m[2], S21); /* 25 */
	GG(d, a, b, c, m[6], S22); /* 26 */
	GG(c, d, a, b, m[10], S23); /* 27 */
	GG(b, c, d, a, m[14], S24); /* 28 */
	GG(a, b, c, d, m[3], S21); /* 29 */
	GG(d, a, b, c, m[7], S22); /* 30 */
	GG(c, d, a, b, m[11], S23); /* 31 */
	GG(b, c, d, a, m[15], S24); /* 32 */

	/* Round 3 */
	HH(a, b, c, d, m[0], S31); /* 33 */
	HH(d, a, b, c, m[8], S32); /* 34 */
	HH(c, d, a, b, m[4], S33); /* 35 */
	HH(b, c, d, a, m[12], S34); /* 36 */
	HH(a, b, c, d, m[2], S31); /* 37 */
	HH(d, a, b, c, m[10], S32); /* 38 */
	HH(c, d, a, b, m[6], S33); /* 39 */
	HH(b, c, d, a, m[14], S34); /* 40 */
	HH(a, b, c, d, m[1], S31); /* 41 */
	HH(d, a, b, c, m[9], S32); /* 42 */
	HH(c, d, a, b, m[5], S33); /* 43 */
	HH(b, c, d, a, m[13], S34); /* 44 */
	HH(a, b, c, d, m[3], S31); /* 45 */
	HH(d, a, b, c, m[11], S32); /* 46 */
	HH(c, d, a, b, m[7], S33); /* 47 */
	HH(b, c, d, a, m[15], S34); /* 48 */

	return a + aa;
}

#undef F
#undef G
#undef H
#undef FF
#undef GG
#undef HH
#undef S11
#undef S12
#undef S13
#undef S14
#undef S21
#undef S22
#undef S23
#undef S24
#undef S31
#undef S32
#undef S33
#undef S34
#undef S41
#undef S42
#undef S43
#undef S44

// https://tools.ietf.org/html/rfc1321
uint32_t MD5(uint32_t input[8]) {
	return 0;
}


//const std::string KERNEL_PATH("../../gpu/walks.cl");


int main(int argc, char* argv[]) {
	const std::string KERNEL_PATH("walk.cl");
	//args given via commandline
	const cl_uint ARRAY_SIZE = atoi(argv[1]);
	const cl_uint SEED = atoi(argv[2]);
	const cl_uint STEPS = atoi(argv[3]);
	const cl_float PROB = atof(argv[4]);

    cl_int errNum;
    std::vector<cl::Platform> platforms;
    cl::Platform platform;
    std::vector<cl::Device> devices;
    cl::Device defaultDevice;
    cl::Context context;

    // choose the first available OpenCL platform
    errNum = cl::Platform::get(&platforms);
    if (errNum != CL_SUCCESS) {
        std::cerr << "Error finding OpenCL platforms!" << std::endl;
        return 1;
    }

    platform = platforms.at(0);
    std::cout << "Using the first found OpenCL platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

    // obtain a list of devices
    errNum = platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    if (errNum != CL_SUCCESS) {
        std::cerr << "Error finding devices!" << std::endl;
    }
    defaultDevice = devices[0];

    // initialize an OpenCL context with the list of devices
    context = cl::Context({ defaultDevice });

    // read in the kernel file as a string
    std::ifstream file(KERNEL_PATH);
    if (!file.is_open()) {
        std::cerr << "Error reading kernel file!" << std::endl;
        return 1;
    }
    std::string kernelCode((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // add the kernel as a program source
    cl::Program::Sources sources;
    sources.push_back({ kernelCode.c_str(), kernelCode.length() });

    // instantiate the program and build it
    cl::Program program(context, sources);
    if (program.build({ defaultDevice }) != CL_SUCCESS) {
        std::cerr << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(defaultDevice) << std::endl;
        return 1;
    }

    // allocate space on the GPU
    cl::Buffer outputBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_double) * ARRAY_SIZE);
    cl::Buffer debugBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_double) * ARRAY_SIZE);
    cl::Buffer seedBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uint));
	cl::Buffer stepBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uint));
	cl::Buffer probBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float));

    cl::CommandQueue queue(context, defaultDevice);
    cl::Kernel run_walk = cl::Kernel(program, "run_walk");

    // copy over the seed to the GPU memory
    queue.enqueueWriteBuffer(seedBuffer, CL_TRUE, 0, sizeof(cl_uint), &SEED);
	queue.enqueueWriteBuffer(stepBuffer, CL_TRUE, 0, sizeof(cl_uint), &STEPS);
	queue.enqueueWriteBuffer(probBuffer, CL_TRUE, 0, sizeof(cl_float), &PROB);

    // run the kernel
    cl::Kernel kernel = cl::Kernel(program, "run_walk");
    kernel.setArg(0, seedBuffer);
	kernel.setArg(1, stepBuffer);
	kernel.setArg(2, probBuffer);
    kernel.setArg(3, outputBuffer);
    kernel.setArg(4, debugBuffer);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(ARRAY_SIZE), cl::NullRange);
    queue.finish();


    // read the output buffer and display the results
    //float output[ARRAY_SIZE];
    double* output = new double[ARRAY_SIZE];
    queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, sizeof(double) * ARRAY_SIZE, output);

    
  /*  std::cout << "Output: " << std::endl;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        std::cout << output[i] << ", ";
    }
    std::cout << std::endl;*/
    
    double* debug = new double[ARRAY_SIZE];
    queue.enqueueReadBuffer(debugBuffer, CL_TRUE, 0, sizeof(cl_double) * ARRAY_SIZE, debug);

    /*
    std::cout << "Debug: " << std::endl;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        std::cout << debug[i] << ", ";
    }
    std::cout << std::endl;
    */

    std::cout << "N = " << ARRAY_SIZE << std::endl;
    
    float average = -1;
    float stdev = -1;

    double sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += pow(output[i],.5);
    }
    average = sum / ARRAY_SIZE;
    std::cout << "Computed an average of: " << average << std::endl;

    double sumDist = 0;
    for (int i = 0; i < ARRAY_SIZE; i++)
        sumDist += pow(pow(output[i], .5) - average, 2);
    stdev = sqrt(sumDist / (ARRAY_SIZE - 1));
    std::cout << "Computed a standard deviation of: " << stdev << std::endl;
    
    delete[] output;

    return 0;
}