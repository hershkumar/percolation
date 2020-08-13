
// length of the random walks
//#define STEPS 1000000
// the probability that the walk is running at
//#define PROB .6

#define UINT_MAX 0xffffffff

// xorshift for rng
uint xorshift(uint state) {
	uint x = state;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	return x;
}

// gets a random number from 0 to 1 using xorshift
uint rng(uint state){
	const uint PREP = 10;
	uint x = state;
	for(uint i = 0; i < PREP; i++)
		x = xorshift(x);
	
	return x;
}

// maps a float between 0 and 1 to an int between 0 and cap (exclusive)
int rng_range(float in, int cap){
	// get a number between 0 and 1
	float tmp = in;
	tmp *= cap;
	int ret = (int) tmp;
	return ret;
}

// the old "stupid" hash function we were using
uint hash(int x, int y){
	int XORSHIFT_TIMES = 10;
	uint seed = (x * 0x1f1f1f1f) ^ y;
	for (int i = 0 ; i < XORSHIFT_TIMES; i++){
		seed = xorshift(seed);
	}
	return seed;
}

// Optimizations marked with "OPT". I don't always know why they help.

constant uint MD2_PI_SUBST[256] = {
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

uint md2(uint* input) {
	// Convert the message to bytes.
	uchar m[64];
	for (int i = 0; i < 8; i++) {
		m[4*i+3] = (uchar)input[i];
		m[4*i+2] = (uchar)(input[i] >> 8);
		m[4*i+1] = (uchar)(input[i] >> 16);
		m[4*i+0] = (uchar)(input[i] >> 24);
	}

	// Pad.
	for (int i = 32; i < 48; i++) m[i] = 0x10;

	// Append checksum.
	uchar l = 0;
	for (int i = 0; i < 16; i++) m[48 + i] = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 16; j++) {
			l = m[48 + j] ^= MD2_PI_SUBST[m[i*16+j] ^ l];
		}
	}

	// Process. Two blocks are needed.
	uchar x[48];
	for (int j = 0; j < 48; j++) x[j] = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 16; j++) {
			x[16+j] = m[i*16+j];
			x[32+j] = x[16+j] ^ x[j];
		}
		uchar t = 0;
		for (int j = 0; j < 18; j++) {
			// OPT: Not unrolling the loop seems to make it faster.
			#pragma unroll 1
			for (int k = 0; k < 48; k++) {
				t = x[k] ^= MD2_PI_SUBST[t];
			}
			t = (t+j) & 0xff;
		}
	}

	return x[3] + (x[2]<<8) + (x[1]<<16) + (x[0]<<24);
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
	    (a) += G ((b), (c), (d)) + (x) + (uint)0x5a827999; \
	    (a) = ROTATE_LEFT ((a), (s)); \
	  }
#define HH(a, b, c, d, x, s) { \
	    (a) += H ((b), (c), (d)) + (x) + (uint)0x6ed9eba1; \
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

uint md4(uint* input) {
	// Copy into buffer, and pad, and append length.
	uint m[16];
	for (int i = 0; i < 8; i++) m[i] = input[i];
	m[8] = 0x00000080;
	for (int i = 9; i < 16; i++) m[i] = 0;
	m[14] = 256;

	// Initialize buffer.
	uint a = 0x67452301;
	uint b = 0xefcdab89;
	uint c = 0x98badcfe;
	uint d = 0x10325476;

	// Process. There's only one block.
	uint aa = a, bb = b, cc = c, dd = d;
	/* Round 1 */
	FF (a, b, c, d, m[ 0], S11); /* 1 */
	FF (d, a, b, c, m[ 1], S12); /* 2 */
	FF (c, d, a, b, m[ 2], S13); /* 3 */
	FF (b, c, d, a, m[ 3], S14); /* 4 */
	FF (a, b, c, d, m[ 4], S11); /* 5 */
	FF (d, a, b, c, m[ 5], S12); /* 6 */
	FF (c, d, a, b, m[ 6], S13); /* 7 */
	FF (b, c, d, a, m[ 7], S14); /* 8 */
	FF (a, b, c, d, m[ 8], S11); /* 9 */
	FF (d, a, b, c, m[ 9], S12); /* 10 */
	FF (c, d, a, b, m[10], S13); /* 11 */
	FF (b, c, d, a, m[11], S14); /* 12 */
	FF (a, b, c, d, m[12], S11); /* 13 */
	FF (d, a, b, c, m[13], S12); /* 14 */
	FF (c, d, a, b, m[14], S13); /* 15 */
	FF (b, c, d, a, m[15], S14); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, m[ 0], S21); /* 17 */
	GG (d, a, b, c, m[ 4], S22); /* 18 */
	GG (c, d, a, b, m[ 8], S23); /* 19 */
	GG (b, c, d, a, m[12], S24); /* 20 */
	GG (a, b, c, d, m[ 1], S21); /* 21 */
	GG (d, a, b, c, m[ 5], S22); /* 22 */
	GG (c, d, a, b, m[ 9], S23); /* 23 */
	GG (b, c, d, a, m[13], S24); /* 24 */
	GG (a, b, c, d, m[ 2], S21); /* 25 */
	GG (d, a, b, c, m[ 6], S22); /* 26 */
	GG (c, d, a, b, m[10], S23); /* 27 */
	GG (b, c, d, a, m[14], S24); /* 28 */
	GG (a, b, c, d, m[ 3], S21); /* 29 */
	GG (d, a, b, c, m[ 7], S22); /* 30 */
	GG (c, d, a, b, m[11], S23); /* 31 */
	GG (b, c, d, a, m[15], S24); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, m[ 0], S31); /* 33 */
	HH (d, a, b, c, m[ 8], S32); /* 34 */
	HH (c, d, a, b, m[ 4], S33); /* 35 */
	HH (b, c, d, a, m[12], S34); /* 36 */
	HH (a, b, c, d, m[ 2], S31); /* 37 */
	HH (d, a, b, c, m[10], S32); /* 38 */
	HH (c, d, a, b, m[ 6], S33); /* 39 */
	HH (b, c, d, a, m[14], S34); /* 40 */
	HH (a, b, c, d, m[ 1], S31); /* 41 */
	HH (d, a, b, c, m[ 9], S32); /* 42 */
	HH (c, d, a, b, m[ 5], S33); /* 43 */
	HH (b, c, d, a, m[13], S34); /* 44 */
	HH (a, b, c, d, m[ 3], S31); /* 45 */
	HH (d, a, b, c, m[11], S32); /* 46 */
	HH (c, d, a, b, m[ 7], S33); /* 47 */
	HH (b, c, d, a, m[15], S34); /* 48 */

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




