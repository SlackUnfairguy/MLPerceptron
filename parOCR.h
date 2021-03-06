#ifndef PAROCR_H_INCLUDED
#define PAROCR_H_INCLUDED

#include <vector>
#include <iostream>

#include "oclp.h"
#include "calcCorrect.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-pedantic"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#pragma GCC diagnostic pop


int parallelOCR(std::vector<float> *pTrainingImages, std::vector<float> *pTrainingClassifications,
                std::vector<float> *pTestImages, std::vector<int> *pTestClassifications,
                unsigned int &pInputwidth, unsigned int &pOutputwidth,
                std::vector<cl::Device> *pDevice, cl::Context *pContext )
{
    std::cout << "constructing opencl perceptron." << std::endl;
    OpenCLPerceptron oclp(0.025f, pInputwidth, 300, pOutputwidth);
    oclp.setOpenCLContext(pDevice, pContext);
    float *outputBuffer = new float[pTrainingClassifications->size()];
    if(oclp.initOpenCL())
    {
        std::cout << "initialising opencl buffers." << std::endl;
        if(oclp.initTraining(pTrainingImages,pTrainingClassifications, pTestImages))
        {
            std::cout << "training." << std::endl;
            unsigned int epoch  = 0;
            float correctness   = 0.0f;
            const float target  = 0.95f;
            while(correctness < target)
            {
                ++epoch;
                oclp.trainAll();
                oclp.testAll(outputBuffer);
                correctness = calcCorrect(outputBuffer, pTestClassifications, pOutputwidth);

                std::cout << "correct: " << correctness << " - epoch: " << epoch << std::endl;
            }
            delete [] outputBuffer;
            return 0;
        }
        else
        {
            delete [] outputBuffer;
            return 1;
        }
    }
    else
    {
        delete [] outputBuffer;
        return 2;
    }
}

#endif // PAROCR_H_INCLUDED
