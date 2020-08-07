
#include<CL/cl.hpp>
#include<iostream>
#include<fstream>

//const std::string KERNEL_PATH("../../gpu/walks.cl");
const std::string KERNEL_PATH("walk.cl");
const cl_uint ARRAY_SIZE = 1000000;
const cl_uint SEED = 317;

int main() {

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
    cl::Buffer outputBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * ARRAY_SIZE);
    cl::Buffer seedBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uint));

    cl::CommandQueue queue(context, defaultDevice);
    cl::Kernel generate = cl::Kernel(program, "generate");

    // copy over the seed to the GPU memory
    queue.enqueueWriteBuffer(seedBuffer, CL_TRUE, 0, sizeof(cl_uint), &SEED);

    // run the kernel
    cl::Kernel kernel = cl::Kernel(program, "generate");
    kernel.setArg(0, seedBuffer);
    kernel.setArg(1, outputBuffer);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(ARRAY_SIZE), cl::NullRange);
    queue.finish();


    // read the output buffer and display the results
    //float output[ARRAY_SIZE];
    float* output = new float[ARRAY_SIZE];
    queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, sizeof(float) * ARRAY_SIZE, output);

    /*
    std::cout << "Results: " << std::endl;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        std::cout << output[i] << ", ";
    }
    std::cout << std::endl;
    */

    std::cout << "N = " << ARRAY_SIZE << std::endl;

    float average = -1;
    float stdev = -1;

    float sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++)
        sum += output[i];
    average = sum / ARRAY_SIZE;
    std::cout << "Computed an average of: " << average << " verus the expected 0.5" << std::endl;

    float sumDist = 0;
    for (int i = 0; i < ARRAY_SIZE; i++)
        sumDist += pow(output[i] - average, 2);
    stdev = sqrt(sumDist / (ARRAY_SIZE - 1));
    std::cout << "Computed a standard deviation of: " << stdev << " verus the expected " << sqrt(1 / 12.0) << std::endl;

    delete[] output;

    return 0;
}