uint md5(const global uint* input) {
	return 0;
}

// function to check whether a site is on or off
bool is_on(int seed, int x, int y, float PROB){
	if (x == 0 && y == 0){
		return true;
	}
	uint dat[] = {seed, x, y, 0 , 0 , 0 , 0 , 0};
	uint hashed = md4(dat);
	bool higher = ((hashed / (float) UINT_MAX) < PROB);
	return higher;
}

// gets a random number from 0 to n [0,n)
// meant for picking the neighbor to move to
int md_rand(int seed, int step_num, int n){
	uint dat[] = {seed, step_num, 0, 0, 0, 0, 0, 0};
	// get the big random number from the hash
	uint hashed = md4(dat);
	// convert it to a float between 0 and 1
	float temp = (float) hashed / UINT_MAX;
	// convert that float to an int between 0 and n
	temp *= n;
	return (int) temp;
	
}

// the actual walk
void kernel run_walk(global const uint* seed, global const uint* stepin, global const float* probin, global double* output, global double* debug){
	const uint STEPS = stepin[0];
	const float PROB = probin[0];

	// get the global id to seed the rng with
	int gid = get_global_id(0);
	
	int lattice_seed = gid + seed[0];
	int rand_seed = ~gid + seed[0];
	
	// set the starting coords to (0,0)
	uint cx = 0;
	uint cy = 0;
	
	double sqdist = 0;
	
	for (uint step = 0; step < STEPS; step++){
		// get the neighbors of the point
		int neb[4][2] = { {cx - 1, cy}, {cx + 1, cy}, {cx, cy - 1}, {cx, cy + 1} };
		// check which neighbors are on
		uint num_on = 0;
		for (int i = 0; i < 4; i++){
			 if (is_on(lattice_seed, neb[i][0], neb[i][1], PROB)){
				num_on++;
			}
		}
		/*
		printf("GID = %d \t at (%d, %d)\n", gid, cx, cy);
		for(int i = 0; i < 4; i++)
			printf("GID = %d \t%dth neighbor:\t(%d, %d)\t%d\n", gid, i, neb[i][0], neb[i][1], is_on(lattice_seed, neb[i][0], neb[i][1]));
		printf("\n");
		*/
		// if there are open neighbors, do the choice thing
		if (num_on != 0){
			// we want to pick the choice'th on point
			int choice = md_rand(rand_seed, step, num_on);
			
			// loop through until we have found the right number of points that are on
			int count = -1;
			for (int i = 0 ; i < 4; i++){
				// advance the counter when we find one that's on
				if (is_on(lattice_seed, neb[i][0], neb[i][1], PROB)){
					count++;
				}
				//once we find the right one, we move to it
				if (count == choice){
					cx = neb[i][0];
					cy = neb[i][1];
					break;
				}
			}
		}
		// if there are no open neighbors, end the run
		else{
			break;
		}
		
	}
	sqdist = cx * cx + cy * cy;
	output[gid] = sqdist;

	/*
	// debug to check our RNG
	int count = 0;
	int total = 1000;

	for(int i = 0; i < total; i++) {
		if(md_rand(rand_seed, i, 100) < PROB * 100) {
			count++;
		}
	}
	printf("RNG Count: %d \n", count);
	// checking the lattice sites
	count = 0;
	for (int i = 0; i < 100; i++){
		for (int j = 0 ; j < 100; j++){
			if (is_on(lattice_seed, i, j)){
				count++;
			}
		}
	}
	printf("Lattice Count: %d \n", count);
	*/

}

void kernel run_debug(global double* debug){
	int gid = get_global_id(0);
	debug[gid] = (double) 1;
}