//
// Sharpening Filter using OpenCL
//
#include <iostream>
#include <fstream>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui.hpp>
#include <CL/cl.h>
#include <math.h>
#include <iostream>
#include <string>
#include <stdio.h>
using namespace std;

extern "C" bool laplacianFilter_CPU(const cv::Mat& input, cv::Mat& output);

int load_kernel(const char *filename, std::string &s){
    size_t size;
    char*  str;
    std::fstream f(filename, (std::fstream::in | std::fstream::binary));
    if(f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);
        str = new char[size+1];
        if(!str)
        {
            f.close();
            return 0;
        }
        f.read(str, fileSize);
        f.close();
        str[size] = '\0';
        s = str;
        delete[] str;
        return 0;
    }
    std::cout<<"Error: failed to open file\n:"<<filename<<std::endl;
    return -1;
}




int main(int argc, char * argv[])
{
    cout << "\nImage filtering on GPU... \n" ;


    // Define input/output variables
    string imageFile =  "input.jpg"; 

    // input & output file names
    string output_file_cpu = "output_cpu.jpeg";
    string output_file_gpu = "output_gpu.jpeg";

    cv::Mat src = cv::imread(imageFile, CV_LOAD_IMAGE_UNCHANGED);
    cv::cvtColor(src,src,CV_BGR2RGBA);

    cv::Mat dst(src.size(),src.type());
    std::string source;

    
    // Load kernel
    const char * kernelFile = "laplacianFilter.cl";
    if(load_kernel(kernelFile, source) < 0){
        std::cerr << "load kernel fail.\n";
        return -1;
    }
    

  
    // Get the first platform
    cl_uint numPlatforms;	//the NO. of platforms
    cl_platform_id platform = NULL;	//the chosen platform
    cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS)
    {
        cout << "Error: Getting platforms!" << endl;
        return -1;
    }
    if(numPlatforms > 0)
    {
        cl_platform_id* platforms = (cl_platform_id* )malloc(numPlatforms* sizeof(cl_platform_id));
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        platform = platforms[0];
        free(platforms);
    }


    // Get the devices list and choose the device you want to run on
    cl_uint numDevices = 0;
    cl_device_id *devices;
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
    if (numDevices == 0)	
    {
        cout << "No GPU available." ;
    }
    else
    {
        devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
        status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
    }


    // Create one OpenCL context for each device in the platform
    cl_context context = clCreateContext(NULL,1, devices,NULL,NULL,NULL);
    if(context == NULL){
        std::cerr << "cannot create context.\n";
        return -1;
    }


    // Create command queue 
    cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
    if(commandQueue == NULL){
        std::cerr << "cannot create command queue.\n";
        return -1;
    }


    // Create a program from the kernel source
    const char *csource = source.c_str();
    size_t sourceSize[] = {strlen(csource)};
    cl_program program = clCreateProgramWithSource(context, 1, &csource, sourceSize, NULL);


    // Build the program
    status=clBuildProgram(program, 1,devices,NULL,NULL,NULL);
    if(status != CL_SUCCESS){
        std::cout << "build program error.\n";
        size_t logLen = 0;
        clGetProgramBuildInfo(program,devices[0],CL_PROGRAM_BUILD_LOG,0,NULL,&logLen);
        char * info = new char[logLen];
        clGetProgramBuildInfo(program,devices[0],CL_PROGRAM_BUILD_LOG,logLen,info,NULL);
        std::cout << "error:\n";
        std::cout << info << std::endl;
        delete[]info;
        return -1;
    }


    // Create memory buffers on the device for image
    int imgsize = src.cols * src.rows * src.channels();
    uchar * buffer = new uchar[imgsize];
    memcpy(buffer,src.data,imgsize);
    cl_image_format imgformat;

    imgformat.image_channel_data_type = CL_UNORM_INT8;
    imgformat.image_channel_order = CL_RGBA;

    cl_mem imgsrc = clCreateImage2D(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    &imgformat,src.cols,src.rows,0,buffer,&status);
    if(status != CL_SUCCESS){
        std::cerr << "cannot create image buffer:" << status << std::endl;
        return -1;
    }
    delete [] buffer;
    if(status != CL_SUCCESS){
        std::cerr << "cannot create image buffer2:" << status << std::endl;
        return -1;
    }

    cl_mem imgdst = clCreateImage2D(context,CL_MEM_WRITE_ONLY,&imgformat,src.cols,src.rows,0,NULL,&status);
    if(status != CL_SUCCESS){
        std::cerr << "cannot create image dst buffer:" << status << std::endl;
        return -1;
    }


    // Define blur filter 
    int mask_width = 3;
    int mask_height = 3;
    const float average_mask[9] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
    cl_mem maskbuffer = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 9*sizeof(float),(void *)&average_mask, &status);
    if(status != CL_SUCCESS){
         std::cerr << "create mask buffer failed.\n";
         return -1;
     }



    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program,"laplacianFilter",&status);
    if(status != CL_SUCCESS){
        std::cerr << "cannot create kernel:" << status << std::endl;
        return -1;
    }


    // Set the arguments of the kernel
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &imgsrc);
    status |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &imgdst);
    status |= clSetKernelArg(kernel, 2, sizeof(cl_kernel), &maskbuffer);
    status |= clSetKernelArg(kernel, 3, sizeof(int), &mask_width);
    if(status != CL_SUCCESS){
        std::cerr << "cannot set kernel arg:" << status << std::endl;
        return -1;
    }

    std::cout << "start run...\n";

    size_t origin [3] = {0,0,0};
    size_t region [3] = {src.cols,src.rows,1};
    size_t localWorkSize[2] = {8,8};
    size_t globalWorkSize[2] = {src.cols,src.rows};

    size_t globalsize = src.cols * src.rows;

    size_t locals[2] = {8,8};
    size_t globals[2];
    globals[0] = (src.cols + locals[0] - 1) / locals[0];
    globals[1] = (src.rows + locals[1] - 1) / locals[1];
    globals[0] *= locals[0];
    globals[1] *= locals[1];

    int64 t0 = cv::getTickCount();
    // Execute the OpenCL kernel
    status = clEnqueueNDRangeKernel(commandQueue,kernel,2,NULL,globals,locals,0,NULL,NULL);
    if(status != CL_SUCCESS){
        std::cerr << "cannot execute clEnqueueNDRangeKernel:" << status << std::endl;
        return -1;
    }
    int64 t1 = cv::getTickCount();
    double secs = (t1-t0)/cv::getTickFrequency();
    cout<< "\nProcessing time for GPU (ms): " << secs*1000 << "\n";


    // Read the memory imgdst on device to the host variable dstdata
    uchar * dstdata = dst.data;
    status = clEnqueueReadImage(commandQueue,imgdst,CL_TRUE,origin,region,0,0,dstdata,0,NULL,NULL);
    if(status != CL_SUCCESS){
        std::cerr << "cannot execute clEnqueueReadImage:" << status << std::endl;
        return -1;
    }

    
    // Display input & output images 
    cv::cvtColor(src,src,CV_RGBA2BGRA);
    cv::cvtColor(dst,dst,CV_RGBA2BGRA);

    // Output image
    imwrite(output_file_gpu, dst);


    // Release all OpenCL allocated objects and host buffers.
    clReleaseCommandQueue(commandQueue);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseContext(context);
    clReleaseMemObject(imgsrc);
    clReleaseMemObject(imgdst);
    clReleaseMemObject(maskbuffer);
   

    cout << "\nImage filtering on CPU...\n" ;
    // Declare the output image  
    cv::Mat dstImage (src.size(), src.type());

    // run laplacian filter on CPU  
    laplacianFilter_CPU(src, dstImage);
    // Output image
    imwrite(output_file_cpu, dstImage);


    return 0;
}



