uint xorshift(uint state) {
	uint x = state;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	return x;
}

void kernel generate(global const uint* seed, global float* output) {
	int gid = get_global_id(0);
	const uint PREP = 10;

	uint x = seed[0] + gid;
	for(uint i = 0; i < PREP; i++)
		x = xorshift(x);
	
	uint max = -1;
	output[gid] = ((float) x) / max;
}
